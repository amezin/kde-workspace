/*
 * This file was generated by dbusxml2cpp version 0.6
 * Command line was: dbusxml2cpp -N -m -p nm-vpn-managerinterface /space/kde/sources/trunk/KDE/kdebase/workspace/solid/networkmanager-0.7/dbus/introspection/nm-vpn-manager.xml
 *
 * dbusxml2cpp is Copyright (C) 2006 Trolltech ASA. All rights reserved.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#include "nm-vpn-managerinterface.h"

/*
 * Implementation of interface class OrgFreedesktopNetworkManagerVPNManagerInterface
 */

OrgFreedesktopNetworkManagerVPNManagerInterface::OrgFreedesktopNetworkManagerVPNManagerInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OrgFreedesktopNetworkManagerVPNManagerInterface::~OrgFreedesktopNetworkManagerVPNManagerInterface()
{
}


#include "nm-vpn-managerinterface.moc"