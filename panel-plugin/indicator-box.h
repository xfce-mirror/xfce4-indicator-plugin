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

#ifndef __INDICATOR_BOX_H__
#define __INDICATOR_BOX_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <libindicator/indicator-object.h>

G_BEGIN_DECLS

GType xfce_indicator_box_get_type (void);

#define XFCE_TYPE_INDICATOR_BOX             (xfce_indicator_box_get_type())
#define XFCE_INDICATOR_BOX(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), XFCE_TYPE_INDICATOR_BOX, XfceIndicatorBox))
#define XFCE_INDICATOR_BOX_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass), XFCE_TYPE_INDICATOR_BOX, XfceIndicatorBoxClass))
#define XFCE_IS_INDICATOR_BOX(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), XFCE_TYPE_INDICATOR_BOX))
#define XFCE_IS_INDICATOR_BOX_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass), XFCE_TYPE_INDICATOR_BOX))
#define XFCE_INDICATOR_BOX_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj), XFCE_TYPE_INDICATOR_BOX, XfceIndicatorBoxClass))

typedef struct _XfceIndicatorBox XfceIndicatorBox;
typedef struct _XfceIndicatorBoxClass XfceIndicatorBoxClass;

struct _XfceIndicatorBox
{
  GtkContainer          __parent__;

  GSList               *children;

  gint                  panel_size;
  gint                  nrows;
  gint                  icon_size_max;
  gboolean              align_left;

  GtkOrientation        panel_orientation;
  GtkOrientation        orientation;
};

struct _XfceIndicatorBoxClass
{
  GtkContainerClass __parent__;
};

void xfce_indicator_box_set_orientation (XfceIndicatorBox *box,
                                         GtkOrientation panel_orientation,
                                         GtkOrientation orientation);

void xfce_indicator_box_set_size (XfceIndicatorBox *box,
                                  gint panel_size,
                                  gint nrows);


GtkOrientation xfce_indicator_box_get_panel_orientation (XfceIndicatorBox *box);

GtkOrientation xfce_indicator_box_get_indicator_orientation (XfceIndicatorBox *box);

gint xfce_indicator_box_get_nrows (XfceIndicatorBox *box);

gint xfce_indicator_box_get_panel_size (XfceIndicatorBox *box);

gint xfce_indicator_box_get_indicator_size (XfceIndicatorBox *box);

gint xfce_indicator_box_get_icon_size_max (XfceIndicatorBox *box);

gboolean xfce_indicator_box_get_align_left (XfceIndicatorBox *box);

void xfce_indicator_box_remove_entry (XfceIndicatorBox     *box,
                                      IndicatorObjectEntry *entry);

GtkWidget *xfce_indicator_box_new ();

G_END_DECLS

#endif /* !__INDICATOR_BOX_H__ */
