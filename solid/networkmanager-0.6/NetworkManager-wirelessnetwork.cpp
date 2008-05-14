/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

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

#include "NetworkManager-wirelessnetwork.h"

#include "NetworkManager-networkinterface_p.h"

// Copied from wireless.h
/* Modes of operation */
#define IW_MODE_AUTO    0   /* Let the driver decides */
#define IW_MODE_ADHOC   1   /* Single cell network */
#define IW_MODE_INFRA   2   /* Multi cell network, roaming, ... */
#define IW_MODE_MASTER  3   /* Synchronization master or Access Point */
#define IW_MODE_REPEAT  4   /* Wireless Repeater (forwarder) */
#define IW_MODE_SECOND  5   /* Secondary master/repeater (backup) */
#define IW_MODE_MONITOR 6   /* Passive monitor (listen only) */

#include <NetworkManager/NetworkManager.h>

#include <kdebug.h>
#include <solid/control/ifaces/authentication.h>

#include "NetworkManager-dbushelper.h"
#include "NetworkManager-wirelessaccesspoint.h"

void dump(const Solid::Control::WirelessNetworkInterface::Capabilities  & cap)
{
    kDebug(1441) << "WPA      " << (cap  & Solid::Control::WirelessNetworkInterface::Wpa ? "X " : " O");
    kDebug(1441) << "Wep40    " << (cap  & Solid::Control::WirelessNetworkInterface::Wep40 ? "X " : " O");
    kDebug(1441) << "Wep104   " << (cap  & Solid::Control::WirelessNetworkInterface::Wep104 ? "X " : " O");
    kDebug(1441) << "TKIP     " << (cap  & Solid::Control::WirelessNetworkInterface::Tkip ? "X " : " O");
    kDebug(1441) << "CCMP     " << (cap  & Solid::Control::WirelessNetworkInterface::Ccmp ? "X " : " O");
    kDebug(1441) << "RSM      " << (cap  & Solid::Control::WirelessNetworkInterface::Rsn ? "X " : " O");
}

void dump(const NMDBusWirelessNetworkProperties  & network)
{
    kDebug(1441) << "Object path: " << network.path.path() << "\nESSID: " << network.essid
        << "\nHardware address: " << network.hwAddr << "\nSignal strength: " << network.strength
        << "\nFrequency: " << network.frequency << "\nBit rate: " << network.rate
        << "\nMode: " << network.mode
        << "\nBroadcast: " << network.broadcast << "\nWireless Capabilities: " << endl;
    dump(network.capabilities);
}

Solid::Control::WirelessNetworkInterface::Capabilities getCapabilities(const int nm)
{
    Solid::Control::WirelessNetworkInterface::Capabilities caps;
    if (nm  & NM_802_11_CAP_NONE)
        caps |= Solid::Control::WirelessNetworkInterface::NoCapability;
#if 0
    if (nm  & NM_802_11_CAP_PROTO_WEP)
        caps |= Solid::Control::WirelessNetworkInterface::Wep;
#endif
    if (nm  & NM_802_11_CAP_PROTO_WPA)
        caps |= Solid::Control::WirelessNetworkInterface::Wpa;
#if 0
    if (nm  & NM_802_11_CAP_PROTO_WPA2)
        caps |= Solid::Control::WirelessNetworkInterface::Wpa2;
    if (nm  & NM_802_11_CAP_KEY_MGMT_PSK)
        caps |= Solid::Control::WirelessNetworkInterface::Psk;
    if (nm  & NM_802_11_CAP_KEY_MGMT_802_1X)
        caps |= Solid::Control::WirelessNetworkInterface::Ieee8021x;
#endif
    if (nm  & NM_802_11_CAP_CIPHER_WEP40)
        caps |= Solid::Control::WirelessNetworkInterface::Wep40;
    if (nm  & NM_802_11_CAP_CIPHER_WEP104)
        caps |= Solid::Control::WirelessNetworkInterface::Wep104;
    if (nm  & NM_802_11_CAP_CIPHER_TKIP)
        caps |= Solid::Control::WirelessNetworkInterface::Tkip;
    if (nm  & NM_802_11_CAP_CIPHER_CCMP)
        caps |= Solid::Control::WirelessNetworkInterface::Ccmp;
    return caps;
}

Solid::Control::WirelessNetworkInterface::OperationMode getOperationMode(const int nm)
{
    Solid::Control::WirelessNetworkInterface::OperationMode mode = Solid::Control::WirelessNetworkInterface::Unassociated;
    switch (nm)
    {
    case IW_MODE_ADHOC:
        mode = Solid::Control::WirelessNetworkInterface::Adhoc;
        break;
    case IW_MODE_INFRA:
    case IW_MODE_MASTER:
        mode = Solid::Control::WirelessNetworkInterface::Managed;
        break;
    case IW_MODE_REPEAT:
        mode = Solid::Control::WirelessNetworkInterface::Repeater;
        break;
    }
    return mode;
}

void deserialize(const QDBusMessage  & message, NMDBusWirelessNetworkProperties  & network)
{
    //Debug(1441) << "signature: " << message.signature() << endl;
    QList<QVariant> args = message.arguments();
    network.path = args.takeFirst().value<QDBusObjectPath>();
    network.essid = args.takeFirst().toString();
    network.hwAddr = args.takeFirst().toString();
    network.strength = args.takeFirst().toInt();
    network.frequency = args.takeFirst().toDouble();
    network.rate = args.takeFirst().toInt();
    network.mode = getOperationMode(args.takeFirst().toInt());
    network.capabilities = getCapabilities(args.takeFirst().toInt());
    network.broadcast = args.takeFirst().toBool();
    network.networks = args.takeLast().toStringList();
}



typedef void Encryption;

class NMWirelessNetworkPrivate : public NMNetworkInterfacePrivate
{
public:
    NMWirelessNetworkPrivate(const QString  & netPath)
        : NMNetworkInterfacePrivate(netPath),
        strength(0), frequency(0.0), rate(0), broadcast(true), authentication(0) { }
    Q_DECLARE_PUBLIC(NMWirelessNetwork)
    /* reimp */ void notifyNewNetwork(const QDBusObjectPath & netPath);
    /* reimp */ void notifyRemoveNetwork(const QDBusObjectPath & netPath);
    MacAddress hwAddr;
    int strength;
    double frequency;
    int rate;
    Solid::Control::WirelessNetworkInterface::OperationMode mode;
    Solid::Control::WirelessNetworkInterface::Capabilities wirelessCapabilities;
    bool broadcast;
    Solid::Control::Authentication * authentication;
    QHash<QString, NMAccessPoint*> accessPoints;
};

void NMWirelessNetworkPrivate::notifyNewNetwork(const QDBusObjectPath & netPath)
{
    Q_Q(NMWirelessNetwork);
    const QString path = netPath.path();
    QHash<QString, NMAccessPoint*>::ConstIterator it = accessPoints.find(path);
    if (it == accessPoints.end()) {
        accessPoints.insert(path, 0);
        emit q->accessPointAppeared(path);
    }
}

void NMWirelessNetworkPrivate::notifyRemoveNetwork(const QDBusObjectPath & netPath)
{
    Q_Q(NMWirelessNetwork);
    const QString path = netPath.path();
    QHash<QString, NMAccessPoint*>::Iterator it = accessPoints.find(path);
    if (it != accessPoints.end()) {
        // ### who owns the AccessPoint's?
        accessPoints.erase(it);
        emit q->accessPointDisappeared(path);
    }
}


NMWirelessNetwork::NMWirelessNetwork(const QString  & networkPath)
 : NMNetworkInterface(*new NMWirelessNetworkPrivate(networkPath))
{
    Q_D(NMWirelessNetwork);
    //kDebug(1441) << "NMWirelessNetwork::NMWirelessNetwork() - " << networkPath;
    QDBusMessage reply = d->iface.call("getProperties");
    NMDBusWirelessNetworkProperties wlan;
    deserialize(reply, wlan);
    //dump(wlan);
    setProperties(wlan);
}

NMWirelessNetwork::~NMWirelessNetwork()
{
}

void NMWirelessNetwork::setProperties(const NMDBusWirelessNetworkProperties  & props)
{
    Q_D(NMWirelessNetwork);
    d->hwAddr.append(props.hwAddr);
    d->strength = props.strength;
    d->frequency = props.frequency;
    d->rate = props.rate;
    d->mode = props.mode;
    d->wirelessCapabilities = props.capabilities;
    d->broadcast = props.broadcast;
    Q_FOREACH (const QString & udi, props.networks) {
        d->accessPoints.insert(udi, 0);
    }
}

int NMWirelessNetwork::signalStrength() const
{
    Q_D(const NMWirelessNetwork);
    return d->strength;
}

int NMWirelessNetwork::bitRate() const
{
    Q_D(const NMWirelessNetwork);
    return d->rate;
}

double NMWirelessNetwork::frequency() const
{
    Q_D(const NMWirelessNetwork);
    return d->frequency;
}

Solid::Control::WirelessNetworkInterface::Capabilities NMWirelessNetwork::wirelessCapabilities() const
{
    Q_D(const NMWirelessNetwork);
    return d->wirelessCapabilities;
}

Solid::Control::WirelessNetworkInterface::OperationMode NMWirelessNetwork::mode() const
{
    Q_D(const NMWirelessNetwork);
    return d->mode;
}

bool NMWirelessNetwork::isAssociated() const
{
#warning NMWirelessNetwork::isAssociated() is unimplemented
    kDebug(1441) << "Fixme: implement NMWirelessNetwork::isAssociated()";
    return true;
}

bool NMWirelessNetwork::isEncrypted() const
{
    Q_D(const NMWirelessNetwork);
    // FIXME
    return !(d->wirelessCapabilities  & Solid::Control::WirelessNetworkInterface::NoCapability) ;
}

bool NMWirelessNetwork::isHidden() const
{
#warning NMWirelessNetwork::isHidden() is unimplemented
    kDebug(1441) << "Fixme: implement NMWirelessNetwork::isHidden()";
    return true;
}

Solid::Control::Authentication * NMWirelessNetwork::authentication() const
{
    Q_D(const NMWirelessNetwork);
    return d->authentication;
}

void NMWirelessNetwork::setAuthentication(Solid::Control::Authentication * auth)
{
    Q_D(NMWirelessNetwork);
    d->authentication = auth;
}

void NMWirelessNetwork::setSignalStrength(const QDBusObjectPath & netPath, int strength)
{
    Q_D(NMWirelessNetwork);
    QHash<QString, NMAccessPoint*>::ConstIterator it = d->accessPoints.find(netPath.path());
    if (it != d->accessPoints.end()) {
        NMAccessPoint * ap = it.value();
        ap->setSignalStrength(strength);
    }
}

void NMWirelessNetwork::setBitrate(int rate)
{
    Q_D(NMWirelessNetwork);
    d->rate = rate;
    emit bitRateChanged(rate);
}

#if 0
void NMWirelessNetwork::setActivated(bool activated)
{
    QDBusInterface manager("org.freedesktop.NetworkManager",
            "/org/freedesktop/NetworkManager",
            "org.freedesktop.NetworkManager",
            QDBusConnection::systemBus());
    QString devicePath = uni().left(uni().indexOf("/Networks"));
    kDebug(1441) << devicePath << " - " << d->essid;
    QDBusObjectPath op(devicePath);
#warning fixme hardcoded false fallback bool in setActiveDevice - fixed, i think
    QList<QVariant> args;
    args << qVariantFromValue(op) << d->essid;// << false;
    bool error;
    args = NMDBusHelper::serialize(d->authentication, d->essid, args, &error);
    kDebug(1441) << " " << args;
    if (error)
        kDebug(1411) << "Error whilst serializing authentication.";
    else
        manager.callWithArgumentList(QDBus::Block, "setActiveDevice", args);
    if (manager.lastError().isValid())
        kDebug(1441) << "setActiveDevice returned error: " << manager.lastError().name() << ": " << manager.lastError().message();

    emit activationStateChanged(activated);
}
#endif

MacAddressList NMWirelessNetwork::accessPoints() const
{
    Q_D(const NMWirelessNetwork);
    return d->accessPoints.keys();
}

QString NMWirelessNetwork::activeAccessPoint() const
{
#warning implement me!
    kDebug();
    return QString();
}

QString NMWirelessNetwork::hardwareAddress() const
{
    Q_D(const NMWirelessNetwork);
    return d->hwAddr;
}

QObject * NMWirelessNetwork::createAccessPoint(const QString & uni)
{
    Q_D(NMWirelessNetwork);
    kDebug() << uni;
    QHash<QString, NMAccessPoint*>::ConstIterator it = d->accessPoints.find(uni);
    if (it != d->accessPoints.end() && it.value()) {
        return it.value();
    }
    NMAccessPoint * ap = new NMAccessPoint(uni);
    d->accessPoints.insert(uni, ap);
    return ap;
}


#include "NetworkManager-wirelessnetwork.moc"

