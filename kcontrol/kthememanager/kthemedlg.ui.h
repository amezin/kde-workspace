/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/
#include <krun.h>

void KThemeDlg::startKonqui( const QString & url )
{
    (void) new KRun(url);
}


void KThemeDlg::startBackground()
{
    KRun::runCommand("kcmshell background");
}


void KThemeDlg::startColors()
{
    KRun::runCommand("kcmshell colors");
}


void KThemeDlg::startStyle()
{
    KRun::runCommand("kcmshell style");
}


void KThemeDlg::startIcons()
{
    KRun::runCommand("kcmshell icons");
}

void KThemeDlg::startFonts()
{
   KRun::runCommand("kcmshell fonts");
}


void KThemeDlg::startSaver()
{
    KRun::runCommand("kcmshell screensaver");
}
