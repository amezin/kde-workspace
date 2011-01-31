/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#ifndef KWIN_COMPOSITINGPREFS_H
#define KWIN_COMPOSITINGPREFS_H

#include <QString>
#include <QStringList>

#include "kwinglutils.h"
#include "kwinglobals.h"


namespace KWin
{

class CompositingPrefs
{
public:
    CompositingPrefs();
    ~CompositingPrefs();

    static bool compositingPossible();
    static QString compositingNotPossibleReason();
    bool recommendCompositing() const;
    bool enableVSync() const  { return mEnableVSync; }
    bool enableDirectRendering() const  { return mEnableDirectRendering; }
    bool strictBinding() const { return mStrictBinding; }

    void detect();

protected:

    void detectDriverAndVersion();
    void applyDriverSpecificOptions();

    bool initGLXContext();
    void deleteGLXContext();
    bool initEGLContext();
    void deleteEGLContext();


private:
    bool mRecommendCompositing;
    bool mEnableVSync;
    bool mEnableDirectRendering;
    bool mStrictBinding;

#ifdef KWIN_HAVE_OPENGL_COMPOSITING
#ifdef KWIN_HAVE_OPENGLES
    EGLDisplay mEGLDisplay;
    EGLContext mEGLContext;
    EGLSurface mEGLSurface;
#else
    GLXContext mGLContext;
#endif
    Window mGLWindow;
#endif
};

}

#endif //KWIN_COMPOSITINGPREFS_H


