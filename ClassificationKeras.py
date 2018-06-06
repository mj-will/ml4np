#!/usr/bin/env python

from ROOT import TMVA, TFile, TTree, TCut
from subprocess import call
from os.path import isfile

from keras.models import Sequential
from keras.layers.core import Dense, Activation, Dropout
from keras.regularizers import l2
from keras import initializers
from keras.optimizers import SGD, Nadam

# Setup TMVA
TMVA.Tools.Instance()
TMVA.PyMethodBase.PyInitialize()

output = TFile.Open('TMVA.root', 'RECREATE')
factory = TMVA.Factory('TMVAClassification', output,
        '!V:!Silent:Color:DrawProgressBar:Transformations=D,G:AnalysisType=Classification')

fnameSignal = './data/rho10_tm1_sm1_stat/Signal.root'
fnameBackground = './data/rho10_tm1_sm1_stat/Background.root'

# Load data
if not isfile(fnameSignal):
    print('File not found: {0}'.format(fnameSignal))
if not isfile(fnameBackground):
    print('File not found: {0}'.format(fnameBackground))

dataSignal = TFile.Open(fnameSignal)
dataBackground = TFile.Open(fnameBackground)
signal = dataSignal.Get('HSParticles')
background = dataBackground.Get('HSParticles')

# make data loader
dataloader = TMVA.DataLoader('dataset')

# list of variables to include
include = ['ElTime', 'ElP', 'ElTh', 'ElPhi', 'PTime', 'PP', 'PTh', 'PPhi', 'PimTime', 'PimP', 'PimTh', 'PimPhi', 'PipTime', 'PipP', 'PipTh', 'PipPhi']
Nvar = len(include)
# loop over branches
for branch in signal.GetListOfBranches():
    if branch.GetName() in include:
        dataloader.AddVariable(branch.GetName())

dataloader.AddSignalTree(signal, 1.0)
dataloader.AddBackgroundTree(background, 1.0)
dataloader.PrepareTrainingAndTestTree(TCut(''),
        'nTrain_Signal=4000:nTrain_Background=4000:SplitMode=Random:NormMode=NumEvents:!V')

# Generate model

# Define model
model = Sequential()
model.add(Dense(32, activation='relu', kernel_regularizer=l2(1e-5), input_dim=Nvar))
model.add(Dropout(0.5))
model.add(Dense(32, activation='relu', kernel_regularizer=l2(1e-5)))
model.add(Dropout(0.5))
#model.add(Dense(32, init=normal, activation='relu', W_regularizer=l2(1e-5)))
model.add(Dense(2, activation='softmax'))

# Set loss and optimizer
model.compile(loss='categorical_crossentropy', optimizer=Nadam(), metrics=['accuracy',])

# Store model to file
model.save('model.h5')
model.summary()

# Book methods

factory.BookMethod(dataloader, TMVA.Types.kBDT, 'BDT',"!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=3:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20")

factory.BookMethod(dataloader, TMVA.Types.kBDT, 'BDT+',"!H:!V:NTrees=850:MinNodeSize=2.5%:MaxDepth=4:BoostType=AdaBoost:AdaBoostBeta=0.5:UseBaggedBoost:BaggedSampleFraction=0.5:SeparationType=GiniIndex:nCuts=20")

factory.BookMethod(dataloader, TMVA.Types.kMLP, "MLP", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=600:HiddenLayers=N+5:TestRate=5:!UseRegulator")

factory.BookMethod(dataloader, TMVA.Types.kMLP, "MLPBNN", "H:!V:NeuronType=tanh:VarTransform=N:NCycles=60:HiddenLayers=N+5:TestRate=5:TrainingMethod=BFGS:UseRegulator")

factory.BookMethod(dataloader, TMVA.Types.kPyKeras, 'PyKeras',
        'H:!V:VarTransform=D,G:FilenameModel=model.h5:NumEpochs=40:BatchSize=32')

# Run training, test and evaluation
factory.TrainAllMethods()
factory.TestAllMethods()
factory.EvaluateAllMethods()
