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


output_name = 'DetALL.root'

output = TFile.Open(output_name, 'RECREATE')
factory = TMVA.Factory('TMVAClassification', output,
        '!V:!Silent:Color:DrawProgressBar:Transformations=G:AnalysisType=Classification')

use_data = True

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
        signalTree = skim(tree.CopyTree('Correct==1'))
        backgroundTree = skim(tree.CopyTree('Correct==0'))

        return signalTree, backgroundTree


if use_data:
    fnameData = '../data/tmva3/Data2Pi,root'.format(detector)

    if not isfile(fnameData):
        print('File not found: {0}'.format(fnameData))

    data = TFile.Open(fnameData)
    dataTree = data.Get('HSParticles'.format(detector))

    signal, background = split(dataTree)


else:

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



    # remove nulls
    signal = skim(signal0)
    background = skim(background0)

    ROOT.gROOT.cd()

# make data loader
dataloader = TMVA.DataLoader('datasetDetALL'.format(detector))

# list of variables to exclude
#exclude = ['MissMass2', 'MissMass', 'NPerm' , 'Correct', 'UID', 'Topo', 'ElDeltaE', 'PDeltaE', 'PipDeltaE', 'PimDeltaE', 'ElTrChi2', 'PTrChi2', 'PipTrChi2', 'PimTrChi2', 'ElDet', 'Detector', 'ElEdep', 'PEdep', 'PipEdep', 'PimEdep', 'ElPreE', 'PPreE', 'PipPreE', 'PimPreE', 'ElVz', 'PVz', 'PipVz', 'PimVz']

exclude = ['MissMass2', 'MissMass', 'UID']

# list of variables to use
branches = signal.GetListOfBranches()
include = [b.GetName() for b in branches if b.GetName() not in exclude]

# number of variables for output layer
Nvar = len(include)
# loop over branches
for b in include:
    dataloader.AddVariable(b)
#branches = {}
#for branch in signal.GetListOfBranches():
#    branchName = branch.GetName()
#    if branchName in include:
#        if branchName in ['NPerm', 'NDet']:
#            #branches[branchName] = array('f', [0])
#            print(branchName)
#            #dataloader.AddVariable(branchName, branches[branchName])
#        else:
#            branches[branchName] = array('f', [-999])
#            print(branchName)
#            dataloader.AddVariable(branchName, branches[branchName])

### General variables
#dataloader.AddVariable( "NPerm",   &NPerm,    'F');
#dataloader.AddVariable( "NDet",    &NDet,     'F');
## Electron
#dataloader.AddVariable( "ElTime",  &ElTime,   'F');
#dataloader.AddVariable( "ElP",     &ElP,      'F');
#dataloader.AddVariable( "ElTh",    &ElTh,     'F');
#dataloader.AddVariable( "ElPhi",   &ElPhi,    'F');
## Proton
#dataloader.AddVariable( "PTime",   &PTime,    'F');
#dataloader.AddVariable( "PP",      &PP,       'F');
#dataloader.AddVariable( "PTh",     &PTh,      'F');
#dataloader.AddVariable( "PPhi",    &PPhi,     'F');
#dataloader.AddVariable( "PDet",    &PDet,     'F');
## Pi +
#dataloader.AddVariable( "PipTime", &PipTime,  'F');
#dataloader.AddVariable( "PipP",    &PipP,     'F');
#dataloader.AddVariable( "PipTh",   &PipTh,    'F');
#dataloader.AddVariable( "PipPhi",  &PipPhi,   'F');
#dataloader.AddVariable( "PipDet",  &PipDet,   'F');
## Pi -
#dataloader.AddVariable( "PimTime", &PimTime,  'F');
#dataloader.AddVariable( "PimP",    &PimP,     'F');
#dataloader.AddVariable( "PimTh",   &PimTh,    'F');
#dataloader.AddVariable( "PimPhi",  &PimPhi,   'F');
#

dataloader.AddSignalTree(signal, 1.0)
dataloader.AddBackgroundTree(background, 1.0)
dataloader.PrepareTrainingAndTestTree(TCut(''),
        'nTrain_Signal=4000:nTrain_Background=4000:SplitMode=Random:NormMode=NumEvents:!V')

if methods['PyKeras']:
    # sav file in given location
    i = 0
    model_path = './testmodels/'

    while isfile('{0}model{1}.h5'.format(model_path, i)):
        i += 1
    name = 'PyKeras'.format(i)
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
    #model.save('{0}/model{1}.h5'.format(model_path, i))
    model.save('./model-Det{0}.h5'.format(detector))
    model.summary()

    factory.BookMethod(dataloader, TMVA.Types.kPyKeras, name,        'H:!V:VarTransform=D,G:FilenameModel=model.h5:NumEpochs=40:BatchSize=32:TriesEarlyStopping=5')

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
