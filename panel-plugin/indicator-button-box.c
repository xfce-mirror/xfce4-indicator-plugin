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



/*
 *  This file implements a container class holding an icon and a label
 *  for use in XfceIndicatorButton.
 *
 */



#include <glib.h>
#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>

#include "indicator-button-box.h"

#define ICON_SIZE 22
#define SPACING 2

static void   indicator_button_box_finalize                        (GObject                *object);
static void   indicator_button_box_get_preferred_width             (GtkWidget              *widget,
                                                                    gint                   *minimum_width,
                                                                    gint                   *natural_width);
static void   indicator_button_box_get_preferred_width_for_height  (GtkWidget              *widget,
                                                                    gint                    height,
                                                                    gint                   *minimum_width,
                                                                    gint                   *natural_width);
static void   indicator_button_box_get_preferred_height            (GtkWidget              *widget,
                                                                    gint                   *minimum_height,
                                                                    gint                   *natural_height);
static void   indicator_button_box_get_preferred_height_for_width  (GtkWidget              *widget,
                                                                    gint                    width,
                                                                    gint                   *minimum_height,
                                                                    gint                   *natural_height);
static void   indicator_button_box_size_allocate                   (GtkWidget             *widget,
                                                                    GtkAllocation         *allocation);
static void   indicator_button_box_add                             (GtkContainer          *container,
                                                                    GtkWidget             *child);
static void   indicator_button_box_remove                          (GtkContainer          *container,
                                                                    GtkWidget             *child);
static GType  indicator_button_box_child_type                      (GtkContainer          *container);
static void   indicator_button_box_forall                          (GtkContainer          *container,
                                                                    gboolean              include_internals,
                                                                    GtkCallback           callback,
                                                                    gpointer              callback_data);


struct _IndicatorButtonBox
{
  GtkContainer        __parent__;

  IndicatorConfig      *config;

  GtkWidget            *label;
  GtkWidget            *icon;

  gboolean              cached;
  gboolean              is_small;
  gint                  pixbuf_w;
  gint                  pixbuf_h;
  GtkOrientation        orientation;

  gulong                configuration_changed_id;
};

struct _IndicatorButtonBoxClass
{
  GtkContainerClass   __parent__;
};




G_DEFINE_TYPE (IndicatorButtonBox, indicator_button_box, GTK_TYPE_CONTAINER)

static void
indicator_button_box_class_init (IndicatorButtonBoxClass *klass)
{
  GObjectClass      *gobject_class;
  GtkWidgetClass    *widget_class;
  GtkContainerClass *container_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = indicator_button_box_finalize;

  widget_class = GTK_WIDGET_CLASS (klass);
  widget_class->get_preferred_width             = indicator_button_box_get_preferred_width;
  widget_class->get_preferred_width_for_height  = indicator_button_box_get_preferred_width_for_height;
  widget_class->get_preferred_height            = indicator_button_box_get_preferred_height;
  widget_class->get_preferred_height_for_width  = indicator_button_box_get_preferred_height_for_width;
  widget_class->size_allocate                   = indicator_button_box_size_allocate;

  container_class = GTK_CONTAINER_CLASS (klass);
  container_class->add               = indicator_button_box_add;
  container_class->remove            = indicator_button_box_remove;
  container_class->child_type        = indicator_button_box_child_type;
  container_class->forall            = indicator_button_box_forall;
}



static void
indicator_button_box_init (IndicatorButtonBox *box)
{
  gtk_widget_set_has_window (GTK_WIDGET (box), FALSE);
  gtk_widget_set_can_focus(GTK_WIDGET(box), FALSE);
  gtk_widget_set_can_default (GTK_WIDGET (box), FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(box), 0);
  gtk_widget_set_name (GTK_WIDGET (box), "indicator-button-box");

  box->config = NULL;

  box->label = NULL;
  box->icon = NULL;

  box->cached = FALSE;
  box->is_small = TRUE;
  box->pixbuf_w = -1;
  box->pixbuf_h = -1;
  box->orientation = GTK_ORIENTATION_HORIZONTAL;

  box->configuration_changed_id = 0;
}



static void
indicator_button_box_finalize (GObject *object)
{
  IndicatorButtonBox *box = XFCE_INDICATOR_BUTTON_BOX (object);

  indicator_button_box_disconnect_signals (box);

  if (box->label != NULL)
    g_object_unref (G_OBJECT (box->label));
  if (box->icon != NULL)
    g_object_unref (G_OBJECT (box->icon));

  G_OBJECT_CLASS (indicator_button_box_parent_class)->finalize (object);
}



static GType
indicator_button_box_child_type (GtkContainer          *container)
{
  return GTK_TYPE_WIDGET;
}





static void
indicator_button_box_forall (GtkContainer *container,
                             gboolean      include_internals,
                             GtkCallback   callback,
                             gpointer      callback_data)
{
  IndicatorButtonBox *box = XFCE_INDICATOR_BUTTON_BOX (container);

  if (box->icon != NULL)
    (*callback) (box->icon, callback_data);
  if (box->label != NULL)
    (*callback) (box->label, callback_data);
}




static void
indicator_button_box_add (GtkContainer *container,
                          GtkWidget    *child)
{
  IndicatorButtonBox  *box = XFCE_INDICATOR_BUTTON_BOX (container);

  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON_BOX (box));
  g_return_if_fail (gtk_widget_get_parent (GTK_WIDGET (child)) == NULL);

  gtk_widget_set_parent (child, GTK_WIDGET (box));
  gtk_widget_queue_resize (GTK_WIDGET (container));
}



static void
indicator_button_box_remove (GtkContainer *container,
                             GtkWidget    *child)
{
  IndicatorButtonBox  *box = XFCE_INDICATOR_BUTTON_BOX (container);

  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON_BOX (box));

  gtk_widget_unparent (child);
  gtk_widget_queue_resize (GTK_WIDGET (container));
}



static void
indicator_button_box_label_changed (GtkLabel            *label,
                                    GParamSpec          *pspec,
                                    IndicatorButtonBox  *box)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON_BOX (box));
  g_return_if_fail (GTK_IS_LABEL (label));

  gtk_widget_queue_resize (GTK_WIDGET (box));
}



void
indicator_button_box_set_label (IndicatorButtonBox  *box,
                                GtkLabel            *label)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON_BOX (box));
  g_return_if_fail (GTK_IS_LABEL (label));

  if (box->label != NULL)
    {
      gtk_container_remove (GTK_CONTAINER (box), box->label);
      g_object_unref (G_OBJECT (box->label));
    }
  box->label = GTK_WIDGET (label);
  g_object_ref (G_OBJECT (box->label));
  g_signal_connect(G_OBJECT(box->label), "notify::label",
                   G_CALLBACK(indicator_button_box_label_changed), box);

  gtk_label_set_ellipsize (GTK_LABEL (box->label), PANGO_ELLIPSIZE_END);
  box->is_small = FALSE;

  gtk_container_add (GTK_CONTAINER (box), box->label);
  gtk_widget_show (box->label);
}




static void
indicator_button_box_icon_changed (GtkImage            *icon,
                                   GParamSpec          *pspec,
                                   IndicatorButtonBox  *box)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON_BOX (box));
  g_return_if_fail (GTK_IS_IMAGE (icon));

  box->cached = FALSE;

  gtk_widget_queue_resize (GTK_WIDGET (box));
}



void
indicator_button_box_set_image (IndicatorButtonBox  *box,
                                GtkImage            *image)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON_BOX (box));
  g_return_if_fail (GTK_IS_IMAGE (image));

  /* g_debug ("indicator-button-box set image, image=%x", (uint) image); */

  if (box->icon != NULL)
    {
      gtk_container_remove (GTK_CONTAINER (box), box->label);
      g_object_unref (G_OBJECT (box->icon));
    }
  box->icon = GTK_WIDGET (image);
  g_object_ref (G_OBJECT (box->icon));
  g_signal_connect(G_OBJECT(box->icon), "notify::pixbuf",
                   G_CALLBACK(indicator_button_box_icon_changed), box);

  box->cached = FALSE;

  gtk_container_add (GTK_CONTAINER (box), box->icon);
  gtk_widget_show (box->icon);
}



static void
indicator_button_box_configuration_changed (IndicatorButtonBox  *box,
                                            IndicatorConfig     *config)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON_BOX (box));
  g_return_if_fail (XFCE_IS_INDICATOR_CONFIG (config));

  box->cached = FALSE;

  gtk_widget_queue_resize (GTK_WIDGET (box));
}



GtkWidget *
indicator_button_box_new (IndicatorConfig      *config)
{
  IndicatorButtonBox *box = g_object_new (XFCE_TYPE_INDICATOR_BUTTON_BOX, NULL);
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), NULL);

  box->config = config;

  box->configuration_changed_id =
    g_signal_connect_swapped (G_OBJECT (box->config), "configuration-changed",
                              G_CALLBACK (indicator_button_box_configuration_changed), box);

  return GTK_WIDGET (box);
}



void
indicator_button_box_disconnect_signals (IndicatorButtonBox *box)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON_BOX (box));

  if (box->configuration_changed_id != 0)
    {
      g_signal_handler_disconnect (box->config, box->configuration_changed_id);
      box->configuration_changed_id = 0;
    }
}



gboolean
indicator_button_box_is_small (IndicatorButtonBox *box)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_BUTTON_BOX (box), FALSE);

  if (box->cached)
    return box->is_small;

  // check sizes
  box->pixbuf_w = -1;
  box->pixbuf_h = -1;

  if (box->label != NULL)
    {
      box->is_small = FALSE;
      box->orientation = indicator_config_get_panel_orientation (box->config);
      if (box->orientation == GTK_ORIENTATION_VERTICAL &&
          indicator_config_get_orientation (box->config) == GTK_ORIENTATION_HORIZONTAL &&
          indicator_config_get_align_left (box->config))
        box->orientation = GTK_ORIENTATION_HORIZONTAL;
      gtk_label_set_angle (GTK_LABEL (box->label),
                           (indicator_config_get_orientation (box->config) == GTK_ORIENTATION_VERTICAL)
                           ? -90 : 0);
    }
  else if (box->icon != NULL &&
           gtk_image_get_storage_type (GTK_IMAGE (box->icon)) == GTK_IMAGE_PIXBUF)
    {
      GdkPixbuf *pixbuf = gtk_image_get_pixbuf (GTK_IMAGE (box->icon));

      box->pixbuf_w = gdk_pixbuf_get_width (pixbuf);
      box->pixbuf_h = gdk_pixbuf_get_height (pixbuf);

      box->is_small = (box->pixbuf_w == box->pixbuf_h);
    }
  else
    {
      box->is_small = TRUE;
    }

  box->cached = TRUE;

  return box->is_small;
}




static void
indicator_button_box_get_preferred_width (GtkWidget *widget,
                                          gint      *minimum_width,
                                          gint      *natural_width)
{
  IndicatorButtonBox  *box = XFCE_INDICATOR_BUTTON_BOX (widget);
  gint                 min_size, nat_size;

  if (indicator_button_box_is_small (box)) // check & cache
    {
      min_size = nat_size = ICON_SIZE;
    }
  else if (box->label != NULL)
    {
      gtk_widget_get_preferred_width (box->label, &min_size, &nat_size);

      if (box->icon != NULL && box->orientation == GTK_ORIENTATION_HORIZONTAL)
        {
          min_size = min_size + ICON_SIZE + SPACING;
          nat_size = nat_size + ICON_SIZE + SPACING;
        }
      else
        {
          min_size = MAX (min_size, ICON_SIZE);
          nat_size = MAX (nat_size, ICON_SIZE);
        }
    }
  else // rectangular icon
    {
      min_size = nat_size = MAX (box->pixbuf_w, ICON_SIZE);
    }

  if (minimum_width != NULL)
    *minimum_width = min_size;

  if (natural_width != NULL)
    *natural_width = nat_size;
}



static void
indicator_button_box_get_preferred_width_for_height (GtkWidget *widget,
                                                     gint       height,
                                                     gint      *minimum_width,
                                                     gint      *natural_width)
{
  indicator_button_box_get_preferred_width (widget, minimum_width, natural_width);
}




static void
indicator_button_box_get_preferred_height (GtkWidget *widget,
                                           gint      *minimum_height,
                                           gint      *natural_height)
{
  IndicatorButtonBox  *box = XFCE_INDICATOR_BUTTON_BOX (widget);
  gint                 min_size, nat_size;

  if (indicator_button_box_is_small (box)) // check & cache
    {
      min_size = nat_size = ICON_SIZE;
    }
  else if (box->label != NULL)
    {
      gtk_widget_get_preferred_height (box->label, &min_size, &nat_size);

      if (box->icon != NULL && box->orientation == GTK_ORIENTATION_VERTICAL)
        {
          min_size = min_size + ICON_SIZE + SPACING;
          nat_size = nat_size + ICON_SIZE + SPACING;
        }
      else
        {
          min_size = MAX (min_size, ICON_SIZE);
          nat_size = MAX (nat_size, ICON_SIZE);
        }
    }
  else // rectangular icon
    {
      min_size = nat_size = MAX (box->pixbuf_h, ICON_SIZE);
    }

  if (minimum_height != NULL)
    *minimum_height = min_size;

  if (natural_height != NULL)
    *natural_height = nat_size;
}



static void
indicator_button_box_get_preferred_height_for_width (GtkWidget *widget,
                                                     gint       width,
                                                     gint      *minimum_height,
                                                     gint      *natural_height)
{
  indicator_button_box_get_preferred_height (widget, minimum_height, natural_height);
}




static void
indicator_button_box_size_allocate (GtkWidget     *widget,
                                    GtkAllocation *allocation)
{
  IndicatorButtonBox  *box = XFCE_INDICATOR_BUTTON_BOX (widget);
  gint                 label_width, label_height;
  gint                 x, y, width, height;
  GtkAllocation        child_allocation;

  gtk_widget_set_allocation (widget, allocation);

  x = allocation->x;
  y = allocation->y;
  width  = allocation->width;
  height = allocation->height;

  if (indicator_button_box_is_small (box) && box->icon != NULL) // check & cache
    {
      child_allocation.x = x + (width - ICON_SIZE + 1) / 2;
      child_allocation.y = y + (height - ICON_SIZE + 1) / 2;
      child_allocation.width = ICON_SIZE;
      child_allocation.height = ICON_SIZE;
      gtk_widget_size_allocate (box->icon, &child_allocation);
    }
  else if (box->icon != NULL)
    {
      /* allocate icon */
      child_allocation.width = MAX (ICON_SIZE, box->pixbuf_w);
      child_allocation.height = MAX (ICON_SIZE, box->pixbuf_h);
      if (box->orientation == GTK_ORIENTATION_HORIZONTAL)
        {
          if (box->label != NULL)
            child_allocation.x = x;
          else
            child_allocation.x = x + (width - child_allocation.width + 1) / 2;
          child_allocation.y = y + (height - child_allocation.height + 1) / 2;
        }
      else
        {
          if (box->label != NULL)
            child_allocation.y = y;
          else
            child_allocation.y = y + (height - child_allocation.height + 1) / 2;
          child_allocation.x = x + (width - child_allocation.width + 1) / 2;
        }
      gtk_widget_size_allocate (box->icon, &child_allocation);
    }
  if (box->label != NULL)
    {
      /* allocate label */
      gtk_widget_get_preferred_width  (box->label, NULL, &label_width);
      gtk_widget_get_preferred_height (box->label, NULL, &label_height);

      if (box->orientation == GTK_ORIENTATION_HORIZONTAL)
        {
          if (box->icon != NULL)
            {
              child_allocation.x = x + ICON_SIZE + SPACING;
              child_allocation.width  = MAX (1, MIN (width - ICON_SIZE - SPACING, label_width));
            }
          else
            {
              child_allocation.x = x;
              child_allocation.width  = MAX (ICON_SIZE, MIN (width, label_width));
            }
          child_allocation.height = MAX (ICON_SIZE, MIN (height, label_height));
          child_allocation.y = y + (height - child_allocation.height + 1) / 2;
        }
      else
        {
          if (box->icon != NULL)
            {
              child_allocation.y = y + ICON_SIZE + SPACING;
              child_allocation.height = MAX (1, MIN (height - ICON_SIZE - SPACING, label_height));
            }
          else
            {
              child_allocation.height = MAX (ICON_SIZE, MIN (height, label_height));
              child_allocation.y = y + (height - child_allocation.height + 1) / 2;
            }
          child_allocation.width  = MAX (ICON_SIZE, MIN (width, label_width));
          child_allocation.x = x + (width - child_allocation.width + 1) / 2;
        }
      gtk_widget_size_allocate (box->label, &child_allocation);
    }
}

