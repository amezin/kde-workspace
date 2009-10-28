/* This file is part of the Nepomuk Project
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

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

#include "nepomuksearchrunner.h"
#include "queryclientwrapper.h"

#include "queryserviceclient.h"

#include <QMenu>

#include <KIcon>
#include <KRun>
#include <KDebug>
#include <KUrl>

#include <Nepomuk/Resource>
#include <Nepomuk/ResourceManager>

#include <Soprano/Vocabulary/NAO>

#include <KFileItemActions>
#include <KFileItemList>
#include <KFileItemListProperties>
#include <KIO/NetAccess>

Q_DECLARE_METATYPE(Nepomuk::Resource)

namespace {
    /**
     * Milliseconds to wait before issuing the next query.
     * This timeout is intended to prevent us from starting
     * a query after each key press by the user. So it should
     * roughly equal the time between key presses of an average user.
     */
    const int s_userActionTimeout = 400;
}


Nepomuk::SearchRunner::SearchRunner( QObject* parent, const QVariantList& args )
    : Plasma::AbstractRunner( parent, args )
{
}


Nepomuk::SearchRunner::SearchRunner( QObject* parent, const QString& serviceId )
    : Plasma::AbstractRunner( parent, serviceId )
{
}


void Nepomuk::SearchRunner::init()
{
    Nepomuk::ResourceManager::instance()->init();

    // we are pretty slow at times and use DBus calls
    setSpeed(SlowSpeed);

    // we are way less important than others, mostly because we are slow
    setPriority(LowPriority);

    m_actions = new KFileItemActions(this);
    addSyntax(Plasma::RunnerSyntax(":q:", i18n("Finds files, documents and other content that matches :q: using the desktop search system.")));
}


Nepomuk::SearchRunner::~SearchRunner()
{
    qDeleteAll(m_konqActions);
}


void Nepomuk::SearchRunner::match( Plasma::RunnerContext& context )
{
    kDebug() << &context << context.query();

    if (Nepomuk::ResourceManager::instance()->initialized()) {
        // This method needs to be thread-safe since KRunner does simply start new threads whenever
        // the query term changes.
        m_mutex.lock();

        // we do not want to restart a query on each key-press. That would result
        // in way too many queries for the rather sluggy Nepomuk query service
        // Thus, we use a little timeout to make sure we do not query too often

        m_waiter.wait(&m_mutex, s_userActionTimeout);
        m_mutex.unlock();

        if (!context.isValid()) {
            kDebug() << "deprecated search:" << context.query();
            // we are no longer the latest call
            return;
        }

        // no queries on very short strings
        if (Search::QueryServiceClient::serviceAvailable() && context.query().count() >= 3) {
            QueryClientWrapper queryWrapper(this, &context);
            queryWrapper.runQuery();
            m_waiter.wakeAll();
        }
    }
}


void Nepomuk::SearchRunner::run( const Plasma::RunnerContext&, const Plasma::QueryMatch& match )
{
    // If no action was selected, the interface doesn't support multiple
    // actions so we simply open the file
    if (QAction *a = match.selectedAction()) {
        if (a != action("open")) {
            match.selectedAction()->trigger();
            return;
        }
    }
    Nepomuk::Resource res = match.data().value<Nepomuk::Resource>();
    KUrl url;

    if (res.hasType( Soprano::Vocabulary::NAO::Tag())) {
        url.setProtocol("nepomuksearch");
        url.setPath(QString("/hasTag:\"%1\"").arg(res.genericLabel()));
    } else {
        url = res.resourceUri();
    }

    (void)new KRun(url, 0);
}

QList<QAction*> Nepomuk::SearchRunner::actionsForMatch(const Plasma::QueryMatch &match)
{
    //Unlike other runners, the actions generated here are likely to see
    //little reuse. Hence, we will clear the actions then generate new
    //ones per iteration to avoid excessive memory consumption.
    qDeleteAll(m_konqActions);
    m_konqActions.clear();

    QList<QAction*> ret;
    if (!action("open")) {
         addAction("open", KIcon("document-open"), i18n("Open"));
    }
    ret << action("open");

    Nepomuk::Resource res = match.data().value<Nepomuk::Resource>();

    KUrl url(res.resourceUri());
    KIO::UDSEntry entry;
    if (!KIO::NetAccess::stat(url.path(), entry, 0)) {
        return QList<QAction*>();
    }

    KFileItemList list;
    list << KFileItem(entry, url);

    KFileItemListProperties prop;
    prop.setItems(list);

    QMenu dummy;
    m_actions->setItemListProperties(prop);
    m_actions->addOpenWithActionsTo(&dummy, QString());
    //Add user defined actions
    m_actions->addServiceActionsTo(&dummy);

    m_konqActions = Plasma::actionsFromMenu(&dummy);

    ret << m_konqActions;

    return ret;
}

K_EXPORT_PLASMA_RUNNER(nepomuksearchrunner, Nepomuk::SearchRunner)

#include "nepomuksearchrunner.moc"
