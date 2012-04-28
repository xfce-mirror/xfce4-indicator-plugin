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



/*
 *  This file implements the main plugin class.
 *
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
#include "indicator-dialog.h"

#define DEFAULT_EXCLUDED_MODULES NULL

#ifdef LIBXFCE4PANEL_CHECK_VERSION
#if LIBXFCE4PANEL_CHECK_VERSION (4,9,0)
#define HAS_PANEL_49
#endif
#endif

/* prototypes */
static void             indicator_construct                        (XfcePanelPlugin       *plugin);
static void             indicator_free                             (XfcePanelPlugin       *plugin);
static gboolean         load_module                                (const gchar           *name,
                                                                    IndicatorPlugin       *indicator);
static void             indicator_configure_plugin                 (XfcePanelPlugin       *plugin);
static gboolean         indicator_size_changed                     (XfcePanelPlugin       *plugin,
                                                                    gint                   size);
#ifdef HAS_PANEL_49
static void             indicator_mode_changed                     (XfcePanelPlugin       *plugin,
                                                                    XfcePanelPluginMode    mode);
#else
static void             indicator_orientation_changed              (XfcePanelPlugin       *plugin,
                                                                    GtkOrientation         orientation);
#endif


struct _IndicatorPluginClass
{
  XfcePanelPluginClass __parent__;
};

/* plugin structure */
struct _IndicatorPlugin
{
  XfcePanelPlugin __parent__;

  /* panel widgets */
  GtkWidget       *item;
  GtkWidget       *buttonbox;
  GtkWidget       *ebox;

  /* indicator settings */
  gchar          **excluded_modules;
};


/* define the plugin */
XFCE_PANEL_DEFINE_PLUGIN (IndicatorPlugin, indicator)




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
  XfcePanelPlugin  *plugin = XFCE_PANEL_PLUGIN (indicator);
  XfconfChannel * channel = xfconf_channel_get ("xfce4-panel");
  gchar * property = g_strconcat (xfce_panel_plugin_get_property_base(plugin),"/blacklist",NULL);
  indicator->excluded_modules = xfconf_channel_get_string_list(channel, property);
  g_free (property);
  property = g_strconcat (xfce_panel_plugin_get_property_base(plugin),"/icon-size-max",NULL);
  xfconf_g_property_bind (channel, property, G_TYPE_INT, indicator->buttonbox, "icon-size-max");
  g_free (property);
  property = g_strconcat (xfce_panel_plugin_get_property_base(plugin),"/align-left",NULL);
  xfconf_g_property_bind (channel, property, G_TYPE_BOOLEAN, indicator->buttonbox, "align-left");
  g_free (property);
  /* something went wrong, apply default values */
  /*
  DBG ("Applying default settings");
  indicator->excluded_modules = DEFAULT_EXCLUDED_MODULES;
  */
}

static void
indicator_class_init (IndicatorPluginClass *klass)
{
  XfcePanelPluginClass *plugin_class;
  GObjectClass         *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  //gobject_class->get_property = indicator_get_property;
  //gobject_class->set_property = indicator_set_property;

  plugin_class = XFCE_PANEL_PLUGIN_CLASS (klass);
  plugin_class->construct = indicator_construct;
  plugin_class->free_data = indicator_free;
  plugin_class->size_changed = indicator_size_changed;
  plugin_class->configure_plugin = indicator_configure_plugin;
#ifdef HAS_PANEL_49
  plugin_class->mode_changed = indicator_mode_changed;
#else
  plugin_class->orientation_changed = indicator_orientation_changed;
#endif
}



static void
indicator_init (IndicatorPlugin *indicator)
{
  XfcePanelPlugin  *plugin = XFCE_PANEL_PLUGIN (indicator);

  indicator->ebox = gtk_event_box_new();
  gtk_widget_set_can_focus(GTK_WIDGET(indicator->ebox), TRUE);

  indicator->buttonbox = xfce_indicator_box_new (plugin);
  gtk_container_add (GTK_CONTAINER (indicator->ebox), GTK_WIDGET(indicator->buttonbox));
  gtk_container_add (GTK_CONTAINER (plugin), GTK_WIDGET(indicator->ebox));
  gtk_widget_show(GTK_WIDGET(indicator->buttonbox));
  gtk_widget_show(GTK_WIDGET(indicator->ebox));
  /* show the panel's right-click menu on this menu */
  xfce_panel_plugin_add_action_widget (plugin, indicator->ebox);
}



static void
indicator_free (XfcePanelPlugin *plugin)
{
  IndicatorPlugin *indicator = XFCE_INDICATOR_PLUGIN (plugin);
  GtkWidget *dialog;

  /* check if the dialog is still open. if so, destroy it */
  dialog = g_object_get_data (G_OBJECT (plugin), "dialog");
  if (G_UNLIKELY (dialog != NULL))
    gtk_widget_destroy (dialog);
  xfconf_shutdown();
}



static void
indicator_configure_plugin (XfcePanelPlugin *plugin)
{
  g_return_if_fail (XFCE_IS_INDICATOR_PLUGIN (plugin));

  indicator_dialog_show (gtk_widget_get_screen (GTK_WIDGET (plugin)), XFCE_INDICATOR_PLUGIN (plugin));
}




#ifdef HAS_PANEL_49
static void
indicator_mode_changed (XfcePanelPlugin     *plugin,
                        XfcePanelPluginMode  mode)
{
  GtkOrientation   orientation;
  GtkOrientation   panel_orientation = xfce_panel_plugin_get_orientation (plugin);
  IndicatorPlugin *indicator = XFCE_INDICATOR_PLUGIN (plugin);

  orientation = (mode == XFCE_PANEL_PLUGIN_MODE_VERTICAL) ?
    GTK_ORIENTATION_VERTICAL : GTK_ORIENTATION_HORIZONTAL;

  xfce_indicator_box_set_orientation (XFCE_INDICATOR_BOX (indicator->buttonbox), panel_orientation, orientation);

  indicator_size_changed (plugin, xfce_panel_plugin_get_size (plugin));
}



#else
static void
indicator_orientation_changed (XfcePanelPlugin *plugin,
                               GtkOrientation   orientation)
{
  IndicatorPlugin *indicator = XFCE_INDICATOR_PLUGIN (plugin);

  xfce_indicator_box_set_orientation (XFCE_INDICATOR_BOX (indicator->buttonbox), orientation, GTK_ORIENTATION_HORIZONTAL);

  indicator_size_changed (plugin, xfce_panel_plugin_get_size (plugin));
}
#endif


static gboolean
indicator_size_changed (XfcePanelPlugin *plugin,
                        gint             size)
{
  IndicatorPlugin *indicator = XFCE_INDICATOR_PLUGIN (plugin);

#ifdef HAS_PANEL_49
  xfce_indicator_box_set_size (XFCE_INDICATOR_BOX (indicator->buttonbox),
                               size, xfce_panel_plugin_get_nrows (plugin));
#else
  xfce_indicator_box_set_size (XFCE_INDICATOR_BOX (indicator->buttonbox),
                               size, 1);
#endif

  return TRUE;
}


static void
indicator_construct (XfcePanelPlugin *plugin)
{
  IndicatorPlugin  *indicator = XFCE_INDICATOR_PLUGIN (plugin);
  GtkRcStyle       *style;
  gint              indicators_loaded = 0;
  GtkWidget        *label;

  xfce_panel_plugin_menu_show_configure (plugin);

  /* setup transation domain */
  xfce_textdomain(GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* Init some theme/icon stuff */
  gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(),
                                  INDICATOR_ICONS_DIR);
  /*gtk_widget_set_name(GTK_WIDGET (indicator->plugin), "indicator-plugin");*/

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
    indicator->item = xfce_indicator_button_new(NULL, NULL, XFCE_INDICATOR_BOX (indicator->buttonbox));
    label = gtk_label_new ( _("No Indicators"));
    xfce_indicator_button_set_label (XFCE_INDICATOR_BUTTON (indicator->item), GTK_LABEL (label));
    gtk_container_add (GTK_CONTAINER (indicator->buttonbox), indicator->item);
    gtk_widget_show (label);
    gtk_widget_show (indicator->item);
  }
}


static void
entry_added (IndicatorObject * io, IndicatorObjectEntry * entry, gpointer user_data)
{
  XfcePanelPlugin *plugin = XFCE_PANEL_PLUGIN (user_data);
  IndicatorPlugin *indicator = XFCE_INDICATOR_PLUGIN (plugin);
  GtkWidget * button = xfce_indicator_button_new (io, entry, XFCE_INDICATOR_BOX (indicator->buttonbox));

  /* remove placeholder item when there are real entries to be added */
  if (indicator->item != NULL)
    {
      xfce_indicator_button_disconnect_signals (XFCE_INDICATOR_BUTTON (indicator->item));
      gtk_widget_destroy (GTK_WIDGET (indicator->item));
      indicator->item = NULL;
    }

  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_button_set_use_underline(GTK_BUTTON (button),TRUE);
  gtk_widget_set_name(GTK_WIDGET (button), "indicator-button");

  if (entry->image != NULL)
    xfce_indicator_button_set_image(XFCE_INDICATOR_BUTTON(button), entry->image);

  if (entry->label != NULL)
    xfce_indicator_button_set_label(XFCE_INDICATOR_BUTTON(button), entry->label);

  if (entry->menu != NULL)
    xfce_indicator_button_set_menu (XFCE_INDICATOR_BUTTON(button), entry->menu);

  gtk_container_add(GTK_CONTAINER (indicator->buttonbox), button);
  gtk_widget_show(button);
}


static void
entry_removed (IndicatorObject * io, IndicatorObjectEntry * entry, gpointer user_data)
{
  xfce_indicator_box_remove_entry (XFCE_INDICATOR_BOX (user_data), entry);
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


XfceIndicatorBox *
indicator_get_buttonbox (IndicatorPlugin *plugin)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_PLUGIN (plugin), NULL);

  return XFCE_INDICATOR_BOX (plugin->buttonbox);
}
