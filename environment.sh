#!/bin/sh
set -e

#--------------------------------------------------------------------------------------------------
# Settings
#--------------------------------------------------------------------------------------------------

Sky="../Sky"

#--------------------------------------------------------------------------------------------------
# environment

compiler_win="mingw"

qt="qt6"

vlc="vlc3"

#--------------------------------------------------------------------------------------------------
# Functions
#--------------------------------------------------------------------------------------------------

replace()
{
    expression='s/'"$1"'=\"'"$2"'"/'"$1"'=\"'"$3"'"/g'

    apply $expression environment.sh

    apply $expression 3rdparty.sh
    apply $expression configure.sh
    apply $expression build.sh
    apply $expression deploy.sh
}

apply()
{
    if [ $host = "macOS" ]; then

        sed -i "" $1 $2
    else
        sed -i $1 $2
    fi
}

#--------------------------------------------------------------------------------------------------

getOs()
{
    case `uname` in
    Darwin*) echo "macOS";;
    *)       echo "other";;
    esac
}

#--------------------------------------------------------------------------------------------------
# Syntax
#--------------------------------------------------------------------------------------------------

if [ $# != 1 -a $# != 2 ] \
   || \
   [ $1 != "mingw" -a $1 != "msvc" -a \
     $1 != "qt4"   -a $1 != "qt5"  -a $1 != "qt6" -a \
     $1 != "vlc3"  -a $1 != "vlc4" ] \
   || \
   [ $# = 2 -a "$2" != "all" ]; then

    echo "Usage: environment <mingw | msvc"
    echo "                    qt4 | qt5 | qt6 |"
    echo "                    vlc3 | vlc4> [all]"

    exit 1
fi

#--------------------------------------------------------------------------------------------------
# Configuration
#--------------------------------------------------------------------------------------------------

host=$(getOs)

#--------------------------------------------------------------------------------------------------
# Sky
#--------------------------------------------------------------------------------------------------

if [ "$2" = "all" ]; then

    echo "ENVIRONMENT Sky"
    echo "---------------"

    cd "$Sky"

    sh environment.sh $1 all

    cd -

    echo "---------------"
    echo ""
fi

#--------------------------------------------------------------------------------------------------
# Replacements
#--------------------------------------------------------------------------------------------------

if [ $1 = "msvc" ]; then

    replace compiler_win $compiler_win msvc

elif [ $1 = "mingw" ]; then

    replace compiler_win $compiler_win mingw

elif [ $1 = "vlc3" -o $1 = "vlc4" ]; then

    replace vlc $vlc $1
else
    replace qt $qt $1
fi
