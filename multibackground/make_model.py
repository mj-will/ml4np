#!/usr/bin/env python

import sys
import os

from keras.models import Sequential
from keras.layers.core import Dense, Activation, Dropout
from keras.layers import BatchNormalization, AlphaDropout
from keras.regularizers import l2
from keras import initializers
from keras.optimizers import SGD, Nadam

import argparse


def main(args):

    # make a model to be trained in TMVA
    model = Sequential()
    model.add(Dense(1024,input_dim=args.Nvariables,
        kernel_initializer=initializers.lecun_uniform(),
        bias_initializer=initializers.lecun_uniform()))
    model.add(Activation('selu'))
    model.add(AlphaDropout(0.5))
    model.add(Dense(512,
        kernel_initializer=initializers.lecun_uniform(),
        bias_initializer=initializers.lecun_uniform()))
    model.add(Activation('selu'))
    model.add(AlphaDropout(0.5))
    model.add(Dense(3, activation='softmax'))

    # Set loss and optimizer
    model.compile(loss='categorical_crossentropy', optimizer=Nadam(), metrics=['accuracy',])

    # print model
    model.summary()

    # save model
    model.save('model.h5')


def parse_args():

    parser = argparse.ArgumentParser(description='')
    parser.add_argument('-N', '--Nvariables',  type=int, default=43,
                                help='Number of input variables')

    return parser.parse_args()


if __name__ == "__main__":
    # execute only if run as a script
    args = parse_args()
    main(args)
