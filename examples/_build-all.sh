#!/bin/sh

make NAME=balance~  -f _make-pd.mk
make NAME=counter   -f _make-pd.mk

rm .depend