#!/bin/sh

#builds shared 32/64bit wine on 64bit system gentoo

mkdir wine64
mkdir wine32

cd wine64
../configure --enable-win64 "$@"
make

cd ../wine32
../configure --with-wine64=../wine64 --libdir=/usr/lib32 "$@"
make

echo "to install run as root:"
echo "cd wine32 && make install && cd ../wine64 && make install"

