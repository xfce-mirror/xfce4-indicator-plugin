/*  $Id: indicator.c 2257 2006-12-19 19:49:00Z nick $
 *
 *  Copyright (c) 2006-2007 John Doo <john@foo.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4panel/xfce-hvbox.h>

#include "indicator.h"
#include "indicator-dialogs.h"

/* default settings */
#define DEFAULT_SETTING1 NULL
#define DEFAULT_SETTING2 1
#define DEFAULT_SETTING3 FALSE



/* prototypes */
static void
indicator_construct (XfcePanelPlugin *plugin);


/* register the plugin */
XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (indicator_construct);



void
indicator_save (XfcePanelPlugin *plugin,
             IndicatorPlugin    *indicator)
{
  XfceRc *rc;
  gchar  *file;

  /* get the config file location */
  file = xfce_panel_plugin_save_location (plugin, TRUE);

  if (G_UNLIKELY (file == NULL))
    {
       DBG ("Failed to open config file");
       return;
    }

  /* open the config file, read/write */
  rc = xfce_rc_simple_open (file, FALSE);
  g_free (file);

  if (G_LIKELY (rc != NULL))
    {
      /* save the settings */
      DBG(".");
      if (indicator->setting1)
        xfce_rc_write_entry    (rc, "setting1", indicator->setting1);

      xfce_rc_write_int_entry  (rc, "setting2", indicator->setting2);
      xfce_rc_write_bool_entry (rc, "setting3", indicator->setting3);

      /* close the rc file */
      xfce_rc_close (rc);
    }
}



static void
indicator_read (IndicatorPlugin *indicator)
{
  XfceRc      *rc;
  gchar       *file;
  const gchar *value;

  /* get the plugin config file location */
  file = xfce_panel_plugin_save_location (indicator->plugin, TRUE);

  if (G_LIKELY (file != NULL))
    {
      /* open the config file, readonly */
      rc = xfce_rc_simple_open (file, TRUE);

      /* cleanup */
      g_free (file);

      if (G_LIKELY (rc != NULL))
        {
          /* read the settings */
          value = xfce_rc_read_entry (rc, "setting1", DEFAULT_SETTING1);
          indicator->setting1 = g_strdup (value);

          indicator->setting2 = xfce_rc_read_int_entry (rc, "setting2", DEFAULT_SETTING2);
          indicator->setting3 = xfce_rc_read_bool_entry (rc, "setting3", DEFAULT_SETTING3);

          /* cleanup */
          xfce_rc_close (rc);

          /* leave the function, everything went well */
          return;
        }
    }

  /* something went wrong, apply default values */
  DBG ("Applying default settings");

  indicator->setting1 = g_strdup (DEFAULT_SETTING1);
  indicator->setting2 = DEFAULT_SETTING2;
  indicator->setting3 = DEFAULT_SETTING3;
}



static IndicatorPlugin *
indicator_new (XfcePanelPlugin *plugin)
{
  IndicatorPlugin   *indicator;
  GtkOrientation  orientation;
  GtkWidget      *label;

  /* allocate memory for the plugin structure */
  indicator = panel_slice_new0 (IndicatorPlugin);

  /* pointer to plugin */
  indicator->plugin = plugin;

  /* read the user settings */
  indicator_read (indicator);

  /* get the current orientation */
  orientation = xfce_panel_plugin_get_orientation (plugin);

  /* TODO: Create menubar */
  /* create some panel widgets */
  indicator->ebox = gtk_event_box_new ();
  gtk_widget_show (indicator->ebox);

  indicator->hvbox = xfce_hvbox_new (orientation, FALSE, 2);
  gtk_widget_show (indicator->hvbox);
  gtk_container_add (GTK_CONTAINER (indicator->ebox), indicator->hvbox);

  /* some indicator widgets */
  label = gtk_label_new (_("Indicator"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (indicator->hvbox), label, FALSE, FALSE, 0);

  label = gtk_label_new (_("Plugin"));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (indicator->hvbox), label, FALSE, FALSE, 0);

  return indicator;
}



static void
indicator_free (XfcePanelPlugin *plugin,
             IndicatorPlugin    *indicator)
{
  GtkWidget *dialog;

  /* check if the dialog is still open. if so, destroy it */
  dialog = g_object_get_data (G_OBJECT (plugin), "dialog");
  if (G_UNLIKELY (dialog != NULL))
    gtk_widget_destroy (dialog);

  /* destroy the panel widgets */
  gtk_widget_destroy (indicator->hvbox);

  /* cleanup the settings */
  if (G_LIKELY (indicator->setting1 != NULL))
    g_free (indicator->setting1);

  /* free the plugin structure */
  panel_slice_free (IndicatorPlugin, indicator);
}



static void
indicator_orientation_changed (XfcePanelPlugin *plugin,
                            GtkOrientation   orientation,
                            IndicatorPlugin    *indicator)
{
  /* change the orienation of the box */
  xfce_hvbox_set_orientation (XFCE_HVBOX (indicator->hvbox), orientation);
}



static gboolean
indicator_size_changed (XfcePanelPlugin *plugin,
                     gint             size,
                     IndicatorPlugin    *indicator)
{
  GtkOrientation orientation;

  /* get the orientation of the plugin */
  orientation = xfce_panel_plugin_get_orientation (plugin);

  /* set the widget size */
  if (orientation == GTK_ORIENTATION_HORIZONTAL)
    gtk_widget_set_size_request (GTK_WIDGET (plugin), -1, size);
  else
    gtk_widget_set_size_request (GTK_WIDGET (plugin), size, -1);

  /* we handled the orientation */
  return TRUE;
}



static void
indicator_construct (XfcePanelPlugin *plugin)
{
  IndicatorPlugin *indicator;

  /* setup transation domain */
  xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* create the plugin */
  indicator = indicator_new (plugin);

  /* add the ebox to the panel */
  gtk_container_add (GTK_CONTAINER (plugin), indicator->ebox);

  /* show the panel's right-click menu on this ebox */
  xfce_panel_plugin_add_action_widget (plugin, indicator->ebox);

  /* connect plugin signals */
  g_signal_connect (G_OBJECT (plugin), "free-data",
                    G_CALLBACK (indicator_free), indicator);

  g_signal_connect (G_OBJECT (plugin), "save",
                    G_CALLBACK (indicator_save), indicator);

  g_signal_connect (G_OBJECT (plugin), "size-changed",
                    G_CALLBACK (indicator_size_changed), indicator);

  g_signal_connect (G_OBJECT (plugin), "orientation-changed",
                    G_CALLBACK (indicator_orientation_changed), indicator);

  /* show the configure menu item and connect signal */
  xfce_panel_plugin_menu_show_configure (plugin);
  g_signal_connect (G_OBJECT (plugin), "configure-plugin",
                    G_CALLBACK (indicator_configure), indicator);

  /* show the about menu item and connect signal */
  xfce_panel_plugin_menu_show_about (plugin);
  g_signal_connect (G_OBJECT (plugin), "about",
                    G_CALLBACK (indicator_about), NULL);
}
