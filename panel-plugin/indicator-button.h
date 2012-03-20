/*  Copyright (c) 2012 Andrzej <ndrwrdck@gmail.com>
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

G_BEGIN_DECLS

GType xfce_indicator_button_get_type (void);

#define XFCE_TYPE_INDICATOR_BUTTON             (xfce_indicator_button_get_type())
#define XFCE_INDICATOR_BUTTON(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_TYPE_INDICATOR_BUTTON, XfceIndicatorButton))
#define XFCE_INDICATOR_BUTTON_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), XFCE_TYPE_INDICATOR_BUTTON, XfceIndicatorButtonClass))
#define XFCE_IS_INDICATOR_BUTTON(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_TYPE_INDICATOR_BUTTON))
#define XFCE_IS_INDICATOR_BUTTON_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_TYPE_INDICATOR_BUTTON))
#define XFCE_INDICATOR_BUTTON_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_TYPE_INDICATOR_BUTTON, XfceIndicatorButtonClass))

typedef struct _XfceIndicatorButton XfceIndicatorButton;
typedef struct _XfceIndicatorButtonClass XfceIndicatorButtonClass;

struct _XfceIndicatorButton
{
  GtkToggleButton       __parent__;

  IndicatorObject      *io;
  IndicatorObjectEntry *entry;
  GtkMenu              *menu;

  GtkWidget            *box;
  GtkWidget            *label;
  GtkWidget            *icon;
  GtkWidget            *orig_icon;
  gulong                orig_icon_handler;

  gint                  panel_size;
  gint                  size;
  gint                  orig_icon_size;

  GtkOrientation        panel_orientation;
  GtkOrientation        orientation;
};

struct _XfceIndicatorButtonClass
{
  GtkToggleButtonClass __parent__;
};

void xfce_indicator_button_set_label (XfceIndicatorButton *button, GtkLabel *label);

void xfce_indicator_button_set_image (XfceIndicatorButton *button, GtkImage *image);

void xfce_indicator_button_set_menu (XfceIndicatorButton *button, GtkMenu *menu);

GtkWidget *xfce_indicator_button_get_label (XfceIndicatorButton *button);

GtkWidget *xfce_indicator_button_get_image (XfceIndicatorButton *button);

IndicatorObjectEntry *xfce_indicator_button_get_entry (XfceIndicatorButton *button);

IndicatorObject *xfce_indicator_button_get_io (XfceIndicatorButton *button);

GtkMenu *xfce_indicator_button_get_menu (XfceIndicatorButton *button);

void xfce_indicator_button_set_orientation (XfceIndicatorButton *button,
                                            GtkOrientation panel_orientation,
                                            GtkOrientation orientation);

void xfce_indicator_button_set_size (XfceIndicatorButton *button,
                                     gint panel_size,
                                     gint size);

GtkWidget *xfce_indicator_button_new (IndicatorObject *io, IndicatorObjectEntry *entry);

G_END_DECLS

#endif /* !__INDICATOR_BUTTON_H__ */
