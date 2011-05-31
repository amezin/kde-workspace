/*
 * Copyright 2011 Sebastian Kügler <sebas@kde.org>
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

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingReply>

#include <KAuthorized>

// kde-workspace/libs
#include <kworkspace/kworkspace.h>

#include "powermanagementjob.h"

#include <kdebug.h>

PowerManagementJob::PowerManagementJob(const QString &operation, QMap<QString, QVariant> &parameters, QObject *parent) :
    ServiceJob(parent->objectName(), operation, parameters, parent)
{
}

PowerManagementJob::~PowerManagementJob()
{
}

void PowerManagementJob::start()
{
    const QString operation = operationName();
    //kDebug() << "starting operation  ... " << operation;

    if (operation == "lockScreen") {
        if (KAuthorized::authorizeKAction("lock_screen")) {
            const QString interface("org.freedesktop.ScreenSaver");
            QDBusInterface screensaver(interface, "/ScreenSaver");
            screensaver.asyncCall("Lock");
            setResult(true);
            return;
        }
        kDebug() << "operation denied " << operation;
        setResult(false);
        return;
    } else if (operation == "suspend" || operation == "suspendToRam") {
        setResult(suspend(Ram));
        return;
    } else if (operation == "suspendToDisk") {
        setResult(suspend(Disk));
        return;
    } else if (operation == "suspendHybrid") {
        setResult(suspend(Hybrid));
        return;
    } else if (operation == "requestShutDown") {
        requestShutDown();
        setResult(true);
        return;
    } else if (operation == "setBrightness") {
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.Solid.PowerManagement")) {
            QDBusMessage call = QDBusMessage::createMethodCall ("org.kde.Solid.PowerManagement",
                                                                "/org/kde/Solid/PowerManagement",
                                                                "org.kde.Solid.PowerManagement",
                                                                "setBrightness");
            int brightness = parameters().value("brightness").toInt();
            call.setArguments(QList<QVariant>() << QVariant::fromValue(brightness));
            QDBusConnection::sessionBus().asyncCall(call);
            setResult(true);
        } else {
            kDebug() << "DBus org.kde.Solid.PowerMangement not available.";
            setResult(false);
        }
    }
    else if (operation == "changeProfile") {
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.Solid.PowerManagement")) {
            QDBusMessage call = QDBusMessage::createMethodCall ("org.kde.Solid.PowerManagement",
                                                                "/org/kde/Solid/PowerManagement",
                                                                "org.kde.Solid.PowerManagement",
                                                                "loadProfile");

            Plasma::DataEngine::Data data = m_engine->query("PowerDevil");
            StringStringMap availableProfiles = data["Available profiles"].value< StringStringMap >();
            QString profile = availableProfiles.key(parameters().value("profile").toString());

            call.setArguments(QList<QVariant>() << QVariant::fromValue(profile));
            QDBusConnection::sessionBus().asyncCall (call);
        } else {
            kDebug() << "DBus org.kde.Solid.PowerMangement not available.";
        }
    }

    kDebug() << "don't know what to do with " << operation;
    setResult(false);
}

bool PowerManagementJob::suspend(const SuspendType &type)
{
    QDBusMessage msg = QDBusMessage::createMethodCall("org.kde.Solid.PowerManagement",
                                                      "/org/kde/Solid/PowerManagement",
                                                      "org.kde.Solid.PowerManagement",
                                                      callForType(type));
    QDBusConnection::sessionBus().asyncCall(msg);
    return true;
}

QString PowerManagementJob::callForType(const SuspendType &type)
{
    switch (type) {
        case Disk:
            return "suspendToDisk";
        break;

        case Hybrid:
            return "suspendHybrid";
        break;

        default:
            return "suspendToRam";
        break;
    }
}

void PowerManagementJob::requestShutDown()
{
    KWorkSpace::requestShutDown();
}

#include "powermanagementjob.moc"