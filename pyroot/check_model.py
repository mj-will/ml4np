#!/usr/bin/env python

import sys
from keras.models import load_model

# get filename
fname = sys.argv[1]
print(fname)
# load model
model = load_model(fname)
model.summary()
