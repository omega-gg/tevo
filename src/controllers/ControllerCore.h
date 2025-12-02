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

#ifndef CONTROLLERCORE_H
#define CONTROLLERCORE_H

// Qt includes
#include <QTimer>

// Sk includes
#include <WController>
#include <WAbstractBackend>

// Defines
#define core ControllerCore::instance()

// Forward declarations
class WControllerFileReply;
class WBackendIndex;
class WPlayer;
class WHookOutput;
class WPlaylist;

class ControllerCore : public WController
{
    Q_OBJECT

private:
    ControllerCore();

public: // Interface
    Q_INVOKABLE int run(int & argc, char ** argv);

    Q_INVOKABLE void quit();

private: // Functions
    void usage() const;
    void help () const;

    void play(const QString & source);

    void loadTrack();

    void createIndex();

    WControllerFileReply * copyBackends() const;

    void createPlaylist();

    int extractMsecs(const QString & text) const;

    QString getText(const QString & text) const;

private: // Static functions
    static void onInterrupt(int id);

private slots:
    void onLoaded();

    void onIndexLoaded ();
    void onIndexUpdated();

    void onQueryEnded    ();
    void onQueryCompleted();

    void onTrackEnded    ();
    void onTrackCompleted();

    void onStateLoadChanged();

    void onCurrentTime();

    void onConnected();

private: // Variables
    QString _path;

    WBackendIndex * _index;

    WPlayer * _player;

    WHookOutput * _hook;

    WPlaylist * _playlist;

    QString _text;

    int _at;
    int _end;

    QString _backend;

    QString _screen;

    WAbstractBackend::Output  _output;
    WAbstractBackend::Quality _quality;

    QTimer _timer;

private:
    Q_DISABLE_COPY      (ControllerCore)
    W_DECLARE_CONTROLLER(ControllerCore)
};

#endif // CONTROLLERCORE_H
