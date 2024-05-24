//=================================================================================================
/*
    Copyright (C) 2015-2020 tevo authors united with omega. <http://omega.gg/about>

    Author: Benjamin Arnaud. <http://bunjee.me> <bunjee@omega.gg>

    This file is part of tevo.

    - GNU General Public License Usage:
    This file may be used under the terms of the GNU General Public License version 3 as published
    by the Free Software Foundation and appearing in the LICENSE.md file included in the packaging
    of this file. Please review the following information to ensure the GNU General Public License
    requirements will be met: https://www.gnu.org/licenses/gpl.html.

    - Private License Usage:
    tevo licensees holding valid private licenses may use this file in accordance with the private
    license agreement provided with the Software or, alternatively, in accordance with the terms
    contained in written agreement between you and tevo authors. For further information contact us
    at contact@omega.gg.
*/
//=================================================================================================

#include "ControllerCore.h"

// Qt includes
#include <QCoreApplication>
#include <QDir>

// Sk includes
#include <WControllerApplication>
#include <WControllerFile>
#include <WControllerMedia>
#include <WControllerTorrent>
#include <WBackendIndex>
#include <WBackendTorrent>
#include <WPlayer>

W_INIT_CONTROLLER(ControllerCore)

//-------------------------------------------------------------------------------------------------
// Defines

#ifdef QT_4
#define qInfo qWarning
#endif

//-------------------------------------------------------------------------------------------------
// Static variables

static const QString CORE_VERSION = "1.0.0-0";

// NOTE: Defaut streaming port for tevo.
static const int CORE_PORT = 8400;

#ifndef SK_DEPLOY
static const QString PATH_STORAGE = "/storage";
static const QString PATH_BACKEND = "../../backend";
#endif

//-------------------------------------------------------------------------------------------------
// Ctor / dtor
//-------------------------------------------------------------------------------------------------

ControllerCore::ControllerCore() : WController()
{
    //---------------------------------------------------------------------------------------------
    // Settings

    sk->setName("tevo");

    sk->setVersion(CORE_VERSION);

#ifdef SK_DEPLOY
    _path = QDir::fromNativeSeparators(WControllerFile::pathWritable());
#else
    _path = QDir::currentPath() + PATH_STORAGE;
#endif
}

//-------------------------------------------------------------------------------------------------
// Interface
//-------------------------------------------------------------------------------------------------

/* Q_INVOKABLE */ bool ControllerCore::run(int & argc, char ** argv)
{
    //---------------------------------------------------------------------------------------------
    // Log

    wControllerFile->initMessageHandler();

    //---------------------------------------------------------------------------------------------
    // Usage

    if (argc != 2)
    {
        qInfo("tevo %s", sk->version().C_STR);

        usage();

        return false;
    }

    _text = argv[1];

    //wControllerFile->setVerbosity(QtInfoMsg);

    qInfo("tevo %s", sk->version().C_STR);

    qInfo("text: %s", _text.C_STR);

    //---------------------------------------------------------------------------------------------
    // Controllers

    W_CREATE_CONTROLLER(WControllerPlaylist);
    W_CREATE_CONTROLLER(WControllerMedia);

#ifndef SK_NO_TORRENT
    W_CREATE_CONTROLLER_2(WControllerTorrent, _path + "/torrents", CORE_PORT);
#endif

    //---------------------------------------------------------------------------------------------
    // Backend index

    QString path = _path + "/backend";

    if (QFile::exists(path) == false)
    {
        if (QDir().mkpath(path) == false)
        {
             qWarning("ControllerCore::run: Failed to create folder %s.", path.C_STR);

             return false;
        }

        WControllerFileReply * reply = copyBackends();

        connect(reply, SIGNAL(complete(bool)), this, SLOT(onLoaded()));
    }
    else createIndex();

    //---------------------------------------------------------------------------------------------
    // Player

    _player = new WPlayer;

    _player->setBackend(new WBackendTorrent);

    _player->setOutput(WAbstractBackend::OutputAudio);

    return true;
}

//-------------------------------------------------------------------------------------------------
// Functions private
//-------------------------------------------------------------------------------------------------

bool ControllerCore::usage() const
{
    qInfo("Usage: tevo <text>");

    return false;
}

//-------------------------------------------------------------------------------------------------

void ControllerCore::createIndex()
{
#ifdef SK_NO_TORRENT
    _index = new WBackendIndex(WControllerFile::fileUrl(_path + "/backend/indexLite.vbml"));
#else
    _index = new WBackendIndex(WControllerFile::fileUrl(_path + "/backend/index.vbml"));
#endif

    connect(_index, SIGNAL(loaded()), this, SLOT(onIndexLoaded()));
}

//-------------------------------------------------------------------------------------------------

WControllerFileReply * ControllerCore::copyBackends() const
{
#ifdef SK_DEPLOY
#ifdef Q_OS_ANDROID
    return WControllerPlaylist::copyBackends("assets:/backend", _path + "/backend/");
#else
    return WControllerPlaylist::copyBackends(WControllerFile::applicationPath("backend"),
                                             _path + "/backend/");
#endif
#else
    return WControllerPlaylist::copyBackends(WControllerFile::applicationPath(PATH_BACKEND),
                                             _path + "/backend/");
#endif
}

//-------------------------------------------------------------------------------------------------
// Private slots
//-------------------------------------------------------------------------------------------------

void ControllerCore::onLoaded()
{
    createIndex();
}

void ControllerCore::onIndexLoaded()
{
    disconnect(_index, SIGNAL(loaded()), this, SLOT(onIndexLoaded()));

#if defined(SK_BACKEND_LOCAL) && defined(SK_DEPLOY) == false
    // NOTE: This makes sure that we have the latest local vbml loaded.
    WControllerFileReply * reply = copyBackends();

    connect(reply, SIGNAL(complete(bool)), _index, SLOT(reload()));
#else
    _index->update();
#endif

    connect(_index, SIGNAL(loaded()), this, SLOT(onIndexUpdated()));
}

void ControllerCore::onIndexUpdated()
{
    // NOTE: We don't want this slot to be called again.
    disconnect(_index, 0, this, SLOT(onIndexUpdated()));

    // FIXME libtorrent 2.0.9: For some reason, we need to wait before deleting the session
    //                         otherwise we get a crash.
    Sk::wait(100);

    QCoreApplication::exit(0);
}
