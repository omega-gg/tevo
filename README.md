tevo
---
[![azure](https://dev.azure.com/bunjee/tevo/_apis/build/status/omega-gg.tevo)](https://dev.azure.com/bunjee/tevo/_build)
[![appveyor](https://ci.appveyor.com/api/projects/status/nc4cf1k90abftiyj?svg=true)](https://ci.appveyor.com/project/3unjee/tevo)
[![GPLv3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl.html)

tevo is a command line [Semantic Player](https://omega.gg/about/SemanticPlayer) built for [Motion Freedom](https://omega.gg/about/MotionFreedom).<br>
Designed to retrieve and play Internet video resources from a single line of text.<br>
With a focus on simplicity, efficiency and lightness.<br>

## Backends

tevo accesses and aggregates videos via the [VBML](https://omega.gg/VBML) language.<br>

It supports [DuckDuckGo](https://en.wikipedia.org/wiki/DuckDuckGo),
            [vox](https://omega.gg/vox),
            [BitTorrent](https://en.wikipedia.org/wiki/BitTorrent),
            [TMDB](https://www.themoviedb.org),
            [Youtube](https://en.wikipedia.org/wiki/Youtube),
            [Dailymotion](https://en.wikipedia.org/wiki/Dailymotion),
            [Vimeo](https://en.wikipedia.org/wiki/Vimeo),
            [Twitch](https://en.wikipedia.org/wiki/Twitch_(service)),
            [IPTV(s)](https://github.com/iptv-org/iptv),
            [TikTok](https://en.wikipedia.org/wiki/TikTok),
            [Twitter](https://en.wikipedia.org/wiki/Twitter),
            [Facebook](https://en.wikipedia.org/wiki/Facebook),
            [Odysee](https://en.wikipedia.org/wiki/Odysee),
            [PeerTube](https://en.wikipedia.org/wiki/PeerTube),
            [Last.fm](https://en.wikipedia.org/wiki/Lastfm) and
            [SoundCloud](https://en.wikipedia.org/wiki/SoundCloud).<br>

## Usage

    tevo [options] <text>

    Where <text> is the query or resource you want to play.

## Options

    --help     Print this help text and exit
    --verbose  Print debugging informations
    --quiet    Mute application output
    --at       Start time with the 00:00:00.000 format
    --end      End time with the 00:00:00.000 format (overrides duration)
    --duration Duration with the 00:00:00.000 format
    --backend  Select a backend based on its id (defaults to duckduckgo)

## Technology

tevo is built in C++ with [Sky kit](https://omega.gg/Sky/sources).<br>

## Platforms

- Windows XP and later.
- macOS 64 bit.
- Linux 32 bit and 64 bit.
- Android 32 bit and 64 bit.

## Requirements

- [Sky](https://omega.gg/Sky/sources) latest version.
- [Qt](https://download.qt.io/official_releases/qt) 4.8.0 / 5.5.0 or later.

On Windows:
- [MinGW](https://sourceforge.net/projects/mingw) or [Git for Windows](https://git-for-windows.github.io) with g++ 4.9.2 or later.

Recommended:
- [Qt Creator](https://download.qt.io/official_releases/qtcreator) 3.6.0 or later.

## Quickstart

You can configure and build tevo with a single line:

    sh build.sh <win32 | win64 | macOS | linux | android> all

For instance you would do that for Windows 64 bit:

    * open Git Bash *
    git clone https://github.com/omega-gg/tevo.git
    cd tevo
    sh build.sh win64 all

That's a convenient way to configure and build everything the first time.

Note: This will create the 3rdparty and Sky folder in the parent directory.

## Building

Alternatively, you can run each step of the build yourself by calling the following scripts:

Install the dependencies:

    sh 3rdparty.sh <win32 | win64 | macOS | linux | android> [all]

Configure the build:

    sh configure.sh <win32 | win64 | macOS | linux | android> [sky | clean]

Build the application:

    sh build.sh <win32 | win64 | macOS | linux | android> [all | deploy | clean]

Deploy the application and its dependencies:

    sh deploy.sh <win32 | win64 | macOS | linux | android> [clean]

## License

Copyright (C) 2015 - 2024 tevo authors | https://omega.gg/tevo

### Authors

- Benjamin Arnaud aka [bunjee](https://bunjee.me) | <bunjee@omega.gg>

### GNU General Public License Usage

tevo may be used under the terms of the GNU General Public License version 3 as published by the
Free Software Foundation and appearing in the LICENSE.md file included in the packaging of this
file. Please review the following information to ensure the GNU General Public License requirements
will be met: https://www.gnu.org/licenses/gpl.html.

### Private License Usage

tevo licensees holding valid private licenses may use this file in accordance with the private
license agreement provided with the Software or, alternatively, in accordance with the terms
contained in written agreement between you and tevo authors. For further information contact us at
contact@omega.gg.
