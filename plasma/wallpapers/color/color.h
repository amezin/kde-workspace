/*
 *   Copyright 2008 by Petri Damsten <damu@iki.fi>
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

#ifndef COLOR_HEADER
#define COLOR_HEADER

#include <plasma/wallpaper.h>

class Color : public Plasma::Wallpaper
{
    Q_OBJECT
    public:
        Color(QObject *parent, const QVariantList &args);
        virtual void paint(QPainter *painter, const QRectF& exposedRect);
};

K_EXPORT_PLASMA_WALLPAPER(color, Color)

#endif
