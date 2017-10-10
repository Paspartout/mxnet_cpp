#!/bin/sh

set -e

baseurl='http://data.mxnet.io/models/imagenet-11k/'

wget "${baseurl}resnet-152/resnet-152-symbol.json"
wget "${baseurl}resnet-152/resnet-152-0000.params"
wget "${baseurl}synset.txt"
