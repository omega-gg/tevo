SK = $$_PRO_FILE_PWD_/../Sky

SK_CORE    = $$SK/src/SkCore/src
SK_GUI     = $$SK/src/SkGui/src
SK_BACKEND = $$SK/src/SkBackend/src
SK_MEDIA   = $$SK/src/SkMedia/src
SK_TORRENT = $$SK/src/SkTorrent/src

TARGET = tevo

DESTDIR = $$_PRO_FILE_PWD_/bin

contains(QT_MAJOR_VERSION, 4) {
    QT += network script xml xmlpatterns

} else:contains(QT_MAJOR_VERSION, 5) {

    QT += network qml xml xmlpatterns
} else {
    QT += network qml xml core5compat
}

# NOTE: This makes sure we don't build against the gui dependency. It does not work for Qt4
#       because we need QDesktopServices.
#greaterThan(QT_MAJOR_VERSION, 4) {
#    QT -= gui
#}

CONFIG += console

contains(QT_MAJOR_VERSION, 5) {
    android:QT += androidextras
}

# C++17
contains(QT_MAJOR_VERSION, 4) {
    QMAKE_CXXFLAGS += -std=c++1z
} else {
    CONFIG += c++1z
}

DEFINES += QUAZIP_BUILD \
           SK_CONSOLE \
           SK_NO_QML \
           SK_CORE_LIBRARY SK_GUI_LIBRARY SK_BACKEND_LIBRARY SK_MEDIA_LIBRARY SK_TORRENT_LIBRARY \
           SK_CHARSET SK_BACKEND_LOCAL #SK_BACKEND_LOG

ios:DEFINES += SK_NO_TORRENT

win32-msvc* {
    # libtorrent: This fixes the winsock2 and std::min errors.
    DEFINES += WIN32_LEAN_AND_MEAN NOMINMAX

    # Boost: This prevents an issue with linking.
    DEFINES += BOOST_ALL_NO_LIB
}

!win32-msvc*:!ios:!android:DEFINES += CAN_COMPILE_SSE2

deploy|ios|android: DEFINES += SK_DEPLOY

!win32-msvc*:!ios:!android:QMAKE_CXXFLAGS += -msse

# NOTE: This is required to load frameworks in the lib folder.
ios:QMAKE_LFLAGS += -F$$SK/lib

unix:QMAKE_LFLAGS += "-Wl,-rpath,'\$$ORIGIN'"

include($$SK/src/Sk.pri)
include(src/global/global.pri)
include(src/controllers/controllers.pri)
include(src/kernel/kernel.pri)
include(src/io/io.pri)
include(src/thread/thread.pri)
include(src/network/network.pri)
include(src/image/image.pri)
include(src/media/media.pri)

contains(DEFINES, SK_VLC) {
    include(src/vlc/vlc.pri)
} else {
    include(src/ffmpeg/ffmpeg.pri)
}

include(src/torrent/torrent.pri)

include(src/3rdparty/qtsingleapplication/qtsingleapplication.pri)
include(src/3rdparty/zlib/zlib.pri)
include(src/3rdparty/quazip/quazip.pri)
include(src/3rdparty/libcharsetdetect/libcharsetdetect.pri)

INCLUDEPATH += $$SK/include/SkCore \
               $$SK/include/SkGui \
               $$SK/include/SkBackend \
               $$SK/include/SkMedia \
               $$SK/include/SkTorrent \
               $$SK/include \
               $$_PRO_FILE_PWD_/include/tevo

# NOTE: FFmpeg plays its audio through Qt Multimedia.
!contains(DEFINES, SK_VLC) {
    QT += multimedia

    INCLUDEPATH += $$SK/include/ffmpeg
}

# NOTE android: Required for qandroidextras_p
android:greaterThan(QT_MAJOR_VERSION, 4) {
    INCLUDEPATH += $$SK/include/$$QTX \
                   $$SK/include/$$QTX/QtCore
}

contains(DEFINES, SK_VLC) {

    ENGINE = -lvlc

    win32:ENGINE = -llibvlc

    vlc4 {
        ios:LIBS += -framework VLCKit
    } else {
        ios:LIBS += -framework MobileVLCKit
    }
} else {
    # NOTE: Our FFmpeg libraries are prefixed with sk beside Qt Multimedia's FFmpeg.

    ENGINE = -lsk-avformat \
             -lsk-avcodec \
             -lsk-avfilter \
             -lsk-avutil \
             -lsk-swscale \
             -lsk-swresample

    ios:LIBS += -framework sk-avformat \
                -framework sk-avcodec \
                -framework sk-avfilter \
                -framework sk-avutil \
                -framework sk-swscale \
                -framework sk-swresample
}

win32:LIBS += -L$$SK/lib $$ENGINE \
              -lws2_32

win32:LIBS += -L$$SK/lib -ltorrent \
              -L$$SK/lib -lboost_system

# Windows dependency for ShellExecuteA
win32-msvc*:LIBS += shell32.lib

unix:!ios:!android:LIBS += -L$$SK/lib $$ENGINE \
                           -L$$SK/lib -ltorrent-rasterbar \
                           -L$$SK/lib -lboost_system

# NOTE iOS: MediaPlayer is required for MP* classes.
ios:LIBS += -framework MediaPlayer

android:LIBS += -L$$ANDROID_LIB $$ENGINE \
                -L$$ANDROID_LIB -ltorrent-rasterbar \
                -L$$ANDROID_LIB -ltry_signal

macx {
    PATH=$${DESTDIR}/$${TARGET}.app/Contents/MacOS

contains(DEFINES, SK_VLC) {
    QMAKE_POST_LINK = install_name_tool -change @rpath/libvlccore.dylib \
                      @loader_path/libvlccore.dylib $${DESTDIR}/libvlc.dylib;

    QMAKE_POST_LINK += install_name_tool -change @rpath/libvlc.dylib \
                       @loader_path/libvlc.dylib $$PATH/$${TARGET};
} else {
    QMAKE_POST_LINK = install_name_tool -add_rpath @loader_path $$PATH/$${TARGET};
}

    QMAKE_POST_LINK += install_name_tool -change libtorrent-rasterbar.dylib.2.0.11 \
                       @loader_path/libtorrent-rasterbar.dylib $$PATH/$${TARGET};

    QMAKE_POST_LINK += install_name_tool -change libboost_system.dylib \
                       @loader_path/libboost_system.dylib $$PATH/$${TARGET};

contains(DEFINES, SK_VLC) {
    QMAKE_POST_LINK += $${QMAKE_COPY} -r $${DESTDIR}/plugins $$PATH;

    QMAKE_POST_LINK += $${QMAKE_COPY} $${DESTDIR}/libvlc.dylib     $$PATH;
    QMAKE_POST_LINK += $${QMAKE_COPY} $${DESTDIR}/libvlccore.dylib $$PATH;
} else {
    QMAKE_POST_LINK += $${QMAKE_COPY} -P $${DESTDIR}/libsk-* $$PATH;
}
}

macx:ICON = dist/icon.icns

RC_FILE = dist/tevo.rc

OTHER_FILES += 3rdparty.sh \
               configure.sh \
               build.sh \
               deploy.sh \
               environment.sh \
               README.md \
               LICENSE.md \
               AUTHORS.md \
               .azure-pipelines.yml \
               .appveyor.yml \
               content/generate.sh \
               dist/tevo.rc \
               dist/script/start.sh \
               dist/changes/1.1.0.md \
