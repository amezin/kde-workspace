/*
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 */

#ifndef __main_h__
#define __main_h__

#include <kcmodule.h>

#include "extensionInfo.h"

class QTabWidget;
class PositionTab;
class HidingTab;
class MenuTab;
class LookAndFeelTab;
class AppletTab;
class ExtensionsTab;

class KickerConfig : public KCModule
{
    Q_OBJECT

public:
    KickerConfig(QWidget *parent = 0L, const char *name = 0L);

    void load();
    void save();
    void defaults();
    QString quickHelp() const;
    const KAboutData* aboutData() const;
    
    void populateExtensionInfoList(QListView* list);
    void reloadExtensionInfo();
    void saveExtentionInfo();
    const extensionInfoList& extensionsInfo();

signals:
    void extensionInfoChanged();

public slots:
    void configChanged();

protected slots:
    void positionPanelChanged(QListViewItem*);
    void hidingPanelChanged(QListViewItem*);

private:
    QTabWidget     *tab;
    PositionTab    *positiontab;
    HidingTab      *hidingtab;
    LookAndFeelTab *lookandfeeltab;
    MenuTab        *menutab;
    AppletTab      *applettab;
    extensionInfoList m_extensionInfo;
};

#endif // __main_h__
