/*  Copyright (c) 2009 Mark Trompell <mark@foresightlinux.org>
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
#include <libindicator/indicator-object.h>

#include "indicator.h"

/* default settings */
#define DEFAULT_SETTING1 NULL
#define DEFAULT_SETTING2 1
#define DEFAULT_SETTING3 FALSE

/* prototypes */
static void
indicator_construct (XfcePanelPlugin *plugin);

static gboolean
load_module (const gchar * name, GtkWidget * menu);

static gboolean
on_menu_press (GtkWidget *widget, GdkEventButton *event, IndicatorPlugin *indicator);


/* register the plugin */
XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (indicator_construct);


#if 0
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
#endif


static IndicatorPlugin *
indicator_new (XfcePanelPlugin *plugin)
{
  IndicatorPlugin   *indicator;
  GtkOrientation  orientation;
  gint indicators_loaded = 0;

  /* allocate memory for the plugin structure */
  indicator = panel_slice_new0 (IndicatorPlugin);

  /* pointer to plugin */
  indicator->plugin = plugin;

  /* get the current orientation */
  orientation = xfce_panel_plugin_get_orientation (plugin);

  /* Init some theme/icon stuff */
  gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(),
                                  INDICATOR_ICONS_DIR);
  /* g_debug("Icons directory: %s", INDICATOR_ICONS_DIR); */
  gtk_rc_parse_string (
    "style \"indicator-applet-style\"\n"
    "{\n"
    "    GtkMenuBar::shadow-type = none\n"
    "    GtkMenuBar::internal-padding = 0\n"
    "    GtkWidget::focus-line-width = 0\n"
    "    GtkWidget::focus-padding = 0\n"
    "}\n"
    "style \"indicator-applet-menubar-style\"\n"
    "{\n"
    "    GtkMenuBar::shadow-type = none\n"
    "    GtkMenuBar::internal-padding = 0\n"
    "    GtkWidget::focus-line-width = 0\n"
    "    GtkWidget::focus-padding = 0\n"
    "    GtkMenuItem::horizontal-padding = 0\n"
    "}\n"
    "style \"indicator-applet-menuitem-style\"\n"
    "{\n"
    "    GtkWidget::focus-line-width = 0\n"
    "    GtkWidget::focus-padding = 0\n"
    "    GtkMenuItem::horizontal-padding = 0\n"
    "}\n"
    "widget \"*.indicator-applet\" style \"indicator-applet-style\""
    "widget \"*.indicator-applet-menuitem\" style \"indicator-applet-menuitem-style\""
    "widget \"*.indicator-applet-menubar\" style \"indicator-applet-menubar-style\"");
  gtk_widget_set_name(GTK_WIDGET (plugin), "indicator-applet-menubar");
  /* create some panel widgets */
  
  /* Build menu */
  indicator->menu = gtk_menu_bar_new();
  GTK_WIDGET_SET_FLAGS (indicator->menu, GTK_WIDGET_FLAGS(indicator->menu) | GTK_CAN_FOCUS);
  gtk_widget_set_name(GTK_WIDGET (indicator->menu), "indicator-applet-menubar");
  g_signal_connect(indicator->menu, "button-press-event", G_CALLBACK(on_menu_press), NULL);
  //g_signal_connect_after(indicator->menu, "expose-event", G_CALLBACK(menu_on_expose), menu);
  gtk_container_set_border_width(GTK_CONTAINER(indicator->menu), 0);

  
  /* load 'em */
  if (g_file_test(INDICATOR_DIR, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
    GDir * dir = g_dir_open(INDICATOR_DIR, 0, NULL);

    const gchar * name;
    while ((name = g_dir_read_name(dir)) != NULL) {
      if (load_module(name, indicator->menu))
        indicators_loaded++;
    }
    g_dir_close (dir);
  }

  if (indicators_loaded == 0) {
    /* A label to allow for click through */
    indicator->item = xfce_create_panel_button();
    gtk_button_set_label(indicator->item, _("No Indicators"));
    gtk_widget_show(indicator->item);
    gtk_container_add (GTK_CONTAINER (plugin), indicator->item);
  } else {
    gtk_widget_show(indicator->menu);
    gtk_container_add (GTK_CONTAINER (plugin), indicator->menu);
  }
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

  /* free the plugin structure */
  panel_slice_free (IndicatorPlugin, indicator);
}



static void
indicator_orientation_changed (XfcePanelPlugin *plugin,
                            GtkOrientation   orientation,
                            IndicatorPlugin    *indicator)
{
  /* change the orienation of the box */
  //xfce_hvbox_set_orientation (XFCE_HVBOX (indicator->hvbox), orientation);
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


static gboolean
on_menu_press (GtkWidget *widget, GdkEventButton *event, IndicatorPlugin *indicator)
{
    TRACE ("enters on_button_press");
    if (indicator != NULL && event->button == 1) /* left click only */
    {
     /*   gtk_menu_popup (GTK_MENU(indicator->menu), NULL, NULL, NULL, NULL, 0,
                        event->time);*/
        return TRUE;
    }
    TRACE ("leaves on_button_press");
    return FALSE ;
}


static void
indicator_construct (XfcePanelPlugin *plugin)
{
  IndicatorPlugin *indicator;

  /* setup transation domain */
  xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* create the plugin */
  indicator = indicator_new (plugin);

  /* show the panel's right-click menu on this menu */
  xfce_panel_plugin_add_action_widget (plugin, indicator->menu);
  xfce_panel_plugin_add_action_widget (plugin, indicator->item);

  /* connect plugin signals */
  g_signal_connect (G_OBJECT (plugin), "free-data",
                    G_CALLBACK (indicator_free), indicator);

  g_signal_connect (G_OBJECT (plugin), "size-changed",
                    G_CALLBACK (indicator_size_changed), indicator);

  g_signal_connect (G_OBJECT (plugin), "orientation-changed",
                    G_CALLBACK (indicator_orientation_changed), indicator);
}


static gboolean
entry_scrolled (GtkWidget *menuitem, GdkEventScroll *event, gpointer data)
{
  IndicatorObject *io = g_object_get_data (G_OBJECT (menuitem), "indicator-custom-object-data");
  IndicatorObjectEntry *entry = g_object_get_data (G_OBJECT (menuitem), "indicator-custom-entry-data");

  g_return_val_if_fail(INDICATOR_IS_OBJECT(io), FALSE);

  g_signal_emit_by_name (io, "scroll", 1, event->direction);
  g_signal_emit_by_name (io, "scroll-entry", entry, 1, event->direction);

  return FALSE;
}


static void
entry_added (IndicatorObject * io, IndicatorObjectEntry * entry, gpointer user_data)
{
  GtkWidget * menuitem = gtk_menu_item_new();
  GtkWidget * hbox = gtk_hbox_new(FALSE, 3);

  g_signal_connect(G_OBJECT(menuitem), "scroll-event", G_CALLBACK(entry_scrolled), entry);

  if (entry->image != NULL)
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(entry->image), FALSE, FALSE, 0);

  if (entry->label != NULL)
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(entry->label), FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(menuitem), hbox);
  gtk_widget_show(hbox);

  if (entry->menu != NULL)
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), GTK_WIDGET(entry->menu));

  gtk_menu_shell_append(GTK_MENU_SHELL(user_data), menuitem);
  gtk_widget_show(menuitem);

  g_object_set_data(G_OBJECT(menuitem), "indicator-custom-object-data", io);
  g_object_set_data(G_OBJECT(menuitem), "indicator-custom-entry-data", entry);
}


static void
entry_removed_cb (GtkWidget * widget, gpointer userdata)
{
  gpointer data = g_object_get_data(G_OBJECT(widget), "indicator-custom-entry-data");

  if (data != userdata)
    return;

  gtk_widget_destroy(widget);
}


static void
entry_removed (IndicatorObject * io, IndicatorObjectEntry * entry, gpointer user_data)
{
  gtk_container_foreach(GTK_CONTAINER(user_data), entry_removed_cb, entry);
}


static gboolean
load_module (const gchar * name, GtkWidget * menu)
{
	g_debug("Looking at Module: %s", name);
	g_return_val_if_fail(name != NULL, FALSE);

    if (!g_str_has_suffix(name,G_MODULE_SUFFIX))
        return FALSE;

	g_debug("Loading Module: %s", name);

	gchar * fullpath = g_build_filename(INDICATOR_DIR, name, NULL);
	IndicatorObject * io = indicator_object_new_from_file(fullpath);
	g_free(fullpath);

    g_signal_connect(G_OBJECT(io), INDICATOR_OBJECT_SIGNAL_ENTRY_ADDED,
                     G_CALLBACK(entry_added), menu);
    g_signal_connect(G_OBJECT(io), INDICATOR_OBJECT_SIGNAL_ENTRY_REMOVED,
                     G_CALLBACK(entry_removed), menu);

	GList * entries = indicator_object_get_entries(io);
	GList * entry = NULL;

	for (entry = entries; entry != NULL; entry = g_list_next(entry)) {
		IndicatorObjectEntry * entrydata = (IndicatorObjectEntry *)entry->data;
		entry_added(io, entrydata, menu);
	}

	g_list_free(entries);

	return TRUE;
}
