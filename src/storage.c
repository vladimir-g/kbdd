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
#include <stdint.h>
#include <stdlib.h>
#include "storage.h"

#ifdef STORAGE_GHASH
#include <glib.h>
#endif

#ifdef STORAGE_GHASH
GHashTable *gStorage = NULL;
#define GINT_TO_POINTER(i) ((gpointer) (glong) (i))
#define GUINT_TO_POINTER(u) ((gpointer) (gulong) (u))
#define POINTER_TO_GUINT(u) ((gulong)(gpointer)(u))
#endif

void debug();

void _kbdd_storage_init() {
    if ( gStorage!=NULL ) return; 
#ifdef STORAGE_GHASH
    gStorage = g_hash_table_new(g_direct_hash, NULL);
//    debug();
#endif
}

void _kbdd_storage_free() {
    if ( gStorage != NULL ) {
#ifdef STORAGE_GHASH
        g_hash_table_destroy(gStorage);
#endif
    }
}

void _kbdd_storage_put(WINDOW_TYPE win, GROUP_TYPE group)
{
    if ( gStorage != NULL ) {
#ifdef STORAGE_GHASH
        gpointer key = GUINT_TO_POINTER(win);
        gpointer val = GUINT_TO_POINTER(group);
        g_hash_table_replace(gStorage, key, val);
//        debug();
    }
#endif
}

GROUP_TYPE _kbdd_storage_get(WINDOW_TYPE win)
{
    GROUP_TYPE group;
    if (gStorage != NULL) 
    {
#ifdef STORAGE_GHASH
        gpointer key = NULL;
        gpointer value = NULL;
        gpointer pWindow = GUINT_TO_POINTER(win);
        if ( g_hash_table_lookup_extended(gStorage, pWindow, &key, &value) )
        {
            group = (GROUP_TYPE)POINTER_TO_GUINT(value);
        }
        else 
        {
            return 0;
        }
#endif
    }
    return group;
}

void _kbdd_storage_remove(WINDOW_TYPE win)
{
    if (gStorage != NULL) {
#ifdef STORAGE_GHASH
        gpointer key = GUINT_TO_POINTER(win);
        g_hash_table_remove(gStorage, key);
#endif
    }
}


/*
void debug() {
    GHashTableIter iter;
    g_hash_table_iter_init (&iter, gStorage);
    int size=g_hash_table_size(gStorage);
    printf("=================\nFirst size: %d\n",size);
    uint32_t *val;
    uint32_t *key_;
    while (g_hash_table_iter_next (&iter, (gpointer) &key_, (gpointer) &val))
    {
        printf("key %u ---> %u\n",(uint32_t)key_,(uint32_t)val);
    }
    printf("=================\n");
}*/

//vim:ts=4:expandtab