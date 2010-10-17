/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *   Copyright 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "activityicon.h"

#include "activity.h"
#include "desktopcorona.h"
#include "plasmaapp.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsGridLayout>
#include <QPainter>
#include <QCursor>

#include <KIconLoader>
#include <KIcon>

#include <Plasma/Label>
#include <Plasma/PushButton>
#include <Plasma/LineEdit>
#include <Plasma/IconWidget>

#define REMOVE_ICON KIcon("edit-delete")
#define STOP_ICON KIcon("media-playback-stop")
#define START_ICON KIcon("media-playback-start")
#define CONFIGURE_ICON KIcon("configure")

class ActivityActionWidget: public QGraphicsWidget {
public:
    ActivityActionWidget(ActivityIcon * parent, const QString & slot,
            const KIcon & icon, const QString & tooltip, const QSize & size = QSize(16, 16))
        : QGraphicsWidget(parent), m_parent(parent), m_slot(slot), m_iconSize(size), m_icon(icon)
    {
        setToolTip(tooltip);
        setMinimumSize(m_iconSize);
        setPreferredSize(m_iconSize);
        setMaximumSize(m_iconSize);

        setGeometry(0, 0, m_iconSize.width(), m_iconSize.height());
        setZValue(1);
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        QGraphicsWidget::paint(painter, option, widget);
        painter->drawPixmap(0, 0, m_icon.pixmap(m_iconSize));
    }

    void mousePressEvent(QGraphicsSceneMouseEvent * event)
    {
        QGraphicsWidget::mouseReleaseEvent(event);

        if (event->button() != Qt::LeftButton)
            return;

        QMetaObject::invokeMethod(m_parent, m_slot.toAscii());

        event->accept();
    }

    ActivityIcon * m_parent;
    QString m_slot;
    QSize m_iconSize;
    KIcon m_icon;

};

ActivityIcon::ActivityIcon(const QString &id)
    :AbstractIcon(0),
    m_inlineWidgetAnim(0),
    m_buttonRemove(0),
    m_buttonStart(0),
    m_buttonConfigure(0),
    m_buttonStop(0)
{
    DesktopCorona *c = qobject_cast<DesktopCorona*>(PlasmaApp::self()->corona());
    m_activity = c->activity(id);
    connect(this, SIGNAL(clicked(Plasma::AbstractIcon*)), m_activity, SLOT(activate()));
    connect(m_activity, SIGNAL(opened()), this, SLOT(updateButtons()));
    connect(m_activity, SIGNAL(closed()), this, SLOT(updateButtons()));
    connect(m_activity, SIGNAL(nameChanged(QString)), this, SLOT(setName(QString)));
    setName(m_activity->name());

    updateButtons();
}

ActivityIcon::~ActivityIcon()
{
}

QPixmap ActivityIcon::pixmap(const QSize &size)
{
    return m_activity ? m_activity->pixmap(size) : QPixmap();
}

QMimeData* ActivityIcon::mimeData()
{
    //TODO: how shall we use d&d?
    return 0;
}

class MakeRoomAnimation : public QAbstractAnimation
{
public:
    MakeRoomAnimation(ActivityIcon *icon, qreal addedWidth, QObject *parent)
        : QAbstractAnimation(parent),
          m_icon(icon),
          m_addWidth(addedWidth)
    {
        qreal l, t, b;
        m_icon->getContentsMargins(&l, &t, &m_startRMargin, &b);
        m_startWidth = icon->contentsRect().width();
    }

    int duration() const
    {
        return 100;
    }

    void updateCurrentTime(int currentTime)
    {
        qreal delta = m_addWidth * (currentTime / 100.0);
        qreal l, t, r, b;
        m_icon->getContentsMargins(&l, &t, &r, &b);
        if (currentTime == 0 && direction() == Backward) {
            m_icon->setContentsMargins(l, t, m_startRMargin, b);
            m_icon->setMinimumSize(0, 0);
            m_icon->setPreferredSize(l + m_startRMargin + m_startWidth, m_icon->size().height());
            m_icon->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        } else {
            QSize s(m_startWidth + l + m_startRMargin + delta, m_icon->size().height());
            m_icon->setContentsMargins(l, t, m_startRMargin + delta, b);
            m_icon->setMaximumSize(s);
            m_icon->setMinimumSize(s);
            m_icon->setPreferredSize(s);
        }

        m_icon->getContentsMargins(&l, &t, &r, &b);
        //kDebug() << currentTime << m_startWidth << m_addWidth << delta << m_icon->size() << l << t << r << b;
    }

private:
    ActivityIcon *m_icon;
    qreal m_startWidth;
    qreal m_startRMargin;
    qreal m_addWidth;
};

void ActivityIcon::showRemovalConfirmation()
{
    QGraphicsWidget *w = new QGraphicsWidget(this);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(w);
    layout->setOrientation(Qt::Vertical);
    layout->setContentsMargins(0, 0, 0, 0);
    w->setLayout(layout);

    Plasma::Label *l = new Plasma::Label(w);
    l->setText(i18n("Remove activity?"));
    l->setAlignment(Qt::AlignCenter);
    layout->addItem(l);

    Plasma::PushButton *p = new Plasma::PushButton(w);
    p->setText(i18n("Confirm Removal"));
    layout->addItem(p);
    connect(p, SIGNAL(clicked()), m_activity, SLOT(destroy()));

    p = new Plasma::PushButton(w);
    p->setText(i18n("Cancel Removal"));
    layout->addItem(p);
    connect(p, SIGNAL(clicked()), this, SLOT(hideInlineWidget()));

    w->setMaximumSize(QSize(0, size().height()));
    w->adjustSize();
    w->setPos(contentsRect().topRight() + QPoint(4, 0));

    m_inlineWidget = w;
    QTimer::singleShot(0, this, SLOT(startInlineAnim()));
}

void ActivityIcon::showConfiguration()
{
    QGraphicsWidget *w = new QGraphicsWidget(this);
    QGraphicsGridLayout *layout = new QGraphicsGridLayout(w);

    layout->setContentsMargins(0, 0, 0, 0);
    w->setLayout(layout);

    Plasma::IconWidget * icon = new Plasma::IconWidget(w);
    icon->setIcon(KIcon("plasma"));
    icon->setMinimumIconSize(QSizeF(32, 32));
    icon->setPreferredIconSize(QSizeF(32, 32));
    // l->setText("###");

    Plasma::Label *labelName = new Plasma::Label(w);
    labelName->setText(i18n("Activity name"));

    Plasma::LineEdit *editName = new Plasma::LineEdit(w);

    Plasma::PushButton * buttonSave = new Plasma::PushButton(w);
    buttonSave->setText(i18n("Save"));
    // connect(p, SIGNAL(clicked()), m_activity, SLOT(destroy()));

    Plasma::PushButton * buttonCancel = new Plasma::PushButton(w);
    buttonCancel->setText(i18n("Cancel"));
    // connect(p, SIGNAL(clicked()), m_activity, SLOT(destroy()));

    // layout
    layout->addItem(icon, 0, 0, 2, 1);

    layout->addItem(labelName, 0, 1);
    layout->addItem(editName,  1, 1);

    layout->addItem(buttonSave,   0, 2);
    layout->addItem(buttonCancel, 1, 2);

    w->setMaximumSize(QSize(0, size().height()));
    w->adjustSize();
    w->setPos(contentsRect().topRight() + QPoint(4, 0));

    m_inlineWidget = w;
    QTimer::singleShot(0, this, SLOT(startInlineAnim()));
}

void ActivityIcon::startInlineAnim()
{
    QGraphicsWidget * w = m_inlineWidget.data();
    //kDebug() << "Booh yah!" << w;
    if (!w) {
        return;
    }

    //kDebug() << w->preferredSize() << w->layout()->preferredSize();
    if (!m_inlineWidgetAnim) {
        m_inlineWidgetAnim = new MakeRoomAnimation(this, w->layout()->preferredSize().width() + 4, this);
        connect(m_inlineWidgetAnim, SIGNAL(finished()), this, SLOT(makeInlineWidgetVisible()));
    }

    m_inlineWidgetAnim->start();
}

void ActivityIcon::hideInlineWidget()
{
    if (m_inlineWidget) {
        m_inlineWidget.data()->deleteLater();
        m_inlineWidget.data()->hide();
    }

    if (m_inlineWidgetAnim) {
        m_inlineWidgetAnim->setDirection(QAbstractAnimation::Backward);
        if (m_inlineWidgetAnim->state() != QAbstractAnimation::Running) {
            m_inlineWidgetAnim->start(QAbstractAnimation::DeleteWhenStopped);
            m_inlineWidgetAnim = 0;
        }
    }
}

void ActivityIcon::makeInlineWidgetVisible()
{
    if (m_inlineWidget) {
        m_inlineWidget.data()->show();
    }
}

void ActivityIcon::setClosable(bool closable)
{
    if (closable == m_closable) {
        return;
    }

    m_closable = closable;
    updateButtons();
}

Activity* ActivityIcon::activity()
{
    return m_activity;
}

void ActivityIcon::activityRemoved()
{
    m_activity = 0;
    deleteLater();
}

void ActivityIcon::setGeometry(const QRectF & geometry)
{
    Plasma::AbstractIcon::setGeometry(geometry);
    updateLayout();
}

void ActivityIcon::updateLayout()
{
    QRectF rect = contentsRect();

    rect.adjust(
            (rect.width() - iconSize()) / 2,
            rect.height() - iconSize(),
            - (rect.width() - iconSize()) / 2,
            0
        );

    if (m_buttonStop) {
        m_buttonStop->setGeometry(QRectF(
            rect.topRight() - QPointF(m_buttonStop->m_iconSize.width(), 0),
            m_buttonStop->m_iconSize
        ));
    }

    if (m_buttonRemove) {
        m_buttonRemove->setGeometry(QRectF(
            rect.topRight() - QPointF(m_buttonRemove->m_iconSize.width(), 0),
            m_buttonRemove->m_iconSize
        ));
    }

    if (m_buttonConfigure) {
        m_buttonConfigure->setGeometry(QRectF(
            rect.bottomRight() - QPointF(m_buttonConfigure->m_iconSize.width(), m_buttonConfigure->m_iconSize.height()),
            m_buttonConfigure->m_iconSize
        ));
    }

    if (m_buttonStart) {
        m_buttonStart->setGeometry(QRectF(
            rect.center() - QPointF(m_buttonStart->m_iconSize.width() / 2, m_buttonStart->m_iconSize.height() / 2),
            m_buttonStart->m_iconSize
        ));
    }
}

void ActivityIcon::updateButtons()
{
    if (!m_activity) {
        return;
    }

    if (!m_buttonConfigure) {
        m_buttonConfigure = new ActivityActionWidget(this, "showConfiguration", CONFIGURE_ICON, i18n("Configure activity"));
    }

#define DESTROY_ACTIVITY_ACTION_WIDIGET(A) \
    if (A) {                               \
        A->hide();                         \
        A->deleteLater();                  \
        A = 0;                             \
    }

    if (m_activity->isRunning()) {
        DESTROY_ACTIVITY_ACTION_WIDIGET(m_buttonStart);
        DESTROY_ACTIVITY_ACTION_WIDIGET(m_buttonRemove);

        if (m_closable) {
            if (!m_buttonStop) {
                m_buttonStop = new ActivityActionWidget(this, "stopActivity", STOP_ICON, i18n("Stop activity"));
            }
        } else {
            DESTROY_ACTIVITY_ACTION_WIDIGET(m_buttonStop);
        }

    } else {
        DESTROY_ACTIVITY_ACTION_WIDIGET(m_buttonStop);

        if (!m_buttonRemove) {
            m_buttonRemove = new ActivityActionWidget(this, "showRemovalConfirmation", REMOVE_ICON, i18n("Stop activity"));
        }

        if (!m_buttonStart) {
            m_buttonStart = new ActivityActionWidget(this, "startActivity", START_ICON, i18n("Stop activity"), QSize(32, 32));
        }

    }

#undef DESTROY_ACTIVITY_ACTION_WIDIGET

    updateLayout();
}

void ActivityIcon::stopActivity()
{
    if (m_activity) {
        m_activity->close();
    }
}

void ActivityIcon::startActivity()
{
    emit clicked(this);
}

#include "activityicon.moc"

