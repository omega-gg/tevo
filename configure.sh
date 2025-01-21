#!/bin/sh
set -e

#--------------------------------------------------------------------------------------------------
# Settings
#--------------------------------------------------------------------------------------------------

Sky="../Sky"

external="../3rdparty"

#--------------------------------------------------------------------------------------------------

SSL_versionA="1.0.2u"
SSL_versionB="1.1.1s"

VLC_version="3.0.21"

libtorrent_version="2.0.10"

#--------------------------------------------------------------------------------------------------
# Windows

MinGW_version="13.1.0"

#--------------------------------------------------------------------------------------------------
# environment

compiler_win="mingw"

qt="qt5"

#--------------------------------------------------------------------------------------------------
# Functions
#--------------------------------------------------------------------------------------------------

copyAndroid()
{
    cp -r "$1"/armeabi-v7a $data/armeabi-v7a/libs
    cp -r "$1"/arm64-v8a   $data/arm64-v8a/libs
    cp -r "$1"/x86         $data/x86/libs
    cp -r "$1"/x86_64      $data/x86_64/libs
}

cleanAndroid()
{
    path="$data/$1/libs/$1"

    rm -f $path/*.so
    rm -f $path/*.a
}

#--------------------------------------------------------------------------------------------------
# Syntax
#--------------------------------------------------------------------------------------------------

if [ $# != 1 -a $# != 2 ] \
   || \
   [ $1 != "win32" -a $1 != "win64" -a $1 != "macOS" -a $1 != "linux" -a $1 != "android" ] \
   || \
   [ $# = 2 -a "$2" != "sky" -a "$2" != "clean" ]; then

    echo "Usage: configure <win32 | win64 | macOS | linux | android> [sky | clean]"

    exit 1
fi

#--------------------------------------------------------------------------------------------------
# Configuration
#--------------------------------------------------------------------------------------------------

external="$PWD/$external/$1"

if [ $1 = "win32" -o $1 = "win64" ]; then

    os="windows"

    compiler="$compiler_win"

    if [ $compiler = "mingw" ]; then

        MinGW="$external/MinGW/$MinGW_version/bin"
    fi
else
    os="default"

    compiler="default"
fi

#--------------------------------------------------------------------------------------------------

if [ $qt = "qt4" ]; then

    SSL="$external/OpenSSL/$SSL_versionA"
else
    SSL="$external/OpenSSL/$SSL_versionB"
fi

VLC="$external/VLC/$VLC_version"

libtorrent="$external/libtorrent/$libtorrent_version"

#--------------------------------------------------------------------------------------------------
# Clean
#--------------------------------------------------------------------------------------------------

echo "CLEANING"

# NOTE: We want to keep the 'storage' folder.
if [ -d "bin/storage" ]; then

    mv bin/storage .

    rm -rf bin/*
    touch  bin/.gitignore

    mv storage bin
else
    rm -rf bin/*
    touch  bin/.gitignore
fi

# NOTE: We have to remove the folder to delete .qmake.stash.
rm -rf build
mkdir  build
touch  build/.gitignore

if [ $1 = "android" ]; then

    data="dist/android/data"

    cleanAndroid armeabi-v7a
    cleanAndroid arm64-v8a
    cleanAndroid x86
    cleanAndroid x86_64
fi

if [ "$2" = "clean" ]; then

    exit 0
fi

#--------------------------------------------------------------------------------------------------
# Sky
#--------------------------------------------------------------------------------------------------

if [ "$2" = "sky" ]; then

    echo "CONFIGURING Sky"
    echo "---------------"

    cd "$Sky"

    sh configure.sh $1

    cd -

    echo "---------------"
    echo ""
fi

#--------------------------------------------------------------------------------------------------
# MinGW
#--------------------------------------------------------------------------------------------------

echo "CONFIGURING tevo"
echo "----------------"

if [ $compiler = "mingw" ]; then

    cp "$MinGW"/libgcc_s_*-1.dll    bin
    cp "$MinGW"/libstdc++-6.dll     bin
    cp "$MinGW"/libwinpthread-1.dll bin
fi

#--------------------------------------------------------------------------------------------------
# SSL
#--------------------------------------------------------------------------------------------------

if [ $os = "windows" ]; then

    echo "COPYING SSL"

    cp "$SSL"/*.dll bin

elif [ $1 = "android" ]; then

    echo "COPYING SSL"

    copyAndroid "$SSL"
fi

#--------------------------------------------------------------------------------------------------
# VLC
#--------------------------------------------------------------------------------------------------

if [ $os = "windows" ]; then

    echo "COPYING VLC"

    rm -rf bin/plugins
    mkdir  bin/plugins

    cp -r "$VLC"/plugins bin

    cp "$VLC"/libvlc*.dll bin

elif [ $1 = "macOS" ]; then

    echo "COPYING VLC"

    rm -rf bin/plugins
    mkdir  bin/plugins

    cp -r "$VLC"/plugins/*.dylib bin/plugins

    cp "$VLC"/lib/libvlc.5.dylib     bin/libvlc.dylib
    cp "$VLC"/lib/libvlccore.9.dylib bin/libvlccore.dylib

elif [ $1 = "linux" ]; then

    echo "COPYING VLC"

    rm -rf bin/vlc
    mkdir  bin/vlc

    cp -r "$VLC"/vlc bin

    cp "$VLC"/lib*.so.* bin

elif [ $1 = "android" ]; then

    echo "COPYING VLC"

    copyAndroid "$VLC"
fi

#--------------------------------------------------------------------------------------------------
# libtorrent
#--------------------------------------------------------------------------------------------------

if [ $1 = "android" ]; then

    echo "COPYING libtorrent"

    copyAndroid "$libtorrent"
fi

echo "----------------"
