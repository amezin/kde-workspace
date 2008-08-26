/*
 * This file was generated by dbusxml2cpp version 0.6
 * Command line was: dbusxml2cpp -N -m -c MprisPlayer -i mprisdbustypes.h -p mprisplayer org.freedesktop.MediaPlayer.player.xml
 *
 * dbusxml2cpp is Copyright (C) 2006 Trolltech ASA. All rights reserved.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef MPRISPLAYER_H_1219792450
#define MPRISPLAYER_H_1219792450

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include "mprisdbustypes.h"

/*
 * Proxy class for interface org.freedesktop.MediaPlayer
 */
class MprisPlayer: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.freedesktop.MediaPlayer"; }

public:
    MprisPlayer(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~MprisPlayer();

public Q_SLOTS: // METHODS
    inline QDBusReply<int> GetCaps()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("GetCaps"), argumentList);
    }

    inline QDBusReply<QVariantMap> GetMetadata()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("GetMetadata"), argumentList);
    }

    inline QDBusReply<MprisDBusStatus> GetStatus()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("GetStatus"), argumentList);
    }

    inline QDBusReply<void> Next()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("Next"), argumentList);
    }

    inline QDBusReply<void> Pause()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("Pause"), argumentList);
    }

    inline QDBusReply<void> Play()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("Play"), argumentList);
    }

    inline QDBusReply<int> PositionGet()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("PositionGet"), argumentList);
    }

    inline QDBusReply<void> PositionSet(int in0)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0);
        return callWithArgumentList(QDBus::Block, QLatin1String("PositionSet"), argumentList);
    }

    inline QDBusReply<void> Prev()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("Prev"), argumentList);
    }

    inline QDBusReply<void> Repeat(bool in0)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0);
        return callWithArgumentList(QDBus::Block, QLatin1String("Repeat"), argumentList);
    }

    inline QDBusReply<void> Stop()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("Stop"), argumentList);
    }

    inline QDBusReply<int> VolumeGet()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("VolumeGet"), argumentList);
    }

    inline QDBusReply<void> VolumeSet(int in0)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(in0);
        return callWithArgumentList(QDBus::Block, QLatin1String("VolumeSet"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void CapsChange(int in0);
    void StatusChange(MprisDBusStatus in0);
    void TrackChange(const QVariantMap &in0);
};

#endif
