//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
//

#ifndef __LOCKENG_H__
#define __LOCKENG_H__

#include <qwidget.h>
#include <kprocess.h>
#include "lockdlg.h"

//===========================================================================
//
// Screen saver handling process.  Handles screensaver window,
// starting screensaver hacks, and password entry.
//
class LockProcess
    : public QWidget
{
    Q_OBJECT
public:
    LockProcess();
    ~LockProcess();

    void lock();

    void defaultSave();
    
    void dontLock();

    void quitSaver();
    
protected:
    virtual bool x11Event(XEvent *);
    virtual void timerEvent(QTimerEvent *);

protected slots:
    void passwordChecked(KProcess *);
    void hackExited(KProcess *);
    void slotStart();
    void sigtermPipeSignal();

protected:
    void configure();
    enum State { Saving, Password };
    void readSaver();
    void createSaverWindow();
    void hideSaverWindow();
    void saveVRoot();
    void setVRoot(Window win, Window rw);
    void removeVRoot(Window win);
    bool grabKeyboard();
    bool grabMouse();
    bool grabInput();
    void ungrabInput();
    void xdmFifoCmd(const char *cmd);
    void startSaver();
    void stopSaver();
    bool startHack();
    void stopHack();
    void showPassDlg();
    void hidePassDlg();
    void setPassDlgTimeout(int t);
    void killPassDlgTimeout();
    void startCheckPassword();
    bool handleKeyPress(XKeyEvent *xke);
    void setupSignals();

protected:
    bool        mEnabled;
    bool        mLock;
    int         mPriority;
    bool        mLockOnce;
    State       mState;
    PasswordDlg *mPassDlg;
    Colormap    mColorMap;
    int         mHidePassTimerId;
    int         mCheckPassTimerId;
    KProcess    mPassProc;
    KProcess    mHackProc;
    bool        mCheckingPass;
    int         mRootWidth;
    int         mRootHeight;
    QString     mSaverExec;
    QString	mSaver;
    QString	mXdmFifoName;
};

#endif

