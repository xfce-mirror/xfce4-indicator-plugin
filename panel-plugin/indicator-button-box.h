/*  Copyright (c) 2013 Andrzej <ndrwrdck@gmail.com>
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

#ifndef __INDICATOR_BUTTON_BOX_H__
#define __INDICATOR_BUTTON_BOX_H__

#include <glib.h>
#include <gtk/gtk.h>

#include "indicator-config.h"

G_BEGIN_DECLS

GType indicator_button_box_get_type (void);

#define XFCE_TYPE_INDICATOR_BUTTON_BOX             (indicator_button_box_get_type())
#define XFCE_INDICATOR_BUTTON_BOX(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_TYPE_INDICATOR_BUTTON_BOX, IndicatorButtonBox))
#define XFCE_INDICATOR_BUTTON_BOX_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), XFCE_TYPE_INDICATOR_BUTTON_BOX, IndicatorButtonBoxClass))
#define XFCE_IS_INDICATOR_BUTTON_BOX(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_TYPE_INDICATOR_BUTTON_BOX))
#define XFCE_IS_INDICATOR_BUTTON_BOX_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_TYPE_INDICATOR_BUTTON_BOX))
#define XFCE_INDICATOR_BUTTON_BOX_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_TYPE_INDICATOR_BUTTON_BOX, IndicatorButtonBoxClass))

typedef struct          _IndicatorButtonBox              IndicatorButtonBox;
typedef struct          _IndicatorButtonBoxClass         IndicatorButtonBoxClass;


void                    indicator_button_box_set_label   (IndicatorButtonBox        *box,
                                                          GtkLabel                  *label);

void                    indicator_button_box_set_image   (IndicatorButtonBox        *box,
                                                          GtkImage                  *image);

gboolean                indicator_button_box_is_small    (IndicatorButtonBox        *box);

GtkWidget              *indicator_button_box_new         (IndicatorConfig           *config);

void                    indicator_button_box_disconnect_signals (IndicatorButtonBox *box);

G_END_DECLS

#endif /* !__INDICATOR_BUTTON_BOX_H__ */
