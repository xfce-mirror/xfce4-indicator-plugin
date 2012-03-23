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
#include <libindicator/indicator-object.h>
#include <xfconf/xfconf.h>

#include "indicator.h"
#include "indicator-box.h"
#include "indicator-button.h"

#define DEFAULT_EXCLUDED_MODULES NULL

#ifdef LIBXFCE4PANEL_CHECK_VERSION
#if LIBXFCE4PANEL_CHECK_VERSION (4,9,0)
#define HAS_PANEL_49
#endif
#endif

/* prototypes */
static void
indicator_construct (XfcePanelPlugin *plugin);

static gboolean
load_module (const gchar * name, IndicatorPlugin * indicator);

static gboolean
indicator_size_changed (XfcePanelPlugin *plugin, gint size, IndicatorPlugin *indicator);

#ifdef HAS_PANEL_49
static void
indicator_mode_changed (XfcePanelPlugin *plugin, XfcePanelPluginMode mode, IndicatorPlugin *indicator);
#else
static void
indicator_orientation_changed (XfcePanelPlugin *plugin, GtkOrientation orientation, IndicatorPlugin *indicator);
#endif


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
      if (indicator->excluded_modules)
        xfce_rc_write_list_entry (rc, "Exclude",
                                  indicator->excluded_modules, NULL);

      /* close the rc file */
      xfce_rc_close (rc);
    }
}
#endif


static void
indicator_read (IndicatorPlugin *indicator)
{
  XfconfChannel * channel = xfconf_channel_get ("xfce4-panel");
  gchar * property = g_strconcat (xfce_panel_plugin_get_property_base(indicator->plugin),"/blacklist",NULL);
  indicator->excluded_modules = xfconf_channel_get_string_list(channel, property);
  g_free (property);
  property = g_strconcat (xfce_panel_plugin_get_property_base(indicator->plugin),"/icon-size-max",NULL);
  xfconf_g_property_bind (channel, property, G_TYPE_INT, indicator->buttonbox, "icon-size-max");
  g_free (property);
  /* something went wrong, apply default values */
  /*
  DBG ("Applying default settings");
  indicator->excluded_modules = DEFAULT_EXCLUDED_MODULES;
  */
}

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
  /*gtk_widget_set_name(GTK_WIDGET (indicator->plugin), "indicator-plugin");*/
  
  indicator->buttonbox = xfce_indicator_box_new ();;
  /* initialize xfconf */
  if (xfconf_init(NULL)){
    /* get the list of excluded modules */
    indicator_read (indicator);
  }
  /* load 'em */
  if (g_file_test(INDICATOR_DIR, (G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR))) {
    GDir * dir = g_dir_open(INDICATOR_DIR, 0, NULL);

    const gchar * name;
    guint i, length;
    gboolean match = FALSE;
 
    length = (indicator->excluded_modules != NULL) ? g_strv_length (indicator->excluded_modules) : 0;
    while ((name = g_dir_read_name(dir)) != NULL) {
      for (i = 0; i < length; ++i) {
        if (match = (g_strcmp0 (name, indicator->excluded_modules[i]) == 0))
          break;
      }

      if (G_UNLIKELY (match)) {
        g_debug ("Excluding module: %s", name);
        continue;
      }

      if (load_module(name, indicator))
        indicators_loaded++;
    }
    g_dir_close (dir);
  }

  if (indicators_loaded == 0) {
    /* A label to allow for click through */
    indicator->item = xfce_indicator_button_new(NULL, NULL);
    xfce_indicator_button_set_label(XFCE_INDICATOR_BUTTON(indicator->item),
                                    GTK_LABEL (gtk_label_new(_("No Indicators"))));
    gtk_container_add (GTK_CONTAINER (plugin), indicator->item);
    gtk_widget_show(indicator->item);  
    /* show the panel's right-click menu on this menu */
    xfce_panel_plugin_add_action_widget (plugin, indicator->item);
  } else {
    indicator->ebox = gtk_event_box_new();
    gtk_widget_set_can_focus(GTK_WIDGET(indicator->ebox), TRUE);
    gtk_container_add (GTK_CONTAINER (indicator->ebox), GTK_WIDGET(indicator->buttonbox));
    gtk_container_add (GTK_CONTAINER (plugin), GTK_WIDGET(indicator->ebox));
    gtk_widget_show(GTK_WIDGET(indicator->buttonbox));
    gtk_widget_show(GTK_WIDGET(indicator->ebox));
    /* show the panel's right-click menu on this menu */
    xfce_panel_plugin_add_action_widget (plugin, indicator->ebox);
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
  xfconf_shutdown();
  /* free the plugin structure */
  panel_slice_free (IndicatorPlugin, indicator);
}



#ifdef HAS_PANEL_49
static void
indicator_mode_changed (XfcePanelPlugin     *plugin,
                        XfcePanelPluginMode  mode,
                        IndicatorPlugin     *indicator)
{
  GtkOrientation orientation;
  GtkOrientation panel_orientation = xfce_panel_plugin_get_orientation (plugin);

  orientation = (mode == XFCE_PANEL_PLUGIN_MODE_VERTICAL) ?
    GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;

  xfce_indicator_box_set_orientation (XFCE_INDICATOR_BOX (indicator->buttonbox), panel_orientation, orientation);

  indicator_size_changed (plugin, xfce_panel_plugin_get_size (plugin), indicator);
}



#else
static void
indicator_orientation_changed (XfcePanelPlugin *plugin,
                            GtkOrientation   orientation,
                            IndicatorPlugin    *indicator)
{
  xfce_indicator_box_set_orientation (XFCE_INDICATOR_BOX (indicator->buttonbox), orientation, GTK_ORIENTATION_HORIZONTAL);

  indicator_size_changed (plugin, xfce_panel_plugin_get_size (plugin), indicator);
}
#endif


static gboolean
indicator_size_changed (XfcePanelPlugin *plugin,
                     gint             size,
                     IndicatorPlugin    *indicator)
{
#ifdef HAS_PANEL_49
  xfce_indicator_box_set_size (XFCE_INDICATOR_BOX (indicator->buttonbox),
                               size, xfce_panel_plugin_get_nrows (plugin));
#else
  xfce_indicator_box_set_size (XFCE_INDICATOR_BOX (indicator->buttonbox),
                               size, 1);
#endif

  return TRUE;
}



static gboolean
on_button_press (GtkWidget *widget, GdkEventButton *event, IndicatorPlugin *indicator)
{
  if (indicator != NULL)
  {
    if( event->button == 1) /* left click only */
    {
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),TRUE);
      gtk_menu_popup (xfce_indicator_button_get_menu (XFCE_INDICATOR_BUTTON(widget)), NULL, NULL,
                      xfce_panel_plugin_position_menu,
                      indicator->plugin, 1, gtk_get_current_event_time ());
      
      return TRUE;
    }
    /* event doesn't make it to the ebox, so I just push it. */
    gtk_widget_event (indicator->ebox, (GdkEvent*)event);
  }
  return FALSE;
}

static void
menu_deactivate (GtkMenu *menu,
                 gpointer      user_data)
{
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_menu_get_attach_widget (menu)), FALSE);
}

static void
indicator_construct (XfcePanelPlugin *plugin)
{
  IndicatorPlugin *indicator;

  /* setup transation domain */
  xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* create the plugin */
  indicator = indicator_new (plugin);

  /* connect plugin signals */
  g_signal_connect (G_OBJECT (plugin), "free-data",
                    G_CALLBACK (indicator_free), indicator);

  g_signal_connect (G_OBJECT (plugin), "size-changed",
                    G_CALLBACK (indicator_size_changed), indicator);

#ifdef HAS_PANEL_49
  g_signal_connect (G_OBJECT (plugin), "mode-changed",
                    G_CALLBACK (indicator_mode_changed), indicator);
#else
  g_signal_connect (G_OBJECT (plugin), "orientation-changed",
                    G_CALLBACK (indicator_orientation_changed), indicator);
#endif
}


static gboolean
entry_scrolled (GtkWidget *menuitem, GdkEventScroll *event, IndicatorPlugin *indicator)
{
  IndicatorObject *io = xfce_indicator_button_get_io (XFCE_INDICATOR_BUTTON (menuitem));
  IndicatorObjectEntry *entry = xfce_indicator_button_get_entry (XFCE_INDICATOR_BUTTON (menuitem));

  g_return_val_if_fail(INDICATOR_IS_OBJECT(io), FALSE);
  g_return_val_if_fail(indicator != NULL, FALSE);

  g_signal_emit_by_name (io, INDICATOR_OBJECT_SIGNAL_ENTRY_SCROLLED, entry, 1, event->direction);

  return TRUE;
}


static void
entry_added (IndicatorObject * io, IndicatorObjectEntry * entry, gpointer user_data)
{
  XfcePanelPlugin *plugin = ((IndicatorPlugin *) user_data)->plugin;
  GtkWidget * button = xfce_indicator_button_new (io, entry);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_button_set_use_underline(GTK_BUTTON (button),TRUE);
  gtk_widget_set_name(GTK_WIDGET (button), "indicator-button");

  if (entry->image != NULL)
    xfce_indicator_button_set_image(XFCE_INDICATOR_BUTTON(button), entry->image);

  if (entry->label != NULL)
    xfce_indicator_button_set_label(XFCE_INDICATOR_BUTTON(button), entry->label);

  if (entry->menu != NULL)
  {
    xfce_indicator_button_set_menu (XFCE_INDICATOR_BUTTON(button), entry->menu);
    g_signal_connect(G_OBJECT(entry->menu), "deactivate", G_CALLBACK(menu_deactivate),NULL);
  }

  g_signal_connect(button, "button-press-event", G_CALLBACK(on_button_press),
                   user_data);
  g_signal_connect(button, "scroll-event", G_CALLBACK(entry_scrolled),
                   user_data);

  gtk_container_add(GTK_CONTAINER (((IndicatorPlugin *)user_data)->buttonbox), button);
  gtk_widget_show(button);
}


static void
entry_removed_cb (GtkWidget * widget, gpointer userdata)
{
  gpointer data = (gpointer) xfce_indicator_button_get_entry (XFCE_INDICATOR_BUTTON (widget));

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
