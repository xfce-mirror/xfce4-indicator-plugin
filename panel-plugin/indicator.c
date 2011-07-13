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

/* prototypes */
static void
indicator_construct (XfcePanelPlugin *plugin);

static gboolean
load_module (const gchar * name, IndicatorPlugin * indicator);

/* register the plugin */
XFCE_PANEL_PLUGIN_REGISTER_EXTERNAL (indicator_construct);


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
  gtk_widget_set_name(GTK_WIDGET (indicator), "indicator-plugin");
  
  indicator->buttonbox = gtk_hbox_new(FALSE,0);
  gtk_widget_set_can_focus(GTK_WIDGET(indicator->buttonbox), TRUE);
  gtk_rc_parse_string (
    "style \"indicator-plugin-style\"\n"
    "{\n"
    "    GtkButton::internal-border= 0\n"
    "    GtkWidget::focus-line-width = 0\n"
    "    GtkWidget::focus-padding = 0\n"
    "}\n"
    "widget \"*.indicator-button\" style \"indicator-plugin-style\"");

  gtk_widget_set_name(GTK_WIDGET (indicator->buttonbox), "indicator-button");
  gtk_container_set_border_width(GTK_CONTAINER(indicator->buttonbox), 0);
  
  /* load 'em */
  if (g_file_test(INDICATOR_DIR, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
    GDir * dir = g_dir_open(INDICATOR_DIR, 0, NULL);

    const gchar * name;
    while ((name = g_dir_read_name(dir)) != NULL) {
      if (load_module(name, indicator))
        indicators_loaded++;
    }
    g_dir_close (dir);
  }

  if (indicators_loaded == 0) {
    /* A label to allow for click through */
    indicator->item = xfce_create_panel_button();
    gtk_button_set_label(GTK_BUTTON(indicator->item), _("No Indicators"));
    gtk_widget_show(indicator->item);
    gtk_container_add (GTK_CONTAINER (plugin), indicator->item);
  } else {
    gtk_widget_show(GTK_WIDGET(indicator->buttonbox));
    gtk_container_add (GTK_CONTAINER (plugin), GTK_WIDGET(indicator->buttonbox));
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
  gint sizex=-1, sizey=-1;
  gtk_orientable_set_orientation (GTK_ORIENTABLE(indicator->buttonbox), orientation);
  gtk_widget_get_size_request (GTK_WIDGET (plugin), &sizex, &sizey);
  gtk_widget_set_size_request (GTK_WIDGET (plugin), sizey, sizex);  
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

  return TRUE;
}

static gboolean
on_button_press (GtkWidget *widget, GdkEventButton *event, IndicatorPlugin *indicator)
{
	if (event->button == 1) /* left click only */
    {
	    gtk_menu_popup (GTK_MENU(g_object_get_data (G_OBJECT(widget),"menu")), NULL,
	                    NULL, NULL, NULL, 1, event->time);
	    /* no approvement to the above */
	    /*
	    gtk_menu_popup (GTK_MENU(g_object_get_data (G_OBJECT(widget),"menu")), NULL, NULL,
	                    xfce_panel_plugin_position_menu,
                        indicator->plugin, 1, gtk_get_current_event_time ());
        */
        return TRUE;
	}
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
  xfce_panel_plugin_add_action_widget (plugin, indicator->buttonbox);
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
  gtk_widget_set_name(GTK_WIDGET (menuitem), "indicator-plugin-menuitem");
  GtkWidget * hbox = gtk_hbox_new(FALSE, 3);
  GtkWidget * button = gtk_button_new();
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_widget_set_name(GTK_WIDGET (button), "indicator-button");

  g_signal_connect(G_OBJECT(menuitem), "scroll-event", G_CALLBACK(entry_scrolled), entry);

  if (entry->image != NULL)
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(entry->image), FALSE, FALSE, 0);
    gtk_button_set_image(GTK_BUTTON(button), GTK_WIDGET(entry->image));

  if (entry->label != NULL)
    gtk_box_pack_start(GTK_BOX(hbox), GTK_WIDGET(entry->label), FALSE, FALSE, 0);
    gtk_button_set_label(GTK_BUTTON(button), gtk_label_get_label (entry->label));

  gtk_container_add(GTK_CONTAINER(menuitem), hbox);
  gtk_widget_show(hbox);

  if (entry->menu != NULL)
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuitem), GTK_WIDGET(entry->menu));
    g_object_set_data(G_OBJECT(button), "menu", entry->menu);

  gtk_widget_show(menuitem);
  g_signal_connect(button, "button-press-event", G_CALLBACK(on_button_press),
                   user_data);
  gtk_widget_show(button);
  gtk_box_pack_start(GTK_BOX(((IndicatorPlugin *)user_data)->buttonbox), button, TRUE, TRUE, 0);

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
load_module (const gchar * name, IndicatorPlugin * indicator)
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
                     G_CALLBACK(entry_added), indicator);
    g_signal_connect(G_OBJECT(io), INDICATOR_OBJECT_SIGNAL_ENTRY_REMOVED,
                     G_CALLBACK(entry_removed), indicator->buttonbox);

	GList * entries = indicator_object_get_entries(io);
	GList * entry = NULL;

	for (entry = entries; entry != NULL; entry = g_list_next(entry)) {
		IndicatorObjectEntry * entrydata = (IndicatorObjectEntry *)entry->data;
		entry_added(io, entrydata, indicator);
	}

	g_list_free(entries);

	return TRUE;
}
