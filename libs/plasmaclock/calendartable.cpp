/*
 *   Copyright 2008 Davide Bettio <davide.bettio@kdemail.net>
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

#include "calendartable.h"

//Qt
#include <QtCore/QDate>
#include <QtGui/QPainter>
#include <QtGui/QWidget>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QStyleOptionGraphicsItem>
//KDECore
#include <kglobal.h>
#include <kdebug.h>

//Plasma
#include <Plasma/Svg>
#include <Plasma/Theme>

namespace Plasma
{

static const int DISPLAYED_WEEKS = 6;

class CalendarCellBorder
{
public:
    CalendarCellBorder(int c, int w, int d, CalendarTable::CellTypes t, QDate dt)
        : cell(c),
          week(w),
          weekDay(d),
          type(t),
          date(dt)
    {
    }

    int cell;
    int week;
    int weekDay;
    CalendarTable::CellTypes type;
    QDate date;
};

class CalendarTablePrivate
{
    public:
        CalendarTablePrivate(CalendarTable *, const QDate &cDate = QDate::currentDate())
        {
            svg = new Svg();
            svg->setImagePath("widgets/calendar");
            svg->setContainsMultipleImages(true);

            calendar = KGlobal::locale()->calendar();

            date = cDate;

            month = calendar->month(date);
            year = calendar->year(date);

            opacity = 0.5; //transparency for the inactive text
        }

        ~CalendarTablePrivate()
        {
            delete svg;
        }

        int firstMonthDayIndex(int y, int m)
        {
            QDate myDate;
            calendar->setDate(myDate, y, m, 1);

            return (((calendar->dayOfWeek(myDate) - 1) + (calendar->daysInWeek(date) - (calendar->weekStartDay() - 1))) % calendar->daysInWeek(date)) + 1;
        }

        QRectF hoveredCellRect(CalendarTable *q, const QPointF &hoverPoint)
        {
            hoverDay = -1;
            hoverWeek = -1;

            if (hoverPoint.isNull()) {
                return QRectF();
            }

            if (hoverPoint.x() < centeringSpace + cellW + weekBarSpace) {
                // skip the weekbar
                return QRectF();
            }

            int x = (hoverPoint.x() - centeringSpace) / (cellW + cellSpace);
            int y = (hoverPoint.y() - headerHeight - headerSpace) / (cellH + cellSpace);

            if (x < 1 || x > 7 || y < 0 || y > DISPLAYED_WEEKS) {
                return QRectF();
            }

            //FIXME: this should be a hint or something somewhere
            hoverDay = x - 1;
            hoverWeek = y;
            //kDebug () << x << y;
            return QRectF(q->cellX(x - 1) - glowRadius, q->cellY(y) - glowRadius,
                          cellW + glowRadius * 2, cellH + glowRadius * 2);
        }

        void updateHoveredPainting(CalendarTable *q, const QPointF &hoverPoint)
        {
            QRectF newHoverRect = hoveredCellRect(q, hoverPoint);

            // now update what is needed, and only what is needed!
            if (newHoverRect.isValid() && newHoverRect != hoverRect) {
                if (hoverRect.isValid()) {
                    q->update(hoverRect);
                }
                q->update(newHoverRect);
            }

            hoverRect = newHoverRect;
        }

        int cell(int week, int weekDay, CalendarTable::CellTypes *type, QDate &cellDate)
        {
            QDate myDate;
            bool valid = calendar->setDate(myDate, year, month, 1);
            int numMonths = calendar->monthsInYear(myDate);

            if ((week == 0) && (weekDay < firstMonthDayIndex(year, month))) {
                if (type) {
                    (*type) |= CalendarTable::NotInCurrentMonth;
                }

                int prevMonth = (month == 1) ? numMonths : month - 1;
                calendar->setDate(myDate, year, prevMonth, 1);
                calendar->setDate(cellDate, (prevMonth == numMonths) ? year - 1 : year, prevMonth,
                                 calendar->daysInMonth(myDate) - (firstMonthDayIndex(year, month) - 1 - weekDay));
                return calendar->daysInMonth(myDate) - (firstMonthDayIndex(year, month) - 1 - weekDay);
            } else {
                int day = (week * calendar->daysInWeek(date) + weekDay) - firstMonthDayIndex(year, month) + 1;

                if (day <= calendar->daysInMonth(myDate)) {
                    if (type) {
                        (*type) &= ~CalendarTable::NotInCurrentMonth;
                    }

                    calendar->setDate(cellDate, year, month, day);
                    return day;
                } else {
                    if (type) {
                        (*type) |= CalendarTable::NotInCurrentMonth;
                    }

                    int nextMonth = (numMonths == month) ? 1 : month + 1;
                    calendar->setDate(cellDate, (nextMonth == 1) ? year + 1 : year, nextMonth, day - calendar->daysInMonth(myDate));
                    return day - calendar->daysInMonth(myDate);
                }
            }
        }

        Plasma::Svg *svg;
        const KCalendarSystem *calendar;
        QDate date;
        QRectF hoverRect;
        int month;
        int year;

        float opacity;
        int hoverWeek;
        int hoverDay;
        int centeringSpace;
        int cellW;
        int cellH;
        int cellSpace;
        int headerHeight;
        int headerSpace;
        int weekBarSpace;
        int glowRadius;
        QHash<QString, QString> specialDates; // ISODate -> what's special about it
};

CalendarTable::CalendarTable(const QDate &date, QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarTablePrivate(this, date))
{
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

CalendarTable::CalendarTable(QGraphicsWidget *parent)
    : QGraphicsWidget(parent), d(new CalendarTablePrivate(this))
{
    setAcceptHoverEvents(true);
    setCacheMode(QGraphicsItem::DeviceCoordinateCache);
}

CalendarTable::~CalendarTable()
{
    delete d;
}

const KCalendarSystem *CalendarTable::calendar() const
{
    return d->calendar;
}

bool CalendarTable::setCalendar(KCalendarSystem *calendar)
{
    d->calendar = calendar;
    return false;
}

bool CalendarTable::setDate(const QDate &date)
{
    int oldYear = d->year;
    int oldMonth = d->month;
    QDate oldDate = d->date;
    d->year = d->calendar->year(date);
    d->month = d->calendar->month(date);
    //kDebug( )<< "setting date to" << date << d->year << d->month;
    bool fullUpdate = false;

    if (oldYear != d->year){
        emit displayedYearChanged(d->year, d->month);
        fullUpdate = true;
    }

    if (oldMonth != d->month){
        emit displayedMonthChanged(d->year, d->month);
        fullUpdate = true;
    }

    // now change the date
    d->date = date;

    d->updateHoveredPainting(this, QPointF());

    if (fullUpdate) {
        update();
    } else {
        // only update the old and the new areas
        int offset = d->firstMonthDayIndex(d->year, d->month);
        int daysInWeek = d->calendar->daysInWeek(d->date);

        int day = d->calendar->day(oldDate);
        int x = ((offset + day - 1) % daysInWeek);
        if (x == 0) {
            x = daysInWeek;
        }
        int y = (offset + day - 2) / daysInWeek;
        update(cellX(x - 1) - d->glowRadius, cellY(y) - d->glowRadius,
               d->cellW + d->glowRadius * 2, d->cellH + d->glowRadius * 2);

        day = d->calendar->day(date);
        x = (offset + day - 1) % daysInWeek;
        if (x == 0) {
            x = daysInWeek;
        }
        y = (offset + day - 2) / daysInWeek;
        update(cellX(x - 1) - d->glowRadius, cellY(y) - d->glowRadius,
               d->cellW + d->glowRadius * 2, d->cellH + d->glowRadius * 2);
    }

    return false;
}

const QDate& CalendarTable::date() const
{
    return d->date;
}

int CalendarTable::cellX(int weekDay)
{
    return boundingRect().x() + d->centeringSpace +
           d->weekBarSpace + d->cellW +
           ((d->cellW + d->cellSpace) * (weekDay));
}

int CalendarTable::cellY(int week)
{
    return (int) boundingRect().y() + (d->cellH + d->cellSpace) * (week) + d->headerHeight + d->headerSpace;
}

void CalendarTable::wheelEvent(QGraphicsSceneWheelEvent * event)
{
    Q_UNUSED(event);

    if (event->delta() < 0) {
        if (d->month == d->calendar->monthsInYear(d->date)) {
            d->month = 1;
            d->year++;
            emit displayedYearChanged(d->year, d->month);
        } else {
            d->month++;
        }

        emit displayedMonthChanged(d->year, d->month);
    } else if (event->delta() > 0) {
        if (d->month == 1) {
            d->month = d->calendar->monthsInYear(d->date);
            d->year--;
            emit displayedYearChanged(d->year, d->month);
        } else {
            d->month--;
        }

        emit displayedMonthChanged(d->year, d->month);
    }

    update();
}

void CalendarTable::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();

    if ((event->pos().x() >= cellX(0)) && (event->pos().x() <= cellX(d->calendar->daysInWeek(d->date)) - d->cellSpace) &&
        (event->pos().y() >= cellY(0)) && (event->pos().y() <= cellY(DISPLAYED_WEEKS) - d->cellSpace)){

        int week = -1;
        int weekDay = -1;
        QDate cellDate;

        for (int i = 0; i < d->calendar->daysInWeek(d->date); i++) {
            if ((event->pos().x() >= cellX(i)) && (event->pos().x() <= cellX(i + 1) - d->cellSpace))
                weekDay = i;
        }

        for (int i = 0; i < DISPLAYED_WEEKS; i++) {
            if ((event->pos().y() >= cellY(i)) && (event->pos().y() <= cellY(i + 1) - d->cellSpace))
                week = i;
        }

        if ((week >= 0) && (weekDay >= 0)) {
            d->hoverDay = -1;
            d->hoverWeek = -1;
            QDate tmpDate;
            d->cell(week, weekDay + 1, 0, tmpDate);

            if (tmpDate == d->date) {
                return;
            }

            QDate oldDate = d->date;
            setDate(tmpDate);
            emit dateChanged(tmpDate, oldDate);
            emit dateChanged(tmpDate);
        }
    }
}

void CalendarTable::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    mousePressEvent(event);
}

void CalendarTable::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);

    emit tableClicked();
}

void CalendarTable::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    d->updateHoveredPainting(this, event->pos());
}

void CalendarTable::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    Q_UNUSED(event);

    QRectF r = contentsRect();
    int rectSizeH = int(r.height() / (DISPLAYED_WEEKS + 1));
    int rectSizeW = int(r.width() / 8);

    //Using integers to help to keep things aligned to the grid
    //kDebug() << r.width() << rectSize;
    d->cellSpace = qMax(1, qMin(4, qMin(rectSizeH, rectSizeW) / 20));
    d->headerSpace = d->cellSpace * 2;
    d->weekBarSpace = d->cellSpace * 2 + 1;
    d->cellH = rectSizeH - d->cellSpace;
    d->cellW = rectSizeW - d->cellSpace;
    d->glowRadius = d->cellW * .1;
    d->headerHeight = (int) (d->cellH / 1.5);
    d->centeringSpace = qMax(0, int((r.width() - (rectSizeW * 8) - (d->cellSpace * 7)) / 2));
}

void CalendarTable::paintCell(QPainter *p, int cell, int week, int weekDay, CellTypes type, const QDate &cellDate)
{
    Q_UNUSED(cellDate);

    QString cellSuffix = type & NotInCurrentMonth ? "inactive" : "active";
    QRectF cellArea = QRectF(cellX(weekDay), cellY(week), d->cellW, d->cellH);

    d->svg->paint(p, cellArea, cellSuffix); // draw background

    QColor numberColor = Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    if (type & NotInCurrentMonth) {
        p->setOpacity(d->opacity);
    }

    p->setPen(numberColor);
    QFont font = Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
    font.setBold(true);
    font.setPixelSize(cellArea.height() * 0.7);
    p->setFont(font);
    p->drawText(cellArea, Qt::AlignCenter, QString::number(cell), &cellArea); //draw number
    p->setOpacity(1.0);
}

void CalendarTable::paintBorder(QPainter *p, int cell, int week, int weekDay, CellTypes type, const QDate &cellDate)
{
    Q_UNUSED(cell);
    Q_UNUSED(cellDate);

    if (type & Hovered) {
        d->svg->paint(p, QRect(cellX(weekDay), cellY(week), d->cellW, d->cellH), "hoverHighlight");
    }

    QString elementId;

    if (type & Today) {
        elementId = "today";
    } else if (type & Selected) {
        elementId = "selected";
    } else if (type & Holiday) {
        elementId = "red";
    } else {
        return;
    }

    d->svg->paint(p, QRectF(cellX(weekDay) - 1, cellY(week) - 1,
                            d->cellW + 1, d->cellH + 2),
                  elementId);
}

void CalendarTable::paint(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    int daysInWeek = d->calendar->daysInWeek(d->date);

    // Draw weeks numbers column and day header
    QRectF r = boundingRect();
    d->svg->paint(p, QRectF(r.x() + d->centeringSpace, cellY(0), d->cellW,
                  cellY(DISPLAYED_WEEKS) - cellY(0) - d->cellSpace),  "weeksColumn");
    d->svg->paint(p, QRectF(r.x() + d->centeringSpace, r.y(),
                  cellX(daysInWeek) - r.x() - d->cellSpace - d->centeringSpace, d->headerHeight), "weekDayHeader");

    QList<CalendarCellBorder> borders;
    QList<CalendarCellBorder> hovers;
    QDate currentDate = QDate::currentDate(); //FIXME: calendar timezone

    //kDebug() << "exposed: " << option->exposedRect;
    for (int week = 0; week < DISPLAYED_WEEKS; week++) {
        for (int weekDay = 0; weekDay < daysInWeek; weekDay++) {
            int x = cellX(weekDay);
            int y = cellY(week);

            QRectF cellRect(x, y, d->cellW, d->cellH);
            if (!cellRect.intersects(option->exposedRect)) {
                continue;
            }

            QDate cellDate(d->date.year(), d->date.month(), (week * 7) + (weekDay + 1));
            CalendarTable::CellTypes type(CalendarTable::NoType);
            // get cell info
            int cell = d->cell(week, weekDay + 1, &type, cellDate);

            // check what kind of cell we are
            if (cellDate == currentDate) {
                type |= Today;
            }

            if (cellDate == d->date) {
                type |= Selected;
            }

            if (d->specialDates.contains(cellDate.toString(Qt::ISODate))) {
                type |= Holiday;
            }

            if (type != CalendarTable::NoType && type != CalendarTable::NotInCurrentMonth) {
                borders.append(CalendarCellBorder(cell, week, weekDay, type, cellDate));
            }

            if (week == d->hoverWeek && weekDay == d->hoverDay) {
                type |= Hovered;
                hovers.append(CalendarCellBorder(cell, week, weekDay, type, cellDate));
            }

            paintCell(p, cell, week, weekDay, type, cellDate);

            if (weekDay == 0) {
                QRectF cellRect(r.x() + d->centeringSpace, y, d->cellW, d->cellH);
                p->setPen(Theme::defaultTheme()->color(Plasma::Theme::TextColor));
                QFont font = Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
                font.setPixelSize(cellRect.height() * 0.7);
                p->setFont(font);
                p->setOpacity(d->opacity);
                QString weekString = QString::number(d->calendar->weekNumber(cellDate));
                if (cellDate.dayOfWeek() != Qt::Monday) {
                    weekString += "/";
                    QDate date(cellDate);
                    date = date.addDays(8 - cellDate.dayOfWeek());
                    weekString += QString::number(d->calendar->weekNumber(date));
                }
                p->drawText(cellRect, Qt::AlignCenter, weekString); //draw number
                p->setOpacity(1.0);
            }
        }
    }

    // Draw days
    if (option->exposedRect.intersects(QRect(r.x(), r.y(), r.width(), d->headerHeight))) {
        p->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
        int weekStartDay = d->calendar->weekStartDay();
        for (int i = 0; i < daysInWeek; i++){
            int weekDay = ((i + weekStartDay - 1) % daysInWeek) + 1;
            QString dayName = d->calendar->weekDayName(weekDay, KCalendarSystem::ShortDayName);
            QFont font = Theme::defaultTheme()->font(Plasma::Theme::DefaultFont);
            font.setPixelSize(d->headerHeight * 0.9);
            p->setFont(font);
            p->drawText(QRectF(cellX(i), r.y(), d->cellW, d->headerHeight),
                        Qt::AlignCenter | Qt::AlignVCenter, dayName);
        }
    }

    // Draw hovers
    foreach (const CalendarCellBorder &border, hovers) {
        p->save();
        paintBorder(p, border.cell, border.week, border.weekDay, border.type, border.date);
        p->restore();
    }

    // Draw borders
    foreach (const CalendarCellBorder &border, borders) {
        p->save();
        paintBorder(p, border.cell, border.week, border.weekDay, border.type, border.date);
        p->restore();
    }

    /*
    p->save();
    p->setPen(Qt::red);
    p->drawRect(option->exposedRect.adjusted(1, 1, -2, -2));
    p->restore();
    */
}

//HACK
void CalendarTable::clearDateProperties()
{
    d->specialDates.clear();
}

void CalendarTable::setDateProperty(QDate date, const QString &description)
{
    d->specialDates.insert(date.toString(Qt::ISODate), description);
}

QString CalendarTable::dateProperty(QDate date) const
{
    return d->specialDates.value(date.toString(Qt::ISODate));
}

} //namespace Plasma

#include "calendartable.moc"
