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
from array import array

# Setup TMVA
TMVA.Tools.Instance()
TMVA.PyMethodBase.PyInitialize()

# choose method
methods = {}
methods['BDT'] =     1
methods['PyKeras'] = 1


use_data = True
detector = "ALL"

output_name = 'Det{}.root'.format(detector)

output = TFile.Open(output_name, 'RECREATE')
factory = TMVA.Factory('TMVAClassification', output,
        '!V:!Silent:Color:DrawProgressBar:Transformations=G:AnalysisType=Classification')

def skim(tree):
    """
    Skim tree for nulls and infs
    """
    ROOT.gROOT.cd()
    t = tree.CopyTree('Detector==0&&PipTime!=0&&PimTime!=0&&PTime!=0&&!TMath::IsNaN(PipDeltaE)&&!TMath::IsNaN(PimDeltaE)&&!TMath::IsNaN(ElDeltaE)&&!TMath::IsNaN(PDeltaE)')
    return t


def split(tree):
        """
        Split tree into signals. Also remove possible nulls
        """

        ROOT.gROOT.cd()
        # create copys
        signalTree = (tree.CopyTree('Correct==1'))
        backgroundTree = (tree.CopyTree('Correct==0'))

        return signalTree, backgroundTree


fnameData = '../data/tmva3/Data2Pi_{0}.root'.format(detector)

if not isfile(fnameData):
    print('File not found: {0}'.format(fnameData))

data = TFile.Open(fnameData)
dataTree = data.Get('D{0}Tree'.format(detector))

signal, background = split(dataTree)

# make data loader
dataloader = TMVA.DataLoader('datasetDet{0}'.format(detector))

# list of variables to exclude

if detector == 0 or detector == "ALL" :
    exclude = ['Correct', 'MissMass2', 'MissMass', 'UID', 'Detector', 'ElEdep', 'ElPreE', 'ElVz', 'ElTrChi2', 'ElDet', 'PDet', 'PipDet','PimDet']
if detector == 1:
    exclude = ['Correct', 'MissMass2', 'MissMass', 'UID', 'Detector', 'ElDet','PDet', 'PipDet','PimDet']

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
        'nTrain_Signal=5000:nTrain_Background=5000:SplitMode=Random:NormMode=NumEvents:!V')

if methods['PyKeras']:

    name = 'PyKeras'
    # Generate model

    # Define model
    # SNN model
    model = Sequential()
    model.add(Dense(1024,input_dim=Nvar,
        kernel_initializer=initializers.lecun_uniform(),
        bias_initializer=initializers.lecun_uniform()))
    model.add(Activation('selu'))
    model.add(AlphaDropout(0.2))
    model.add(Dense(512,
        kernel_initializer=initializers.lecun_uniform(),
        bias_initializer=initializers.lecun_uniform()))
    model.add(Activation('selu'))
    model.add(AlphaDropout(0.2))
    model.add(Dense(2, activation='softmax'))

    # Set loss and optimizer
    model.compile(loss='categorical_crossentropy', optimizer=Nadam(), metrics=['accuracy',])

    # Store model to file
    model.save('./model-Det{0}.h5'.format(detector))
    model.summary()

    factory.BookMethod(dataloader, TMVA.Types.kPyKeras, name,        'H:!V:VarTransform=D,G:FilenameModel=model-Det{0}.h5:NumEpochs=40:BatchSize=32:TriesEarlyStopping=5'.format(detector))

# Boosted descision trees
if methods['BDT']:

    # checl BDTs used so far
    BDTcount = 0
    while isfile('./datasetTest/weights/TMVAClassification_BDT.class.C'.format(BDTcount)):
        BDTcount += 1

    factory.BookMethod(dataloader, TMVA.Types.kBDT, 'BDT'.format(BDTcount),"!H:!V:NTrees=1700:MinNodeSize=2.5%:MaxDepth=4:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20")


factory.TrainAllMethods()
factory.TestAllMethods()
factory.EvaluateAllMethods()
