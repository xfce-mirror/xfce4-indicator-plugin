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

#ifndef __INDICATOR_BOX_H__
#define __INDICATOR_BOX_H__

#include <glib.h>
#include <gtk/gtk.h>
#include <libayatana-indicator/indicator-object.h>
#include <libxfce4panel/libxfce4panel.h>
#include "indicator-config.h"

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

void xfce_indicator_box_remove_entry (XfceIndicatorBox     *box,
                                      IndicatorObjectEntry *entry);

GtkWidget       *xfce_indicator_box_new                       (IndicatorConfig        *config);

G_END_DECLS

#endif /* !__INDICATOR_BOX_H__ */
