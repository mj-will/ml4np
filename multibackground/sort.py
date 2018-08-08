#!/usr/bin/env python

import pandas as pd
import numpy as np
from root_pandas import read_root, to_root
from sklearn.utils import shuffle

pd.options.mode.use_inf_as_na = True

fname = '../data/tmva3/Data2Pi_clean.root'
tname = 'HSParticles'

print("Loading TTree '{0}' from {1}".format(tname, fname))
df = read_root(fname, tname)

fname2 = '../data/tmva3/Data2K_clean.root'
print("Loading TTree '{0}' from {1}".format(tname, fname2))
df2 = read_root(fname, tname)
df2['Correct'] = 2

data = pd.concat([df, df2], ignore_index = True)
data = shuffle(data)
print(data)

data.to_root('../data/tmva3/Data_clean.root', key='HSParticles')
