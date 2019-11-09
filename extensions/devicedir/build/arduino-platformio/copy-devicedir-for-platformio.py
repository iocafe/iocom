# copy-devicedir-for-platformio.py 9.11.2019/pekka
# Copies devicedir library files needed for PlatformIO Arduino build
# into /coderoot/lib/arduino-platformio/devicedir directory. 
# To make this look like Arduino library all .c and .cpp
# files are copied to target root folder, and all header
# files info subfolders.
from os import listdir, makedirs
from os.path import isfile, isdir, join, splitext, exists
from shutil import copyfile


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
    infodir = sourcedir + '/build/arduino-library'
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
#    copy_level_2(sourcedir + '/extensions', targetdir, targetdir + '/extensions')

    # Copy informative arduino files
    copy_info('library.json', sourcedir, targetdir)
    copy_info('library.properties', sourcedir, targetdir)


copy_level_1("/coderoot/iocom/extensions/devicedir", "/coderoot/lib/arduino-platformio/devicedir")