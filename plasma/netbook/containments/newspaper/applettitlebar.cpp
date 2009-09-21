/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "applettitlebar.h"
#include "newspaper.h"

#include <QGraphicsGridLayout>
#include <QGraphicsSceneMouseEvent>
#include <QLabel>
#include <QTimer>

#include <KIconLoader>

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Separator>
#include <Plasma/Theme>


AppletTitleBar::AppletTitleBar(Plasma::Applet *applet)
       : QGraphicsWidget(applet),
         m_applet(applet)
{
    QGraphicsGridLayout *lay = new QGraphicsGridLayout(this);

    int column = 0;
    if (applet->hasValidAssociatedApplication()) {
        Plasma::IconWidget *maximizeButton = new Plasma::IconWidget(this);
        maximizeButton->setMinimumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
        maximizeButton->setMaximumSize(maximizeButton->minimumSize());
        maximizeButton->setSvg("widgets/configuration-icons", "maximize");
        lay->addItem(maximizeButton, 0, column);
        ++column;
        connect(maximizeButton, SIGNAL(clicked()), applet, SLOT(runAssociatedApplication()));
    }

    Plasma::Label *title = new Plasma::Label(this);
    title->setAlignment(Qt::AlignCenter);
    title->nativeWidget()->setWordWrap(false);
    title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    title->setText(applet->name());
    lay->addItem(title, 0, column);
    ++column;

    Plasma::IconWidget *closeButton = new Plasma::IconWidget(this);
    closeButton->setSvg("widgets/configuration-icons", "close");
    closeButton->setMinimumSize(KIconLoader::SizeSmallMedium, KIconLoader::SizeSmallMedium);
    closeButton->setMaximumSize(closeButton->minimumSize());
    lay->addItem(closeButton, 0, column);
    ++column;
    connect(closeButton, SIGNAL(clicked()), applet, SLOT(destroy()));

    if (applet->backgroundHints() & Plasma::Applet::StandardBackground) {
        Plasma::Separator *separator = new Plasma::Separator(this);
        separator->setOrientation(Qt::Horizontal);
        lay->addItem(separator, 1, 0, 1, column);
    }

    applet->installEventFilter(this);
    lay->activate();
    syncMargins();
    syncSize();

    if (applet->containment()) {
        connect(applet->containment(), SIGNAL(appletRemoved(Plasma::Applet *)), this, SLOT(appletRemoved(Plasma::Applet *)));
    }
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
}

AppletTitleBar::~AppletTitleBar()
{
    
}


void AppletTitleBar::syncMargins()
{
    qreal left, right, bottom;
    m_applet->getContentsMargins(&left, &m_savedAppletTopMargin, &right, &bottom);
    m_applet->setContentsMargins(left, m_savedAppletTopMargin + size().height(), right, bottom);
}

void AppletTitleBar::syncSize()
{
    setGeometry(QRectF(QPointF(m_applet->contentsRect().left(), m_savedAppletTopMargin),
                QSizeF(m_applet->contentsRect().size().width(),
                size().height())));
}

bool AppletTitleBar::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    if (event->type() == QEvent::GraphicsSceneResize) {
        syncSize();
    }

    return false;
}

void AppletTitleBar::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
}

void AppletTitleBar::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
}

void AppletTitleBar::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
}

void AppletTitleBar::appletRemoved(Plasma::Applet *applet)
{
    if (applet == m_applet) {
        qreal left, top, right, bottom;
        m_applet->getContentsMargins(&left, &top, &right, &bottom);
        m_applet->setContentsMargins(left, m_savedAppletTopMargin + size().height(), right, bottom);
        deleteLater();
    }
}

void AppletTitleBar::themeChanged()
{
    //send the margin update in the back of eveny queue,
    //so it will be executed after the margins update by Plasma::Applet
    QTimer::singleShot(0, this, SLOT(syncMargins()));
}

#include <applettitlebar.moc>
