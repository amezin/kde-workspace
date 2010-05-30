/***************************************************************************
 *   Copyright (C) 2008 by Marco Martin <notmart@gmail.com>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "activitybar.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>

#include <KWindowSystem>
#include <KIcon>

#include <Plasma/View>
#include <Plasma/Corona>
#include <Plasma/Context>
#include <Plasma/Containment>
#include <Plasma/TabBar>

#include <kactivitycontroller.h>
#include <kactivityinfo.h>


ActivityBar::ActivityBar(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_activeContainment(-1),
      m_activityController(0)
{
    resize(200, 60);
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding));
}


ActivityBar::~ActivityBar()
{
}

void ActivityBar::init()
{
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(this);
    m_tabBar = new Plasma::TabBar(this);
    layout->addItem(m_tabBar);
    layout->setContentsMargins(0,0,0,0);
    //layout->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding));

    //This is an awful hack, but I need to keep the old behaviour for plasma-netbook
    //while using the new activity API for plasma-desktop.
    //TODO 4.6 convert netbook to the activity API so we won't need this
    if (qApp->applicationName() == "plasma-desktop") {
        m_activityController = new KActivityController(this);
        QStringList activities = m_activityController->availableActivities();
        foreach (const QString &id, activities) {
            insertActivity(id);
        }
        m_tabBar->setCurrentIndex(activities.indexOf(m_activityController->currentActivity()));

        connect(m_activityController, SIGNAL(activityAdded(QString)), this, SLOT(activityAdded(QString)));
        connect(m_activityController, SIGNAL(activityRemoved(QString)), this, SLOT(activityRemoved(QString)));
        connect(m_activityController, SIGNAL(currentActivityChanged(QString)), this, SLOT(currentActivityChanged(QString)));

        connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(switchActivity(int)));
    } else {
        m_tabBar->nativeWidget()->installEventFilter(this);
        if (containment()) {
            Plasma::Corona *c = containment()->corona();

            if (!c) {
                kDebug() << "No corona, can't happen";
                setFailedToLaunch(true);
                return;
            }

            QList<Plasma::Containment*> containments = c->containments();
            foreach (Plasma::Containment *cont, containments) {
                if (cont->containmentType() == Plasma::Containment::PanelContainment || cont->containmentType() == Plasma::Containment::CustomPanelContainment || c->offscreenWidgets().contains(cont)) {
                    continue;
                }

                insertContainment(cont);

                connect(cont, SIGNAL(destroyed(QObject *)), this, SLOT(containmentDestroyed(QObject *)));
                connect(cont, SIGNAL(screenChanged(int, int, Plasma::Containment *)), this, SLOT(screenChanged(int, int, Plasma::Containment *)));
                connect(cont, SIGNAL(contextChanged(Plasma::Context *)), this, SLOT(contextChanged(Plasma::Context *)));
            }

            connect(c, SIGNAL(containmentAdded(Plasma::Containment *)), this, SLOT(containmentAdded(Plasma::Containment *)));
        }

        connect(m_tabBar, SIGNAL(currentChanged(int)), this, SLOT(switchContainment(int)));

        connect(KWindowSystem::self(), SIGNAL(currentDesktopChanged(int)), this, SLOT(currentDesktopChanged(int)));
    }

    setPreferredSize(m_tabBar->nativeWidget()->sizeHint());
    emit sizeHintChanged(Qt::PreferredSize);
}

void ActivityBar::insertContainment(Plasma::Containment *cont)
{
     QList<Plasma::Containment *>::iterator i = m_containments.begin();
     int index = 0;
     int myScreen = containment()->screen();

     for (; i != m_containments.end(); ++i) {
          if (cont->id() < (*i)->id()) {
              m_containments.insert(i, cont);
              break;
          }
          ++index;
     }
     if (i == m_containments.end()) {
         m_containments.append(cont);
     }

     if (cont->activity().isNull()) {
        m_tabBar->insertTab(index, cont->name());
     } else {
        m_tabBar->insertTab(index, cont->activity());
     }

     QString iconName = cont->icon();

     if (!iconName.isEmpty() && iconName != "user-desktop") {
         m_tabBar->nativeWidget()->setTabIcon(index, KIcon(iconName));
     }

     if (cont->screen() != -1 &&
         cont->screen() == myScreen &&
         (cont->desktop() == -1 || cont->desktop() ==  KWindowSystem::currentDesktop()-1)) {
         m_activeContainment = index;
         m_tabBar->setCurrentIndex(m_activeContainment);
     }
}

void ActivityBar::insertActivity(const QString &id)
{
    //assumption: activities are always added at the end of the list
    KActivityInfo *activity = new KActivityInfo(id, this);
    m_activities.append(activity);
    if (activity->isValid() && !activity->icon().isEmpty()) {
        m_tabBar->addTab(KIcon(activity->icon()), activity->name());
    } else {
        m_tabBar->addTab(activity->name());
    }

    connect(activity, SIGNAL(nameChanged(QString)), this, SLOT(activityNameChanged(QString)));
}

void ActivityBar::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint ) {
        if (formFactor() == Plasma::Vertical) {
            m_tabBar->nativeWidget()->setShape(QTabBar::RoundedWest);
        } else {
            m_tabBar->nativeWidget()->setShape(QTabBar::RoundedNorth);
        }

        m_tabBar->nativeWidget()->setDrawBase(formFactor() != Plasma::Vertical && formFactor() != Plasma::Horizontal);

        setPreferredSize(m_tabBar->nativeWidget()->sizeHint());
        setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
        emit sizeHintChanged(Qt::PreferredSize);
    }
}

bool ActivityBar::eventFilter(QObject *watched, QEvent *event)
{
    //request an activate also if the user clicks on the current active tab
    if (watched == m_tabBar->nativeWidget() && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *me = static_cast<QMouseEvent *>(event);
        switchContainment(m_tabBar->nativeWidget()->tabAt(me->pos()));
    }
    return false;
}

void ActivityBar::switchContainment(int newActive)
{
    Plasma::Containment *ownCont = containment();
    if (!ownCont || newActive > m_containments.count()-1 || newActive < 0) {
        return;
    }
    Plasma::Corona *c = containment()->corona();
    if (!c) {
        return;
    }


    Plasma::Containment *oldCont = c->containmentForScreen(ownCont->screen(), KWindowSystem::currentDesktop() - 1);

    m_activeContainment = newActive;
    if (oldCont) {
        m_containments[newActive]->setScreen(oldCont->screen(), oldCont->desktop());
    } else {
        m_containments[newActive]->setScreen(ownCont->screen(), ownCont->desktop());
    }
}

void ActivityBar::switchActivity(int newActive)
{
    if (newActive >= m_activities.count() || newActive < 0) {
        return;
    }
    m_activityController->setCurrentActivity(m_activities.at(newActive)->id());
}

void ActivityBar::currentDesktopChanged(const int currentDesktop)
{
    Plasma::Corona *c = containment()->corona();
    if (!c) {
        return;
    }

    //-1 because kwindowsystem counts desktop from 1 :)
    Plasma::Containment *cont = c->containmentForScreen(containment()->screen(), currentDesktop - 1);

    if (!cont) {
        return;
    }

    int index = m_containments.indexOf(cont);

    if (index != -1 &&
        index != m_activeContainment) {
        m_activeContainment = index;
        m_tabBar->setCurrentIndex(index);
    }
}

void ActivityBar::currentActivityChanged(const QString &newId)
{
    int index = 0;
    bool found = false;
    foreach (KActivityInfo *info, m_activities) {
        if (info->id() == newId) {
            found = true;
            break;
        }
        ++index;
    }

    if (found) {
        m_tabBar->setCurrentIndex(index);
    }
}

void ActivityBar::containmentAdded(Plasma::Containment *cont)
{
    if (cont->containmentType() == Plasma::Containment::PanelContainment ||
        cont->containmentType() == Plasma::Containment::CustomPanelContainment ||
        m_containments.contains(cont) || (containment() && containment()->corona()->offscreenWidgets().contains(cont))) {
        return;
    }

    insertContainment(cont);

    connect(cont, SIGNAL(destroyed(QObject *)), this, SLOT(containmentDestroyed(QObject *)));
    connect(cont, SIGNAL(screenChanged(int, int, Plasma::Containment *)), this, SLOT(screenChanged(int, int, Plasma::Containment *)));
    connect(cont, SIGNAL(contextChanged(Plasma::Context *)), this, SLOT(contextChanged(Plasma::Context *)));

    setPreferredSize(m_tabBar->nativeWidget()->sizeHint());
    emit sizeHintChanged(Qt::PreferredSize);
}

void ActivityBar::activityAdded(const QString &id)
{
    insertActivity(id);

    setPreferredSize(m_tabBar->nativeWidget()->sizeHint());
    emit sizeHintChanged(Qt::PreferredSize);
}

void ActivityBar::containmentDestroyed(QObject *obj)
{
    Plasma::Containment *containment = static_cast<Plasma::Containment *>(obj);

    int index = m_containments.indexOf(containment);
    if (index != -1) {
        if (index < m_activeContainment) {
            --m_activeContainment;
        }

        m_containments.removeAt(index);
        m_tabBar->blockSignals(true);
        m_tabBar->removeTab(index);
        m_tabBar->blockSignals(false);
    }

    setPreferredSize(m_tabBar->nativeWidget()->sizeHint());
    emit sizeHintChanged(Qt::PreferredSize);
}

void ActivityBar::activityRemoved(const QString &id)
{
    int index = 0;
    bool found = false;
    foreach (KActivityInfo *info, m_activities) {
        if (info->id() == id) {
            found = true;
            m_activities.removeAt(index);
            delete info;
            break;
        }
        ++index;
    }
    if (!found) {
        return;
    }

    m_tabBar->blockSignals(true);
    m_tabBar->removeTab(index);
    m_tabBar->blockSignals(false);

    setPreferredSize(m_tabBar->nativeWidget()->sizeHint());
    emit sizeHintChanged(Qt::PreferredSize);
}

void ActivityBar::screenChanged(int wasScreen, int isScreen, Plasma::Containment *cont)
{
    Q_UNUSED(wasScreen)
    //Q_UNUSED(isScreen)

    int index = m_containments.indexOf(cont);

    //FIXME: how is supposed to work containment()->desktop() when the pervirtialthing is off?
    if (index != -1 &&
        index != m_activeContainment &&
        containment()->screen() == isScreen &&
        (cont->desktop() == -1 || cont->desktop() == KWindowSystem::currentDesktop()-1)) {
        m_activeContainment = index;
        m_tabBar->setCurrentIndex(index);
    }
}

void ActivityBar::contextChanged(Plasma::Context *context)
{
    Plasma::Containment *cont = qobject_cast<Plasma::Containment *>(sender());

    if (!cont) {
        return;
    }

    int index = m_containments.indexOf(cont);
    if (index != -1) {
        m_tabBar->setTabText(index, context->currentActivity());
    }
}

void ActivityBar::activityNameChanged(const QString &newName)
{
    KActivityInfo *info = qobject_cast<KActivityInfo*>(sender());
    if (!info) {
        return;
    }
    int index = m_activities.indexOf(info);
    if (index != -1) {
        m_tabBar->setTabText(index, newName);
    }
}

#include "activitybar.moc"
