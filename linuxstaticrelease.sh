#!/bin/sh
DATESTAMP=`date +%Y%m%d`
echo "Configuring without OpenGL"
#export CXXFLAGS=-O3
#export CFLAGS=-O3
#export CXX=g++
#export CC=gcc
export G2VER=0.8.4
./configure --with-sdl-prefix=/usr/local --disable-opengl
echo "Making glob2"
make >& /dev/null
cd src
echo "Creating static binary"
./staticlink.sh
cd ..
echo "Creating source distrib"
make dist >& /dev/null
echo "Decompressing dist archive in /tmp"
gunzip -cd glob2-$G2VER.tar.gz | tar x -C/tmp
echo "Patching archive with binary files"
cp src/glob2 /tmp/glob2-$G2VER/src
echo "Recompressing archive"
cd /tmp
tar cfz glob2-$G2VER-static.tar.gz glob2-$G2VER/
scp glob2-$G2VER-static.tar.gz nct@lappc22.epfl.ch:~/public_html/
ssh nct@lappc22.epfl.ch ln -f -s ~/public_html/glob2-$G2VER-static.tar.gz ~/public_html/glob2-latest-static.tar.gz
