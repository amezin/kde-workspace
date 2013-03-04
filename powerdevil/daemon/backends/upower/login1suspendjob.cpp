/*  This file is part of the KDE project
    Copyright (C) 2006 Kevin Ottens <ervin@kde.org>
    Copyright (C) 2010 Alejandro Fiestas <alex@eyeos.org>
    Copyright (C) 2013 Lukáš Tinkl <ltinkl@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "login1suspendjob.h"

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QTimer>
#include <KDebug>
#include <KLocale>

Login1SuspendJob::Login1SuspendJob(QDBusInterface *login1Interface,
                                   PowerDevil::BackendInterface::SuspendMethod method,
                                   PowerDevil::BackendInterface::SuspendMethods supported)
    : KJob(), m_login1Interface(login1Interface)
{
    kDebug() << "Starting Login1 suspend job";
    m_method = method;
    m_supported = supported;
}

Login1SuspendJob::~Login1SuspendJob()
{

}

void Login1SuspendJob::start()
{
    QTimer::singleShot(0, this, SLOT(doStart()));
}

void Login1SuspendJob::kill(bool /*quietly */)
{

}

void Login1SuspendJob::doStart()
{
    if (m_supported & m_method)
    {
        QVariantList args;
        args << true; // interactive, ie. with polkit dialogs

        QDBusPendingReply<void> reply;
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
        connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher*)), this, SLOT(sendResult(QDBusPendingCallWatcher*)));

        switch(m_method)
        {
        case PowerDevil::BackendInterface::ToRam:
            reply = m_login1Interface->asyncCallWithArgumentList("Suspend", args);
            break;
        case PowerDevil::BackendInterface::ToDisk:
            reply = m_login1Interface->asyncCallWithArgumentList("Hibernate", args);
            break;
        case PowerDevil::BackendInterface::HybridSuspend:
            reply = m_login1Interface->asyncCallWithArgumentList("HybridSleep", args);
            break;
        default:
            kDebug() << "Unsupported suspend method";
            setError(1);
            setErrorText(i18n("Unsupported suspend method"));
            break;
        }
    }
}

void Login1SuspendJob::sendResult(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<void> reply = *watcher;
    if (!reply.isError()) {
        emitResult();
    } else {
        kWarning() << "Failed to start suspend job" << reply.error().name() << reply.error().message();
    }

    watcher->deleteLater();
}


#include "login1suspendjob.moc"