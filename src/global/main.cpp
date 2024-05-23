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

// Sk includes
#include <WCoreApplication>

// Application includes
#include <ControllerCore.h>

//-------------------------------------------------------------------------------------------------
// Static variables

static const QString VERSION = "1.0.0-0";

//-------------------------------------------------------------------------------------------------
// Functions
//-------------------------------------------------------------------------------------------------

int main(int argc, char * argv[])
{
    QCoreApplication * application = WCoreApplication::create(argc, argv);

    if (application == NULL) return 0;

    W_CREATE_CONTROLLER(ControllerCore);

    if (core->run(argc, argv))
    {
        return application->exec();
    }
    else return 1;
}
