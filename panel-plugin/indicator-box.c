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
static void                 xfce_indicator_box_get_property   (GObject          *object,
                                                               guint             prop_id,
                                                               GValue           *value,
                                                               GParamSpec       *pspec);
static void                 xfce_indicator_box_set_property   (GObject          *object,
                                                               guint             prop_id,
                                                               const GValue     *value,
                                                               GParamSpec       *pspec);
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
static gint                 xfce_indicator_box_get_row_size   (XfceIndicatorBox *box);


enum
{
  PROP_0,
  PROP_ICON_SIZE_MAX
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
  gobject_class->get_property = xfce_indicator_box_get_property;
  gobject_class->set_property = xfce_indicator_box_set_property;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->size_request = xfce_indicator_box_size_request;
  gtkwidget_class->size_allocate = xfce_indicator_box_size_allocate;

  gtkcontainer_class = GTK_CONTAINER_CLASS (klass);
  gtkcontainer_class->add = xfce_indicator_box_add;
  gtkcontainer_class->remove = xfce_indicator_box_remove;
  gtkcontainer_class->forall = xfce_indicator_box_forall;
  gtkcontainer_class->child_type = xfce_indicator_box_child_type;

  g_object_class_install_property (gobject_class,
                                   PROP_ICON_SIZE_MAX,
                                   g_param_spec_uint ("icon-size-max",
                                                      NULL, NULL,
                                                      1,
                                                      128,
                                                      24,
                                                      EXO_PARAM_READWRITE));
}



static void
xfce_indicator_box_init (XfceIndicatorBox *box)
{
  GTK_WIDGET_SET_FLAGS (box, GTK_NO_WINDOW);

  gtk_widget_set_can_focus(GTK_WIDGET(box), TRUE);
  gtk_container_set_border_width(GTK_CONTAINER(box), 0);

  box->children = NULL;

  box->nrows = 1;
  box->icon_size_max = 24;
  box->panel_size = 16;
  box->panel_orientation = GTK_ORIENTATION_HORIZONTAL;
  box->orientation = GTK_ORIENTATION_HORIZONTAL;
}



static void
xfce_indicator_box_finalize (GObject *object)
{
  XfceIndicatorBox *box = XFCE_INDICATOR_BOX (object);

  if (box->children != NULL)
    {
      g_slist_free (box->children);
      g_debug ("Not all icons have been removed from the indicator icon box.");
    }

  G_OBJECT_CLASS (xfce_indicator_box_parent_class)->finalize (object);
}



static void
xfce_indicator_box_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  XfceIndicatorBox *box = XFCE_INDICATOR_BOX (object);
  GPtrArray        *array;

  switch (prop_id)
    {
    case PROP_ICON_SIZE_MAX:
      g_value_set_uint (value, box->icon_size_max);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
xfce_indicator_box_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  XfceIndicatorBox     *box = XFCE_INDICATOR_BOX (object);
  gint                  val;
  gint                  size;
  XfceIndicatorButton  *child;
  GSList               *li;

  switch (prop_id)
    {
    case PROP_ICON_SIZE_MAX:
      val = g_value_get_uint (value);
      if (box->icon_size_max != val)
        {
          box->icon_size_max = val;
          size = xfce_indicator_box_get_row_size (box);
          for (li = box->children; li != NULL; li = li->next)
            {
              child = XFCE_INDICATOR_BUTTON (li->data);
              g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (child));
              xfce_indicator_button_set_size (child, box->panel_size, size);
            }
          gtk_widget_queue_resize (GTK_WIDGET (box));
        }
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



void
xfce_indicator_box_set_orientation (XfceIndicatorBox *box,
                                    GtkOrientation    panel_orientation,
                                    GtkOrientation    orientation)
{
  gboolean              needs_update = FALSE;
  XfceIndicatorButton  *child;
  GSList               *li;

  g_return_if_fail (XFCE_IS_INDICATOR_BOX (box));

  if (box->orientation != orientation)
    {
      box->orientation = orientation;
      needs_update = TRUE;
    }

  if (box->panel_orientation != panel_orientation)
    {
      box->panel_orientation = panel_orientation;
      needs_update = TRUE;
    }

  if (needs_update)
    {
      for (li = box->children; li != NULL; li = li->next)
        {
          child = XFCE_INDICATOR_BUTTON (li->data);
          g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (child));
          xfce_indicator_button_set_orientation (child, panel_orientation, orientation);
        }
      gtk_widget_queue_resize (GTK_WIDGET (box));
    }
}



void
xfce_indicator_box_set_size (XfceIndicatorBox *box,
                             gint              panel_size,
                             gint              nrows)
{
  gboolean              needs_update = FALSE;
  XfceIndicatorButton  *child;
  GSList               *li;
  gint                  size;

  g_return_if_fail (XFCE_IS_INDICATOR_BOX (box));

  if (box->nrows != nrows)
    {
      box->nrows = nrows;
      needs_update = TRUE;
    }

  if (box->panel_size != panel_size)
    {
      box->panel_size = panel_size;
      needs_update = TRUE;
    }

  if (needs_update)
    {
      size = xfce_indicator_box_get_row_size (box);
      for (li = box->children; li != NULL; li = li->next)
        {
          child = XFCE_INDICATOR_BUTTON (li->data);
          g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (child));
          xfce_indicator_button_set_size (child, panel_size, size);
        }
      gtk_widget_queue_resize (GTK_WIDGET (box));
    }
}



GtkWidget *
xfce_indicator_box_new ()
{
  XfceIndicatorBox *box = g_object_new (XFCE_TYPE_INDICATOR_BOX, NULL);
  return GTK_WIDGET (box);
}



static void
xfce_indicator_box_add (GtkContainer *container,
                        GtkWidget    *child)
{
  XfceIndicatorBox    *box = XFCE_INDICATOR_BOX (container);
  XfceIndicatorButton *button = XFCE_INDICATOR_BUTTON (child);
  gint                 size;

  g_return_if_fail (XFCE_IS_INDICATOR_BOX (box));
  g_return_if_fail (GTK_IS_WIDGET (child));
  g_return_if_fail (child->parent == NULL);

  box->children = g_slist_append (box->children, child);

  gtk_widget_set_parent (child, GTK_WIDGET (box));
  xfce_indicator_button_set_orientation (button, box->panel_orientation, box->orientation);
  size = xfce_indicator_box_get_row_size (box);
  xfce_indicator_button_set_size (button, box->panel_size, size);

  gtk_widget_queue_resize (GTK_WIDGET (container));
}



static void
xfce_indicator_box_remove (GtkContainer *container,
                           GtkWidget    *child)
{
  XfceIndicatorBox *box = XFCE_INDICATOR_BOX (container);
  GSList           *li;

  /* search the child */
  li = g_slist_find (box->children, child);
  if (G_LIKELY (li != NULL))
    {
      g_assert (GTK_WIDGET (li->data) == child);

      /* unparent widget */
      box->children = g_slist_remove_link (box->children, li);
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
  GSList           *li, *lnext;

  /* run callback for all children */
  for (li = box->children; li != NULL; li = lnext)
    {
      lnext = li->next;
      (*callback) (GTK_WIDGET (li->data), callback_data);
    }
}



static GType
xfce_indicator_box_child_type (GtkContainer *container)
{
  return GTK_TYPE_WIDGET;
}



static void
xfce_indicator_box_size_request (GtkWidget      *widget,
                                 GtkRequisition *requisition)
{
  XfceIndicatorBox *box = XFCE_INDICATOR_BOX (widget);
  GtkWidget        *child;
  GtkRequisition    child_req;
  GSList           *li;
  gint              panel_size;
  gint              length;
  gint              row;
  gint              nrows;
  gint              x;
  gboolean          has_label;

  panel_size = box->panel_size;
  row = 0;
  length = 0;
  x = 0;
  //nrows = MAX (box->nrows,
  //             box->panel_size / xfce_indicator_box_get_row_size (box));
  nrows = box->panel_size / xfce_indicator_box_get_row_size (box);

  for (li = box->children; li != NULL; li = li->next)
    {
      child = GTK_WIDGET (li->data);
      g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (child));

      gtk_widget_size_request (child, &child_req);
      has_label = (xfce_indicator_button_get_label (XFCE_INDICATOR_BUTTON (child)) != NULL);

      /* wrap rows if column is overflowing or a label is encountered */
      if (row > 0 && (has_label || row >= nrows))
        {
          x += length;
          row = 0;
          length = 0;
        }

      length =
        MAX (length, (box->panel_orientation == GTK_ORIENTATION_HORIZONTAL) ? child_req.width :child_req.height);

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

  x += length;

  if (box->panel_orientation == GTK_ORIENTATION_HORIZONTAL)
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
  XfceIndicatorBox *box = XFCE_INDICATOR_BOX (widget);
  GtkWidget        *child;
  GtkAllocation     child_alloc;
  GtkRequisition    child_req;
  gint              panel_size, size;
  gint              x, y;
  gint              x0, y0;
  GSList           *li;
  gint              length, width;
  gint              row;
  gint              nrows;
  gboolean          has_label;

  row = 0;
  length = 0;
  width = 0;
  x = y = 0;
  x0 = allocation->x;
  y0 = allocation->y;

  //nrows = MAX (box->nrows, box->panel_size / box->icon_size_max);
  nrows = box->panel_size / xfce_indicator_box_get_row_size (box);
  panel_size = box->panel_size;
  size = panel_size / nrows;

  for (li = box->children; li != NULL; li = li->next)
    {
      child = GTK_WIDGET (li->data);
      g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (child));

      gtk_widget_get_child_requisition (child, &child_req);

      has_label = (xfce_indicator_button_get_label (XFCE_INDICATOR_BUTTON (child)) != NULL);

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
                    (box->panel_orientation == GTK_ORIENTATION_HORIZONTAL) ? child_req.width :child_req.height);

      if (box->panel_orientation == GTK_ORIENTATION_HORIZONTAL)
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

      gtk_widget_size_allocate (child, &child_alloc);

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



static gint
xfce_indicator_box_get_row_size (XfceIndicatorBox *box)
{
  gint                 border_thickness;
  GtkStyle            *style;

  g_return_val_if_fail (XFCE_IS_INDICATOR_BOX (box), 24);

  style = gtk_widget_get_style (GTK_WIDGET (box));
  border_thickness = 2 * MAX (style->xthickness, style->ythickness) + 2;

  return MIN (box->panel_size / box->nrows, box->icon_size_max + border_thickness);
}


XfceIndicatorButton *
xfce_indicator_box_get_button (XfceIndicatorBox     *box,
                               IndicatorObjectEntry *entry)
{
  GSList              *li;
  GtkWidget           *child;
  XfceIndicatorButton *button;

  g_return_val_if_fail (XFCE_IS_INDICATOR_BOX (box), NULL);

  for (li = box->children; li != NULL; li = li->next)
    {
      child = GTK_WIDGET (li->data);
      g_return_val_if_fail (XFCE_IS_INDICATOR_BUTTON (child), NULL);

      button = XFCE_INDICATOR_BUTTON (child);
      if (xfce_indicator_button_get_entry (button) == entry)
        return button;
    }
  return NULL;
}
