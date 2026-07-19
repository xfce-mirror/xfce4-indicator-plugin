/*  Copyright (c) 2012-2013 Andrzej <ndrwrdck@gmail.com>
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

#ifndef __INDICATOR_BUTTON_H__
#define __INDICATOR_BUTTON_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <libayatana-indicator/indicator-object.h>

#include "indicator-config.h"
#include "indicator-box.h"

G_BEGIN_DECLS

#define XFCE_TYPE_INDICATOR_BUTTON (xfce_indicator_button_get_type())
G_DECLARE_FINAL_TYPE (XfceIndicatorButton, xfce_indicator_button, XFCE, INDICATOR_BUTTON, GtkToggleButton)

void                    xfce_indicator_button_set_label   (XfceIndicatorButton        *button,
                                                           GtkLabel                   *label);

void                    xfce_indicator_button_set_image   (XfceIndicatorButton        *button,
                                                           GtkImage                   *image);

void                    xfce_indicator_button_set_menu    (XfceIndicatorButton        *button,
                                                           GtkMenu                    *menu);

IndicatorObjectEntry   *xfce_indicator_button_get_entry   (XfceIndicatorButton        *button);

IndicatorObject        *xfce_indicator_button_get_io      (XfceIndicatorButton        *button);

const gchar            *xfce_indicator_button_get_io_name (XfceIndicatorButton        *button);

guint                   xfce_indicator_button_get_pos     (XfceIndicatorButton        *button);

GtkMenu                *xfce_indicator_button_get_menu    (XfceIndicatorButton        *button);

gboolean                xfce_indicator_button_is_small    (XfceIndicatorButton        *button);

gint                    xfce_indicator_button_get_button_border (XfceIndicatorButton  *button);

GtkWidget              *xfce_indicator_button_new         (IndicatorObject            *io,
                                                           const gchar                *io_name,
                                                           IndicatorObjectEntry       *entry,
                                                           XfcePanelPlugin            *plugin,
                                                           IndicatorConfig            *config);

void                    xfce_indicator_button_destroy     (XfceIndicatorButton *button);

G_END_DECLS

#endif /* !__INDICATOR_BUTTON_H__ */
