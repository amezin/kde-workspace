/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#include "utils.h"
#include "workspace.h"
#include "client.h"
#include "unmanaged.h"
#include "effects.h"
#include "scene.h"
#include "scene_basic.h"
#include "scene_xrender.h"
#include "scene_opengl.h"

namespace KWinInternal
{

//****************************************
// Workspace
//****************************************

#if defined( HAVE_XCOMPOSITE ) && defined( HAVE_XDAMAGE ) && defined( HAVE_XFIXES )
void Workspace::setupCompositing()
    {
    if( !options->useTranslucency )
        return;
    if( !Extensions::compositeAvailable() || !Extensions::damageAvailable())
        return;
    if( scene != NULL )
        return;
    compositeTimer.start( 20 );
    XCompositeRedirectSubwindows( display(), rootWindow(), CompositeRedirectManual );
//    scene = new SceneBasic( this );
//    scene = new SceneXrender( this );
    scene = new SceneOpenGL( this );
    effects = new EffectsHandler( this );
    addDamage( 0, 0, displayWidth(), displayHeight());
    foreach( Client* c, clients )
        c->setupCompositing();
    foreach( Unmanaged* c, unmanaged )
        c->setupCompositing();
    foreach( Client* c, clients )
        scene->windowAdded( c );
    foreach( Unmanaged* c, unmanaged )
        scene->windowAdded( c );
    }

void Workspace::finishCompositing()
    {
    if( scene == NULL )
        return;
    foreach( Client* c, clients )
        scene->windowDeleted( c );
    foreach( Unmanaged* c, unmanaged )
        scene->windowDeleted( c );
    foreach( Client* c, clients )
        c->finishCompositing();
    foreach( Unmanaged* c, unmanaged )
        c->finishCompositing();
    XCompositeUnredirectSubwindows( display(), rootWindow(), CompositeRedirectManual );
    compositeTimer.stop();
    delete effects;
    effects = NULL;
    delete scene;
    scene = NULL;
    damage_region = QRegion();
    for( ClientList::ConstIterator it = clients.begin();
         it != clients.end();
         ++it )
        { // forward all opacity values to the frame in case there'll be other CM running
        if( (*it)->opacity() != 1.0 )
            {
            NETWinInfo i( display(), (*it)->frameId(), rootWindow(), 0 );
            i.setOpacity( static_cast< unsigned long >((*it)->opacity() * 0xffffffff ));
            }
        }
    }

void Workspace::addDamage( int x, int y, int w, int h )
    {
    if( !compositing())
        return;
    damage_region += QRegion( x, y, w, h );
    }

void Workspace::addDamage( const QRect& r )
    {
    if( !compositing())
        return;
    damage_region += r;
    }

void Workspace::addDamage( Toplevel* c, int x, int y, int w, int h )
    {
    if( !compositing())
        return;
    addDamage( c, QRect( x, y, w, h ));
    }

void Workspace::addDamage( Toplevel* c, const QRect& r )
    {
    if( !compositing())
        return;
    QRegion r2( r );
    scene->transformWindowDamage( c, r2 );
    damage_region += r2;
    }

void Workspace::compositeTimeout()
    {
    if( damage_region.isEmpty()) // no damage
        return;
    ToplevelList windows;
    Window* children;
    unsigned int children_count;
    Window dummy;
    XQueryTree( display(), rootWindow(), &dummy, &dummy, &children, &children_count );
    for( unsigned int i = 0;
         i < children_count;
         ++i )
        {
        if( Client* c = findClient( FrameIdMatchPredicate( children[ i ] )))
            {
            if( c->isShown( true ) && c->isOnCurrentDesktop())
                windows.append( c );
            }
        else if( Unmanaged* c = findUnmanaged( HandleMatchPredicate( children[ i ] )))
            windows.append( c );
        }
    scene->paint( damage_region, windows );
    damage_region = QRegion();
    }

//****************************************
// Toplevel
//****************************************

void Toplevel::setupCompositing()
    {
    if( !compositing())
        return;
    if( damage_handle != None )
        return;
    damage_handle = XDamageCreate( display(), handle(), XDamageReportRawRectangles );
    damage_region = QRegion( 0, 0, width(), height());
    }

void Toplevel::finishCompositing()
    {
    if( damage_handle == None )
        return;
    XDamageDestroy( display(), damage_handle );
    damage_handle = None;
    if( window_pixmap != None )
        XFreePixmap( display(), window_pixmap );
    window_pixmap = None;
    damage_region = QRegion();
    }

void Toplevel::resetWindowPixmap()
    {
    if( !compositing())
        return;
    if( window_pixmap != None )
        XFreePixmap( display(), window_pixmap );
    window_pixmap = None;
    }

Pixmap Toplevel::windowPixmap() const
    {
    if( window_pixmap == None && compositing())
        window_pixmap = XCompositeNameWindowPixmap( display(), handle());
    return window_pixmap;
    }

void Toplevel::damageNotifyEvent( XDamageNotifyEvent* e )
    {
    addDamage( e->area.x, e->area.y, e->area.width, e->area.height );
    }

void Toplevel::addDamage( const QRect& r )
    {
    addDamage( r.x(), r.y(), r.width(), r.height());
    }

void Toplevel::addDamage( int x, int y, int w, int h )
    {
    if( !compositing())
        return;
    QRect r( x, y, w, h );
    damage_region += r;
    r.translate( this->x(), this->y());
    // this could be possibly optimized to damage Workspace only if the toplevel
    // is actually visible there and not obscured by something, but I guess
    // that's not really worth it
    workspace()->addDamage( this, r );
    }

void Toplevel::resetDamage()
    {
    damage_region = QRegion();
    }

#endif

} // namespace
