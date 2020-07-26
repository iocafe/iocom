25.7.2020 / pekka
iocompython is Python wrapper for the IOCOM library.


Upgrading cmake 
****************
Linux build requires cmake 3.12 or newer. Older versions do not have
right support for fining Python headers. To upgrade smake, first
remove current cmake which came with distribution:

sudo apt-get remove cmake

Install cmake to /usr/local. This is not nice debian package install, but should work.

sudo wget -qO- "https://cmake.org/files/v3.18/cmake-3.18.0-Linux-x86_64.tar.gz" | sudo tar --strip-components=1 -xz -C /usr/local

qtcreator: path to cmake needs to be set in [Tools][Options], "Build and run", [Cmake] tab: Set"/usr/local/bin/cmake".
