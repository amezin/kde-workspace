/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy 
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "wirednetworkinterface.h"
#include "wirednetworkinterface_p.h"
#include "manager.h"

#define NM_DEVICE_802_3_ETHERNET_HW_ADDRESS  "hw-address"
#define NM_DEVICE_802_3_ETHERNET_SPEED       "speed"
#define NM_DEVICE_802_3_ETHERNET_CARRIER     "carrier"

NMWiredNetworkInterfacePrivate::NMWiredNetworkInterfacePrivate(const QString & path, QObject * owner)
    : NMNetworkInterfacePrivate(path, owner), wiredIface(NMNetworkManager::DBUS_SERVICE, path, QDBusConnection::systemBus()),
    bitrate(0), carrier(false)
{
}

NMWiredNetworkInterface::NMWiredNetworkInterface(const QString & path, NMNetworkManager * manager, QObject * parent)
    : NMNetworkInterface(*new NMWiredNetworkInterfacePrivate(path, this), manager, parent)
{
    Q_D(NMWiredNetworkInterface);
    d->hardwareAddress = d->wiredIface.hwAddress();
    d->bitrate = d->wiredIface.speed();
    d->carrier = d->wiredIface.carrier();
    //d->propHelper.registerProperty();
    connect( &d->wiredIface, SIGNAL(PropertiesChanged(const QVariantMap &)),
                this, SLOT(wiredPropertiesChanged(const QVariantMap &)));
}

NMWiredNetworkInterface::~NMWiredNetworkInterface()
{
}

QString NMWiredNetworkInterface::hardwareAddress() const
{
    Q_D(const NMWiredNetworkInterface);
    return d->hardwareAddress;
}

int NMWiredNetworkInterface::bitRate() const
{
    Q_D(const NMWiredNetworkInterface);
    return d->bitrate;
}

bool NMWiredNetworkInterface::carrier() const
{
    Q_D(const NMWiredNetworkInterface);
    return d->carrier;
}

void NMWiredNetworkInterface::setCarrier(const QVariant& carrier)
{
    Q_D(NMWiredNetworkInterface);
    d->carrier = carrier.toBool();
}

void NMWiredNetworkInterface::setBitRate(const QVariant& bitrate)
{
    Q_D(NMWiredNetworkInterface);
    d->bitrate = bitrate.toInt();
}

void NMWiredNetworkInterface::wiredPropertiesChanged(const QVariantMap &properties)
{
    Q_D(NMWiredNetworkInterface);
    QStringList propKeys = properties.keys();
    kDebug(1441) << properties.keys();
    QLatin1String carrierKey("Carrier");
    QLatin1String hwAddressKey("HwAddress");
    QLatin1String speedKey("Speed");
    if (properties.contains(carrierKey)) {
        d->carrier = properties.value(carrierKey).toBool();
        emit carrierChanged(d->carrier);
        propKeys.removeOne(carrierKey);
    }
    if (properties.contains(speedKey)) {
        d->bitrate = properties.value(speedKey).toUInt();
        emit bitRateChanged(d->bitrate);
        propKeys.removeOne(speedKey);
    }
    if (properties.contains(hwAddressKey)) {
        d->hardwareAddress = properties.value(hwAddressKey).toString();
        propKeys.removeOne(hwAddressKey);
    }
    if (propKeys.count()) {
        kDebug(1441) << "Unhandled properties: " << propKeys;
    }
}

#include "wirednetworkinterface.moc"

