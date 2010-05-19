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

#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <QObject>
#include <QHash>

class QSize;
class QString;
class QPixmap;
class KActivityInfo;
class KConfig;
namespace Plasma
{
    class Containment;
} // namespace Plasma

/**
 * This class represents one activity.
 * an activity has an ID and a name, from nepomuk.
 * it also is associated with one or more containments.
 */
class Activity : public QObject
{
    Q_OBJECT
public:
    Activity(const QString &id, QObject *parent = 0);
    ~Activity();
    //FIXME what's the Right Way to set up the initial data?

    QString id();
    QString name();
    QPixmap pixmap(const QSize &size); //FIXME do we want diff. sizes? updates?

    /**
     * whether this is the currently active activity
     */
    bool isActive();
    /**
     * whether this is one of the activities currently loaded
     */
    bool isRunning();

    /**
     * save (copy) the activity out to an @p external config
     */
    void save(KConfig &external);

    /**
     * return the containment that belongs on @p screen and @p desktop
     * or null if none exists
     */
     Plasma::Containment* containmentForScreen(int screen, int desktop = -1);

    /**
     * make this activity's containments the active ones, loading them if necessary
     */
    void ensureActive();

signals:
    void nameChanged(const QString &name);
    void opened();
    void closed();
//TODO signals for other changes

public slots:
    void setName(const QString &name);
    /**
     * delete the activity forever
     */
    void destroy();
    /**
     * make this activity the current activity
     */
    void activate();

    /**
     * save and remove all our containments
     */
    void close();

    /**
     * load the saved containment(s) for this activity
     */
    void open();

private:
    void activateContainment(int screen, int desktop);
    void insertContainment(Plasma::Containment* cont);

    QString m_id;
    QString m_name;
    QHash<QPair<int,int>, Plasma::Containment*> m_containments;
    KActivityInfo *m_info;

};

#endif
