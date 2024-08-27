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

// C++ includes
#include <signal.h>

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
#include <WCache>
#include <WLoaderVbml>
#include <WLoaderTorrent>
#include <WBackendIndex>
#ifndef SK_NO_TORRENT
#include <WBackendTorrent>
#endif
#include <WPlayer>
#include <WHookOutput>

W_INIT_CONTROLLER(ControllerCore)

//-------------------------------------------------------------------------------------------------
// Static variables

static const QString CORE_VERSION = "1.0.0-0";

static const int CORE_CACHE = 1048576 * 100; // 100 megabytes

// NOTE: Defaut streaming port for tevo.
static const int CORE_PORT = 8400;

#ifndef SK_DEPLOY
#ifdef Q_OS_MACX
static const QString PATH_STORAGE = "/../../../storage";
static const QString PATH_BACKEND = "../../../../../backend";
#else
static const QString PATH_STORAGE = "/storage";
static const QString PATH_BACKEND = "../../backend";
#endif
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

    wControllerFile->setPathStorage(_path);
}

//-------------------------------------------------------------------------------------------------
// Interface
//-------------------------------------------------------------------------------------------------

/* Q_INVOKABLE */ bool ControllerCore::run(int & argc, char ** argv)
{
    _playlist = NULL;

    _at  = -1;
    _end = -1;

    _output = WAbstractBackend::OutputMedia;

    _quality = WAbstractBackend::Quality720;

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
#ifdef QT_4
                verbosity = static_cast<QtMsgType> (QtSystemMsg + 1);
#else
                verbosity = static_cast<QtMsgType> (QtInfoMsg + 1);
#endif
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
            else if (name.startsWith("screen="))
            {
                name = WControllerApplication::extractParameter(name);

                _screen = WControllerPlaylist::vbmlUriFromCode(name);
            }
            else if (name.startsWith("output="))
            {
                name = WControllerApplication::extractParameter(name);

                _output = WAbstractBackend::outputFromString(name);
            }
            else if (name.startsWith("quality="))
            {
                name = WControllerApplication::extractParameter(name);

                _quality = WAbstractBackend::qualityFromString(name);
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

    if (verbosity == QtDebugMsg)
    {
        qDebug("tevo %s", sk->version().C_STR);

        qDebug("Path storage: %s", _path.C_STR);
        qDebug("Path log:     %s", wControllerFile->pathLog().C_STR);

        qDebug("text: %s", _text.C_STR);

        qDebug("at : %d", _at);
        qDebug("end: %d", _end);

        qDebug("backend: %s", getText(_backend).C_STR);

        qDebug("screen : %s", getText(_screen)                                    .C_STR);
        qDebug("output : %s", getText(WAbstractBackend::outputToString (_output)) .C_STR);
        qDebug("quality: %s", getText(WAbstractBackend::qualityToString(_quality)).C_STR);
    }
    else wControllerFile->setVerbosity(verbosity);

    //---------------------------------------------------------------------------------------------
    // Controllers

    W_CREATE_CONTROLLER(WControllerPlaylist);
    W_CREATE_CONTROLLER(WControllerMedia);

#ifndef SK_NO_TORRENT
    W_CREATE_CONTROLLER_2(WControllerTorrent, _path + "/torrents", CORE_PORT);
#endif

    //---------------------------------------------------------------------------------------------
    // Cache

    WCache * cache = new WCache(_path + "/cache", CORE_CACHE);

    wControllerFile->setCache(cache);

    //---------------------------------------------------------------------------------------------
    // LoaderVbml

    wControllerPlaylist->registerLoader(WBackendNetQuery::TypeVbml, new WLoaderVbml(this));

#ifndef SK_NO_TORRENT
    //---------------------------------------------------------------------------------------------
    // LoaderTorrent

    WLoaderTorrent * loaderTorrent = new WLoaderTorrent(this);

    wControllerPlaylist->registerLoader(WBackendNetQuery::TypeTorrent, loaderTorrent);
    wControllerTorrent ->registerLoader(WBackendNetQuery::TypeTorrent, loaderTorrent);
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

#ifdef SK_NO_TORRENT
    WBackendManager * backend = new WBackendManager;
#else
    WBackendTorrent * backend = new WBackendTorrent;
#endif

    _player->setBackend(backend);

    return true;
}

void ControllerCore::quit()
{
    disconnect(_player, 0, this, 0);

    if (_playlist) disconnect(_playlist, 0, this, 0);

    if (_player && _player->isPlaying())
    {
        qInfo("Stopped at %s",
              WControllerPlaylist::getPlayerTime(_player->currentTime()).C_STR);
    }

    // FIXME libtorrent 2.0.9: For some reason, we need to wait before deleting the session
    //                         otherwise we get a crash.
    Sk::wait(200);

    sk->quit();
}

//-------------------------------------------------------------------------------------------------
// Functions private
//-------------------------------------------------------------------------------------------------

void ControllerCore::usage() const
{
    // NOTE: Create the storage folder to save the logs.
    QDir().mkpath(_path);

    qInfo("Usage: tevo <text> [options]\n"
          "\n"
          "tevo --help for more informations");
}

void ControllerCore::help() const
{
    // NOTE: Create the storage folder to save the logs.
    QDir().mkpath(_path);

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
          "--backend=<string>     Set a backend based on its name in lowercase\n"
          "                       (defaults to duckduckgo)\n"
          "\n"
          "--screen=<string>      Set a tevolution screen based on its magic number\n"
          "                       (xxx-xxx-xxx-xxx)\n"
          "--output=<string>      Set the output type (media, audio, video)\n"
          "                       (defaults to media)\n"
          "--quality=<string>     Set the quality (144, 240, 360, 480, 720, 1080, 1440, 2160)\n"
          "                       (defaults to 720)");
}

void ControllerCore::play(const QString & source)
{
    if (_screen.isEmpty())
    {
        _player->setOutput(WAbstractBackend::OutputAudio);

        _player->setQuality(_quality);

        _player->setSource(source);

        _player->seek(_at);

        _player->play();
    }
    else
    {
        QList<WAbstractHook *> list;

        _hook = new WHookOutput(_player->backend());

        connect(_hook, SIGNAL(connectedChanged()), this, SLOT(onConnected()));

        list.append(_hook);

        _player->setHooks(list);

        _hook->connectToHost(_screen);

        _player->setOutput(_output);

        _player->setQuality(_quality);

        _player->setSource(source);

        _player->seek(_at);
    }

    connect(_player, SIGNAL(ended()), this, SLOT(quit()));

    connect(_player, SIGNAL(stateLoadChanged()), this, SLOT(onStateLoadChanged()));

    signal(SIGINT, onInterrupt);

    if (_end == -1) return;

    connect(_player, SIGNAL(currentTimeChanged()), this, SLOT(onCurrentTime()));

    _timer.setSingleShot(true);

#ifdef QT_NEW
    _timer.setTimerType(Qt::PreciseTimer);
#endif

    QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(quit()));
}

void ControllerCore::loadTrack()
{
    createPlaylist();

    connect(_playlist, SIGNAL(trackQueryEnded    ()), this, SLOT(onTrackEnded    ()));
    connect(_playlist, SIGNAL(trackQueryCompleted()), this, SLOT(onTrackCompleted()));

    _playlist->addSource(_player->source(), true);
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

void ControllerCore::createPlaylist()
{
    if (_playlist) return;

    _playlist = new WPlaylist;

    _playlist->setParent(this);
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

QString ControllerCore::getText(const QString & text) const
{
    if (text.isEmpty())
    {
        return "default";
    }
    else return text;
}

//-------------------------------------------------------------------------------------------------
// Private static functions
//-------------------------------------------------------------------------------------------------

/* static */ void ControllerCore::onInterrupt(int)
{
    core->quit();
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

    QString id;

    QString query;

    if (_backend.isEmpty())
    {
        id = wControllerPlaylist->backendIdFromText(_text);

        if (id.isEmpty())
        {
            if (WControllerNetwork::textIsUri(_text))
            {
                play(_text);

                return;
            }

            id = wControllerPlaylist->backendSearchId();

            if (id.isEmpty())
            {
                quit();

                return;
            }

            query = _text.trimmed();
        }
        else query = _text.mid(id.length() + 1).trimmed();
    }
    else
    {
        id = _backend;

        query = _text.trimmed();
    }

    createPlaylist();

    connect(_playlist, SIGNAL(queryEnded    ()), this, SLOT(onQueryEnded    ()));
    connect(_playlist, SIGNAL(queryCompleted()), this, SLOT(onQueryCompleted()));

    QString source = WControllerPlaylist::createSource(id, "search", "tracks", query);

    if (_playlist->loadSource(source)) return;

    quit();
}

void ControllerCore::onQueryEnded()
{
    if (_playlist->isEmpty()) return;

    disconnect(_playlist, 0, this, 0);

    QString source = _playlist->trackSource(0);

    _playlist->clearTracks();

    play(source);
}

void ControllerCore::onQueryCompleted()
{
    if (_player->source().isEmpty() == false) return;

    quit();
}

void ControllerCore::onTrackEnded()
{
    const WTrack * track = _playlist->trackPointerAt(0);

    QString title = track->title();

    if (title.isEmpty()) return;

    qInfo("%s", title.C_STR);

    onTrackCompleted();
}

void ControllerCore::onTrackCompleted()
{
    disconnect(_playlist, 0, this, 0);

    qInfo("%s", _player->source().C_STR);

    _playlist->clearTracks();
}

void ControllerCore::onStateLoadChanged()
{
    if (_player->isDefault() == false) return;

    disconnect(_player, SIGNAL(stateLoadChanged()), this, SLOT(onStateLoadChanged()));

    loadTrack();
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

void ControllerCore::onConnected()
{
    if (_hook->isConnected())
    {
        _player->play();
    }
    else quit();
}
