/*
 *   Copyright 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
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

// plasma.loadEngine("hardware")
// LineGraph graph
// plasma.connect(graph, "hardware", "cpu");

#include "plasmaapp.h"

#include <unistd.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmapCache>
#include <QTimer>
#include <QtDBus/QtDBus>

#include <KCrash>
#include <KDebug>
#include <KCmdLineArgs>
#include <KWindowSystem>

#include <ksmserver_interface.h>

#include <plasma/corona.h>
#include <plasma/containment.h>

#include "appadaptor.h"
#include "rootwidget.h"
#include "desktopview.h"
#include "panelview.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

Display* dpy = 0;
Colormap colormap = 0;
Visual *visual = 0;

void checkComposite()
{
    dpy = XOpenDisplay(0); // open default display
    if (!dpy) {
        kError() << "Cannot connect to the X server" << endl;
        return;
    }

    int screen = DefaultScreen(dpy);
    int eventBase, errorBase;

    if (XRenderQueryExtension(dpy, &eventBase, &errorBase)) {
        int nvi;
        XVisualInfo templ;
        templ.screen  = screen;
        templ.depth   = 32;
        templ.c_class = TrueColor;
        XVisualInfo *xvi = XGetVisualInfo(dpy,
                                          VisualScreenMask | VisualDepthMask | VisualClassMask,
                                          &templ, &nvi);
        for (int i = 0; i < nvi; ++i) {
            XRenderPictFormat *format = XRenderFindVisualFormat(dpy, xvi[i].visual);
            if (format->type == PictTypeDirect && format->direct.alphaMask) {
                visual = xvi[i].visual;
                colormap = XCreateColormap(dpy, RootWindow(dpy, screen), visual, AllocNone);
                break;
            }
        }
    }

    kDebug() << (colormap ? "Plasma has an argb visual" : "Plasma lacks an argb visual") << visual << colormap;
    kDebug() << ((KWindowSystem::compositingActive() && colormap) ? "Plasma can use COMPOSITE for effects"
                                                                    : "Plasma is COMPOSITE-less") << "on" << dpy;
}

PlasmaApp* PlasmaApp::self()
{
    if (!kapp) {
        checkComposite();
        return new PlasmaApp(dpy, visual ? Qt::HANDLE(visual) : 0, colormap ? Qt::HANDLE(colormap) : 0);
    }

    return qobject_cast<PlasmaApp*>(kapp);
}

PlasmaApp::PlasmaApp(Display* display, Qt::HANDLE visual, Qt::HANDLE colormap)
    : KUniqueApplication(display, visual, colormap),
      m_root(0),
      m_corona(0)
{
    KGlobal::locale()->insertCatalog("libplasma");

    new AppAdaptor(this); 
    QDBusConnection::sessionBus().registerObject("/App", this);
    notifyStartup(false);

    // TODO: this same pattern is in KRunner (see workspace/krunner/restartingapplication.h)
    // would be interesting to see if this could be shared.
    if (!KCrash::crashHandler())
    {
        // this means we've most likely crashed once. so let's see if we
        // stay up for more than 2 minutes time, and if so reset the
        // crash handler since the crash isn't a frequent offender
        QTimer::singleShot(120000, this, SLOT(setCrashHandler()));
    }
    else
    {
        // See if a crash handler was installed. It was if the -nocrashhandler
        // argument was given, but the app eats the kde options so we can't
        // check that directly. If it wasn't, don't install our handler either.
        setCrashHandler();
    }

    // Enlarge application pixmap cache
    
    // Calculate the size required to hold background pixmaps for all screens.
    // Add 10% so that other (smaller) pixmaps can also be cached.
    int cacheSize = 0;
    QDesktopWidget *desktop = QApplication::desktop();
    for (int i = 0; i < desktop->numScreens(); i++) {
        QRect geometry = desktop->screenGeometry(i);
        cacheSize += 4 * geometry.width() * geometry.height() / 1024;
    }
    cacheSize += cacheSize / 10;

    // Calculate the size of physical system memory
    int memorySize = sysconf(_SC_PHYS_PAGES);
    memorySize *= sysconf(_SC_PAGESIZE) / 1024;

    // Increase the pixmap cache size to 1% of system memory if it isn't already
    // larger so as to maximize cache usage. 1% of 1GB ~= 10MB.
    if (cacheSize < memorySize / 100) {
        cacheSize = memorySize / 100;
    }

    kDebug() << "Setting the pixmap cache size to" << cacheSize << "kilobytes";
    QPixmapCache::setCacheLimit(cacheSize);

    m_root = new RootWidget();
    m_root->setAsDesktop(KCmdLineArgs::parsedArgs()->isSet("desktop"));
    m_root->show();

    createPanels();
    notifyStartup(true);

    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
}

PlasmaApp::~PlasmaApp()
{

}

void PlasmaApp::cleanup()
{
    corona()->saveApplets();
    delete m_root;
    m_root = 0;
    qDeleteAll(m_panels);
    m_panels.clear();
    delete corona();
}

void PlasmaApp::initializeWallpaper()
{
    if (!m_root) {
        return;
    }

    //FIXME this has moved to Containment, so .....
    //m_root->desktop()->initializeWallpaper();
}

void PlasmaApp::toggleDashboard()
{
    if (m_root) {
        m_root->toggleDashboard();
    }
}

void PlasmaApp::setCrashHandler()
{
    KCrash::setEmergencySaveFunction(PlasmaApp::crashHandler);
}

void PlasmaApp::crashHandler(int signal)
{
    Q_UNUSED(signal);

    fprintf(stderr, "Plasma crashed, attempting to automatically recover\n");

    sleep(1);
    system("plasma --nocrashhandler &"); // try to restart
}

Plasma::Corona* PlasmaApp::corona()
{
    if (!m_corona) {
        m_corona = new Plasma::Corona(this);
        m_corona->setItemIndexMethod(QGraphicsScene::NoIndex);
        //TODO: Figure out a way to use rubberband and ScrollHandDrag
        m_corona->loadApplets();
    }

    return m_corona;
}

bool PlasmaApp::hasComposite()
{
    return colormap && KWindowSystem::compositingActive();
}

void PlasmaApp::notifyStartup(bool completed)
{
    org::kde::KSMServerInterface ksmserver("org.kde.ksmserver", "/KSMServer", QDBusConnection::sessionBus());

    const QString startupID("workspace desktop");
    if (completed) {
        ksmserver.resumeStartup(startupID);
    } else {
        ksmserver.suspendStartup(startupID);
    }
}

void PlasmaApp::createPanels()
{
    foreach (Plasma::Containment *containment, corona()->containments()) {
        kDebug() << "Containment name:" << containment->name()
                 << "| type" << containment->containmentType()
                 << "| screen:" << containment->screen()
                 << "| geometry:" << containment->geometry()
                 << "| zValue:" << containment->zValue();
        if (containment->containmentType() == Plasma::Containment::PanelContainment) {
            kDebug() << "we have a panel!";
            PanelView *panelView = new PanelView(containment);
            m_panels << panelView;
            panelView->show();
        }
    }
}

#include "plasmaapp.moc"
