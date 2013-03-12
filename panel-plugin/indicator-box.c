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



/*
 *  This file implements a container class for holding indicator buttons.
 *
 */



#include <glib.h>
#include <gtk/gtk.h>
#include <exo/exo.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libindicator/indicator-object.h>

#include "indicator-box.h"
#include "indicator-button.h"

static void                 xfce_indicator_box_finalize       (GObject          *object);
static void                 xfce_indicator_box_list_changed   (XfceIndicatorBox *box,
                                                               IndicatorConfig  *config);
static void                 xfce_indicator_box_add            (GtkContainer     *container,
                                                               GtkWidget        *child);
static void                 xfce_indicator_box_remove         (GtkContainer     *container,
                                                               GtkWidget        *child);
static void                 xfce_indicator_box_forall         (GtkContainer     *container,
                                                               gboolean          include_internals,
                                                               GtkCallback       callback,
                                                               gpointer          callback_data);
static GType                xfce_indicator_box_child_type     (GtkContainer     *container);
static void                 xfce_indicator_box_size_request   (GtkWidget        *widget,
                                                               GtkRequisition   *requisition);
static void                 xfce_indicator_box_size_allocate  (GtkWidget        *widget,
                                                               GtkAllocation    *allocation);


struct _XfceIndicatorBox
{
  GtkContainer          __parent__;

  IndicatorConfig      *config;

  GHashTable           *children;

  gint                  panel_size;
  gint                  nrows;
  gint                  icon_size_max;
  gboolean              align_left;

  GtkOrientation        panel_orientation;
  GtkOrientation        orientation;

  gulong                indicator_list_changed_id;
};

struct _XfceIndicatorBoxClass
{
  GtkContainerClass __parent__;
};




G_DEFINE_TYPE (XfceIndicatorBox, xfce_indicator_box, GTK_TYPE_CONTAINER)

static void
xfce_indicator_box_class_init (XfceIndicatorBoxClass *klass)
{
  GObjectClass      *gobject_class;
  GtkWidgetClass    *gtkwidget_class;
  GtkContainerClass *gtkcontainer_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_indicator_box_finalize;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->size_request = xfce_indicator_box_size_request;
  gtkwidget_class->size_allocate = xfce_indicator_box_size_allocate;

  gtkcontainer_class = GTK_CONTAINER_CLASS (klass);
  gtkcontainer_class->add = xfce_indicator_box_add;
  gtkcontainer_class->remove = xfce_indicator_box_remove;
  gtkcontainer_class->forall = xfce_indicator_box_forall;
  gtkcontainer_class->child_type = xfce_indicator_box_child_type;
}



static void
xfce_indicator_box_init (XfceIndicatorBox *box)
{
  GTK_WIDGET_SET_FLAGS (box, GTK_NO_WINDOW);

  gtk_widget_set_can_focus(GTK_WIDGET(box), TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(box), 0);

  /* todo: no deallocation function for values */
  box->children = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
}



static void
xfce_indicator_box_finalize (GObject *object)
{
  XfceIndicatorBox *box = XFCE_INDICATOR_BOX (object);

  if (box->indicator_list_changed_id != 0)
    {
      g_signal_handler_disconnect (box->config, box->indicator_list_changed_id);
      box->indicator_list_changed_id = 0;
    }

  g_hash_table_destroy (box->children);

  G_OBJECT_CLASS (xfce_indicator_box_parent_class)->finalize (object);
}



GtkWidget *
xfce_indicator_box_new (IndicatorConfig *config)
{
  XfceIndicatorBox *box = g_object_new (XFCE_TYPE_INDICATOR_BOX, NULL);

  box->config = config;

  box->indicator_list_changed_id =
    g_signal_connect_swapped (G_OBJECT (box->config), "indicator-list-changed",
                              G_CALLBACK (xfce_indicator_box_list_changed), box);

  return GTK_WIDGET (box);
}



static void
xfce_indicator_box_list_changed (XfceIndicatorBox *box,
                                 IndicatorConfig  *config)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BOX (box));
  g_return_if_fail (XFCE_IS_INDICATOR_CONFIG (config));

  gtk_widget_queue_resize (GTK_WIDGET (box));
}



static void
xfce_indicator_box_add (GtkContainer *container,
                        GtkWidget    *child)
{
  XfceIndicatorBox    *box = XFCE_INDICATOR_BOX (container);
  XfceIndicatorButton *button = XFCE_INDICATOR_BUTTON (child);
  GList               *li;
  const gchar         *io_name;

  g_return_if_fail (XFCE_IS_INDICATOR_BOX (box));
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));
  g_return_if_fail (child->parent == NULL);

  io_name = xfce_indicator_button_get_io_name (button);
  li = g_hash_table_lookup (box->children, io_name);
  li = g_list_append (li, button);
  g_hash_table_replace (box->children, g_strdup (io_name), li);

  gtk_widget_set_parent (child, GTK_WIDGET (box));

  gtk_widget_queue_resize (GTK_WIDGET (container));
}



static void
xfce_indicator_box_remove (GtkContainer *container,
                           GtkWidget    *child)
{
  XfceIndicatorBox    *box = XFCE_INDICATOR_BOX (container);
  XfceIndicatorButton *button = XFCE_INDICATOR_BUTTON (child);
  GList               *li, *li_tmp;
  const gchar         *io_name;

  /* search the child */
  io_name = xfce_indicator_button_get_io_name (button);
  li = g_hash_table_lookup (box->children, io_name);
  li_tmp = g_list_find (li, child);
  if (G_LIKELY (li_tmp != NULL))
    {
      g_assert (GTK_WIDGET (li_tmp->data) == child);

      /* unparent widget */
      li = g_list_remove_link (li, li_tmp);
      g_hash_table_replace (box->children, g_strdup (io_name), li);
      gtk_widget_unparent (child);

      /* resize, so we update has-hidden */
      gtk_widget_queue_resize (GTK_WIDGET (container));
    }
}



static void
xfce_indicator_box_forall (GtkContainer *container,
                           gboolean      include_internals,
                           GtkCallback   callback,
                           gpointer      callback_data)
{
  XfceIndicatorBox *box = XFCE_INDICATOR_BOX (container);
  GList            *known_indicators;
  GList            *li, *li_int, *li_tmp;

  /* run callback for all children */
  known_indicators = indicator_config_get_known_indicators (box->config);
  for (li = known_indicators; li != NULL; li = li->next)
    {
      li_int = g_hash_table_lookup (box->children, li->data);
      for (li_tmp = li_int; li_tmp != NULL; li_tmp = li_tmp->next)
        {
          (*callback) (GTK_WIDGET (li_tmp->data), callback_data);
        }
    }
  li_int = g_hash_table_lookup (box->children, "<placeholder>");
  for (li_tmp = li_int; li_tmp != NULL; li_tmp = li_tmp->next)
    {
      (*callback) (GTK_WIDGET (li_tmp->data), callback_data);
    }
}



static GType
xfce_indicator_box_child_type (GtkContainer *container)
{
  return XFCE_TYPE_INDICATOR_BUTTON;
}



static gint
xfce_indicator_box_get_row_size (XfceIndicatorBox *box)
{
  gint                 border_thickness;
  GtkStyle            *style;

  g_return_val_if_fail (XFCE_IS_INDICATOR_BOX (box), 24);

  style = gtk_widget_get_style (GTK_WIDGET (box));
  border_thickness = 2 * MAX (style->xthickness, style->ythickness) + 2;

  return MIN (indicator_config_get_panel_size (box->config) /
              indicator_config_get_nrows (box->config),
              indicator_config_get_icon_size_max (box->config) + border_thickness);
}





static void
xfce_indicator_box_size_request (GtkWidget      *widget,
                                 GtkRequisition *requisition)
{
  XfceIndicatorBox    *box = XFCE_INDICATOR_BOX (widget);
  XfceIndicatorButton *button;
  GtkRequisition       child_req;
  GList               *known_indicators, *li, *li_int, *li_tmp;
  gint                 panel_size;
  gint                 length;
  gint                 row;
  gint                 nrows;
  gint                 x;
  gboolean             has_label;
  GtkOrientation       panel_orientation;

  panel_size = indicator_config_get_panel_size (box->config);
  panel_orientation = indicator_config_get_panel_orientation (box->config);
  row = 0;
  length = 0;
  x = 0;
  nrows = panel_size / xfce_indicator_box_get_row_size (box);

  if (g_hash_table_lookup (box->children, "<placeholder>") != NULL)
    known_indicators = g_list_append (NULL, "<placeholder>");
  else
    known_indicators = indicator_config_get_known_indicators (box->config);
  for (li = known_indicators; li != NULL; li = li->next)
    {
      li_int = g_hash_table_lookup (box->children, li->data);
      for (li_tmp = li_int; li_tmp != NULL; li_tmp = li_tmp->next)
        {
          button = XFCE_INDICATOR_BUTTON (li_tmp->data);

          gtk_widget_size_request (GTK_WIDGET (button), &child_req);
          has_label = (xfce_indicator_button_get_label (button) != NULL);

          /* wrap rows if column is overflowing or a label is encountered */
          if (row > 0 && (has_label || row >= nrows))
            {
              x += length;
              row = 0;
              length = 0;
            }

          length =
            MAX (length, (panel_orientation == GTK_ORIENTATION_HORIZONTAL) ? child_req.width :child_req.height);

          if (has_label || row >= nrows)
            {
              x += length;
              row = 0;
              length = 0;
            }
          else
            {
              row += 1;
            }
        }
    }

  x += length;

  if (panel_orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      requisition->width = x;
      requisition->height = panel_size;
    }
  else
    {
      requisition->width = panel_size;
      requisition->height = x;
    }
  /* g_debug ("indicator-box size request: w=%d h=%d", requisition->width, requisition->height); */
}



static void
xfce_indicator_box_size_allocate (GtkWidget     *widget,
                                  GtkAllocation *allocation)
{
  XfceIndicatorBox    *box = XFCE_INDICATOR_BOX (widget);
  XfceIndicatorButton *button;
  GtkAllocation        child_alloc;
  GtkRequisition       child_req;
  gint                 panel_size, size;
  gint                 x, y;
  gint                 x0, y0;
  GList               *known_indicators, *li, *li_int, *li_tmp;
  gint                 length, width;
  gint                 row;
  gint                 nrows;
  gboolean             has_label;
  GtkOrientation       panel_orientation;

  row = 0;
  length = 0;
  width = 0;
  x = y = 0;
  x0 = allocation->x;
  y0 = allocation->y;

  panel_size = indicator_config_get_panel_size (box->config);
  panel_orientation = indicator_config_get_panel_orientation (box->config);
  nrows = panel_size / xfce_indicator_box_get_row_size (box);
  size = panel_size / nrows;

  if (g_hash_table_lookup (box->children, "<placeholder>") != NULL)
    known_indicators = g_list_append (NULL, "<placeholder>");
  else
    known_indicators = indicator_config_get_known_indicators (box->config);
  for (li = known_indicators; li != NULL; li = li->next)
    {
      li_int = g_hash_table_lookup (box->children, li->data);
      for (li_tmp = li_int; li_tmp != NULL; li_tmp = li_tmp->next)
        {
          button = XFCE_INDICATOR_BUTTON (li_tmp->data);

          gtk_widget_get_child_requisition (GTK_WIDGET (button), &child_req);

          has_label = (xfce_indicator_button_get_label (button) != NULL);

          /* wrap rows if column is overflowing or a label is encountered */
          if (row > 0 && (has_label || row >= nrows))
            {
              x += length;
              y = 0;
              row = 0;
              length = 0;
            }

          width = (has_label) ? panel_size : size;
          length = MAX (length,
                        (panel_orientation == GTK_ORIENTATION_HORIZONTAL) ? child_req.width :child_req.height);

          if (panel_orientation == GTK_ORIENTATION_HORIZONTAL)
            {
              child_alloc.x = x0 + x;
              child_alloc.y = y0 + y;
              child_alloc.width = length;
              child_alloc.height = width;
            }
          else
            {
              child_alloc.x = x0 + y;
              child_alloc.y = y0 + x;
              child_alloc.width = width;
              child_alloc.height = length;
            }

          /* g_debug ("indicator-box size allocate: x=%d y=%d w=%d h=%d", */
          /*          child_alloc.x, child_alloc.y, child_alloc.width, child_alloc.height); */

          gtk_widget_size_allocate (GTK_WIDGET (button), &child_alloc);

          if (has_label || row >= nrows)
            {
              x += length;
              y = 0;
              row = 0;
              length = 0;
            }
          else
            {
              row += 1;
              y += size;
            }
        }
    }
}



void
xfce_indicator_box_remove_entry (XfceIndicatorBox     *box,
                                 IndicatorObjectEntry *entry)
{
  GList               *known_indicators, *li, *li_int, *li_tmp;
  XfceIndicatorButton *button;

  g_return_if_fail (XFCE_IS_INDICATOR_BOX (box));

  known_indicators = indicator_config_get_known_indicators (box->config);
  for (li = known_indicators; li != NULL; li = li->next)
    {
      li_int = g_hash_table_lookup (box->children, li->data);
      for (li_tmp = li_int; li_tmp != NULL; li_tmp = li_tmp->next)
        {
          button = XFCE_INDICATOR_BUTTON (li_tmp->data);
          if (xfce_indicator_button_get_entry (button) == entry)
            {
              xfce_indicator_button_disconnect_signals (button);
              gtk_widget_destroy (GTK_WIDGET (button));
              return;
            }
        }
    }
}

