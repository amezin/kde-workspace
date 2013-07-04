/*
 * Copyright 2013 Heena <heena393@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License version 2 as
 * published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "taskwindowjob.h"
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingReply>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QtCore/QFutureWatcher>
#include <QtCore/QSettings>
#include <QtCore/QtConcurrentRun>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeView>
#include <QtDeclarative/qdeclarative.h>
#include <QtGui/QMenu>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValue>


#include <KAuthorized>

TaskWindowJob::TaskWindowJob(TaskSource *source, const QString &operation , QMap<QString, QVariant> &parameters, QObject *parent) :
    ServiceJob(source->objectName(), operation, parameters, parent),
    m_source(source)
{
}

TaskWindowJob::~TaskWindowJob()
{
}

void TaskWindowJob::start()
{
   /* if (!m_source->task()) {
        return;
    }*/
     
    // only a subset of task operations are exported
    const QString operation = operationName();
    if (operation == "cascade") {
        QDBusInterface  *kwinInterface = new QDBusInterface("org.kde.kwin", "/KWin", "org.kde.KWin");
        QDBusPendingCall pcall = kwinInterface->asyncCall("cascadeDesktop");
        kDebug() << "couldn't connect to kwin! ";
        setResult(true);
        return;
    } else if (operation == "unclutter") {
        QDBusInterface  *kwinInterface = new QDBusInterface("org.kde.kwin", "/KWin", "org.kde.KWin");
        QDBusPendingCall pcall = kwinInterface->asyncCall("unclutterDesktop");
        kDebug() << "couldn't connect to kwin! ";
        setResult(true);
        return;
    } 
     
    //setResult(false);
}

#include "taskwindowjob.moc"