/*
 * kcmsambaimports.cpp
 *
 * Copyright (c) 2000 Alexander Neundorf <alexander.neundorf@rz.tu-ilmenau.de>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include "kcmsambaimports.h"
#include "kcmsambaimports.moc"

#include <qlayout.h>
#include <qcstring.h>
#include <klocale.h>

#include <stdio.h>
#include <iostream.h>
#include <fstream.h>

#define SCREEN_XY_OFFSET 20

ImportsView::ImportsView(QWidget * parent, KConfig *config, const char * name )
   : QWidget (parent, name)
   ,configFile(config)
   ,list(this)
{
    QBoxLayout *topLayout = new QVBoxLayout(this);
    topLayout->setAutoAdd(true);
    topLayout->setMargin(SCREEN_XY_OFFSET);
    topLayout->setSpacing(10);
    
    list.setAllColumnsShowFocus(true);
    list.setMinimumSize(425,200);
    list.addColumn(i18n("Resource"), 150);
    list.addColumn(i18n("Mounted under"), 190);
    list.addColumn(i18n("Type"), 55);
 
    timer.start(10000);
    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(updateList()));
    updateList();
};

void ImportsView::updateList()
{
   list.clear();
   char *e;
   char buf[250];
   QCString s(""),strSource, strMount, strType;
   FILE *f=popen("mount","r");
   if (f==0) return;
   do
   {
      e=fgets(buf,250,f);
      if (e!=0)
      {
         s=buf;
         if ((s.contains(" nfs ")) || (s.contains(" smbfs ")))
         {
            strSource=s.left(s.find(" on /"));
            strMount=s.mid(s.find(" on /")+4,s.length());
            if ((s.contains(" nfs ")) || (s.contains("/remote on ")))
               strType=" nfs";
            else if (s.contains(" smbfs "))
               strType=" smbfs";
            int pos(strMount.find(" type ");
            if (pos==-1) pos=strMount.find(" read/");
            strMount=strMount.left(pos);
            new QListViewItem(&list,strSource,strMount,strType);
         };
      };
   }
   while (!feof(f));
   pclose(f);
};

