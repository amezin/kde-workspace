/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2006 Lubos Lunak <l.lunak@kde.org>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/

#include "toplevel.h"

#include "scene.h"

namespace KWinInternal
{

Toplevel::Toplevel( Workspace* ws )
    : id( None )
    , wspace( ws )
    , damage_handle( None )
    , window_pixmap( None )
    , vis( None )
    {
    }

Toplevel::~Toplevel()
    {
    if( window_pixmap != None )
        XFreePixmap( display(), window_pixmap );
    if( scene != NULL )
        scene->windowDeleted( this );
    }

#ifndef NDEBUG
kdbgstream& operator<<( kdbgstream& stream, const Toplevel* cl )
    {
    if( cl == NULL )
        return stream << "\'NULL\'";
    cl->debug( stream );
    return stream;
    }

kdbgstream& operator<<( kdbgstream& stream, const ToplevelList& list )
    {
    stream << "LIST:(";
    bool first = true;
    for( ToplevelList::ConstIterator it = list.begin();
         it != list.end();
         ++it )
        {
        if( !first )
            stream << ":";
        first = false;
        stream << *it;
        }
    stream << ")";
    return stream;
    }

kdbgstream& operator<<( kdbgstream& stream, const ConstToplevelList& list )
    {
    stream << "LIST:(";
    bool first = true;
    for( ConstToplevelList::ConstIterator it = list.begin();
         it != list.end();
         ++it )
        {
        if( !first )
            stream << ":";
        first = false;
        stream << *it;
        }
    stream << ")";
    return stream;
    }
#endif


} // namespace

#include "toplevel.moc"
