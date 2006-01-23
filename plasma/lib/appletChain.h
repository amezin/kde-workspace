/*
 *   Copyright (C) 2005 by Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_APPLETCHAIN_H
#define PLASMA_APPLETCHAIN_H

#include <ksharedptr.h>

#include "plasma.h"

namespace Plasma
{

class Applet;

class KDE_EXPORT AppletChain : public QObject, public KShared
{
    Q_OBJECT
    Q_PROPERTY(AppletConstraint constraint READ constraint WRITE setConstraint)
    Q_PROPERTY(ScreenEdge screenEdge READ screenEdge WRITE setScreenEdge)
    Q_PROPERTY(int XineramaScreen READ screenEdge WRITE setXineramaScreen)

    public:
        typedef KSharedPtr<AppletChain> Ptr;

        AppletChain(QObject* parent);
        ~AppletChain();

        Plasma::AppletConstraint constraint();
        void setConstraint(Plasma::AppletConstraint constraint);
        Plasma::Direction popupDirection() const;

        Plasma::ScreenEdge screenEdge();
        void setScreenEdge(Plasma::ScreenEdge edge);

        int xineramaScreen();
        void setXineramaScreen(int screen);

    public Q_SLOTS:
        void loadApplet(KService::Ptr);
        void addApplet(Plasma::Applet*);

    Q_SIGNALS:
        void appletAdded(Applet*);
        void appletRemoved(Applet*);

    private:
        class Private;
        Private* d;
};

} // Plasma namespace

#endif // multiple inclusion guard
