/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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


#include "plasma-shell-desktop.h"


#include <QPixmap>
#include <QString>
#include <QSize>
#include <QFile>

#include <KIcon>
#include <KWindowSystem>
#include <kephal/screens.h>

#include <Plasma/Containment>
#include <Plasma/Corona>

#include "plasmaapp.h"

#include "activity.h"

Activity::Activity(const QString &id, QObject *parent)
    :QObject(parent),
    m_id(id)
{
    m_name = id; //TODO get it from libactivities

    Plasma::Corona *corona = PlasmaApp::self()->corona();

    //find your containments
    foreach (Plasma::Containment *cont, corona->containments()) {
        if ((cont->containmentType() == Plasma::Containment::DesktopContainment ||
            cont->containmentType() == Plasma::Containment::CustomContainment) &&
                !corona->offscreenWidgets().contains(cont) && cont->activity() == id) {
            m_containments << cont;
            break;
        }
    }
    kDebug() << m_containments.size();

}

Activity::~Activity()
{
}


QString Activity::id()
{
    return m_id;
}

QString Activity::name()
{
    return m_name;
}

QPixmap Activity::thumbnail(const QSize &size)
{
    //TODO
    return KIcon("plasma").pixmap(size);
}

bool Activity::isActive()
{
    //TODO
    return false;
}

bool Activity::isRunning()
{
    return ! m_containments.isEmpty();
}

void Activity::destroy()
{
    //TODO
    //-kill the activity in nepomuk
    //-destroy all our containments
    if (m_containments.isEmpty()) {
        return;
    }
    m_containments.first()->destroy();
}

void Activity::activate()
{
    if (m_containments.isEmpty()) {
        open();
        if (m_containments.isEmpty()) {
            kDebug() << "open failed??";
            return;
        }
    }
    //FIXME also ensure there's a containment for every screen. it's possible numscreens changed
    //since we were opened.

    //figure out where we are
    int currentScreen = Kephal::ScreenUtils::screenId(QCursor::pos());
    int currentDesktop = -1;
    if (AppSettings::perVirtualDesktopViews()) {
        currentDesktop = KWindowSystem::currentDesktop()-1;
    }
    //and bring the containment to where we are
    m_containments.first()->setScreen(currentScreen, currentDesktop);
    //TODO handle other screens

    //TODO libactivities stuff
}

void Activity::setName(const QString &name)
{
    if (m_name == name) {
        return;
    }
    m_name = name;
    emit nameChanged(name);
    //TODO libactivities stuff
    //propogate change to ctmts
}

void Activity::close()
{
    //FIXME activity id isn't guaranteed unique
    //until we start using real activities
    QString name = "activities/";
    name += m_id;
    KConfig external(name, KConfig::SimpleConfig, "appdata");
    int i=0;
    foreach (Plasma::Containment *c, m_containments) {
        //FIXME if it's not active, the screen # is -1
        //so I'm going by position in the list
        //which is a horrible hack
        QString groupName = QString("Containment%1").arg(i++);
        KConfigGroup newConf(&external, groupName);
        c->config().copyTo(&newConf);
        c->destroy(false);
    }
    m_containments.clear();
    kDebug() << "attempting to write to" << external.name();
    external.sync();
    //FIXME only destroy it if nothing went wrong

    //TODO save a thumbnail to a file too
}

void Activity::open()
{
    QString fileName = "activities/";
    fileName += m_id;
    KConfig external(fileName, KConfig::SimpleConfig, "appdata");

    //TODO iterate over all screens (kephal tells us how many exist, right?)
    //unless we choose to be lazy and not load other screens until we're activated.
    KConfigGroup config(&external, "Containment0");
    QString plugin = config.readEntry("plugin", QString());
    Plasma::Containment *newContainment = PlasmaApp::self()->corona()->addContainment(plugin, QVariantList());
    // load the configuration of the old containment into the new one
    // luckily some other part of libplasma seems to ensure the applet ID's are unique
    KConfigGroup newCfg = newContainment->config();
    config.copyTo(&newCfg);
    newContainment->restore(newCfg);
    foreach (Plasma::Applet *applet, newContainment->applets()) {
        applet->init();
        // We have to flush the applet constraints manually
        applet->flushPendingConstraintsEvents();
    }
    newContainment->save(newCfg);
    m_containments << newContainment;
    config.deleteGroup();

    PlasmaApp::self()->corona()->requireConfigSync();
    external.sync();
}


// vim: sw=4 sts=4 et tw=100
