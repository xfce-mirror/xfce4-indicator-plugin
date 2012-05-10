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
 *  This file implements an indicator button class corresponding to
 *  a single indicator object entry.
 *
 */



#include <glib.h>
#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libindicator/indicator-object.h>

#include "indicator-button.h"


#include <libindicator/indicator-object.h>
//#ifndef INDICATOR_OBJECT_SIGNAL_ENTRY_SCROLLED
//#define INDICATOR_OBJECT_SIGNAL_ENTRY_SCROLLED "scroll-entry"
//#endif


static void                 xfce_indicator_button_finalize        (GObject                *object);
static gint                 xfce_indicator_button_get_icon_size   (XfceIndicatorButton    *button);
static gboolean             xfce_indicator_button_button_press    (GtkWidget              *widget,
                                                                   GdkEventButton         *event);
static gboolean             xfce_indicator_button_scroll          (GtkWidget              *widget,
                                                                   GdkEventScroll         *event);
static void                 xfce_indicator_button_menu_deactivate (XfceIndicatorButton    *button,
                                                                   GtkMenu                *menu);


G_DEFINE_TYPE (XfceIndicatorButton, xfce_indicator_button, GTK_TYPE_TOGGLE_BUTTON)

static void
xfce_indicator_button_class_init (XfceIndicatorButtonClass *klass)
{
  GObjectClass      *gobject_class;
  GtkWidgetClass    *widget_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = xfce_indicator_button_finalize;

  widget_class = GTK_WIDGET_CLASS (klass);
  widget_class->button_press_event = xfce_indicator_button_button_press;
  widget_class->scroll_event = xfce_indicator_button_scroll;

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
  button->buttonbox = NULL;
  button->menu = NULL;

  button->label = NULL;
  button->orig_icon = NULL;
  button->icon = NULL;
  button->orig_icon_changed_id = 0;
  button->box_layout_changed_id = 0;

  button->align_box = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_container_add (GTK_CONTAINER (button), button->align_box);
  gtk_widget_show (button->align_box);

  button->box = xfce_hvbox_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 1);
  gtk_container_add (GTK_CONTAINER (button->align_box), button->box);
  gtk_widget_show (button->box);
}



static void
xfce_indicator_button_finalize (GObject *object)
{
  XfceIndicatorButton *button = XFCE_INDICATOR_BUTTON (object);

  xfce_indicator_button_disconnect_signals (button);

  if (button->label != NULL)
    g_object_unref (G_OBJECT (button->label));
  if (button->orig_icon != NULL)
    g_object_unref (G_OBJECT (button->orig_icon));
  if (button->icon != NULL)
    g_object_unref (G_OBJECT (button->icon));
  if (button->menu != NULL)
    g_object_unref (G_OBJECT (button->menu));
  if (button->buttonbox != NULL)
    g_object_unref (G_OBJECT (button->buttonbox));
  /* IndicatorObjectEntry is not GObject */
  /* if (button->entry != NULL) */
  /*   g_object_unref (G_OBJECT (button->entry)); */
  if (button->io != NULL)
    g_object_unref (G_OBJECT (button->io));

  G_OBJECT_CLASS (xfce_indicator_button_parent_class)->finalize (object);
}



static void
xfce_indicator_button_update_layout (XfceIndicatorButton *button)
{
  GtkRequisition          label_size;
  gfloat                  align_x;

  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));
  g_return_if_fail (XFCE_IS_INDICATOR_BOX (button->buttonbox));

  if (button->label != NULL)
    gtk_label_set_ellipsize (GTK_LABEL (button->label), PANGO_ELLIPSIZE_NONE);

  /* deskbar mode? */
  if (button->label != NULL &&
      xfce_indicator_box_get_panel_orientation (button->buttonbox) == GTK_ORIENTATION_VERTICAL &&
      xfce_indicator_box_get_indicator_orientation (button->buttonbox) == GTK_ORIENTATION_HORIZONTAL)
    {
      gtk_widget_size_request (button->label, &label_size);

      /* check if icon and label fit side by side */
      if (!xfce_indicator_box_get_align_left (button->buttonbox)
          || (button->icon != NULL
              && label_size.width >
              xfce_indicator_box_get_panel_size (button->buttonbox)
              - xfce_indicator_box_get_indicator_size (button->buttonbox)))
        {
          align_x = 0.5;
          gtk_orientable_set_orientation (GTK_ORIENTABLE (button->box), GTK_ORIENTATION_VERTICAL);
        }
      else
        {
          align_x = 0.0;
          gtk_orientable_set_orientation (GTK_ORIENTABLE (button->box), GTK_ORIENTATION_HORIZONTAL);
        }

      /* check if label alone fits in the panel */
      if (label_size.width > xfce_indicator_box_get_panel_size (button->buttonbox) - 6)
        {
          gtk_alignment_set (GTK_ALIGNMENT (button->align_box), align_x, 0.5, 1.0, 0.0);
          gtk_label_set_ellipsize (GTK_LABEL (button->label), PANGO_ELLIPSIZE_END);
        }
      else
        {
          gtk_alignment_set (GTK_ALIGNMENT (button->align_box), align_x, 0.5, 0.0, 0.0);
        }
    }
  else
    {
      gtk_alignment_set (GTK_ALIGNMENT (button->align_box), 0.5, 0.5, 0.0, 0.0);
      gtk_orientable_set_orientation (GTK_ORIENTABLE (button->box),
                                      xfce_indicator_box_get_indicator_orientation (button->buttonbox));
    }


  if (button->label != NULL)
    gtk_label_set_angle (GTK_LABEL (button->label),
                         (xfce_indicator_box_get_indicator_orientation (button->buttonbox) == GTK_ORIENTATION_VERTICAL)
                         ? -90 : 0);
  if (button->icon != NULL)
    xfce_panel_image_set_size (XFCE_PANEL_IMAGE (button->icon), xfce_indicator_button_get_icon_size (button));
}




static void
xfce_indicator_button_update_icon (XfceIndicatorButton *button)
{
  GdkPixbuf    *pixbuf_s, *pixbuf_d;
  gdouble       aspect;
  gint          size;

  g_return_if_fail (GTK_IS_IMAGE (button->orig_icon));
  g_return_if_fail (XFCE_IS_PANEL_IMAGE (button->icon));

  size = xfce_indicator_button_get_icon_size (button);

#if 0
  if (size > 16 && size < 22)
    size = 16;
  else if (size > 22 && size < 24)
    size = 22;
  else if (size > 24 && size < 32)
    size = 24;
#endif

  pixbuf_s = gtk_image_get_pixbuf (GTK_IMAGE (button->orig_icon));

  if (pixbuf_s != NULL)
    {
      aspect = (gdouble) gdk_pixbuf_get_width (pixbuf_s) /
        (gdouble) gdk_pixbuf_get_height (pixbuf_s);
      if (aspect > 1.0)
        pixbuf_d = gdk_pixbuf_scale_simple
          (pixbuf_s, size, (gint) (size / aspect),
           GDK_INTERP_BILINEAR);
      else
        pixbuf_d = gdk_pixbuf_scale_simple
          (pixbuf_s, (gint) (size * aspect), size,
           GDK_INTERP_BILINEAR);
      xfce_panel_image_set_from_pixbuf (XFCE_PANEL_IMAGE (button->icon), pixbuf_d);
    }
  else
    {
      xfce_panel_image_set_from_source (XFCE_PANEL_IMAGE (button->icon), "image-missing");
    }

  xfce_panel_image_set_size (XFCE_PANEL_IMAGE (button->icon), size);
}



static void
xfce_indicator_button_label_changed (GtkLabel            *label,
                                     GParamSpec          *pspec,
                                     XfceIndicatorButton *button)
{
  xfce_indicator_button_update_layout (button);
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
      g_signal_connect(G_OBJECT(button->label), "notify::label", G_CALLBACK(xfce_indicator_button_label_changed), button);
    }
  xfce_indicator_button_update_layout (button);
}




static void
on_pixbuf_changed (GtkImage *image, GParamSpec *pspec, XfceIndicatorButton *button)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));
  g_return_if_fail (GTK_IS_IMAGE (image));
  g_return_if_fail (XFCE_IS_PANEL_IMAGE (button->icon));

  xfce_indicator_button_update_icon (button);
}



void
xfce_indicator_button_set_image (XfceIndicatorButton *button,
                                 GtkImage            *image)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));
  g_return_if_fail (GTK_IS_IMAGE (image));

  g_debug ("indicator-button set image, image=%x\n", (uint) image);

  if (button->orig_icon != GTK_WIDGET (image))
    {
      if (button->orig_icon != NULL)
        {
          if (button->orig_icon_changed_id != 0)
            {
              g_signal_handler_disconnect (G_OBJECT (button->orig_icon), button->orig_icon_changed_id);
              button->orig_icon_changed_id = 0;
            }
          g_object_unref (G_OBJECT (button->orig_icon));
        }

      if (button->icon != NULL)
        {
          gtk_container_remove (GTK_CONTAINER (button->box), button->icon);
          g_object_unref (G_OBJECT (button->icon));
        }

      button->orig_icon = GTK_WIDGET (image);
      g_object_ref (G_OBJECT (button->orig_icon));

      button->orig_icon_changed_id = g_signal_connect
        (G_OBJECT (image), "notify::pixbuf", G_CALLBACK (on_pixbuf_changed), button);


      button->icon = xfce_panel_image_new ();
      xfce_indicator_button_update_icon (button);

      gtk_box_pack_start (GTK_BOX (button->box), button->icon, TRUE, FALSE, 1);
      gtk_widget_show (button->icon);

      xfce_indicator_button_update_layout (button);
    }
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
      g_signal_connect_swapped (G_OBJECT (button->menu), "deactivate",
                                G_CALLBACK (xfce_indicator_button_menu_deactivate), button);
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

  return button->orig_icon;
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



static gint
xfce_indicator_button_get_icon_size (XfceIndicatorButton *button)
{
  gint                 indicator_size;
  gint                 border_thickness;
  GtkStyle            *style;

  g_return_val_if_fail (XFCE_IS_INDICATOR_BUTTON (button), 24);
  g_return_val_if_fail (XFCE_IS_INDICATOR_BOX (button->buttonbox), 24);

  indicator_size = xfce_indicator_box_get_indicator_size (button->buttonbox);

  style = gtk_widget_get_style (GTK_WIDGET (button));
  border_thickness = 2 * MAX (style->xthickness, style->ythickness) + 2;

  return MIN (indicator_size - border_thickness,
              xfce_indicator_box_get_icon_size_max (button->buttonbox));
}



static void
xfce_indicator_button_box_layout_changed (XfceIndicatorButton *button,
                                          XfceIndicatorBox    *box)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));
  g_return_if_fail (XFCE_IS_INDICATOR_BOX (box));
  g_return_if_fail (GTK_WIDGET (button)->parent != NULL);

  //printf ("parent %x\n", (guint) GTK_WIDGET (button)->parent);

  if (button->orig_icon != NULL)
    xfce_indicator_button_update_icon (button);
  xfce_indicator_button_update_layout (button);
}



GtkWidget *
xfce_indicator_button_new (IndicatorObject      *io,
                           IndicatorObjectEntry *entry,
                           XfceIndicatorBox     *buttonbox)
{
  XfceIndicatorButton *button = g_object_new (XFCE_TYPE_INDICATOR_BUTTON, NULL);
  g_return_val_if_fail (XFCE_IS_INDICATOR_BOX (buttonbox), NULL);

  button->io = io;
  button->entry = entry;
  button->buttonbox = buttonbox;
  if (button->io != NULL)
    g_object_ref (G_OBJECT (button->io));
  /* IndicatorObjectEntry is not GObject */
  /* g_object_ref (G_OBJECT (button->entry)); */
  if (button->buttonbox != NULL)
    g_object_ref (G_OBJECT (button->buttonbox));

  button->box_layout_changed_id =
    g_signal_connect_swapped (G_OBJECT (button->buttonbox), "box-layout-changed",
                              G_CALLBACK (xfce_indicator_button_box_layout_changed), button);

  return GTK_WIDGET (button);
}



void
xfce_indicator_button_disconnect_signals (XfceIndicatorButton *button)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));

  if (button->box_layout_changed_id != 0)
    {
      g_signal_handler_disconnect (button->buttonbox, button->box_layout_changed_id);
      button->box_layout_changed_id = 0;
    }

  if (button->orig_icon_changed_id != 0)
    {
      g_signal_handler_disconnect (G_OBJECT (button->orig_icon), button->orig_icon_changed_id);
      button->orig_icon_changed_id = 0;
    }

}


static gboolean
xfce_indicator_button_button_press (GtkWidget      *widget,
                                    GdkEventButton *event)
{
  XfceIndicatorButton *button = XFCE_INDICATOR_BUTTON (widget);

  if(event->button == 1 && button->menu != NULL) /* left click only */
    {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);
      gtk_menu_popup (button->menu, NULL, NULL,
                      xfce_panel_plugin_position_menu,
                      xfce_indicator_box_get_plugin (button->buttonbox),
                      1, gtk_get_current_event_time ());
      return TRUE;
    }

  return FALSE;
}


static gboolean
xfce_indicator_button_scroll (GtkWidget *widget, GdkEventScroll *event)
{
  XfceIndicatorButton *button = XFCE_INDICATOR_BUTTON (widget);

  g_signal_emit_by_name (button->io, INDICATOR_OBJECT_SIGNAL_ENTRY_SCROLLED,
                         button->entry, 1, event->direction);

  return TRUE;
}


static void
xfce_indicator_button_menu_deactivate (XfceIndicatorButton *button,
                                       GtkMenu             *menu)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));
  g_return_if_fail (GTK_IS_MENU (menu));

  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
}
