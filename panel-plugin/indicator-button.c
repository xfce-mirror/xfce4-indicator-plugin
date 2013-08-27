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

//#if GTK_CHECK_VERSION (3, 0, 0) && !GTK_CHECK_VERSION (3, 10, 0)
#if GTK_CHECK_VERSION (3, 0, 0)
#define GTK_BUTTON_SIZING_FIX
#endif


static void                 xfce_indicator_button_finalize        (GObject                *object);
static gint                 xfce_indicator_button_get_icon_size   (XfceIndicatorButton    *button);
static gboolean             xfce_indicator_button_button_press    (GtkWidget              *widget,
                                                                   GdkEventButton         *event);
static gboolean             xfce_indicator_button_scroll          (GtkWidget              *widget,
                                                                   GdkEventScroll         *event);
static void                 xfce_indicator_button_menu_deactivate (XfceIndicatorButton    *button,
                                                                   GtkMenu                *menu);
#ifdef GTK_BUTTON_SIZING_FIX
static void            xfce_indicator_button_get_preferred_width  (GtkWidget              *widget,
                                                                   gint                   *minimum_width,
                                                                   gint                   *natural_width);
static void            xfce_indicator_button_get_preferred_height (GtkWidget              *widget,
                                                                   gint                   *minimum_height,
                                                                   gint                   *natural_height);
#endif


struct _XfceIndicatorButton
{
  GtkToggleButton       __parent__;

  IndicatorObject      *io;
  const gchar          *io_name;
  IndicatorObjectEntry *entry;
  GtkMenu              *menu;
  XfcePanelPlugin      *plugin;
  IndicatorConfig      *config;

  GtkWidget            *align_box;
  GtkWidget            *box;
  GtkWidget            *label;
  GtkWidget            *icon;
  GtkWidget            *orig_icon;
  gboolean              rectangular_icon;

  gulong                orig_icon_changed_id;
  gulong                configuration_changed_id;
};

struct _XfceIndicatorButtonClass
{
  GtkToggleButtonClass __parent__;
};




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
#ifdef GTK_BUTTON_SIZING_FIX
  widget_class->get_preferred_width = xfce_indicator_button_get_preferred_width;
  widget_class->get_preferred_height = xfce_indicator_button_get_preferred_height;
#endif
}



static void
xfce_indicator_button_init (XfceIndicatorButton *button)
{
  //GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET (button), GTK_CAN_DEFAULT | GTK_CAN_FOCUS);
  gtk_widget_set_can_focus(GTK_WIDGET(button), FALSE);
  gtk_widget_set_can_default (GTK_WIDGET (button), FALSE);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_button_set_use_underline (GTK_BUTTON (button),TRUE);
  gtk_button_set_focus_on_click (GTK_BUTTON (button), FALSE);
  gtk_widget_set_name (GTK_WIDGET (button), "indicator-button");

  button->io = NULL;
  button->entry = NULL;
  button->plugin = NULL;
  button->config = NULL;
  button->menu = NULL;

  button->label = NULL;
  button->orig_icon = NULL;
  button->icon = NULL;
  button->orig_icon_changed_id = 0;
  button->configuration_changed_id = 0;
  button->rectangular_icon = FALSE;

  button->align_box = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
  gtk_container_add (GTK_CONTAINER (button), button->align_box);
  gtk_widget_show (button->align_box);

  //button->box = xfce_hvbox_new (GTK_ORIENTATION_HORIZONTAL, FALSE, 1);
  button->box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 2);
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
  gint                    border_thickness;
  GtkStyleContext        *ctx;
  GtkBorder               padding, border;

  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));

  if (button->label != NULL)
    gtk_label_set_ellipsize (GTK_LABEL (button->label), PANGO_ELLIPSIZE_NONE);

  /* deskbar mode? */
  if (button->label != NULL &&
      indicator_config_get_panel_orientation (button->config) == GTK_ORIENTATION_VERTICAL &&
      indicator_config_get_orientation (button->config) == GTK_ORIENTATION_HORIZONTAL)
    {
      ctx = gtk_widget_get_style_context (GTK_WIDGET (button));
      gtk_style_context_get_padding (ctx,
                                     gtk_widget_get_state_flags (GTK_WIDGET (button)),
                                     &padding);
      gtk_style_context_get_border (ctx,
                                    gtk_widget_get_state_flags (GTK_WIDGET (button)),
                                    &border);
      border_thickness = MAX (padding.left+padding.right+border.left+border.right,
                              padding.top+padding.bottom+border.top+border.bottom);

      gtk_widget_get_preferred_size (button->label, NULL, &label_size);

      /* check if icon and label fit side by side */
      if (!indicator_config_get_align_left (button->config)
          || (button->icon != NULL
              && label_size.width >
              indicator_config_get_panel_size (button->config) - 2 - border_thickness
              - xfce_indicator_button_get_icon_size (button)))
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
      if (label_size.width > indicator_config_get_panel_size (button->config) - border_thickness)
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
                                      indicator_config_get_orientation (button->config));
    }


  if (button->label != NULL)
    gtk_label_set_angle (GTK_LABEL (button->label),
                         (indicator_config_get_orientation (button->config) == GTK_ORIENTATION_VERTICAL)
                         ? -90 : 0);
}




static void
xfce_indicator_button_update_icon (XfceIndicatorButton *button)
{
  GdkPixbuf        *pixbuf_s, *pixbuf_d;
  gdouble           aspect;
  gint              w, h, w2, h2, size;
  gint              border_thickness;
  GtkStyleContext  *ctx;
  GtkBorder         padding, border;
  GtkImageType      storage;

  g_return_if_fail (GTK_IS_IMAGE (button->orig_icon));
  g_return_if_fail (GTK_IS_IMAGE (button->icon));

  size = xfce_indicator_button_get_icon_size (button);

  if (size > 22 && size <= 26)
    size = 22;

#if 0
  if (size > 16 && size < 22)
    size = 16;
  else if (size > 22 && size < 24)
    size = 22;
  else if (size > 24 && size < 32)
    size = 24;
#endif

  storage = gtk_image_get_storage_type (GTK_IMAGE (button->orig_icon));

  if (storage == GTK_IMAGE_PIXBUF || storage == GTK_IMAGE_EMPTY)
    {
      pixbuf_s = gtk_image_get_pixbuf (GTK_IMAGE (button->orig_icon));
    }
  else if (storage == GTK_IMAGE_GICON)
    {
      GIcon *icon = NULL;
      GtkIconSize size;
      gtk_image_get_gicon (GTK_IMAGE (button->orig_icon), &icon, &size);
      //printf ("G_ICON %s ... %d\n", g_icon_to_string (icon), size);
      gtk_image_set_from_gicon (GTK_IMAGE (button->icon),
                                icon, size);
      return;
    }
  else if (storage == GTK_IMAGE_ICON_NAME)
    {
      const gchar *icon_name = NULL;
      GtkIconSize size;
      gtk_image_get_icon_name (GTK_IMAGE (button->orig_icon), &icon_name, &size);
      //printf ("ICON_NAME %s ... %d\n", icon_name, size);
      gtk_image_set_from_icon_name (GTK_IMAGE (button->icon),
                                    icon_name, size);
      return;
    }
  else
    {
      printf ("Unrecognized storage type");
      return;
    }

  if (pixbuf_s != NULL)
    {
      w = gdk_pixbuf_get_width (pixbuf_s);
      h = gdk_pixbuf_get_height (pixbuf_s);
      aspect = (gdouble) w / (gdouble) h;
      ctx = gtk_widget_get_style_context (GTK_WIDGET (button));
      gtk_style_context_get_padding (ctx,
                                     gtk_widget_get_state_flags (GTK_WIDGET (button)),
                                     &padding);
      gtk_style_context_get_border (ctx,
                                    gtk_widget_get_state_flags (GTK_WIDGET (button)),
                                    &border);
      border_thickness = MAX (padding.left+padding.right+border.left+border.right,
                              padding.top+padding.bottom+border.top+border.bottom);

      button->rectangular_icon = (w != h);

      if (indicator_config_get_panel_orientation (button->config) == GTK_ORIENTATION_VERTICAL &&
          size * aspect > indicator_config_get_panel_size (button->config) - border_thickness)
        {
          w2 = indicator_config_get_panel_size (button->config) - border_thickness;
          h2 = (gint) (w2 / aspect);
        }
      else
        {
          h2 = size;
          w2 = (gint) (h2 * aspect);
        }

      if (h2 == h)
        {
          gtk_image_set_from_pixbuf (GTK_IMAGE (button->icon), pixbuf_s);
        }
      else
        {
          pixbuf_d = gdk_pixbuf_scale_simple (pixbuf_s, w2, h2, GDK_INTERP_BILINEAR);
          gtk_image_set_from_pixbuf (GTK_IMAGE (button->icon), pixbuf_d);
          g_object_unref (G_OBJECT (pixbuf_d));
        }
    }
  else
    {
      gtk_image_set_from_icon_name (GTK_IMAGE (button->icon),
                                    "image-missing", GTK_ICON_SIZE_MENU);
    }
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
  g_return_if_fail (GTK_IS_IMAGE (button->icon));

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

      button->icon = gtk_image_new ();
      xfce_indicator_button_update_icon (button);

      gtk_box_pack_start (GTK_BOX (button->box), button->icon, TRUE, FALSE, 2);
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



const gchar *
xfce_indicator_button_get_io_name (XfceIndicatorButton *button)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_BUTTON (button), NULL);

  return button->io_name;
}



guint
xfce_indicator_button_get_pos (XfceIndicatorButton *button)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_BUTTON (button), 0);

  return indicator_object_get_location (button->io, button->entry);
}







GtkMenu *
xfce_indicator_button_get_menu (XfceIndicatorButton *button)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_BUTTON (button), NULL);

  return button->menu;
}





gboolean
xfce_indicator_button_is_icon_rectangular (XfceIndicatorButton *button)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_BUTTON (button), FALSE);

  return button->rectangular_icon;
}




static gint
xfce_indicator_button_get_icon_size (XfceIndicatorButton *button)
{
  gint                 indicator_size;
  gint                 border_thickness;
  GtkStyleContext     *ctx;
  GtkBorder            padding, border;

  g_return_val_if_fail (XFCE_IS_INDICATOR_BUTTON (button), 22);

  indicator_size = MIN (indicator_config_get_panel_size (button->config) /
                        indicator_config_get_nrows (button->config),
                        indicator_config_get_row_size_max (button->config));

  ctx = gtk_widget_get_style_context (GTK_WIDGET (button));
  gtk_style_context_get_padding (ctx, gtk_widget_get_state_flags (GTK_WIDGET (button)), &padding);
  gtk_style_context_get_border (ctx, gtk_widget_get_state_flags (GTK_WIDGET (button)), &border);
  border_thickness = MAX (padding.left+padding.right+border.left+border.right,
                          padding.top+padding.bottom+border.top+border.bottom);

  return indicator_size - border_thickness;
}



static void
xfce_indicator_configuration_changed (XfceIndicatorButton *button,
                                      IndicatorConfig     *config)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));
  g_return_if_fail (XFCE_IS_INDICATOR_CONFIG (config));
  g_return_if_fail (gtk_widget_get_parent (GTK_WIDGET (button)) != NULL);

  if (button->orig_icon != NULL)
    xfce_indicator_button_update_icon (button);
  xfce_indicator_button_update_layout (button);
}



GtkWidget *
xfce_indicator_button_new (IndicatorObject      *io,
                           const gchar          *io_name,
                           IndicatorObjectEntry *entry,
                           XfcePanelPlugin      *plugin,
                           IndicatorConfig      *config)
{
  XfceIndicatorButton *button = g_object_new (XFCE_TYPE_INDICATOR_BUTTON, NULL);
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), NULL);
  g_return_val_if_fail (XFCE_IS_PANEL_PLUGIN (plugin), NULL);

  button->io = io;
  button->io_name = io_name;
  button->entry = entry;
  button->plugin = plugin;
  button->config = config;

  if (button->io != NULL)
    g_object_ref (G_OBJECT (button->io));
  /* IndicatorObjectEntry is not GObject */
  /* g_object_ref (G_OBJECT (button->entry)); */

  button->configuration_changed_id =
    g_signal_connect_swapped (G_OBJECT (button->config), "configuration-changed",
                              G_CALLBACK (xfce_indicator_configuration_changed), button);

  return GTK_WIDGET (button);
}



void
xfce_indicator_button_disconnect_signals (XfceIndicatorButton *button)
{
  g_return_if_fail (XFCE_IS_INDICATOR_BUTTON (button));

  if (button->menu != 0)
    {
      gtk_menu_popdown (button->menu);
    }

  if (button->configuration_changed_id != 0)
    {
      g_signal_handler_disconnect (button->config, button->configuration_changed_id);
      button->configuration_changed_id = 0;
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
      gtk_menu_reposition (GTK_MENU (button->menu));
      gtk_menu_popup (button->menu, NULL, NULL,
                      xfce_panel_plugin_position_menu, button->plugin,
                      event->button, event->time);
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




#ifdef GTK_BUTTON_SIZING_FIX
/* When can_focus is true, GtkButton allocates larger size than requested *
 * and causes the panel image to grow indefinitely.                       *
 * This workaround compensates for this difference.                       *
 * Details in https://bugzilla.gnome.org/show_bug.cgi?id=698030           *
 */
static gint
xfce_indicator_button_padding_correction (GtkWidget *widget)
{
  GtkStyleContext       *context;
  gint                   focus_width;
  gint                   focus_pad;
  gint                   correction;

  if (!gtk_widget_get_can_focus (widget))
    {
      context = gtk_widget_get_style_context (widget);
      gtk_style_context_get_style (context,
                                   "focus-line-width", &focus_width,
                                   "focus-padding", &focus_pad,
                                   NULL);
      correction = (focus_width + focus_pad) * 2;
    }
  else
    {
      correction = 0;
    }

  return correction;
}



static void
xfce_indicator_button_get_preferred_width (GtkWidget *widget,
                                           gint      *minimum_width,
                                           gint      *natural_width)
{
  gint correction;

  (*GTK_WIDGET_CLASS (xfce_indicator_button_parent_class)->get_preferred_width) (widget, minimum_width, natural_width);

  correction = xfce_indicator_button_padding_correction (widget);

  if (minimum_width != NULL)
    *minimum_width = MAX (0, *minimum_width - correction);

  if (natural_width != NULL)
    *natural_width = MAX (0, *natural_width - correction);
}



static void
xfce_indicator_button_get_preferred_height (GtkWidget *widget,
                                            gint      *minimum_height,
                                            gint      *natural_height)
{
  gint correction;

  (*GTK_WIDGET_CLASS (xfce_indicator_button_parent_class)->get_preferred_height) (widget, minimum_height, natural_height);

  correction = xfce_indicator_button_padding_correction (widget);

  if (minimum_height != NULL)
    *minimum_height = MAX (0, *minimum_height - correction);

  if (natural_height != NULL)
    *natural_height = MAX (0, *natural_height - correction);
}
#endif
