#!/bin/sh
set -e

#--------------------------------------------------------------------------------------------------
# Settings
#--------------------------------------------------------------------------------------------------

target="tevo"

Sky="../Sky"

backend="../backend"

#--------------------------------------------------------------------------------------------------
# environment

compiler_win="mingw"

qt="qt5"

#--------------------------------------------------------------------------------------------------
# Syntax
#--------------------------------------------------------------------------------------------------

if [ $# != 1 -a $# != 2 ] \
   || \
   [ $1 != "win32" -a $1 != "win64" -a $1 != "macOS" -a $1 != "linux" -a $1 != "android" ] \
   || \
   [ $# = 2 -a "$2" != "clean" ]; then

    echo "Usage: deploy <win32 | win64 | macOS | linux | android> [clean]"

    exit 1
fi

#--------------------------------------------------------------------------------------------------
# Configuration
#--------------------------------------------------------------------------------------------------

if [ $1 = "win32" -o $1 = "win64" ]; then

    os="windows"

    compiler="$compiler_win"
else
    os="default"

    compiler="default"
fi

if [ $qt = "qt5" ]; then

    QtX="Qt5"

    qx="5"

elif [ $qt = "qt6" ]; then

    QtX="Qt6"

    if [ $1 = "macOS" ]; then

        qx="A"
    else
        qx="6"
    fi
fi

#--------------------------------------------------------------------------------------------------
# Clean
#--------------------------------------------------------------------------------------------------

echo "CLEANING"

rm -rf deploy/*

touch deploy/.gitignore

if [ "$2" = "clean" ]; then

    exit 0
fi

echo ""

#--------------------------------------------------------------------------------------------------
# Bundle
#--------------------------------------------------------------------------------------------------

deploy="deploy"

#--------------------------------------------------------------------------------------------------
# Sky
#--------------------------------------------------------------------------------------------------

echo "DEPLOYING Sky"
echo "-------------"

cd "$Sky"

sh deploy.sh $1 tools

cd -

path="$Sky/deploy"

#--------------------------------------------------------------------------------------------------
# Qt
#--------------------------------------------------------------------------------------------------

if [ $os = "windows" ]; then

    if [ $compiler = "mingw" ]; then

        cp "$path"/libgcc_s_*-1.dll    $deploy
        cp "$path"/libstdc++-6.dll     $deploy
        cp "$path"/libwinpthread-1.dll $deploy
    fi

    if [ $qt = "qt4" ]; then

        cp "$path"/QtCore4.dll        $deploy
        cp "$path"/QtGui4.dll         $deploy
        cp "$path"/QtNetwork4.dll     $deploy
        cp "$path"/QtScript4.dll      $deploy
        cp "$path"/QtXml4.dll         $deploy
        cp "$path"/QtXmlPatterns4.dll $deploy
    else
        cp "$path/$QtX"Core.dll    $deploy
        cp "$path/$QtX"Gui.dll     $deploy
        cp "$path/$QtX"Network.dll $deploy
        cp "$path/$QtX"Qml.dll     $deploy
        cp "$path/$QtX"Xml.dll     $deploy

        if [ $qt = "qt5" ]; then

            cp "$path/$QtX"XmlPatterns.dll $deploy
        else
            cp "$path/$QtX"Core5Compat.dll $deploy
        fi
    fi

elif [ $1 = "macOS" ]; then

    if [ $qt != "qt4" ]; then

        # FIXME Qt 5.14 macOS: We have to copy qt.conf to avoid a segfault.
        cp "$path"/qt.conf $deploy

        cp "$path"/QtCore.dylib    $deploy
        cp "$path"/QtGui.dylib     $deploy
        cp "$path"/QtNetwork.dylib $deploy
        cp "$path"/QtQml.dylib     $deploy
        cp "$path"/QtXml.dylib     $deploy

        if [ $qt = "qt5" ]; then

            cp "$path"/QtXmlPatterns.dylib $deploy
        else
            cp "$path"/QtCore5Compat.dylib $deploy
        fi
    fi

elif [ $1 = "linux" ]; then

    if [ $qt = "qt4" ]; then

        cp "$path"/libQtCore.so.4        $deploy
        cp "$path"/libQtGui.so.4         $deploy
        cp "$path"/libQtNetwork.so.4     $deploy
        cp "$path"/libQtScript.so.4      $deploy
        cp "$path"/libQtXml.so.4         $deploy
        cp "$path"/libQtXmlPatterns.so.4 $deploy
    else
        cp "$path"/libicudata.so.* $deploy
        cp "$path"/libicui18n.so.* $deploy
        cp "$path"/libicuuc.so.*   $deploy

        cp "$path/lib$QtX"Core.so.$qx    $deploy
        cp "$path/lib$QtX"Gui.so.$qx     $deploy
        cp "$path/lib$QtX"Network.so.$qx $deploy
        cp "$path/lib$QtX"Qml.so.$qx     $deploy
        cp "$path/lib$QtX"Xml.so.$qx     $deploy

        if [ $qt = "qt5" ]; then

            cp "$path/lib$QtX"XmlPatterns.so.$qx $deploy
        else
            cp "$path/lib$QtX"Core5Compat.so.$qx $deploy
        fi
    fi

elif [ $1 = "android" ]; then

    if [ $qt != "qt4" ]; then

        cp "$path/lib$QtX"Core_*.so    $deploy
        cp "$path/lib$QtX"Gui_*.so     $deploy
        cp "$path/lib$QtX"Network_*.so $deploy
        cp "$path/lib$QtX"Qml_*.so     $deploy
        cp "$path/lib$QtX"Xml_*.so     $deploy

        if [ $qt = "qt5" ]; then

            cp "$path/lib$QtX"XmlPatterns_*.so $deploy
        else
            cp "$path/lib$QtX"Core5Compat_*.so $deploy
        fi
    fi
fi

#--------------------------------------------------------------------------------------------------
# SSL
#--------------------------------------------------------------------------------------------------

if [ $os = "windows" ]; then

    if [ $qt = "qt4" ]; then

        cp "$path"/libeay32.dll $deploy
        cp "$path"/ssleay32.dll $deploy
    else
        cp "$path"/libssl*.dll    $deploy
        cp "$path"/libcrypto*.dll $deploy
    fi

elif [ $1 = "linux" ]; then

    cp "$path"/libssl.so*    $deploy
    cp "$path"/libcrypto.so* $deploy
fi

#--------------------------------------------------------------------------------------------------
# VLC
#--------------------------------------------------------------------------------------------------

if [ $os = "windows" ]; then

    cp -r "$path"/plugins $deploy

    cp "$path"/libvlc*.dll $deploy

elif [ $1 = "macOS" ]; then

    cp -r "$path"/plugins $deploy

    cp "$path"/libvlc*.dylib $deploy

elif [ $1 = "linux" ]; then

    cp -r "$path"/vlc $deploy

    cp "$path"/libvlc*.so* $deploy
fi

#--------------------------------------------------------------------------------------------------
# libtorrent
#--------------------------------------------------------------------------------------------------

if [ $os = "windows" ]; then

    cp "$path"/*torrent-rasterbar.dll $deploy

elif [ $1 = "macOS" ]; then

    cp "$path"/libtorrent-rasterbar.dylib $deploy

elif [ $1 = "linux" ]; then

    cp "$path"/libtorrent-rasterbar*.so* $deploy
fi

#--------------------------------------------------------------------------------------------------
# Boost
#--------------------------------------------------------------------------------------------------

if [ $1 = "macOS" ]; then

    cp "$path"/libboost*.dylib $deploy

elif [ $1 = "linux" ]; then

    cp "$path"/libboost*.so* $deploy
fi

echo "-------------"
echo ""

#--------------------------------------------------------------------------------------------------
# tevo
#--------------------------------------------------------------------------------------------------

echo "COPYING $target"

if [ $os = "windows" ]; then

    cp bin/$target.exe $deploy

elif [ $1 = "macOS" ]; then

    cp bin/$target $deploy

    cd $deploy

    #----------------------------------------------------------------------------------------------
    # Qt

    install_name_tool -change @rpath/QtGui.framework/Versions/$qx/QtGui \
                              @loader_path/QtGui.dylib $target

    install_name_tool -change @rpath/QtCore.framework/Versions/$qx/QtGui \
                              @loader_path/QtGui.dylib $target

    install_name_tool -change @rpath/QtNetwork.framework/Versions/$qx/QtNetwork \
                              @loader_path/QtNetwork.dylib $target

    install_name_tool -change @rpath/QtQml.framework/Versions/$qx/QtQml \
                              @loader_path/QtQml.dylib $target

    install_name_tool -change @rpath/QtXml.framework/Versions/$qx/QtXml \
                              @loader_path/QtXml.dylib $target

    if [ $qt = "qt5" ]; then

        install_name_tool -change @rpath/QtXmlPatterns.framework/Versions/$qx/QtXmlPatterns \
                                  @loader_path/QtXmlPatterns.dylib $target
    else
        install_name_tool -change @rpath/QtCore5Compat.framework/Versions/$qx/QtCore5Compat \
                                  @loader_path/QtCore5Compat.dylib $target
    fi

    otool -L $target

    #----------------------------------------------------------------------------------------------

    cd -

elif [ $1 = "linux" ]; then

    cp bin/$target $deploy

    # NOTE: This script is useful for compatibilty. It enforces the application path for libraries.
    cp dist/script/start.sh $deploy

    chmod +x $deploy/start.sh

elif [ $1 = "android" ]; then

    cp bin/lib$target* $deploy
fi

#--------------------------------------------------------------------------------------------------
# backend
#--------------------------------------------------------------------------------------------------

if [ $1 != "android" ]; then

    echo "COPYING backend"

    mkdir -p $deploy/backend/cover

    cp "$backend"/cover/* $deploy/backend/cover

    cp "$backend"/*.vbml $deploy/backend
fi
