#!/usr/bin/env python

import shutil, subprocess

examples = ["counter", "balance_tilde"]

for name in examples:
    subprocess.call("python ../generate.py {} -o $(pwd)/{}".format(name,name), shell=True)
    shutil.copy2(name+".cpp", "{}/{}.cpp".format(name,name)) 