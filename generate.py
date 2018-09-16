#!/usr/bin/env python

import os, sys, argparse, subprocess
from distutils.dir_util import copy_tree

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Creates a new TRextern project")
    parser.add_argument(
        "name",
        help = "Name of the TRextern project. Projects with '~' in the name will be created as DSP objects")
    parser.add_argument(
        "-o", "--out",
        default=os.getcwd(),
        help = "Output folder")
    # parser.add_argument(
    #     "-c", "--copyfiles",
    #     type=int,
    #     default=0,
    #     help = "Optional copy of dependencies to project directory")
    
    try:
        options = parser.parse_args()

        scriptDir = os.path.realpath(__file__ + "/..")
        print scriptDir

        # Check if dependencies are downloaded and fetch if missing
        dependencyDir = os.path.join(scriptDir, "dependencies")
        if not os.path.exists(os.path.join(dependencyDir, "m_pd.h")) \
        or not os.path.exists(os.path.join(dependencyDir, "max-sdk-7.3.3")):
            print "Downloading dependencies"
            setupPath = os.path.join(scriptDir, "setup.sh")
            subprocess.call("sh " + setupPath, shell=True)
            
        if os.path.exists(options.out):
            shutil.rmtree(options.out)
            
        print "Creating Project " + options.name + " at path: " + options.out
        copy_tree(os.path.join(scriptDir,"templates"), options.out)
        
        # Replace __template__ with 'name' in file contents
        filesToRename = []
        for dname, dirs, files in os.walk(options.out):
            for fname in files:
                fpath = os.path.join(dname, fname)
                with open(fpath) as f:
                    s = f.read()
                s = s.replace("__template__", options.name)
                s = s.replace("{TREXTERN_PATH}", scriptDir)
                with open(fpath, "w") as f:
                    f.write(s)
                # Add files we need to rename
                if '__template__' in fname:
                    filesToRename.append(fpath)
                if '__template__' in dname:
                    filesToRename.append(dname)

        # Rename files with '__template__' in name
        for filepath in filesToRename:
            newpath = filepath.replace("__template__", options.name)
            os.rename(filepath, newpath)

    except:
        sys.exit(0)
