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

#include <glib.h>
#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libindicator/indicator-object.h>

#include "indicator-button.h"

static void                 xfce_indicator_button_finalize     (GObject *object);


G_DEFINE_TYPE (XfceIndicatorButton, xfce_indicator_button, GTK_TYPE_TOGGLE_BUTTON)

static void
xfce_indicator_button_class_init (XfceIndicatorButtonClass *klass)
{
  GObjectClass   *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_indicator_button_finalize;
}



static void
xfce_indicator_button_init (XfceIndicatorButton *button)
{
  GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET (button), GTK_CAN_DEFAULT | GTK_CAN_FOCUS);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_button_set_use_underline (GTK_BUTTON (button),TRUE);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  gtk_widget_set_name (GTK_WIDGET (button), "indicator-button");

  button->io = NULL;
  button->entry = NULL;
  button->menu = NULL;

  button->label = NULL;
  button->icon = NULL;
  button->size = -1;
  button->panel_size = -1;
  button->panel_orientation = GTK_ORIENTATION_HORIZONTAL;
  button->orientation = GTK_ORIENTATION_HORIZONTAL;


  button->outer_box = xfce_hvbox_new (GTK_ORIENTATION_VERTICAL, FALSE, 1);
  gtk_container_add (GTK_CONTAINER (button), button->outer_box);
  gtk_widget_show (button->outer_box);

  button->box = xfce_hvbox_new (button->orientation, FALSE, 1);
  gtk_box_pack_start (GTK_BOX (button->outer_box), button->box, TRUE, FALSE, 0);
  gtk_widget_show (button->box);
}



static void
xfce_indicator_button_finalize (GObject *object)
{
  XfceIndicatorButton *button = XFCE_INDICATOR_BUTTON (object);

  if (button->label != NULL)
    {
      g_object_unref (G_OBJECT (button->label));
      button->label = NULL;
    }
  if (button->icon != NULL)
    {
      g_object_unref (G_OBJECT (button->icon));
      button->icon = NULL;
    }
  if (button->io != NULL)
    {
      g_object_unref (G_OBJECT (button->io));
      button->io = NULL;
    }
  if (button->entry != NULL)
    {
      g_object_unref (G_OBJECT (button->entry));
      button->entry = NULL;
    }
  if (button->menu != NULL)
    {
      g_object_unref (G_OBJECT (button->menu));
      button->menu = NULL;
    }

  G_OBJECT_CLASS (xfce_indicator_button_parent_class)->finalize (object);
}



static void
xfce_indicator_button_check_label_size (XfceIndicatorButton *button)
{
  GtkRequisition          label_size;
  gint                    width, border_thickness;
  GtkStyle               *style;

  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));

  if (button->panel_orientation == GTK_ORIENTATION_VERTICAL &&
      button->orientation == GTK_ORIENTATION_HORIZONTAL &&
      button->icon != NULL &&
      button->label != NULL)
    {
      gtk_widget_size_request (button->label, &label_size);

      width = gdk_pixbuf_get_width (gtk_image_get_pixbuf (button->icon));
      style = gtk_widget_get_style (GTK_WIDGET (button));
      border_thickness = 2 * MAX (style->xthickness, style->ythickness) + 2;

      if (label_size.width > button->panel_size - width - border_thickness)
        gtk_orientable_set_orientation (GTK_ORIENTABLE (button->box), GTK_ORIENTATION_VERTICAL);
      else
        gtk_orientable_set_orientation (GTK_ORIENTABLE (button->box), GTK_ORIENTATION_HORIZONTAL);
    }
}



void
xfce_indicator_button_set_label (XfceIndicatorButton *button,
                                 GtkLabel            *label)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));
  g_return_if_fail (GTK_IS_LABEL (label));

  if (button->label != GTK_WIDGET (label))
    {
      if (button->label != NULL)
        {
          gtk_container_remove (GTK_CONTAINER (button->box), button->label);
          g_object_unref (G_OBJECT (button->label));
        }

      button->label = GTK_WIDGET (label);
      g_object_ref (G_OBJECT (button->label));
      gtk_box_pack_end (GTK_BOX (button->box), button->label, TRUE, FALSE, 1);
    }
  xfce_indicator_button_check_label_size (button);
}



void
xfce_indicator_button_set_image (XfceIndicatorButton *button,
                                 GtkImage            *image)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));
  g_return_if_fail (GTK_IS_IMAGE (image));

  if (button->icon != GTK_WIDGET (image))
    {
      if (button->icon != NULL)
        {
          gtk_container_remove (GTK_CONTAINER (button->box), button->icon);
          g_object_unref (G_OBJECT (button->icon));
        }

      button->icon = GTK_WIDGET (image);
      g_object_ref (G_OBJECT (button->icon));
      gtk_box_pack_start (GTK_BOX (button->box), button->icon, TRUE, FALSE, 1);
    }
  xfce_indicator_button_check_label_size (button);
}



void
xfce_indicator_button_set_menu (XfceIndicatorButton *button,
                                GtkMenu             *menu)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));
  g_return_if_fail (GTK_IS_MENU (menu));

  if (button->menu != menu)
    {
      if (button->menu != NULL)
        g_object_unref (G_OBJECT (button->menu));
      button->menu = menu;
      g_object_ref (G_OBJECT (button->menu));
      gtk_menu_attach_to_widget(menu, GTK_WIDGET (button), NULL);
    }
}



GtkWidget *
xfce_indicator_button_get_label (XfceIndicatorButton *button)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_BUTTON (button), NULL);

  return button->label;
}



GtkWidget *
xfce_indicator_button_get_image (XfceIndicatorButton *button)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_BUTTON (button), NULL);

  return button->icon;
}



IndicatorObjectEntry *
xfce_indicator_button_get_entry (XfceIndicatorButton *button)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_BUTTON (button), NULL);

  return button->entry;
}



IndicatorObject *
xfce_indicator_button_get_io (XfceIndicatorButton *button)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_BUTTON (button), NULL);

  return button->io;
}



GtkMenu *
xfce_indicator_button_get_menu (XfceIndicatorButton *button)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_BUTTON (button), NULL);

  return button->menu;
}



void
xfce_indicator_button_set_orientation (XfceIndicatorButton *button,
                                       GtkOrientation       orientation)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));

  button->orientation = orientation;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (button->box), orientation);

  if (button->label != NULL)
    gtk_label_set_angle (GTK_LABEL (button->label),
                         (orientation == GTK_ORIENTATION_VERTICAL) ? -90 : 0);
  xfce_indicator_button_check_label_size (button);
}



void
xfce_indicator_button_set_panel_orientation (XfceIndicatorButton *button,
                                             GtkOrientation       orientation)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));

  button->panel_orientation = orientation;

  gtk_orientable_set_orientation
    (GTK_ORIENTABLE (button->outer_box),
     (orientation == GTK_ORIENTATION_HORIZONTAL) ?
     GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL);
  xfce_indicator_button_check_label_size (button);
}



void
xfce_indicator_button_set_panel_size (XfceIndicatorButton *button,
                                      gint                 size)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));

  button->panel_size = size;
  xfce_indicator_button_check_label_size (button);
}



void
xfce_indicator_button_set_size (XfceIndicatorButton *button,
                                gint                 size)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));

  button->size = size;

  size -= (2 + 2 * MAX (GTK_WIDGET (button)->style->xthickness,
                        GTK_WIDGET (button)->style->ythickness));
  if (button->icon != NULL)
    {
      gtk_widget_set_size_request (button->icon, size, size);
      xfce_indicator_button_check_label_size (button);
    }
}



GtkWidget *
xfce_indicator_button_new (IndicatorObject *io,
                           IndicatorObjectEntry *entry)
{
  XfceIndicatorButton *button = g_object_new (XFCE_TYPE_INDICATOR_BUTTON, NULL);
  button->io = io;
  button->entry = entry;
  g_object_ref (G_OBJECT (button->io));
  g_object_ref (G_OBJECT (button->entry));
  return GTK_WIDGET (button);
}



