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
  GtkWidget   *outer_container;

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

  button->size = 0;
  button->panel_size = 0;
  button->orig_icon_size = 0;
  button->panel_orientation = GTK_ORIENTATION_HORIZONTAL;
  button->orientation = GTK_ORIENTATION_HORIZONTAL;

  outer_container = gtk_table_new (1, 1, FALSE);
  gtk_container_add (GTK_CONTAINER (button), outer_container);
  gtk_widget_show (outer_container);

  button->box = xfce_hvbox_new (button->orientation, FALSE, 1);
  gtk_table_attach (GTK_TABLE (outer_container), button->box,
                    0, 1, 0, 1,
                    GTK_EXPAND | GTK_SHRINK, GTK_EXPAND | GTK_SHRINK, 0, 0);
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
xfce_indicator_button_update_layout (XfceIndicatorButton *button)
{
  GtkRequisition          label_size;
  gint                    border_thickness;
  GtkStyle               *style;
  gint                    icon_size;

  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));

  if (button->icon != NULL && button->size != 0)
    {
      style = gtk_widget_get_style (GTK_WIDGET (button));
      border_thickness = 2 * MAX (style->xthickness, style->ythickness) + 2;
      icon_size = MIN (button->size - border_thickness, button->orig_icon_size);

      if (button->label != NULL &&
          button->panel_orientation == GTK_ORIENTATION_VERTICAL &&
          button->orientation == GTK_ORIENTATION_HORIZONTAL)
        {
          gtk_widget_size_request (button->label, &label_size);

          /* put icon above the label if number of rows > 1 (they look better)
             or if they don't fit when arranged horizontally */
          if (button->panel_size != button->size ||
              label_size.width > button->panel_size - icon_size - border_thickness)
            gtk_orientable_set_orientation (GTK_ORIENTABLE (button->box), GTK_ORIENTATION_VERTICAL);
          else
            gtk_orientable_set_orientation (GTK_ORIENTABLE (button->box), GTK_ORIENTATION_HORIZONTAL);
        }

      xfce_panel_image_set_size (XFCE_PANEL_IMAGE (button->icon), icon_size);
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
  xfce_indicator_button_update_layout (button);
}




static void
on_pixbuf_changed (GtkImage *image, GParamSpec *pspec, XfceIndicatorButton *button)
{
  GdkPixbuf     *pixbuf;

  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));
  g_return_if_fail (GTK_IS_IMAGE (image));

  pixbuf = gtk_image_get_pixbuf (image);
  button->orig_icon_size = gdk_pixbuf_get_width (pixbuf);

  xfce_panel_image_set_from_pixbuf (button->icon, pixbuf);
}



void
xfce_indicator_button_set_image (XfceIndicatorButton *button,
                                 GtkImage            *image)
{
  GdkPixbuf     *pixbuf;

  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));
  g_return_if_fail (GTK_IS_IMAGE (image));

  if (button->icon != GTK_WIDGET (image))
    {
      if (button->icon != NULL)
        {
          gtk_container_remove (GTK_CONTAINER (button->box), button->icon);
          g_object_unref (G_OBJECT (button->icon));
        }

      pixbuf = gtk_image_get_pixbuf (image);
      g_signal_connect(G_OBJECT(image), "notify::pixbuf", G_CALLBACK(on_pixbuf_changed), button);
      button->orig_icon_size = gdk_pixbuf_get_width (pixbuf);

      button->icon = xfce_panel_image_new_from_pixbuf (pixbuf);

      gtk_box_pack_start (GTK_BOX (button->box), button->icon, TRUE, FALSE, 1);
      gtk_widget_show (button->icon);
    }
  xfce_indicator_button_update_layout (button);
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
                                       GtkOrientation       panel_orientation,
                                       GtkOrientation       orientation)
{
  gboolean    needs_update = FALSE;

  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));

  if (button->orientation != orientation)
    {
      button->orientation = orientation;

      if (button->label != NULL)
        gtk_label_set_angle (GTK_LABEL (button->label),
                             (orientation == GTK_ORIENTATION_VERTICAL) ? -90 : 0);
      needs_update = TRUE;
    }

  if (button->panel_orientation != panel_orientation)
    {
      button->panel_orientation = panel_orientation;
      needs_update = TRUE;
    }

  if (needs_update)
    {
      gtk_orientable_set_orientation (GTK_ORIENTABLE (button->box), orientation);
      xfce_indicator_button_update_layout (button);
    }
}



void
xfce_indicator_button_set_size (XfceIndicatorButton *button,
                                gint                 panel_size,
                                gint                 size)
{
  gboolean    needs_update = FALSE;

  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));

  if (button->size != size)
    {
      button->size = size;
      needs_update = TRUE;
    }

  if (button->panel_size != panel_size)
    {
      button->panel_size = panel_size;
      needs_update = TRUE;
    }

  if (needs_update)
    xfce_indicator_button_update_layout (button);
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



