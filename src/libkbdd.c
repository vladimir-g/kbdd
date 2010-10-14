/********************************************************************* 
 * Kbdd - simple per-window-keyboard layout library and deamon 
 * Copyright (C) 2010  Alexander V Vershilov and collaborators
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/Xmd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

#include "libkbdd.h"
#include "common-defs.h"

volatile int _xkbEventType;
volatile UpdateCallback    _updateCallback = NULL;
volatile void *            _updateUserdata = NULL;
volatile static Display *  _display        = NULL;

void Kbdd_init()
{
    _kbdd_storage_init(); 
}

void Kbdd_clean()
{
    _kbdd_storage_free();
}

int Kbdd_add_window(Display * display, Window window)
{
    XkbStateRec state;
    if ( XkbGetState(display, XkbUseCoreKbd, &state) == Success ) 
    {
        WINDOW_TYPE win = (WINDOW_TYPE)window;
        _kbdd_storage_put(win, state.group);
        if ( _updateCallback != NULL ) 
            _updateCallback(state.group, (void *)_updateUserdata);
    }
    return 0;
}

void Kbdd_remove_window(Window window)
{
    WINDOW_TYPE win = (WINDOW_TYPE)window;
    _kbdd_storage_remove(win);
}

int Kbdd_set_window_layout ( Display * display, Window win ) 
{
    GROUP_TYPE group = _kbdd_storage_get( (WINDOW_TYPE)win );
    int result = XkbLockGroup(display, XkbUseCoreKbd, group);
    if (result && _updateCallback != NULL) 
        _updateCallback(group, (void *)_updateUserdata);
    return result;
}

void Kbdd_update_window_layout ( Display * display, Window window, unsigned char grp ) 
{
//    XkbStateRec state;
//    if ( XkbGetState(display, XkbUseCoreKbd, &state) == Success )
//    {
//      _kbdd_storage_put(win, state.group);
//    }
    WINDOW_TYPE win = (WINDOW_TYPE) window;
    GROUP_TYPE  g   = (GROUP_TYPE)grp;
    _kbdd_storage_put(win, g);
    if ( _updateCallback != NULL ) 
        _updateCallback(g, (void *)_updateUserdata);
}

Display * Kbdd_initialize_display( )
{
    Display * display;
    int xkbEventType,xkbError,  reason_rtrn;
    char * display_name = NULL;
    int mjr = XkbMajorVersion;
    int mnr = XkbMinorVersion;
    display = XkbOpenDisplay(display_name,&xkbEventType,&xkbError, &mjr,&mnr,&reason_rtrn);
    _xkbEventType = xkbEventType;
    return display;
}

void Kbdd_setupUpdateCallback(UpdateCallback callback,void * userData ) 
{
    _updateCallback = callback;
    _updateUserdata = userData;
}

void Kbdd_initialize_listeners( Display * display )
{
    assert(display!=NULL);
    int scr = DefaultScreen( display );
    Window root_win = RootWindow( display, scr );
    XkbSelectEventDetails( display, XkbUseCoreKbd, XkbStateNotify,
                XkbAllStateComponentsMask, XkbGroupStateMask);
    XSelectInput( display, root_win, StructureNotifyMask | SubstructureNotifyMask
            | EnterWindowMask | FocusChangeMask | LeaveWindowMask );
}

void Kbdd_setDisplay(Display * display)
{
    assert(display != NULL);
    _display = display;
}

int Kbdd_default_iter(void * data)
{
    assert( _display != NULL );
    while ( XPending( (Display *)_display ) ) 
    {
        Window focused_win; 
        XkbEvent ev; 
        int revert;
        uint32_t grp;

        XNextEvent( (Display *)_display, &ev.core);
        dbg( "LIBKBDD event caught\n");
        if ( ev.type == _xkbEventType)
        {
           switch (ev.any.xkb_type)
           {
                case XkbStateNotify:
                    grp = ev.state.locked_group;
                    XGetInputFocus( (Display *)_display, &focused_win, &revert);
                    Kbdd_update_window_layout( (Display *)_display, focused_win,grp);
                    break;
                default:
                    break;
            }
        } 
        else 
        {
            switch (ev.type)
            {
                case DestroyNotify:
                    Kbdd_remove_window(ev.core.xdestroywindow.window);
                    break;
                case CreateNotify:
                    Kbdd_add_window((Display *)_display, ev.core.xcreatewindow.window);
                    break;
                case FocusIn:
                    XGetInputFocus((Display *)_display, &focused_win, &revert);
                    Kbdd_set_window_layout((Display *)_display, focused_win);
                    break;
                case FocusOut:
                    XGetInputFocus((Display *)_display, &focused_win, &revert);
                    Kbdd_set_window_layout((Display *)_display, focused_win);
                default:
                    XGetInputFocus((Display *)_display, &focused_win, &revert);
                    break;
            }
        }
    }
    return 1;
}

void * Kbdd_default_loop(Display * display) 
{
    dbg( "default loop started\n");
    if (display == NULL)
        display = (Display *)_display;
    assert(display!=NULL);

    Window focused_win;
    int revert;
    unsigned char grp;
    XkbEvent ev; 
    while ( 1 ) 
    {
        XNextEvent(display, &ev.core);
        dbg( "LIBKBDD event caught\n");
        if ( ev.type == _xkbEventType)
        {
            switch (ev.any.xkb_type)
            {
                case XkbStateNotify:
                    grp = ev.state.locked_group;
                    XGetInputFocus(display, &focused_win, &revert);
                    Kbdd_update_window_layout( display, focused_win,grp);
                    break;
                default:
                    break;
            }
        } 
        else 
        {
            switch (ev.type)
            {
                case DestroyNotify:
                    Kbdd_remove_window(ev.core.xdestroywindow.window);
                    break;
                case CreateNotify:
                    Kbdd_add_window(display, ev.core.xcreatewindow.window);
                    break;
                case FocusIn:
                    XGetInputFocus(display, &focused_win, &revert);
                    Kbdd_set_window_layout(display, focused_win);
                    break;
                case FocusOut:
                    XGetInputFocus(display, &focused_win, &revert);
                    Kbdd_set_window_layout(display, focused_win);
                default:
                    XGetInputFocus(display, &focused_win, &revert);
                    break;
            }
        }

    }
}

//vim:ts=4:expandtab
