#!/usr/bin/env python

from ROOT import TMVA, TFile, TString
from array import array
from subprocess import call
from os.path import isfile

import numpy as np

import matplotlib
matplotlib.use('Agg')
from matplotlib import pyplot as plt

# Setup TMVA
TMVA.Tools.Instance()
TMVA.PyMethodBase.PyInitialize()
reader = TMVA.Reader("Color:!Silent")

fnameSignal = '../data/tmva3/Signal2Pi.root'
fnameBackground = '../data/tmva3/BG2Pi.root'

# Load data
if not isfile(fnameSignal):
    print('File not found: {0}'.format(fnameSignal))
if not isfile(fnameBackground):
    print('File not found: {0}'.format(fnameBackground))

dataSignal = TFile.Open(fnameSignal)
dataBackground = TFile.Open(fnameBackground)

signal = dataSignal.Get('HSParticles')
background = dataBackground.Get('HSParticles')


exclude = ['MissMass2', 'MissMass', 'NPerm' 'ElDet', 'Correct', 'UID', 'Topo', 'ElDeltaE', 'PDeltaE', 'PipDeltaE', 'PimDeltaE', 'ElTrChi2', 'PTrChi2', 'PipTrChi2', 'PimTrChi2', 'ElDet', 'Detector', 'ElEdep', 'PEdep', 'PipEdep', 'PimEdep', 'ElPreE', 'PPreE', 'PipPreE', 'PimPreE', 'ElVz', 'PVz', 'PipVz', 'PimVz']

# list of variables to use
branches = signal.GetListOfBranches()
include = [b.GetName() for b in branches if b.GetName() not in exclude]

del branches

branches = {}
for branch in signal.GetListOfBranches():
    branchName = branch.GetName()
    if branchName in include:
        branches[branchName] = array('f', [-999])
        reader.AddVariable(branchName, branches[branchName])
        signal.SetBranchAddress(branchName, branches[branchName])
        background.SetBranchAddress(branchName, branches[branchName])

# Book methods
reader.BookMVA('PyKeras', TString('datasetTest/weights/TMVAClassification_PyKeras.weights.xml'))

# check results
pred_signals = np.empty(1000,)
pred_bkg = np.empty(1000,)

# Print some example classifications
print('Some signal example classifications:')
for i in range(1000):
    signal.GetEntry(i)
    p = reader.EvaluateMVA('PyKeras')
    pred_signals[i] = p
    print(p)
print('')

print('Some background example classifications:')
for i in range(1000):
    background.GetEntry(i)
    p = reader.EvaluateMVA('PyKeras')
    pred_bkg[i] = p
    print(p)

# plots
fig = plt.figure()
plt.hist(pred_signals, 100, label = 'S')
plt.hist(pred_bkg, 100, label = 'B')
plt.legend()
fig.savefig('hists.png')
