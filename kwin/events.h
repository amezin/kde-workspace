/*****************************************************************
kwin - the KDE window manager

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#ifndef EVENTS_H
#define EVENTS_H

class Events
{
public:

    enum Event {
	Close,
	Iconify,
	DeIconify,
	Maximize,
	UnMaximize,
	Sticky,
	UnSticky,
	New,
	Delete,
	TransNew,
	TransDelete,
	ShadeUp,
	ShadeDown,
	MoveStart,
	MoveEnd,
	ResizeStart,
	ResizeEnd
    };
	
    static void raise( Event );
};

#endif
