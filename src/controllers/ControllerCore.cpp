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
#include <WControllerNetwork>
#include <WControllerPlaylist>
#include <WControllerMedia>
#include <WControllerTorrent>
#include <WBackendIndex>
#include <WBackendTorrent>
#include <WPlayer>

W_INIT_CONTROLLER(ControllerCore)

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
    _at  = -1;
    _end = -1;

    //---------------------------------------------------------------------------------------------
    // Log

    wControllerFile->initMessageHandler();

    //---------------------------------------------------------------------------------------------
    // Arguments

    QtMsgType verbosity = QtSystemMsg;

    int duration = -1;

    for (int i = 1; i < argc; i++)
    {
        QString string = argv[i];

        if (string.startsWith("--"))
        {
            QString name = string;

            name = name.remove(0, 2).toLower();

            if (name == "help")
            {
                help();

                return false;
            }
            else if (name == "verbose")
            {
                verbosity = QtDebugMsg;
            }
            else if (name == "quiet")
            {
                verbosity = static_cast<QtMsgType> (QtSystemMsg + 1);
            }
            else if (name.startsWith("at="))
            {
                _at = extractMsecs(name);
            }
            else if (name.startsWith("end="))
            {
                _end = extractMsecs(name);
            }
            else if (name.startsWith("duration="))
            {
                duration = extractMsecs(name);
            }
            else if (name.startsWith("backend="))
            {
                _backend = WControllerApplication::extractParameter(name);
            }
        }
        else _text = string;
    }

    if (_end < _at) _end = -1;

    if (duration != -1 && _end == -1)
    {
        if (_at == -1)
        {
             _end = duration;
        }
        else _end = _at + duration;
    }

    //---------------------------------------------------------------------------------------------
    // Usage

    if (_text.isEmpty())
    {
        usage();

        return false;
    }

    if (verbosity != QtDebugMsg)
    {
        wControllerFile->setVerbosity(verbosity);
    }

    qDebug("tevo %s", sk->version().C_STR);

    qInfo("text: %s", _text.C_STR);

    qDebug("at: %d", _at);
    qDebug("end: %d", _end);
    qDebug("backend: %s", _backend.C_STR);

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

    _player = new WPlayer(this);

    _player->setBackend(new WBackendTorrent);

    _player->setOutput(WAbstractBackend::OutputAudio);

    if (_end != -1)
    {
        connect(_player, SIGNAL(currentTimeChanged()), this, SLOT(onCurrentTime()));
    }

    connect(_player, SIGNAL(ended()), this, SLOT(quit()));

    return true;
}

//-------------------------------------------------------------------------------------------------
// Functions private
//-------------------------------------------------------------------------------------------------

void ControllerCore::usage() const
{
    qInfo("Usage: tevo <text> [options]\n"
          "\n"
          "tevo --help for more informations");
}

void ControllerCore::help() const
{
    qInfo("tevo %s\n", sk->version().C_STR);

    qInfo("Usage: tevo <text> [options]\n"
          "\n"
          "--help                 Print this text\n"
          "\n"
          "--verbose,             Print debug informations\n"
          "--quiet                Mute application output\n"
          "\n"
          "--at=<time>            Track start time  (00:00:00.000 format)\n"
          "--end=<time>           Track end time    (00:00:00.000 format) (overrides duration)\n"
          "--duration=<time>      Playback duration (00:00:00.000 format)\n"
          "\n"
          "--backend=<string>     Select a backend based on its name in lowercase\n"
          "                       (defaults to duckduckgo)");
}

void ControllerCore::play(const QString & source)
{
    _player->setSource(source);

    _player->seek(_at);

    _player->play();

    if (_end == -1) return;

    _timer.setSingleShot(true);

#ifdef QT_NEW
    _timer.setTimerType(Qt::PreciseTimer);
#endif

    QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(quit()));

    if (_at == -1)
    {
         _timer.start(_end);
    }
    else _timer.start(_end - _at);
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

int ControllerCore::extractMsecs(const QString & text) const
{
    int msecs = Sk::extractMsecs(WControllerApplication::extractParameter(text), -1);

    if (msecs == -1)
    {
        return -1;
    }
    else return qMax(0, msecs);
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

    if (WControllerNetwork::textIsUri(_text) == false)
    {
        QString id;

        if (_backend.isEmpty())
        {
            id = wControllerPlaylist->backendSearchId();

            if (id.isEmpty())
            {
                 quit();

                 return;
            }
        }
        else id = _backend;

        _playlist = new WPlaylist;

        _playlist->setParent(this);

        connect(_playlist, SIGNAL(queryEnded    ()), this, SLOT(onQueryEnded    ()));
        connect(_playlist, SIGNAL(queryCompleted()), this, SLOT(onQueryCompleted()));

        QString source = WControllerPlaylist::createSource(id, "search", "tracks", _text);

        if (_playlist->loadSource(source)) return;

        quit();
    }
    else play(_text);
}

void ControllerCore::onQueryEnded()
{
    if (_playlist->isEmpty()) return;

    disconnect(_playlist, 0, this, 0);

    play(_playlist->trackSource(0));

    _playlist->clearTracks();
}

void ControllerCore::onQueryCompleted()
{
    if (_player->source().isEmpty() == false) return;

    quit();
}

void ControllerCore::onCurrentTime()
{
    int time = _player->currentTime();

    if (time < _end)
    {
        _timer.start(_end - time);
    }
    else quit();
}

//-------------------------------------------------------------------------------------------------

void ControllerCore::quit()
{
    // FIXME libtorrent 2.0.9: For some reason, we need to wait before deleting the session
    //                         otherwise we get a crash.
    Sk::wait(200);

    sk->quit();
}
