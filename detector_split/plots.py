#!/usr/bin/env python

from __future__ import print_function

import numpy as np

from sklearn import metrics

import matplotlib
matplotlib.use('Agg')
from matplotlib import pyplot as plt

import ROOT
from rootpy.io import root_open
from root_numpy import root2array, array2tree
from root_pandas import read_root

def getROC(detector = 0, methods = ['BDT']):
    """Get the ROC curve for a dectector given a set of methods testes

    Keyword arguments:
    detector -- detector used (default 0)
    methods  -- list of methods used (default ['BDT'])
    """

    # retrive root tree as datafram
    df= read_root('resultsDet{0}.root'.format(detector), 'resultsTree')
    print(df)

    # array for results
    res = np.ndarray([len(methods),2],dtype = np.ndarray)

    # loop over methods and get roc curves
    for i,m in enumerate(methods):
        res[i,0], res[i,1], _ = metrics.roc_curve(df['eventclass'], df['p{}'.format(m)])

    return res


def getROC_special(fdet = 'ALL',
        detectors = [0, 1],
        methods = ['BDT']):

    """Get ROC curves for each detector in a combined dataset

    Keyword arguments:
    fdet -- detector name for file
    detectors -- list of detectors (default [0, 1])
    methods  -- list of methods used (default ['BDT'])
    """

    # array for results
    res = np.ndarray([len(detectors), len(methods),2],dtype = np.ndarray)

    # retrive root tree as datafram
    df= read_root('resultsDet{0}.root'.format(fdet), 'resultsTree')


    # loop over methods and get roc curves
    for i,d in enumerate(detectors):
        df_tmp = df[df['Detector'] == d]
        for j,m in enumerate(methods):
            res[i,j,0], res[i,j,1], _ = metrics.roc_curve(df_tmp['eventclass'], df_tmp['p{}'.format(m)])

    return res



# define variables
detectors = [0, 1, 'ALL']
methods = ['BDT']
dets = [0,1]

# get roc curves
results = np.empty(len(detectors), dtype = np.ndarray)

for i,d in enumerate(detectors):
    results[i] = getROC(d, methods)

res = results_split = getROC_special()

# plots
fig = plt.figure()
for j,m in enumerate(methods):
    for i,d in enumerate(detectors):
       plt.plot(results[i][j,0], results[i][j,1], '--', label = 'Detector {0} - {1}'.format(d,m))

for i,d in enumerate(dets):
    for j,m in enumerate(methods):
        plt.plot(res[i,j,0], res[i,j,1], '--', label = 'Detector ALL Det {0} - {1}'.format(d,m))

plt.xlabel('False positive')
plt.ylabel('True positive')
plt.legend()
plt.legend()
fig.savefig('roc.png')
