/*
 *   Copyright 2012 Viranch Mehta <viranch.mehta@gmail.com>
 *   Copyright 2013 David Edmundson <davidedmundson@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import "logic.js" as Logic

Item {
    id: cell
    property int dday
    property int daysInThisMonth: Logic.daysInMonth(month)
    property int daysInPrevMonth: Logic.daysInMonth( month==1 ? 12 : month-1 )
    property bool valid: (dday>0 && dday<=daysInThisMonth)
    property alias text: date.text
    property QtObject calendarSvg
    signal selected

    PlasmaCore.SvgItem {
        id: cellBg
        svg: calendarSvg
        elementId: mouseArea.containsMouse ? "hoverHighlight" : (valid ? "active" : "inactive")
        anchors.fill: parent
    }

    Text {
        id: date
        font.pixelSize: 18
        anchors.centerIn: parent
        text: {
            if (dday<1) return (daysInPrevMonth+dday).toString();
            else if (dday>daysInThisMonth) return (dday-daysInThisMonth).toString();
            else return dday.toString();
        }
        opacity: valid ? 1 : 0.5
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: selected();
    }
}
