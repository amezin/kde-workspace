/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcmdisplay.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include <qstring.h>
#include <qstringlist.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qpixmap.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qmap.h>
#include <qdragobject.h>

#include <kapp.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstddirs.h>
#include <kbuttonbox.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include <bgdialogs.h>
#include <bgrender.h>


/*** KProgramSelectDialog ***/


KProgramSelectDialog::KProgramSelectDialog(QWidget *parent, char *name)
    : QDialog(parent, name, true)
{
    QVBoxLayout *top = new QVBoxLayout(this);
    top->setSpacing(10);
    top->setMargin(10);

    QLabel *lbl = new QLabel(i18n("The programs below are available:"), this);
    top->addWidget(lbl);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
    top->addWidget(frame);
    QVBoxLayout *vbox = new QVBoxLayout(frame);
    vbox->setMargin(10);
    vbox->setSpacing(10);

    // Create the listview
    m_ListView = new QListView(frame);
    m_ListView->addColumn("");
    m_ListView->setColumnAlignment(0, AlignCenter);
    m_ListView->addColumn(i18n("Program"));
    m_ListView->addColumn(i18n("Comment"));
    m_ListView->addColumn(i18n("Refresh"));
    m_ListView->setAllColumnsShowFocus(true);
    m_ListView->setItemMargin(2);

    // Fill it
    QStringList lst = KBackgroundProgram::list();
    QStringList::Iterator it;
    for (it=lst.begin(); it != lst.end(); it++)
	updateItem(*it);

    // Make the listview a little broader
    m_ListView->setMinimumWidth(m_ListView->sizeHint().width() + 20);
    m_ListView->setFixedSize(m_ListView->width(), 200);

    connect(m_ListView, SIGNAL(clicked(QListViewItem *)),
	    SLOT(slotItemClicked(QListViewItem *)));
    connect(m_ListView, SIGNAL(doubleClicked(QListViewItem *)),
	    SLOT(slotItemDoubleClicked(QListViewItem *)));
    vbox->addWidget(m_ListView);

    // Add/Remove/Modify buttons
    QHBoxLayout *hbox = new QHBoxLayout(vbox);
    QPushButton *but = new QPushButton(i18n("&Add"), frame);
    hbox->addWidget(but);
    connect(but, SIGNAL(clicked()), SLOT(slotAdd()));
    but = new QPushButton(i18n("&Remove"), frame);
    hbox->addWidget(but);
    connect(but, SIGNAL(clicked()), SLOT(slotRemove()));
    but = new QPushButton(i18n("&Modify"), frame);
    hbox->addWidget(but);
    connect(but, SIGNAL(clicked()), SLOT(slotModify()));

    // OK, Cancel button
    KButtonBox *bbox = new KButtonBox(this);
    but = bbox->addButton(i18n("&Help"));
    connect(but, SIGNAL(clicked()), SLOT(slotHelp()));
    bbox->addStretch();
    but = bbox->addButton(i18n("&OK"));
    connect(but, SIGNAL(clicked()), SLOT(accept()));
    but = bbox->addButton(i18n("&Cancel"));
    connect(but, SIGNAL(clicked()), SLOT(reject()));
    bbox->layout();
    top->addSpacing(10);
    top->addWidget(bbox);
}

	
/*
 * Set the current program.
 */
void KProgramSelectDialog::setCurrent(QString name)
{
    if (m_Items.contains(name)) {
	QListViewItem *item = m_Items[name];
	m_ListView->ensureItemVisible(item);
	m_ListView->setSelected(item, true);
	m_Current = name;
    }
}


/*
 * Add/update an item in the listview.
 */
void KProgramSelectDialog::updateItem(QString name, bool select)
{
    if (m_Items.contains(name)) {
	delete m_Items[name];
	m_Items.remove(name);
    }

    KBackgroundProgram prog(name);
    if (prog.command().isEmpty() || (prog.isGlobal() && !prog.isAvailable()))
	return;

    QListViewItem *item = new QListViewItem(m_ListView);
    QPixmap pm(locate("data", "kcontrol/pics/mini-world.png"));
    if (prog.isGlobal())
	item->setPixmap(0, pm);
    else
	item->setText(0, "  ");
    item->setText(1, prog.name());
    item->setText(2, prog.comment());
    item->setText(3, i18n("%1 min.").arg(prog.refresh()));

    m_Items[name] = item;
    if (select) {
	m_ListView->ensureItemVisible(item);
	m_ListView->setSelected(item, true);
    }
}


/*
 * Add a program.
 */
void KProgramSelectDialog::slotAdd()
{
    KProgramEditDialog dlg;
    dlg.exec();
    if (dlg.result() == QDialog::Accepted) {
	m_Current = dlg.program();
	updateItem(m_Current, true);
    }
}


/*
 * Remove a program.
 */
void KProgramSelectDialog::slotRemove()
{
    if (m_Current.isEmpty())
	return;

    KBackgroundProgram prog(m_Current);
    if (prog.isGlobal()) {
	KMessageBox::sorry(this,
		i18n("Unable to remove the program! The program is global\n"
		"and can only be removed by the System Administrator.\n"),
		i18n("Cannot remove Program"));
	return;
    }
    if (KMessageBox::warningYesNo(this,
	    i18n("Are you sure you want to remove the program `%1'?")
	    .arg(prog.name())) == KMessageBox::No)
	return;

    prog.remove();
    updateItem(m_Current);
    m_Current = QString::null;
}


/*
 * Modify a program.
 */
void KProgramSelectDialog::slotModify()
{
    if (m_Current.isEmpty())
	return;

    KProgramEditDialog dlg(m_Current);
    dlg.exec();
    if (dlg.result() == QDialog::Accepted) {
	if (dlg.program() != m_Current) {
	    KBackgroundProgram prog(m_Current);
	    prog.remove();
	    updateItem(m_Current);
	}
	m_Current = dlg.program();
	updateItem(m_Current, true);
    }
}


void KProgramSelectDialog::slotHelp()
{
}


void KProgramSelectDialog::slotItemClicked(QListViewItem *item)
{
    if ( item )
        m_Current = item->text(1);
}


void KProgramSelectDialog::slotItemDoubleClicked(QListViewItem *item)
{
    if ( item )
    {
        m_Current = item->text(1);
        accept();
    }
}


/**** KProgramEditDialog ****/

KProgramEditDialog::KProgramEditDialog(QString program, QWidget *parent, char *name)
    : QDialog(parent, name, true)
{
    QVBoxLayout *top = new QVBoxLayout(this);
    top->setMargin(10);
    top->setSpacing(10);
    QLabel *lbl = new QLabel( i18n("You can fill out the form below to create "
	    "or modify a desktop paint program"), this);
    lbl->setTextFormat(Qt::RichText); // make text wrap
    top->addWidget(lbl);

    // A nice frame
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
    top->addWidget(frame);

    QGridLayout *grid = new QGridLayout(frame, 6, 2);
    grid->setSpacing(10);
    grid->setMargin(10);

    lbl = new QLabel(i18n("&Name"), frame);
    grid->addWidget(lbl, 0, 0);
    m_NameEdit = new QLineEdit(frame);
    lbl->setBuddy(m_NameEdit);
    grid->addWidget(m_NameEdit, 0, 1);

    lbl = new QLabel(i18n("&Comment"), frame);
    grid->addWidget(lbl, 1, 0);
    m_CommentEdit = new QLineEdit(frame);
    lbl->setBuddy(m_CommentEdit);
    grid->addWidget(m_CommentEdit, 1, 1);

    lbl = new QLabel(i18n("&Command"), frame);
    grid->addWidget(lbl, 2, 0);
    m_CommandEdit = new QLineEdit(frame);
    lbl->setBuddy(m_CommandEdit);
    grid->addWidget(m_CommandEdit, 2, 1);

    lbl = new QLabel(i18n("&Preview cmd"), frame);
    grid->addWidget(lbl, 3, 0);
    m_PreviewEdit = new QLineEdit(frame);
    lbl->setBuddy(m_PreviewEdit);
    grid->addWidget(m_PreviewEdit, 3, 1);

    lbl = new QLabel(i18n("&Executable"), frame);
    grid->addWidget(lbl, 4, 0);
    m_ExecEdit = new QLineEdit(frame);
    lbl->setBuddy(m_ExecEdit);
    grid->addWidget(m_ExecEdit, 4, 1);

    lbl = new QLabel(i18n("&Refresh time"), frame);
    grid->addWidget(lbl, 5, 0);
    m_RefreshEdit = new QSpinBox(frame);
    m_RefreshEdit->setRange(5, 60);
    m_RefreshEdit->setSteps(5, 10);
    m_RefreshEdit->setSuffix(i18n(" minutes"));
    m_RefreshEdit->setFixedSize(m_RefreshEdit->sizeHint());
    lbl->setBuddy(m_RefreshEdit);
    grid->addWidget(m_RefreshEdit, 5, 1, AlignLeft);

    // Buttons
    KButtonBox *bbox = new KButtonBox(this);
    QPushButton *but = bbox->addButton(i18n("&Help"));
    connect(but, SIGNAL(clicked()), SLOT(slotHelp()));
    bbox->addStretch();
    but = bbox->addButton(i18n("&OK"));
    connect(but, SIGNAL(clicked()), SLOT(slotOK()));
    but = bbox->addButton(i18n("&Cancel"));
    connect(but, SIGNAL(clicked()), SLOT(reject()));
    bbox->layout();
    top->addWidget(bbox);
    // Make the dialog a little broader
    bbox->setMinimumWidth(bbox->sizeHint().width() + 80);

    m_Program = program;
    if (m_Program.isEmpty()) {
	KBackgroundProgram prog(i18n("New Command"));
	int i = 1;
	while (!prog.command().isEmpty())
	    prog.load(i18n("New Command <%1>").arg(i++));
	m_NameEdit->setText(prog.name());
	m_NameEdit->setSelection(0, 100);
	m_RefreshEdit->setValue(15);
	return;
    }

    // Fill in the fields
    m_NameEdit->setText(m_Program);
    KBackgroundProgram prog(m_Program);
    m_CommentEdit->setText(prog.comment());
    m_ExecEdit->setText(prog.executable());
    m_CommandEdit->setText(prog.command());
    m_PreviewEdit->setText(prog.previewCommand());
    m_RefreshEdit->setValue(prog.refresh());
}


QString KProgramEditDialog::program()
{
    return m_NameEdit->text();
}


void KProgramEditDialog::slotHelp()
{
}


void KProgramEditDialog::slotOK()
{
    QString s = m_NameEdit->text();
    if (s.isEmpty()) {
	KMessageBox::sorry(this, i18n("You did not fill in the `Name' field.\n"
		"This is a required field."));
	return;
    }

    KBackgroundProgram prog(s);
    if ((s != m_Program) && !prog.command().isEmpty()) {
	int ret = KMessageBox::warningYesNo(this,
	    i18n("There is already a program with the name `%1'.\n"
	    "Do you want to overwrite it?").arg(s));
	if (ret != KMessageBox::Yes)
	    return;
    }

    if (m_ExecEdit->text().isEmpty()) {
	KMessageBox::sorry(this, i18n("You did not fill in the `Executable' field.\n"
		"This is a required field."));
	return;
    }
    if (m_CommandEdit->text().isEmpty()) {
	KMessageBox::sorry(this, i18n("You did not fill in the `Command' field.\n"
		"This is a required field."));
	return;
    }

    prog.setComment(m_CommentEdit->text());
    prog.setExecutable(m_ExecEdit->text());
    prog.setCommand(m_CommandEdit->text());
    prog.setPreviewCommand(m_PreviewEdit->text());
    prog.setRefresh(m_RefreshEdit->value());

    prog.writeSettings();
    accept();
}



/*** KProgramSelectDialog ***/


KPatternSelectDialog::KPatternSelectDialog(QWidget *parent, char *name)
    : QDialog(parent, name, true)
{
    QVBoxLayout *top = new QVBoxLayout(this);
    top->setSpacing(10);
    top->setMargin(10);

    QLabel *lbl = new QLabel(i18n("The patterns below are available:"), this);
    top->addWidget(lbl);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
    top->addWidget(frame);
    QVBoxLayout *vbox = new QVBoxLayout(frame);
    vbox->setMargin(10);
    vbox->setSpacing(10);

    // Create the listview
    m_ListView = new QListView(frame);
    m_ListView->addColumn("");
    m_ListView->setColumnAlignment(0, AlignCenter);
    m_ListView->addColumn(i18n("Pattern"));
    m_ListView->addColumn(i18n("Comment"));
    m_ListView->setAllColumnsShowFocus(true);
    m_ListView->setItemMargin(2);

    // Fill it
    QStringList lst = KBackgroundPattern::list();
    QStringList::Iterator it;
    for (it=lst.begin(); it != lst.end(); it++)
	updateItem(*it);

    // Make the listview a little broader
    m_ListView->setMinimumWidth(m_ListView->sizeHint().width() + 40);

    connect(m_ListView, SIGNAL(clicked(QListViewItem *)),
	    SLOT(slotItemClicked(QListViewItem *)));
    connect(m_ListView, SIGNAL(doubleClicked(QListViewItem *)),
	    SLOT(slotItemDoubleClicked(QListViewItem *)));
    vbox->addWidget(m_ListView);

    // Add/Remove/Modify buttons
    QHBoxLayout *hbox = new QHBoxLayout(vbox);
    QPushButton *but = new QPushButton(i18n("&Add"), frame);
    hbox->addWidget(but);
    connect(but, SIGNAL(clicked()), SLOT(slotAdd()));
    but = new QPushButton(i18n("&Remove"), frame);
    hbox->addWidget(but);
    connect(but, SIGNAL(clicked()), SLOT(slotRemove()));
    but = new QPushButton(i18n("&Modify"), frame);
    hbox->addWidget(but);
    connect(but, SIGNAL(clicked()), SLOT(slotModify()));

    // OK, Cancel button
    KButtonBox *bbox = new KButtonBox(this);
    but = bbox->addButton(i18n("&Help"));
    connect(but, SIGNAL(clicked()), SLOT(slotHelp()));
    bbox->addStretch();
    but = bbox->addButton(i18n("&OK"));
    connect(but, SIGNAL(clicked()), SLOT(accept()));
    but = bbox->addButton(i18n("&Cancel"));
    connect(but, SIGNAL(clicked()), SLOT(reject()));
    bbox->layout();
    top->addSpacing(10);
    top->addWidget(bbox);
}

	
/*
 * Set the current pattern.
 */
void KPatternSelectDialog::setCurrent(QString name)
{
    if (m_Items.contains(name)) {
	QListViewItem *item = m_Items[name];
	m_ListView->ensureItemVisible(item);
	m_ListView->setSelected(item, true);
	m_Current = name;
    }
}


/*
 * Add/update an item in the listview.
 */
void KPatternSelectDialog::updateItem(QString name, bool select)
{
    if (m_Items.contains(name)) {
	delete m_Items[name];
	m_Items.remove(name);
    }

    KBackgroundPattern pat(name);
    if (pat.pattern().isEmpty() || (pat.isGlobal() && !pat.isAvailable()))
	return;

    QListViewItem *item = new QListViewItem(m_ListView);
    QPixmap pm(locate("data", "kcontrol/pics/mini-world.png"));
    if (pat.isGlobal())
	item->setPixmap(0, pm);
    else
	item->setText(0, "  ");
    item->setText(1, pat.name());
    item->setText(2, pat.comment());

    m_Items[name] = item;
    if (select) {
	m_ListView->ensureItemVisible(item);
	m_ListView->setSelected(item, true);
    }
}


/*
 * Add a pattern
 */
void KPatternSelectDialog::slotAdd()
{
    KPatternEditDialog dlg;
    dlg.exec();
    if (dlg.result() == QDialog::Accepted) {
	m_Current = dlg.pattern();
	updateItem(m_Current, true);
    }
}


/*
 * Remove a pattern.
 */
void KPatternSelectDialog::slotRemove()
{
    if (m_Current.isEmpty())
	return;

    KBackgroundPattern pat(m_Current);
    if (pat.isGlobal()) {
	KMessageBox::sorry(this,
		i18n("Unable to remove the pattern! The pattern is global\n"
		"and can only be removed by the System Administrator.\n"),
		i18n("Cannot remove Pattern"));
	return;
    }
    if (KMessageBox::warningYesNo(this,
	    i18n("Are you sure you want to remove the pattern `%1'?")
	    .arg(pat.name())) == KMessageBox::No)
	return;

    pat.remove();
    updateItem(m_Current);
    m_Current = QString::null;
}


/*
 * Modify a pattern.
 */
void KPatternSelectDialog::slotModify()
{
    if (m_Current.isEmpty())
	return;

    KPatternEditDialog dlg(m_Current);
    dlg.exec();
    if (dlg.result() == QDialog::Accepted) {
	if (dlg.pattern() != m_Current) {
	    KBackgroundPattern pat(m_Current);
	    pat.remove();
	    updateItem(m_Current);
	}
	m_Current = dlg.pattern();
	updateItem(m_Current, true);
    }
}


void KPatternSelectDialog::slotHelp()
{
}


void KPatternSelectDialog::slotItemClicked(QListViewItem *item)
{
    m_Current = item->text(1);
    kdDebug() << "current: " << m_Current << endl;
}


void KPatternSelectDialog::slotItemDoubleClicked(QListViewItem *item)
{
    m_Current = item->text(1);
    accept();
}

/**** KPatternEditDialog ****/

KPatternEditDialog::KPatternEditDialog(QString pattern, QWidget *parent,
	char *name) : QDialog(parent, name, true)
{
    QVBoxLayout *top = new QVBoxLayout(this);
    top->setMargin(10);
    top->setSpacing(10);
    QLabel *lbl = new QLabel( i18n("You can fill out the form below to create "
	    "or modify a desktop pattern"), this);
    lbl->setTextFormat(Qt::RichText); // make text wrap
    top->addWidget(lbl);

    // A nice frame
    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
    top->addWidget(frame);

    QGridLayout *grid = new QGridLayout(frame, 3, 2);
    grid->setSpacing(10);
    grid->setMargin(10);

    lbl = new QLabel(i18n("&Name"), frame);
    grid->addWidget(lbl, 0, 0);
    m_NameEdit = new QLineEdit(frame);
    lbl->setBuddy(m_NameEdit);
    grid->addWidget(m_NameEdit, 0, 1);

    lbl = new QLabel(i18n("&Comment"), frame);
    grid->addWidget(lbl, 1, 0);
    m_CommentEdit = new QLineEdit(frame);
    lbl->setBuddy(m_CommentEdit);
    grid->addWidget(m_CommentEdit, 1, 1);

    lbl = new QLabel(i18n("&Image"), frame);
    grid->addWidget(lbl, 2, 0);
    QHBoxLayout *hbox = new QHBoxLayout();
    grid->addLayout(hbox, 2, 1);
    m_FileEdit = new QLineEdit(frame);
    lbl->setBuddy(m_FileEdit);
    hbox->addWidget(m_FileEdit);
    QPushButton *but = new QPushButton(i18n("&Browse"), frame);
    connect(but, SIGNAL(clicked()), SLOT(slotBrowse()));
    hbox->addWidget(but);

    // Buttons
    KButtonBox *bbox = new KButtonBox(this);
    but = bbox->addButton(i18n("&Help"));
    connect(but, SIGNAL(clicked()), SLOT(slotHelp()));
    bbox->addStretch();
    but = bbox->addButton(i18n("&OK"));
    connect(but, SIGNAL(clicked()), SLOT(slotOK()));
    but = bbox->addButton(i18n("&Cancel"));
    connect(but, SIGNAL(clicked()), SLOT(reject()));
    bbox->layout();
    top->addWidget(bbox);
    // Make the dialog a little broader
    bbox->setMinimumWidth(bbox->sizeHint().width() + 80);

    m_Pattern = pattern;
    if (m_Pattern.isEmpty()) {
	KBackgroundPattern pat(i18n("New Pattern"));
	int i = 1;
	while (!pat.pattern().isEmpty())
	    pat.load(i18n("New Pattern <%1>").arg(i++));
	m_NameEdit->setText(pat.name());
	m_NameEdit->setSelection(0, 100);
	return;
    }

    // Fill in the fields
    m_NameEdit->setText(m_Pattern);
    KBackgroundPattern pat(m_Pattern);
    m_CommentEdit->setText(pat.comment());
    m_FileEdit->setText(pat.pattern());
}


void KPatternEditDialog::slotHelp()
{
}


void KPatternEditDialog::slotBrowse()
{
    KURL url = KFileDialog::getOpenURL();
    if (url.isEmpty())
	return;

    m_FileEdit->setText(url.url());
}


QString KPatternEditDialog::pattern()
{
    return m_NameEdit->text();
}


void KPatternEditDialog::slotOK()
{
    QString s = m_NameEdit->text();
    if (s.isEmpty()) {
	KMessageBox::sorry(this, i18n("You did not fill in the `Name' field.\n"
		"This is a required field."));
	return;
    }

    KBackgroundPattern pat(s);
    if ((s != m_Pattern) && !pat.pattern().isEmpty()) {
	int ret = KMessageBox::warningYesNo(this,
	    i18n("There is already a pattern with the name `%1'.\n"
	    "Do you want to overwrite it?").arg(s));
	if (ret != KMessageBox::Yes)
	    return;
    }

    if (m_FileEdit->text().isEmpty()) {
	KMessageBox::sorry(this, i18n("You did not fill in the `Image' field.\n"
		"This is a required field."));
	return;
    }

    pat.setComment(m_CommentEdit->text());
    pat.setPattern(m_FileEdit->text());
    pat.writeSettings();

    accept();
}


/**** KMultiWallpaperList ****/

KMultiWallpaperList::KMultiWallpaperList(QWidget *parent, char *name)
	: QListBox(parent, name)
{
    setAcceptDrops(true);
}


void KMultiWallpaperList::dragEnterEvent(QDragEnterEvent *ev)
{
    ev->accept(QUriDrag::canDecode(ev));
}


void KMultiWallpaperList::dropEvent(QDropEvent *ev)
{
    QStringList files;
    QUriDrag::decodeLocalFiles(ev, files);
    insertStringList(files);
}


/**** KMultiWallpaperDialog ****/

KMultiWallpaperDialog::KMultiWallpaperDialog(KBackgroundSettings *setts,
	QWidget *parent, char *name)
	: QDialog(parent, name, true)
{
    m_pSettings = setts;
    m_Wallpapers = m_pSettings->wallpaperList();
    m_Interval = m_pSettings->wallpaperChangeInterval();
    m_Mode = m_pSettings->multiWallpaperMode();

    QVBoxLayout *top = new QVBoxLayout(this);
    top->setSpacing(10);
    top->setMargin(10);
    top->addSpacing(10);

    QHBoxLayout *hbox = new QHBoxLayout();
    top->addLayout(hbox);

    QLabel *lbl = new QLabel(i18n("&Interval"), this);
    hbox->addWidget(lbl);
    m_pIntervalEdit = new QSpinBox(this);
    m_pIntervalEdit->setRange(5, 240);
    m_pIntervalEdit->setSteps(5, 15);
    m_pIntervalEdit->setValue(QMAX(5,m_Interval));
    m_pIntervalEdit->setSuffix(i18n(" minutes"));
    lbl->setBuddy(m_pIntervalEdit);
    hbox->addWidget(m_pIntervalEdit);
    hbox->addStretch();

    lbl = new QLabel(i18n("&Mode"), this);
    hbox->addWidget(lbl);
    m_pModeEdit = new QComboBox(this);
    m_pModeEdit->insertItem(i18n("In Order"));
    m_pModeEdit->insertItem(i18n("Random"));
    m_pModeEdit->setCurrentItem(m_Mode-1);
    lbl->setBuddy(m_pModeEdit);
    hbox->addWidget(m_pModeEdit);
    hbox->addStretch();

    top->addSpacing(10);
    lbl = new QLabel(i18n("You can select files and directories below:"), this);
    top->addWidget(lbl);

    QFrame *frame = new QFrame(this);
    frame->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
    top->addWidget(frame);
    QVBoxLayout *vbox = new QVBoxLayout(frame);
    vbox->setSpacing(10);
    vbox->setMargin(10);
    m_pListBox = new KMultiWallpaperList(frame);
    m_pListBox->setMinimumSize(QSize(300,150));
    vbox->addWidget(m_pListBox);
    m_pListBox->insertStringList(m_Wallpapers);

    hbox = new QHBoxLayout();
    vbox->addLayout(hbox);
    QPushButton *pbut = new QPushButton(i18n("&Add"), frame);
    connect(pbut, SIGNAL(clicked()), SLOT(slotAdd()));
    hbox->addWidget(pbut);
    pbut = new QPushButton(i18n("&Remove"), frame);
    connect(pbut, SIGNAL(clicked()), SLOT(slotRemove()));
    hbox->addWidget(pbut);

    KButtonBox *bbox = new KButtonBox(this);
    pbut = bbox->addButton("&Help");
    connect(pbut, SIGNAL(clicked()), SLOT(slotHelp()));
    bbox->addStretch();
    pbut = bbox->addButton(i18n("&OK"));
    connect(pbut, SIGNAL(clicked()), SLOT(slotOK()));
    pbut = bbox->addButton(i18n("&Cancel"));
    connect(pbut, SIGNAL(clicked()), SLOT(reject()));
    top->addWidget(bbox);
}


void KMultiWallpaperDialog::slotAdd()
{
    KFileDialog fileDialog(QString::null, "*", this, "fileDialog", true);

    fileDialog.setCaption(i18n("Select"));
    KFile::Mode mode = static_cast<KFile::Mode> (KFile::File |
                                                 KFile::Directory |
                                                 KFile::ExistingOnly);
    fileDialog.setMode(mode);
    fileDialog.exec();
    KURL url = fileDialog.selectedFile();
    if (url.isEmpty())
	return;

    m_pListBox->insertItem(url.url());
}


void KMultiWallpaperDialog::slotRemove()
{
    int item = m_pListBox->currentItem();
    if (item == -1)
	return;
    m_pListBox->removeItem(item);
}


void KMultiWallpaperDialog::slotHelp()
{
}


void KMultiWallpaperDialog::slotOK()
{
    QStringList lst;
    for (unsigned i=0; i<m_pListBox->count(); i++)
	lst.append(m_pListBox->text(i));
    m_pSettings->setWallpaperList(lst);
    m_pSettings->setWallpaperChangeInterval(m_pIntervalEdit->value());
    m_pSettings->setMultiWallpaperMode(m_pModeEdit->currentItem()+1);
    accept();
}


#include "bgdialogs.moc"
