#!/usr/bin/env python3
# copy_iodevice_for_duino.py 15.7.2020/pekka
# Copies iodevice library files needed for PlatformIO Arduino build
# into /coderoot/lib/arduino-platformio/iodevice directory.
# To make this look like Arduino library, all .c and .cpp
# files are copied directly to target directory, and all header
# files info subdirectories as in original.
from os import listdir, makedirs
from os.path import isfile, isdir, join, splitext, exists
from shutil import copyfile
import sys

def mymakedir(targetdir):
    if not exists(targetdir):
        makedirs(targetdir)

def copy_level_2(sourcedir,roottargetdir,targetdir):
    files = listdir(sourcedir)

    # Copy header files
    for f in files:
        p = join(sourcedir, f)
        if isfile(p):
            e = splitext(p)[1]
            if e == '.h':
                mymakedir(targetdir)
                t = join(targetdir, f)
                copyfile(p, t)
            if e == '.c' or e == '.cpp':
                t = join(roottargetdir, f)
                copyfile(p, t)

def copy_info(f,sourcedir,targetdir):
    infodir = sourcedir + '/osbuild/arduino-library'
    p = join(infodir, f)
    t = join(targetdir, f)
    if exists(p):
        copyfile(p, t)

def copy_level_1(sourcedir,targetdir):
    mymakedir(targetdir)
    files = listdir(sourcedir)

    # Copy header files
    for f in files:
        p = join(sourcedir, f)
        if isfile(p):
            e = splitext(p)[1]
            if e == '.h':
                t = join(targetdir, f)
                copyfile(p, t)

    # Copy code and extensions folders
    copy_level_2(sourcedir + '/code', targetdir, targetdir + '/code')

    # Copy informative arduino files
    copy_info('library.json', sourcedir, targetdir)
    copy_info('library.properties', sourcedir, targetdir)

def mymain():
    outdir = "/coderoot/lib/arduino-platformio/iodevice"
    expectplatform = True
    n = len(sys.argv)
    for i in range(1, n):
        if sys.argv[i][0] == "-":
            if sys.argv[i][1] == "o":
                expectplatform = False

        else:
            if not expectplatform:
                outdir = sys.argv[i];

            expectplatform = True

    copy_level_1("/coderoot/iocom/extensions/iodevice", outdir)

# Usage copy_iodevice_for_duino.py -o /coderoot/lib/esp32/iodevice
mymain()

