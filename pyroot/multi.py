
#!/usr/bin/env python

import sys
import os

from keras.models import Sequential
from keras.layers.core import Dense, Activation, Dropout
from keras.layers import BatchNormalization, AlphaDropout
from keras.regularizers import l2
from keras import initializers
from keras.optimizers import SGD, Nadam

import ROOT
from ROOT import TMVA, TFile, TTree, TCut
from subprocess import call
from os.path import isfile, isdir

# Setup TMVA
TMVA.Tools.Instance()
TMVA.PyMethodBase.PyInitialize()

# choose method
methods = {}
methods['BDT'] =     1
methods['PyKeras'] = 1

# file names
fnameSignal = '../data/tmva3/Signal2Pi.root'
fnameBackground = '../data/tmva3/BG2Pi.root'
fnameBackground2K = '../data/tmva3/Data2K.root'
fnameData = '../data/tmva3/Data2Pi.root'

# check files
if not isfile(fnameSignal):
    print('File not found: {0}'.format(fnameSignal))
if not isfile(fnameBackground):
    print('File not found: {0}'.format(fnameBackground))
if not isfile(fnameData):
    print('File not found: {0}'.format(fnameData))
if not isfile(fnameBackground2K):
    print('File not found: {0}'.format(fnameBackground2K))

# load data
dataSignal = TFile.Open(fnameSignal)
dataBackground = TFile.Open(fnameBackground)
dataBackground2K = TFile.Open(fnameBackground2K)
dataData = TFile.Open(fnameData)

# get trees
signal0 = dataSignal.Get('HSParticles')
background0 = dataBackground.Get('HSParticles')
background2K = dataBackground2K.Get('HSParticles')
# combined signal0 and background0
data0 = dataData.Get('HSParticles')

# functions for processing trees

def skim(tree):
    """
    Skim tree for nulls and infs
    """
    ROOT.gROOT.cd()
    t = tree.CopyTree('Detector==0&&PipTime!=0&&PimTime!=0&&PTime!=0')
    return t

def split(tree):
        """
        Split tree into signals. Also remove possible nulls
        """

        ROOT.gROOT.cd()
        # create copys
        signalTree = skim(tree.CopyTree('Correct==1'))
        backgroundTree = skim(tree.CopyTree('Correct==0'))

        return signalTree, backgroundTree

def topo_split(tree):
    """
    Split a tree based on signal topology
    - 0 -> all
    - 1 -> miss pi-
    - 2 -> miss pi+
    - 3 -> miss proton
    """

    ROOT.gROOT.cd()
    # create copies with Correct criteria
    signalTree0 = tree.CopyTree('Correct==1&&Topo==0')
    signalTree1 = tree.CopyTree('Correct==1&&Topo==1')
    signalTree2 = tree.CopyTree('Correct==1&&Topo==2')
    signalTree3 = tree.CopyTree('Correct==1&&Topo==3')

    return signalTree0, signalTree1, signalTree2, signalTree3

# Use full data instead of split

# traning trees (Signal2Pi and BG2Pi)
#signalTrain = skim(signal0)
#backgroundTrain = skim(background0)

#signalTrain0, signalTrain1, signalTrain2, signalTrain5 = topo_split(signalTrain)

# Data2Pi
signal2Pi, background2Pi = split(data0)

# split based on topology
signalTest0, signalTest1, signalTest2, signalTest3 = topo_split(signal2Pi)

# Data2K
background2K = skim(background2K)

ROOT.gROOT.cd()

# list of variables to exclude
exclude = ['MissMass2', 'MissMass', 'NPerm' 'ElDet', 'Correct', 'UID', 'Topo', 'ElDeltaE', 'PDeltaE', 'PipDeltaE', 'PimDeltaE', 'ElTrChi2', 'PTrChi2', 'PipTrChi2', 'PimTrChi2', 'ElDet', 'Detector', 'ElEdep', 'PEdep', 'PipEdep', 'PimEdep', 'ElPreE', 'PPreE', 'PipPreE', 'PimPreE', 'ElVz', 'PVz', 'PipVz', 'PimVz']

# list of variables to use
branches = signal2Pi.GetListOfBranches()
include = [b.GetName() for b in branches if b.GetName() not in exclude]
# number of variables
Nvar = len(include)

#################
# Generate model#
#################

if methods['PyKeras']:

# Define model
    """
    model = Sequential()
    model.add(Dense(1024, input_dim=Nvar))
    model.add(Activation('relu'))
    model.add(Dropout(0.5))
    model.add(BatchNormalization())
    model.add(Dense(512, ))
    model.add(Activation('relu'))
    model.add(Dropout(0.5))
    model.add(BatchNormalization())
    model.add(Dense(2, activation='softmax'))
    """
# SNN model
    model = Sequential()
    model.add(Dense(1024,input_dim=Nvar,
        kernel_initializer=initializers.lecun_uniform(),
        bias_initializer=initializers.lecun_uniform()))
    model.add(Activation('selu'))
    model.add(AlphaDropout(0.5))
    model.add(Dense(512,
        kernel_initializer=initializers.lecun_uniform(),
        bias_initializer=initializers.lecun_uniform()))
    model.add(Activation('selu'))
    model.add(AlphaDropout(0.5))
    model.add(Dense(2, activation='softmax'))

# Set loss and optimizer
    model.compile(loss='categorical_crossentropy', optimizer=Nadam(), metrics=['accuracy',])

# Store model to file
model.summary()

################
# background2Pi#
################

output = TFile.Open('TMVAMultiBkg.root', 'RECREATE')
factory = TMVA.Factory('TMVAMultiBkg0', output,'!V:!Silent:Color:DrawProgressBar:Transformations=D,G:AnalysisType=Classification')

dataloader = TMVA.DataLoader('datasetBkg0')

# loop over branches
for b in include:
    dataloader.AddVariable(b)

dataloader.AddSignalTree(signal2Pi, 1.0)
dataloader.AddBackgroundTree(background2Pi, 1.0)
dataloader.PrepareTrainingAndTestTree(TCut(''),
        'nTrain_Signal=4000:nTrain_Background=4000:SplitMode=Random:NormMode=NumEvents:!V')

# Keras neural net
if methods['PyKeras']:
# sav file in given location
    i = 0
    model_path = './models_multi/'

    while isfile('{0}model{1}.h5'.format(model_path, i)):
        i += 1
    name = 'PyKeras-Bkg0'#'PyKeras-{0}'.format(i)

    # Booke method
    factory.BookMethod(dataloader, TMVA.Types.kPyKeras, name,        'H:!V:VarTransform=D,G:FilenameModel=model.h5:NumEpochs=40:BatchSize=32:TriesEarlyStopping=5')

# Boosted descision tree
if methods['BDT']:
    factory.BookMethod(dataloader, TMVA.Types.kBDT, 'BDT',"!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20")

factory.TrainAllMethods()
factory.TestAllMethods()
factory.EvaluateAllMethods()

output.Close()
del factory
del dataloader
###############
# background2K#
###############

output = TFile.Open('TMVAMultiBkg.root', 'UPDATE')
factory = TMVA.Factory('TMVAMultiBkg1', output,'!V:!Silent:Color:DrawProgressBar:Transformations=D,G:AnalysisType=Classification')

dataloader = TMVA.DataLoader('datasetBkg1')

# loop over branches
for b in include:
    dataloader.AddVariable(b)

dataloader.AddSignalTree(signal2Pi, 1.0)
dataloader.AddBackgroundTree(background2K, 1.0)
dataloader.PrepareTrainingAndTestTree(TCut(''),
        'nTrain_Signal=4000:nTrain_Background=4000:SplitMode=Random:NormMode=NumEvents:!V')

# Keras neural net
if methods['PyKeras']:
# sav file in given location
    i = 0
    model_path = './models_multi/'

    while isfile('{0}model{1}.h5'.format(model_path, i)):
        i += 1
    name = 'PyKeras-Bkg1'#'PyKeras-{0}'.format(i)

    # Booke method
    factory.BookMethod(dataloader, TMVA.Types.kPyKeras, name,        'H:!V:VarTransform=D,G:FilenameModel=model.h5:NumEpochs=40:BatchSize=32:TriesEarlyStopping=5')

# Boosted descision tree
if methods['BDT']:
    factory.BookMethod(dataloader, TMVA.Types.kBDT, 'BDT-Bkg1',"!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20")

factory.TrainAllMethods()
factory.TestAllMethods()
factory.EvaluateAllMethods()

output.Close()
del factory
del dataloader
