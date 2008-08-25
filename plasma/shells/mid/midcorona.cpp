/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "midcorona.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QGraphicsLayout>

#include <KCmdLineArgs>
#include <KDebug>
#include <KDialog>
#include <KGlobalSettings>
#include <KStandardDirs>

#include <plasma/containment.h>
#include <plasma/dataenginemanager.h>

MidCorona::MidCorona(QObject *parent)
    : Plasma::Corona(parent)
{
    init();
}

void MidCorona::init()
{
    QDesktopWidget *desktop = QApplication::desktop();
    QObject::connect(desktop, SIGNAL(resized(int)), this, SLOT(screenResized(int)));
}

void MidCorona::loadDefaultLayout()
{
    QString defaultConfig = KStandardDirs::locate("appdata", "plasma-default-layoutrc");
    if (!defaultConfig.isEmpty()) {
        kDebug() << "attempting to load the default layout from:" << defaultConfig;
        loadLayout(defaultConfig);
        return;
    }

    // used to force a save into the config file
    KConfigGroup invalidConfig;

    // FIXME: need to load the MID-specific containment
    // passing in an empty string will get us whatever the default
    // containment type is!
    Plasma::Containment* c = addContainmentDelayed(QString());

    if (!c) {
        return;
    }

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    bool isDesktop = args->isSet("desktop");

    c->init();

    if (isDesktop) {
        c->setScreen(0);
    } else {
        QString geom = args->getOption("screen");
        int x = geom.indexOf('x');

        if (x > 0)  {
            int width = qMax(400, geom.left(x).toInt());
            int height = qMax(200, geom.right(geom.length() - x - 1).toInt());
            c->resize(width, height);
        }
    }

    c->setWallpaper("image", "SingleImage");
    c->setFormFactor(Plasma::Planar);
    c->updateConstraints(Plasma::StartupCompletedConstraint);
    c->flushPendingConstraintsEvents();
    c->save(invalidConfig);

    emit containmentAdded(c);

    /*
    todo: replace with an applet layout at the top, perhaps reserve a WM strut while we're at it?

    loadDefaultApplet("systemtray", panel);

    foreach (Plasma::Applet* applet, panel->applets()) {
        applet->init();
        applet->flushPendingConstraintsEvents();
        applet->save(invalidConfig);
    }
    */

    requestConfigSync();
    /*
    foreach (Plasma::Containment *c, containments()) {
        kDebug() << "letting the world know about" << (QObject*)c;
        emit containmentAdded(c);
    }
    */
}

Plasma::Applet *MidCorona::loadDefaultApplet(const QString &pluginName, Plasma::Containment *c)
{
    QVariantList args;
    Plasma::Applet *applet = Plasma::Applet::load(pluginName, 0, args);

    if (applet) {
        c->addApplet(applet);
    }

    return applet;
}

void MidCorona::screenResized(int screen)
{
    int numScreens = QApplication::desktop()->numScreens();
    if (screen < numScreens) {
        foreach (Plasma::Containment *c, containments()) {
            if (c->screen() == screen) {
                // trigger a relayout
                c->setScreen(screen);
            }
        }
    }
}

#include "midcorona.moc"

