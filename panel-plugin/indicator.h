/*  $Id: indicator.h 2257 2006-12-19 19:49:00Z nick $
 *
 *  Copyright (c) 2006-2007 John Doo <john@foo.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __INDICATOR_H__
#define __INDICATOR_H__

G_BEGIN_DECLS

/* plugin structure */
typedef struct
{
    XfcePanelPlugin *plugin;

    /* panel widgets */
    GtkWidget       *menu;
    GtkWidget       *label;    
    GtkWidget       *item;
    GtkWidget       *button;
    GdkPixbuf       *button_pb;
    gchar           *icon;

    /* indicator settings */
    gchar           *setting1;
    gint             setting2;
    gboolean         setting3;
}
IndicatorPlugin;



void
indicator_save (XfcePanelPlugin *plugin,
             IndicatorPlugin    *indicator);

G_END_DECLS

#endif /* !__INDICATOR_H__ */
