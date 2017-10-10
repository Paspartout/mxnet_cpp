# MXnet C++ Usage

This repository shows how one could use [MXnet](https://mxnet.apache.org/) and
the pretrained resnet model in C++ using
[Amalgamation](https://github.com/apache/incubator-mxnet/tree/master/amalgamation) and OpenCV.

## Compilation

In order to compile the example make sure you got a recent version of OpenCV
installed as well as the `libmxnet_predict.a` library coming from the
amalgamation process.

After that simply type `make` to compile the example.
In case this doesn't work you may have to adjust the CFLAGS or LDFLAGS in the
Makefile.

## Usage

The example simply loads an image file which you provdie as an command line argument.
It also assumes that you have the model and synset available in the model
folder relative to the execution. You can use the `download_model.sh` script in the folder
to download the resnet model.

