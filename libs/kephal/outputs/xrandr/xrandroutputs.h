/*
 *   Copyright 2008 Aike J Sommer <dev@aikesommer.name>
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


#ifndef KEPHAL_XRANDROUTPUTS_H
#define KEPHAL_XRANDROUTPUTS_H

#include <QDebug>

#include "../backendoutputs.h"
#include "xrandr12/randrdisplay.h"


namespace Kephal {

    class XRandROutputs;

    class XRandROutput : public BackendOutput {
        Q_OBJECT
        public:
            XRandROutput(XRandROutputs * parent, RROutput rrId);

            QString id();

            QSize size();
            QSize preferredSize();
            QList<QSize> availableSizes();
            QPoint position();
            bool isConnected();
            bool isActivated();
            QString vendor();
            int productId();
            unsigned int serialNumber();
            Rotation rotation();
            bool reflectX();
            bool reflectY();
            float rate();
            QList<float> availableRates();

            bool applyGeom(const QRect & rect, float rate);
            bool applyOrientation(Rotation rotation, bool reflectX, bool reflectY);

            //void _revert();
            void deactivate();
            //void _activate();
            RROutput _id();

        public Q_SLOTS:
            void outputChanged(RROutput id, int changes);

        Q_SIGNALS:
            void outputConnected(Kephal::Output * o);
            void outputDisconnected(Kephal::Output * o);
            void outputActivated(Kephal::Output * o);
            void outputDeactivated(Kephal::Output * o);
            void outputResized(Kephal::Output * o, QSize oldSize, QSize newSize);
            void outputMoved(Kephal::Output * o, QPoint oldPosition, QPoint newPosition);
            void outputRateChanged(Kephal::Output * o, float oldRate, float newRate);
            void outputRotated(Kephal::Output * o, Kephal::Rotation oldRotation, Kephal::Rotation newRotation);
            void outputReflected(Kephal::Output * o, bool oldX, bool oldY, bool newX, bool newY);

        private:
            RandROutput * output();
            void parseEdid();
            void saveAsPrevious();

            XRandROutputs * m_outputs;
            RROutput m_rrId;
            QString m_vendor;
            int m_productId;
            unsigned int m_serialNumber;
            QRect m_previousGeom;
            bool m_previousConnected;
            bool m_previousActivated;
            Rotation m_previousRotation;
            float m_previousRate;
            bool m_previousReflectX;
            bool m_previousReflectY;
    };


    class XRandROutputs : public BackendOutputs {
        Q_OBJECT
        public:
            XRandROutputs(QObject * parent, RandRDisplay * display);

            QList<Output *> outputs();

            RandROutput * output(RROutput rrId);
            using Outputs::output;
            RandRDisplay * display();

        private:
            void init();

            RandRDisplay * m_display;
            QMap<QString, XRandROutput *> m_outputs;
    };

}


#endif // KEPHAL_XRANDROUTPUTS_H

