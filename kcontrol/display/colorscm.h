//-----------------------------------------------------------------------------
//
// KDE Display color scheme setup module
//
// Copyright (c)  Mark Donohoe 1997
//

#ifndef __COLORSCM_H__
#define __COLORSCM_H__

#include <qobject.h>
#include <qstring.h>
#include <qstrlist.h>
#include <qcolor.h>
#include <qdialog.h>

#include <kcmodule.h>

#include "widgetcanvas.h"

class QSlider;
class QComboBox;
class QPushButton;
class QResizeEvent;
class QLineEdit;
class QPalette;
class KColorButton;
class KConfig;
class KStdDirs;

/**
 * The Desktop/Colors tab in kcontrol.
 */
class KColorScheme: public KCModule
{
    Q_OBJECT

public:
    KColorScheme(QWidget *parent, const char *name);
    ~KColorScheme();
	
    virtual void load();
    virtual void save();
    virtual void defaults();

signals:
    void changed(bool);

private slots:
    void sliderValueChanged(int val);
    void slotSave();
    void slotAdd();
    void slotRemove();
    void slotSelectColor(const QColor &col);
    void slotWidgetColor(int);
    void slotColorForWidget(int, const QColor &);
    void slotPreviewScheme(int);

private:
    void readScheme(int index=0);
    void readSchemeNames();
    QPalette createPalette();

    int nSysSchemes;
    bool m_bChanged, useRM;

    QColor colorPushColor;
    QSlider *sb;
    QComboBox *wcCombo;
    QPushButton *saveBt, *addBt, *removeBt;
    QListBox *sList;
    QStrList *schemeList, *sFileList;

    KConfig *m_pConfig;
    KStandardDirs *m_pDirs;
    KColorButton *colorButton;
    WidgetCanvas *cs;
};


/**
 * A little dialog which prompts for a name for a color scheme.
 */
class SaveScm: public QDialog 
{
    Q_OBJECT

public:
    SaveScm(QWidget *parent, const char *name);
	
    QLineEdit* nameLine;
    QPushButton* ok;
    QPushButton* cancel;
};

#endif
