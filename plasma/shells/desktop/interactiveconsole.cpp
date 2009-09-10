/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "interactiveconsole.h"

#include <QDateTime>
#include <QFile>
#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>

#include <KFileDialog>
#include <KLocale>
#include <KPushButton>
#include <KShell>
#include <KStandardGuiItem>
#include <KTextEdit>
#include <KTextBrowser>

#include <Plasma/Corona>

#include "scriptengine.h"

//TODO:
// save and restore splitter sizes
// better initial size of editor to output
// use text editor KPart for syntax highlighting?
// interative help?

InteractiveConsole::InteractiveConsole(Plasma::Corona *corona, QWidget *parent)
    : KDialog(parent),
      m_engine(new ScriptEngine(corona, this)),
      m_editor(new KTextEdit(this)),
      m_output(new KTextBrowser(this)),
      m_loadButton(new KPushButton(KStandardGuiItem::open(), this)),
      m_saveButton(new KPushButton(KStandardGuiItem::save(), this)),
      m_clearButton(new KPushButton(KIcon("edit-clear"), i18n("&Clear"), this)),
      m_executeButton(new KPushButton(KIcon("system-run"), i18n("&Run Script"), this)),
      m_fileDialog(0)
{
    setWindowTitle(KDialog::makeStandardCaption(i18n("Desktop Shell Scripting Console")));
    setAttribute(Qt::WA_DeleteOnClose);
    setButtons(KDialog::None);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);

    QWidget *widget = new QWidget(splitter);
    QVBoxLayout *editorLayout = new QVBoxLayout(widget);
    editorLayout->addWidget(m_editor);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_loadButton);
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addStretch(10);
    buttonLayout->addWidget(m_clearButton);
    buttonLayout->addWidget(m_executeButton);
    editorLayout->addLayout(buttonLayout);

    splitter->addWidget(widget);
    splitter->addWidget(m_output);
    setMainWidget(splitter);

    setInitialSize(QSize(500, 400));
    KConfigGroup cg(KGlobal::config(), "InteractiveConsole");
    restoreDialogSize(cg);

    scriptTextChanged();

    connect(m_loadButton, SIGNAL(clicked()), this, SLOT(openScriptFile()));
    connect(m_saveButton, SIGNAL(clicked()), this, SLOT(saveScript()));
    connect(m_executeButton, SIGNAL(clicked()), this, SLOT(evaluateScript()));
    connect(m_clearButton, SIGNAL(clicked()), this, SLOT(clearEditor()));
    connect(m_editor, SIGNAL(textChanged()), this, SLOT(scriptTextChanged()));
    connect(m_engine, SIGNAL(print(QString)), this, SLOT(print(QString)));
    connect(m_engine, SIGNAL(printError(QString)), this, SLOT(print(QString)));
}

InteractiveConsole::~InteractiveConsole()
{
    KConfigGroup cg(KGlobal::config(), "InteractiveConsole");
    saveDialogSize(cg);
}

void InteractiveConsole::loadScript(const QString &script)
{
    QFile file(KShell::tildeExpand(script));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        m_editor->setText(file.readAll());
    } else {
        m_output->append(i18n("Unable to load script file <b>%1</b>", script));
    }
}

void InteractiveConsole::showEvent(QShowEvent *)
{
    m_editor->setFocus();
}

void InteractiveConsole::print(const QString &string)
{
    m_output->append(string);
}

void InteractiveConsole::scriptTextChanged()
{
    const bool enable = !m_editor->document()->isEmpty();
    m_saveButton->setEnabled(enable);
    m_clearButton->setEnabled(enable);
    m_executeButton->setEnabled(enable);
}

void InteractiveConsole::openScriptFile()
{
    if (m_fileDialog) {
        delete m_fileDialog;
    }

    m_fileDialog = new KFileDialog(KUrl(), QString(), 0);
    m_fileDialog->setOperationMode(KFileDialog::Opening);
    m_fileDialog->setCaption(i18n("Open Script File"));

    QStringList mimetypes;
    mimetypes << "application/javascript";
    m_fileDialog->setMimeFilter(mimetypes);

    connect(m_fileDialog, SIGNAL(finished()), this, SLOT(openScriptUrlSelected()));
    m_fileDialog->show();
}

void InteractiveConsole::openScriptUrlSelected()
{
    if (!m_fileDialog) {
        return;
    }

    KUrl url = m_fileDialog->selectedUrl();
    m_fileDialog->deleteLater();
    m_fileDialog = 0;

    if (url.isEmpty()) {
        return;
    }

    m_editor->clear();
    m_editor->setEnabled(false);

    if (m_job) {
        m_job->kill();
    }

    m_job = KIO::get(url, KIO::Reload, KIO::HideProgressInfo);
    connect(m_job, SIGNAL(data(KIO::Job*,QByteArray)), this, SLOT(scriptFileDataRecvd(KIO::Job*,QByteArray)));
    connect(m_job, SIGNAL(result(KJob*)), this, SLOT(reenableEditor()));
}

void InteractiveConsole::scriptFileDataRecvd(KIO::Job *job, const QByteArray &data)
{
    if (job == m_job) {
        m_editor->insertPlainText(data);
    }
}

void InteractiveConsole::saveScript()
{
    if (m_fileDialog) {
        delete m_fileDialog;
    }

    m_fileDialog = new KFileDialog(KUrl(), QString(), 0);
    m_fileDialog->setOperationMode(KFileDialog::Saving);
    m_fileDialog->setCaption(i18n("Open Script File"));

    QStringList mimetypes;
    mimetypes << "application/javascript";
    m_fileDialog->setMimeFilter(mimetypes);

    connect(m_fileDialog, SIGNAL(finished()), this, SLOT(saveScriptUrlSelected()));
    m_fileDialog->show();
}

void InteractiveConsole::saveScriptUrlSelected()
{
    if (!m_fileDialog) {
        return;
    }

    KUrl url = m_fileDialog->selectedUrl();
    if (url.isEmpty()) {
        return;
    }

    m_editor->setEnabled(false);

    if (m_job) {
        m_job->kill();
    }

    m_job = KIO::put(url, -1, KIO::HideProgressInfo);
    connect(m_job, SIGNAL(dataReq(KIO::Job*,QByteArray&)), this, SLOT(scriptFileDataReq(KIO::Job*,QByteArray&)));
    connect(m_job, SIGNAL(result(KJob*)), this, SLOT(reenableEditor()));
}

void InteractiveConsole::scriptFileDataReq(KIO::Job *job, QByteArray &data)
{
    if (!m_job || m_job != job) {
        return;
    }

    data.append(m_editor->toPlainText().toLocal8Bit());
    m_job = 0;
}

void InteractiveConsole::reenableEditor()
{
    m_editor->setEnabled(true);
}

void InteractiveConsole::evaluateScript()
{
    //kDebug() << "evaluating" << m_editor->toPlainText();
    m_output->moveCursor(QTextCursor::End);
    QTextCursor cursor = m_output->textCursor();
    m_output->setTextCursor(cursor);

    QTextCharFormat format;
    format.setFontWeight(QFont::Bold);
    format.setFontUnderline(true);

    if (cursor.position() > 0) {
        cursor.insertText("\n\n");
    }

    QDateTime dt = QDateTime::currentDateTime();
    cursor.insertText(i18n("Executing script at %1", KGlobal::locale()->formatDateTime(dt)), format);

    format.setFontWeight(QFont::Normal);
    format.setFontUnderline(false);
    QTextBlockFormat block = cursor.blockFormat();
    block.setLeftMargin(10);
    cursor.insertBlock(block, format);
    QTime t;
    t.start();
    m_engine->evaluateScript(m_editor->toPlainText());

    cursor.insertText("\n\n");
    format.setFontWeight(QFont::Bold);
    // xgettext:no-c-format
    cursor.insertText(i18n("Runtime: %1ms", QString::number(t.elapsed())), format);
    block.setLeftMargin(0);
    cursor.insertBlock(block);
    m_output->ensureCursorVisible();
}

void InteractiveConsole::clearEditor()
{
    m_editor->clear();
}

#include "interactiveconsole.moc"

