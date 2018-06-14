#!/usr/bin/env python

import sys
import os
#sys.path = [''] + sys.path
#print(sys.path)
#print(sys.version)
#print(sys.version_info)
#print(sys.executable)


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
methods['PyKeras'] = 0



output = TFile.Open('TMVABDT.root', 'UPDATE')
factory = TMVA.Factory('TMVAClassification', output,
        '!V:!Silent:Color:DrawProgressBar:Transformations=D,G:AnalysisType=Classification')

fnameSignal = '../data/tmva3/Signal2Pi.root'
fnameBackground = '../data/tmva3/BG2Pi.root'

# Load data
if not isfile(fnameSignal):
    print('File not found: {0}'.format(fnameSignal))
if not isfile(fnameBackground):
    print('File not found: {0}'.format(fnameBackground))

dataSignal = TFile.Open(fnameSignal)
dataBackground = TFile.Open(fnameBackground)


signal0 = dataSignal.Get('HSParticles')
background0 = dataBackground.Get('HSParticles')


def skim(tree):
    """
    Skim tree for nulls and infs
    """
    ROOT.gROOT.cd()
    t = tree.CopyTree('Detector==0&&PipTime!=0&&PimTime!=0&&PTime!=0')
    return t

# remove nulls
signal = skim(signal0)
background = skim(background0)

ROOT.gROOT.cd()

# make data loader
dataloader = TMVA.DataLoader('datasetBDT')

# list of variables to exclude
exclude = ['MissMass2', 'MissMass', 'NPerm' 'ElDet', 'Correct', 'UID', 'Topo', 'ElDeltaE', 'PDeltaE', 'PipDeltaE', 'PimDeltaE', 'ElTrChi2', 'PTrChi2', 'PipTrChi2', 'PimTrChi2', 'ElDet', 'Detector', 'ElEdep', 'PEdep', 'PipEdep', 'PimEdep', 'ElPreE', 'PPreE', 'PipPreE', 'PimPreE', 'ElVz', 'PVz', 'PipVz', 'PimVz']

# list of variables to use
branches = signal.GetListOfBranches()
include = [b.GetName() for b in branches if b.GetName() not in exclude]

# number of variables for output layer
Nvar = len(include)
# loop over branches
for b in include:
    dataloader.AddVariable(b)

dataloader.AddSignalTree(signal, 1.0)
dataloader.AddBackgroundTree(background, 1.0)
dataloader.PrepareTrainingAndTestTree(TCut(''),
        'nTrain_Signal=0:nTrain_Background=0:SplitMode=Random:NormMode=NumEvents:!V')

if methods['PyKeras']:
    # sav file in given location
    i = 0
    model_path = './models/'

    while isfile('{0}model{1}.h5'.format(model_path, i)):
        i += 1
    name = 'PyKeras-{0}'.format(i)
    # Generate model

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
    model.add(Dense(2048,input_dim=Nvar,
        kernel_initializer=initializers.lecun_uniform(),
        bias_initializer=initializers.lecun_uniform()))
    model.add(Activation('selu'))
    model.add(AlphaDropout(0.5))
    model.add(Dense(1024,
        kernel_initializer=initializers.lecun_uniform(),
        bias_initializer=initializers.lecun_uniform()))
    model.add(Activation('selu'))
    model.add(AlphaDropout(0.5))
    model.add(Dense(2, activation='softmax'))

    # Set loss and optimizer
    model.compile(loss='categorical_crossentropy', optimizer=Nadam(), metrics=['accuracy',])

    # Store model to file
    model.save('{0}/model{1}.h5'.format(model_path, i))
    model.save('./model.h5')
    model.summary()

    factory.BookMethod(dataloader, TMVA.Types.kPyKeras, name,        'H:!V:VarTransform=D,G:FilenameModel=model.h5:NumEpochs=40:BatchSize=32:TriesEarlyStopping=5')

# Boosted descision trees
if methods['BDT']:

    # checl BDTs used so far
    BDTcount = 0
    while isfile('./datasetBDT/weights/TMVAClassification_BDT{0}.class.C'.format(BDTcount)):
        BDTcount += 1

    factory.BookMethod(dataloader, TMVA.Types.kBDT, 'BDT{0}'.format(BDTcount),"!H:!V:NTrees=1700:MinNodeSize=2.5%:MaxDepth=5:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20")


factory.TrainAllMethods()
factory.TestAllMethods()
factory.EvaluateAllMethods()
