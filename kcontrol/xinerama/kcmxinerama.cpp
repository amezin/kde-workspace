/**
 * kcmxinerama.cpp
 *
 * Copyright (c) 2002-2004 George Staikos <staikos@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "kcmxinerama.h"
#include <kaboutdata.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kwin.h>
#include <QtDBus/QtDBus>

#include <QCheckBox>
#include <qdesktopwidget.h>
#include <QLayout>
#include <QLabel>
#include <QComboBox>
#include <QColor>
#include <QPushButton>
//Added by qt3to4:
#include <QFrame>
#include <QGridLayout>


KCMXinerama::KCMXinerama(KInstance *inst, QWidget *parent )
  : KCModule(inst, parent) {

	KAboutData *about =
	new KAboutData(I18N_NOOP("kcmxinerama"),
			I18N_NOOP("KDE Multiple Monitor Configurator"),
			0, 0, KAboutData::License_GPL,
			I18N_NOOP("(c) 2002-2003 George Staikos"));

	about->addAuthor("George Staikos", 0, "staikos@kde.org");
	setAboutData( about );

	setQuickHelp( i18n("<h1>Multiple Monitors</h1> This module allows you to configure KDE support"
     " for multiple monitors."));

	config = new KConfig("kdeglobals", false, false);
	ksplashrc = new KConfig("ksplashrc", false, false);

	_timer.setSingleShot(true);
	connect(&_timer, SIGNAL(timeout()), this, SLOT(clearIndicator()));

	QGridLayout *grid = new QGridLayout(this );
        grid->setMargin( KDialog::marginHint() );
        grid->setSpacing( KDialog::spacingHint() );

	// Setup the panel
	_displays = QApplication::desktop()->numScreens();

	if (QApplication::desktop()->isVirtualDesktop()) {
		QStringList dpyList;
		xw = new XineramaWidget(this);
		grid->addWidget(xw, 0, 0);

		xw->headTable->setNumRows(_displays);

		for (int i = 0; i < _displays; i++) {
			QString l = i18n("Display %1", i+1);
			QRect geom = QApplication::desktop()->screenGeometry(i);
			xw->_unmanagedDisplay->addItem(l);
			xw->_ksplashDisplay->addItem(l);
			dpyList.append(l);
			xw->headTable->setText(i, 0, QString::number(geom.x()));
			xw->headTable->setText(i, 1, QString::number(geom.y()));
			xw->headTable->setText(i, 2, QString::number(geom.width()));
			xw->headTable->setText(i, 3, QString::number(geom.height()));
		}

		xw->_unmanagedDisplay->addItem(i18n("Display Containing the Pointer"));

		xw->headTable->setRowLabels(dpyList);

		connect(xw->_ksplashDisplay, SIGNAL(activated(int)),
			this, SLOT(windowIndicator(int)));
		connect(xw->_unmanagedDisplay, SIGNAL(activated(int)),
			this, SLOT(windowIndicator(int)));
		connect(xw->_identify, SIGNAL(clicked()),
			this, SLOT(indicateWindows()));

		connect(xw, SIGNAL(configChanged()), this, SLOT(changed()));
	} else { // no Xinerama
		QLabel *ql = new QLabel(i18n("<qt><p>This module is only for configuring systems with a single desktop spread across multiple monitors. You do not appear to have this configuration.</p></qt>"), this);
		grid->addWidget(ql, 0, 0);
	}

	grid->activate();

	load();
}

KCMXinerama::~KCMXinerama() {
	_timer.stop();
	delete ksplashrc;
	ksplashrc = 0;
	delete config;
	config = 0;
	clearIndicator();
}

#define KWIN_XINERAMA              "XineramaEnabled"
#define KWIN_XINERAMA_MOVEMENT     "XineramaMovementEnabled"
#define KWIN_XINERAMA_PLACEMENT    "XineramaPlacementEnabled"
#define KWIN_XINERAMA_MAXIMIZE     "XineramaMaximizeEnabled"
#define KWIN_XINERAMA_FULLSCREEN   "XineramaFullscreenEnabled"

void KCMXinerama::load() {
	if (QApplication::desktop()->isVirtualDesktop()) {
		int item = 0;
		config->setGroup("Windows");
		xw->_enableXinerama->setChecked(config->readEntry(KWIN_XINERAMA, true));
		xw->_enableResistance->setChecked(config->readEntry(KWIN_XINERAMA_MOVEMENT, true));
		xw->_enablePlacement->setChecked(config->readEntry(KWIN_XINERAMA_PLACEMENT, true));
		xw->_enableMaximize->setChecked(config->readEntry(KWIN_XINERAMA_MAXIMIZE, true));
		xw->_enableFullscreen->setChecked(config->readEntry(KWIN_XINERAMA_FULLSCREEN, true));
		item = config->readEntry("Unmanaged", QApplication::desktop()->primaryScreen());
		if ((item < 0 || item >= _displays) && (item != -3))
			xw->_unmanagedDisplay->setCurrentIndex(QApplication::desktop()->primaryScreen());
		else if (item == -3) // pointer warp
			xw->_unmanagedDisplay->setCurrentIndex(_displays);
		else	xw->_unmanagedDisplay->setCurrentIndex(item);

		ksplashrc->setGroup("Xinerama");
		item = ksplashrc->readEntry("KSplashScreen", QApplication::desktop()->primaryScreen());
		if (item < 0 || item >= _displays)
			xw->_ksplashDisplay->setCurrentIndex(QApplication::desktop()->primaryScreen());
		else xw->_ksplashDisplay->setCurrentIndex(item);

	}
	emit changed(false);
}


void KCMXinerama::save() {
	if (QApplication::desktop()->isVirtualDesktop()) {
		config->setGroup("Windows");
		config->writeEntry(KWIN_XINERAMA,
					xw->_enableXinerama->isChecked());
		config->writeEntry(KWIN_XINERAMA_MOVEMENT,
					xw->_enableResistance->isChecked());
		config->writeEntry(KWIN_XINERAMA_PLACEMENT,
					xw->_enablePlacement->isChecked());
		config->writeEntry(KWIN_XINERAMA_MAXIMIZE,
					xw->_enableMaximize->isChecked());
		config->writeEntry(KWIN_XINERAMA_FULLSCREEN,
					xw->_enableFullscreen->isChecked());
		int item = xw->_unmanagedDisplay->currentIndex();
		config->writeEntry("Unmanaged", item == _displays ? -3 : item);
		config->sync();

		QDBusInterface kwin("org.kde.kwin", "/", "org.kde.KWin");
		kwin.call("reconfigure");

		ksplashrc->setGroup("Xinerama");
		ksplashrc->writeEntry("KSplashScreen", xw->_enableXinerama->isChecked() ? xw->_ksplashDisplay->currentIndex() : -2 /* ignore Xinerama */);
		ksplashrc->sync();
	}

	KMessageBox::information(this, i18n("Some settings may affect only newly started applications."), i18n("KDE Multiple Monitors"), "nomorexineramaplease");

	emit changed(false);
}

void KCMXinerama::defaults() {
	if (QApplication::desktop()->isVirtualDesktop()) {
		xw->_enableXinerama->setChecked(true);
		xw->_enableResistance->setChecked(true);
		xw->_enablePlacement->setChecked(true);
		xw->_enableMaximize->setChecked(true);
		xw->_enableFullscreen->setChecked(true);
		xw->_unmanagedDisplay->setCurrentIndex(
				QApplication::desktop()->primaryScreen());
		xw->_ksplashDisplay->setCurrentIndex(
				QApplication::desktop()->primaryScreen());
		emit changed(true);
	} else {
		emit changed(false);
	}
}

void KCMXinerama::indicateWindows() {
	_timer.stop();

	clearIndicator();
	for (int i = 0; i < _displays; i++)
		_indicators.append(indicator(i));

	_timer.start(1500);
}

void KCMXinerama::windowIndicator(int dpy) {
	if (dpy >= _displays)
		return;

	_timer.stop();

	clearIndicator();
	_indicators.append(indicator(dpy));

	_timer.start(1500);
}

QWidget *KCMXinerama::indicator(int dpy) {
	QLabel *si = new QLabel(QString::number(dpy+1), 0, "Screen Indicator", Qt::X11BypassWindowManagerHint);

	QFont fnt = KGlobalSettings::generalFont();
	fnt.setPixelSize(100);
	si->setFont(fnt);
	si->setFrameStyle(QFrame::Panel);
	si->setFrameShadow(QFrame::Plain);
	si->setAlignment(Qt::AlignCenter);

	QPoint screenCenter(QApplication::desktop()->screenGeometry(dpy).center());
	QRect targetGeometry(QPoint(0,0), si->sizeHint());
        targetGeometry.moveCenter(screenCenter);
	si->setGeometry(targetGeometry);
	si->show();

	return si;
}

void KCMXinerama::clearIndicator() {
	qDeleteAll(_indicators);
	_indicators.clear();
}

extern "C" {
        KDE_EXPORT KCModule *create_xinerama(QWidget *parent, const char *name) {
            KInstance *inst = new KInstance( "kcmxinerama" );
	    return new KCMXinerama(inst, parent);
        }
}


#include "kcmxinerama.moc"

