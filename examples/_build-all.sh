#!/bin/sh

make SOURCES=balance~.cpp -f _make-pd.mk
make SOURCES=counter.cpp  -f _make-pd.mk
