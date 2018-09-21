#!/usr/bin/env python

import os, shutil, subprocess

examples = ["counter", "balance_tilde"]

for name in examples:
    subprocess.call("python ../generate.py {} -o {}/{}".format(name,os.getcwd(),name), shell=True)
    shutil.copy2(name+".cpp", "{}/{}.cpp".format(name,name)) 