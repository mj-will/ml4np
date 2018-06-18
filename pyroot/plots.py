#!/usr/bin/env python

import os
import numpy as np

import matplotlib
matplotlib.use('Agg')
from matplotlib import pyplot as plt

from sklearn import metrics

# PyKeras
signals = np.load('./signals.npy')
bkg = np.load('bkg.npy')
# BDT
signals_BDT = np.load('./signals_BDT.npy')
bkg_BDT = np.load('./bkg_BDT.npy')

targets = np.concatenate([np.ones(1000,), np.zeros(1000,)])
preds = np. concatenate([signals, bkg])
print(preds[-50:])

#########
# plots #
#########

# pykeras
fig = plt.figure()
plt.hist(signals, 100, label = 'S')
plt.hist(bkg, 100, label = 'B')
plt.legend()
fig.savefig('hists_PyKeras.png')
# BDT
fig = plt.figure()
plt.hist(signals_BDT, 100, label = 'S')
plt.hist(bkg_BDT, 100, label = 'B')
plt.legend()
fig.savefig('hists_BDT.png')

#ROC curves
f0, t0, _ = metrics.roc_curve(targets,preds)
fig = plt.figure()
plt.plot(f0,t0)
fig.savefig('roc.png')

# comparison
fig, axs = plt.subplots(2,2)
axs = axs.ravel()

# signals
axs[0].plot(signals, signals_BDT)
axs[1].plot(bkg, bkg_BDT)

fig.savefig('./comp.png')
# bkg


