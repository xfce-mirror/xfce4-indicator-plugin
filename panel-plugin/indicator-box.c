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
//#include <exo/exo.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libindicator/indicator-object.h>

#include "indicator-box.h"
#include "indicator-button.h"

#define ICON_SIZE 22

static void                 xfce_indicator_box_finalize             (GObject          *object);
static void                 xfce_indicator_box_list_changed         (XfceIndicatorBox *box,
                                                                     IndicatorConfig  *config);
static void                 xfce_indicator_box_add                  (GtkContainer     *container,
                                                                     GtkWidget        *child);
static void                 xfce_indicator_box_remove               (GtkContainer     *container,
                                                                     GtkWidget        *child);
static void                 xfce_indicator_box_forall               (GtkContainer     *container,
                                                                     gboolean          include_internals,
                                                                     GtkCallback       callback,
                                                                     gpointer          callback_data);
static GType                xfce_indicator_box_child_type           (GtkContainer     *container);
static void                 xfce_indicator_box_get_preferred_length (GtkWidget        *widget,
                                                                     gint             *minimal_length,
                                                                     gint             *natural_length);
static void                 xfce_indicator_box_get_preferred_width  (GtkWidget        *widget,
                                                                     gint             *minimal_width,
                                                                     gint             *natural_width);
static void                 xfce_indicator_box_get_preferred_height (GtkWidget        *widget,
                                                                     gint             *minimal_height,
                                                                     gint             *natural_height);
static void                 xfce_indicator_box_size_allocate        (GtkWidget        *widget,
                                                                     GtkAllocation    *allocation);


struct _XfceIndicatorBox
{
  GtkContainer          __parent__;

  IndicatorConfig      *config;

  GHashTable           *children;

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
  //gtkwidget_class->size_request = xfce_indicator_box_size_request;
  gtkwidget_class->get_preferred_width = xfce_indicator_box_get_preferred_width;
  gtkwidget_class->get_preferred_height = xfce_indicator_box_get_preferred_height;
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
  gtk_widget_set_has_window (GTK_WIDGET (box), FALSE);
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



static gint
xfce_indicator_box_sort_buttons (gconstpointer a,
                                 gconstpointer b)
{
  XfceIndicatorButton *a0 = XFCE_INDICATOR_BUTTON (a);
  XfceIndicatorButton *b0 = XFCE_INDICATOR_BUTTON (b);
  guint                a1, b1;
  const gchar         *a_io;
  gint                 result = 0;

  a1 = xfce_indicator_button_get_pos (a0);
  b1 = xfce_indicator_button_get_pos (b0);

  /* only need one */
  a_io = xfce_indicator_button_get_io_name (a0);

  // printf ("=== %s, %s; %s, %s; %d, %d", xfce_indicator_button_get_io_name (a0), xfce_indicator_button_get_io_name (b0), xfce_indicator_button_get_entry (a0)->name_hint, xfce_indicator_button_get_entry (b0)->name_hint, a1, b1);

  /* special case for Application indicators (unreliable ordering) */
  /* always compare by name and ignore location field */
  if (a_io != NULL && g_strcmp0 (a_io, "libapplication.so") == 0)
    result = g_strcmp0 (xfce_indicator_button_get_entry(a0)->name_hint,
                        xfce_indicator_button_get_entry(b0)->name_hint);

  /* group all entries with location==0 at the beginning of the list
   * but don't sort them (they may depend on insertion order) */

  if (result == 0 && (a1 != 0 || b1 != 0))
    result = a1-b1;

  /* if there are two entries with the same non-zero location
   * try to resolve their order by their name_hint */

  if (result == 0)
    result =  g_strcmp0 (xfce_indicator_button_get_entry(a0)->name_hint,
                         xfce_indicator_button_get_entry(b0)->name_hint);

  // printf (" -> %d\n", result);
  return result;
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
  g_return_if_fail (gtk_widget_get_parent (GTK_WIDGET (child)) == NULL);

  io_name = xfce_indicator_button_get_io_name (button);
  li = g_hash_table_lookup (box->children, io_name);
  // printf ("   +++ %s %s\n", io_name, xfce_indicator_button_get_entry (button)->name_hint);
  if (g_strcmp0 (io_name, "libapplication.so") != 0 &&
      xfce_indicator_button_get_pos (button) == 0)
    li = g_list_append (li, button);
  else
    li = g_list_insert_sorted (li, button, xfce_indicator_box_sort_buttons);

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



static void
xfce_indicator_box_get_preferred_length (GtkWidget *widget,
                                         gint      *minimum_length,
                                         gint      *natural_length)
{
  XfceIndicatorBox    *box = XFCE_INDICATOR_BOX (widget);
  XfceIndicatorButton *button;
  GtkRequisition       child_req;
  GList               *known_indicators, *li, *li_int, *li_tmp;
  gint                 panel_size, size;
  gint                 length;
  gint                 row;
  gint                 nrows;
  gint                 x;
  gboolean             is_small;
  GtkOrientation       panel_orientation;
  GtkStyleContext     *ctx;
  GtkBorder            padding, border;
  gint                 border_thickness;
  gboolean             allow_small;

  /* check border thickness of the first button */
  li = gtk_container_get_children (GTK_CONTAINER (box));
  if (li == NULL)
    return;

  button = XFCE_INDICATOR_BUTTON (li->data);
  ctx = gtk_widget_get_style_context (GTK_WIDGET (button));
  gtk_style_context_get_padding (ctx, gtk_widget_get_state_flags (GTK_WIDGET (button)), &padding);
  gtk_style_context_get_border (ctx, gtk_widget_get_state_flags (GTK_WIDGET (button)), &border);
  border_thickness = MAX (padding.left+padding.right+border.left+border.right,
                          padding.top+padding.bottom+border.top+border.bottom);

  size = ICON_SIZE + border_thickness;
  panel_size = indicator_config_get_panel_size (box->config);
  nrows = MAX (1, panel_size / size);
  allow_small = !((nrows == 1) || indicator_config_get_single_row (box->config));

  panel_orientation = indicator_config_get_panel_orientation (box->config);

  row = 0;
  length = 0;
  x = 0;

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

          gtk_widget_get_preferred_size (GTK_WIDGET (button), NULL, &child_req);

          is_small = allow_small && xfce_indicator_button_is_small (button);

          /* wrap rows if column is overflowing or a label is encountered */
          if (row > 0 && (row >= nrows || !is_small))
            {
              x += length;
              row = 0;
              length = 0;
            }

          length =
            MAX (length, (panel_orientation == GTK_ORIENTATION_HORIZONTAL) ? child_req.width :child_req.height);

          if (row >= nrows || !is_small)
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

  if (minimum_length != NULL)
    *minimum_length = x;

  if (natural_length != NULL)
    *natural_length = x;

  /* g_debug ("indicator-box size request: %d", x); */
}



static void
xfce_indicator_box_get_preferred_width (GtkWidget *widget,
                                        gint      *minimum_width,
                                        gint      *natural_width)
{
  XfceIndicatorBox *box = XFCE_INDICATOR_BOX (widget);
  gint              panel_size;

  if (indicator_config_get_panel_orientation (box->config) == GTK_ORIENTATION_HORIZONTAL)
    {
      xfce_indicator_box_get_preferred_length (widget, minimum_width, natural_width);
    }
  else
    {
      panel_size = indicator_config_get_panel_size (box->config);
      if (minimum_width != NULL)
        *minimum_width = panel_size;
      if (natural_width != NULL)
        *natural_width = panel_size;
    }
}



static void
xfce_indicator_box_get_preferred_height (GtkWidget *widget,
                                         gint      *minimum_height,
                                         gint      *natural_height)
{
  XfceIndicatorBox *box = XFCE_INDICATOR_BOX (widget);
  gint              panel_size;

  if (indicator_config_get_panel_orientation (box->config) == GTK_ORIENTATION_VERTICAL)
    {
      xfce_indicator_box_get_preferred_length (widget, minimum_height, natural_height);
    }
  else
    {
      panel_size = indicator_config_get_panel_size (box->config);
      if (minimum_height != NULL)
        *minimum_height = panel_size;
      if (natural_height != NULL)
        *natural_height = panel_size;
    }
}




static void
xfce_indicator_box_size_allocate (GtkWidget     *widget,
                                  GtkAllocation *allocation)
{
  XfceIndicatorBox    *box = XFCE_INDICATOR_BOX (widget);
  XfceIndicatorButton *button;
  GtkAllocation        child_alloc;
  GtkRequisition       child_req;
  gint                 panel_size, size, full_size;
  gint                 x, y;
  gint                 x0, y0;
  GList               *known_indicators, *li, *li_int, *li_tmp;
  gint                 length, width;
  gint                 row;
  gint                 nrows;
  gboolean             is_small;
  GtkOrientation       panel_orientation;
  GtkStyleContext     *ctx;
  GtkBorder            padding, border;
  gint                 border_thickness;
  gboolean             allow_small;

  row = 0;
  length = 0;
  width = 0;
  x = y = 0;
  x0 = allocation->x;
  y0 = allocation->y;

  gtk_widget_set_allocation (widget, allocation);

  /* check border thickness of the first button */
  li = gtk_container_get_children (GTK_CONTAINER (box));
  if (li == NULL)
    return;

  button = XFCE_INDICATOR_BUTTON (li->data);
  ctx = gtk_widget_get_style_context (GTK_WIDGET (button));
  gtk_style_context_get_padding (ctx, gtk_widget_get_state_flags (GTK_WIDGET (button)), &padding);
  gtk_style_context_get_border (ctx, gtk_widget_get_state_flags (GTK_WIDGET (button)), &border);
  border_thickness = MAX (padding.left+padding.right+border.left+border.right,
                          padding.top+padding.bottom+border.top+border.bottom);

  panel_size = indicator_config_get_panel_size (box->config);
  size = MIN (ICON_SIZE + border_thickness, panel_size);
  nrows = panel_size / size;
  //full_size = ((nrows-1)*panel_size + nrows*size) / nrows; // regular pitch, margins
  full_size = panel_size; // irregular pitch, no margins
  allow_small = !((nrows == 1) || indicator_config_get_single_row (box->config));

  panel_orientation = indicator_config_get_panel_orientation (box->config);

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

          gtk_widget_get_preferred_size (GTK_WIDGET (button), NULL, &child_req);

          is_small = allow_small && xfce_indicator_button_is_small (button);

          /* wrap rows if column is overflowing or a label is encountered */
          if (row > 0 && (row >= nrows || !is_small))
            {
              x += length;
              row = 0;
              length = 0;
            }

          // regular pitch
          // y = ((2*row+1) * panel_size + nrows - nrows * size) / 2 / nrows;
          // no margins
          if (!is_small)
            y = 0;
          else if (nrows == 1)
            y = (panel_size - size + 1) / 2;
          else
            y = (2*row*(panel_size-size) + nrows - 1) / (2*nrows-2);

          width = (is_small) ? size : full_size;
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

          if (row >= nrows || !is_small)
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
              xfce_indicator_button_destroy (button);
              return;
            }
        }
    }
}

