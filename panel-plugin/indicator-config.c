/*
 *  Copyright (C) 2013 Andrzej <ndrwrdck@gmail.com>
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
 *  This file implements a configuration store. The class extends GObject.
 *
 */



#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <xfconf/xfconf.h>
//#include <exo/exo.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "indicator.h"
#include "indicator-config.h"




#define DEFAULT_SINGLE_ROW         FALSE
#define DEFAULT_ALIGN_LEFT         FALSE
#define DEFAULT_SQUARE_ICONS       FALSE
#define DEFAULT_EXCLUDED_MODULES   NULL
#define DEFAULT_ORIENTATION        GTK_ORIENTATION_HORIZONTAL
#define DEFAULT_PANEL_ORIENTATION  GTK_ORIENTATION_HORIZONTAL
#define DEFAULT_PANEL_SIZE         28
#define DEFAULT_MODE_WHITELIST     FALSE




static void                 indicator_config_finalize       (GObject          *object);
static void                 indicator_config_get_property   (GObject          *object,
                                                             guint             prop_id,
                                                             GValue           *value,
                                                             GParamSpec       *pspec);
static void                 indicator_config_set_property   (GObject          *object,
                                                             guint             prop_id,
                                                             const GValue     *value,
                                                             GParamSpec       *pspec);



struct _IndicatorConfigClass
{
  GObjectClass      __parent__;
};

struct _IndicatorConfig
{
  GObject          __parent__;

  gboolean         single_row;
  gboolean         align_left;
  gboolean         square_icons;
  gboolean         mode_whitelist;
  GHashTable      *blacklist;
  GHashTable      *whitelist;
  GList           *known_indicators;

  gchar          **excluded_modules;

  /* not xfconf properties but it is still convenient to have them here */
  GtkOrientation   orientation;
  GtkOrientation   panel_orientation;
  gint             nrows;
  gint             panel_size;
};



enum
{
  PROP_0,
  PROP_SINGLE_ROW,
  PROP_ALIGN_LEFT,
  PROP_SQUARE_ICONS,
  PROP_MODE_WHITELIST,
  PROP_BLACKLIST,
  PROP_WHITELIST,
  PROP_KNOWN_INDICATORS
};

enum
{
  CONFIGURATION_CHANGED,
  INDICATOR_LIST_CHANGED,
  LAST_SIGNAL
};

static guint indicator_config_signals[LAST_SIGNAL] = { 0, };




G_DEFINE_TYPE (IndicatorConfig, indicator_config, G_TYPE_OBJECT)




#ifdef XFCONF_LEGACY
GType
indicator_config_value_array_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      type = dbus_g_type_get_collection ("GPtrArray", G_TYPE_VALUE);
      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}
#endif


static void
indicator_config_class_init (IndicatorConfigClass *klass)
{
  GObjectClass                 *gobject_class;

  gobject_class               = G_OBJECT_CLASS (klass);
  gobject_class->finalize     = indicator_config_finalize;
  gobject_class->get_property = indicator_config_get_property;
  gobject_class->set_property = indicator_config_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_SINGLE_ROW,
                                   g_param_spec_boolean ("single-row", NULL, NULL,
                                                         DEFAULT_SINGLE_ROW,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
                                   PROP_ALIGN_LEFT,
                                   g_param_spec_boolean ("align-left", NULL, NULL,
                                                         DEFAULT_ALIGN_LEFT,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
                                   PROP_SQUARE_ICONS,
                                   g_param_spec_boolean ("square-icons", NULL, NULL,
                                                         DEFAULT_SQUARE_ICONS,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
                                   PROP_MODE_WHITELIST,
                                   g_param_spec_boolean ("mode-whitelist", NULL, NULL,
                                                         DEFAULT_MODE_WHITELIST,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
                                   PROP_BLACKLIST,
                                   g_param_spec_boxed ("blacklist",
                                                       NULL, NULL,
                                                       XFCE_TYPE_INDICATOR_CONFIG_VALUE_ARRAY,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_STATIC_STRINGS));

  g_object_class_install_property (gobject_class,
                                   PROP_WHITELIST,
                                   g_param_spec_boxed ("whitelist",
                                                       NULL, NULL,
                                                       XFCE_TYPE_INDICATOR_CONFIG_VALUE_ARRAY,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_STATIC_STRINGS));


  g_object_class_install_property (gobject_class,
                                   PROP_KNOWN_INDICATORS,
                                   g_param_spec_boxed ("known-indicators",
                                                       NULL, NULL,
                                                       XFCE_TYPE_INDICATOR_CONFIG_VALUE_ARRAY,
                                                       G_PARAM_READWRITE |
                                                       G_PARAM_STATIC_STRINGS));


  indicator_config_signals[CONFIGURATION_CHANGED] =
    g_signal_new (g_intern_static_string ("configuration-changed"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  indicator_config_signals[INDICATOR_LIST_CHANGED] =
    g_signal_new (g_intern_static_string ("indicator-list-changed"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);
}



static void
indicator_config_init (IndicatorConfig *config)
{
  config->single_row           = DEFAULT_SINGLE_ROW;
  config->align_left           = DEFAULT_ALIGN_LEFT;
  config->square_icons         = DEFAULT_SQUARE_ICONS;
  config->mode_whitelist       = DEFAULT_MODE_WHITELIST;
  config->blacklist            = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  config->whitelist            = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  config->known_indicators     = NULL;

  config->orientation          = DEFAULT_ORIENTATION;
  config->panel_orientation    = DEFAULT_PANEL_ORIENTATION;
  config->nrows                = 1;
  config->panel_size           = DEFAULT_PANEL_SIZE;
}



static void
indicator_config_finalize (GObject *object)
{
  IndicatorConfig *config = XFCE_INDICATOR_CONFIG (object);

  xfconf_shutdown();

  g_hash_table_destroy (config->blacklist);
  g_hash_table_destroy (config->whitelist);
  g_list_foreach (config->known_indicators, (GFunc) g_free, NULL);
  g_list_free (config->known_indicators);

  G_OBJECT_CLASS (indicator_config_parent_class)->finalize (object);
}



static void
indicator_config_free_array_element (gpointer data)
{
  GValue *value = (GValue *) data;

  g_value_unset (value);
  g_free (value);
}



static void
indicator_config_collect_keys (gpointer key,
                               gpointer value,
                               gpointer array)
{
  GValue *tmp;

  tmp = g_new0 (GValue, 1);
  g_value_init (tmp, G_TYPE_STRING);
  g_value_set_string (tmp, key);
  g_ptr_array_add (array, tmp);
}



static void
indicator_config_get_property (GObject    *object,
                               guint       prop_id,
                               GValue     *value,
                               GParamSpec *pspec)
{
  IndicatorConfig     *config = XFCE_INDICATOR_CONFIG (object);
  GPtrArray           *array;
  GList               *li;
  GValue              *tmp;

  switch (prop_id)
    {
    case PROP_SINGLE_ROW:
      g_value_set_boolean (value, config->single_row);
      break;

    case PROP_ALIGN_LEFT:
      g_value_set_boolean (value, config->align_left);
      break;

    case PROP_SQUARE_ICONS:
      g_value_set_boolean (value, config->square_icons);
      break;

    case PROP_MODE_WHITELIST:
      g_value_set_boolean (value, config->mode_whitelist);
      break;

    case PROP_BLACKLIST:
      array = g_ptr_array_new_full (1, indicator_config_free_array_element);
      g_hash_table_foreach (config->blacklist, indicator_config_collect_keys, array);
      g_value_set_boxed (value, array);
      g_ptr_array_unref (array);
      break;

    case PROP_WHITELIST:
      array = g_ptr_array_new_full (1, indicator_config_free_array_element);
      g_hash_table_foreach (config->whitelist, indicator_config_collect_keys, array);
      g_value_set_boxed (value, array);
      g_ptr_array_unref (array);
      break;

    case PROP_KNOWN_INDICATORS:
      array = g_ptr_array_new_full (1, indicator_config_free_array_element);
      for(li = config->known_indicators; li != NULL; li = li->next)
        {
          tmp = g_new0 (GValue, 1);
          g_value_init (tmp, G_TYPE_STRING);
          g_value_set_string (tmp, li->data);
          g_ptr_array_add (array, tmp);
        }
      g_value_set_boxed (value, array);
      g_ptr_array_unref (array);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
indicator_config_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  IndicatorConfig     *config = XFCE_INDICATOR_CONFIG (object);
  gint                 val;
  GPtrArray           *array;
  const GValue        *tmp;
  gchar               *name;
  guint                i;

  switch (prop_id)
    {
    case PROP_SINGLE_ROW:
      val = g_value_get_boolean (value);
      if (config->single_row != val)
        {
          config->single_row = val;
          g_signal_emit (G_OBJECT (config), indicator_config_signals [CONFIGURATION_CHANGED], 0);
        }
      break;

    case PROP_ALIGN_LEFT:
      val = g_value_get_boolean (value);
      if (config->align_left != val)
        {
          config->align_left = val;
          g_signal_emit (G_OBJECT (config), indicator_config_signals [CONFIGURATION_CHANGED], 0);
        }
      break;

    case PROP_SQUARE_ICONS:
      val = g_value_get_boolean (value);
      if (config->square_icons != val)
        {
          config->square_icons = val;
          g_signal_emit (G_OBJECT (config), indicator_config_signals [CONFIGURATION_CHANGED], 0);
        }
      break;

    case PROP_MODE_WHITELIST:
      val = g_value_get_boolean (value);
      if (config->mode_whitelist != val)
        {
          config->mode_whitelist = val;
          g_signal_emit (G_OBJECT (config), indicator_config_signals [INDICATOR_LIST_CHANGED], 0);
        }
      break;

    case PROP_BLACKLIST:
      g_hash_table_remove_all (config->blacklist);
      array = g_value_get_boxed (value);
      if (G_LIKELY (array != NULL))
        {
          for (i = 0; i < array->len; i++)
            {
              tmp = g_ptr_array_index (array, i);
              g_assert (G_VALUE_HOLDS_STRING (tmp));
              name = g_value_dup_string (tmp);
              g_hash_table_replace (config->blacklist, name, name);
            }
        }
      g_signal_emit (G_OBJECT (config), indicator_config_signals [INDICATOR_LIST_CHANGED], 0);
      break;

    case PROP_WHITELIST:
      g_hash_table_remove_all (config->whitelist);
      array = g_value_get_boxed (value);
      if (G_LIKELY (array != NULL))
        {
          for (i = 0; i < array->len; i++)
            {
              tmp = g_ptr_array_index (array, i);
              g_assert (G_VALUE_HOLDS_STRING (tmp));
              name = g_value_dup_string (tmp);
              g_hash_table_replace (config->whitelist, name, name);
            }
        }
      g_signal_emit (G_OBJECT (config), indicator_config_signals [INDICATOR_LIST_CHANGED], 0);
      break;

    case PROP_KNOWN_INDICATORS:
      g_list_free_full (config->known_indicators, g_free);
      config->known_indicators = NULL;
      array = g_value_get_boxed (value);
      if (G_LIKELY (array != NULL))
        {
          for (i = 0; i < array->len; i++)
            {
              tmp = g_ptr_array_index (array, i);
              g_assert (G_VALUE_HOLDS_STRING (tmp));
              name = g_value_dup_string (tmp);
              config->known_indicators = g_list_append (config->known_indicators, name);
            }
        }
      g_signal_emit (G_OBJECT (config), indicator_config_signals [INDICATOR_LIST_CHANGED], 0);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}




gboolean
indicator_config_get_single_row (IndicatorConfig *config)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), DEFAULT_SINGLE_ROW);

  return config->single_row;
}




gboolean
indicator_config_get_align_left (IndicatorConfig *config)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), DEFAULT_ALIGN_LEFT);

  return config->align_left;
}




gboolean
indicator_config_get_square_icons (IndicatorConfig *config)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), DEFAULT_SQUARE_ICONS);

  return config->square_icons;
}




void
indicator_config_set_orientation (IndicatorConfig  *config,
                                  GtkOrientation    panel_orientation,
                                  GtkOrientation    orientation)
{
  gboolean              needs_update = FALSE;

  g_return_if_fail (XFCE_IS_INDICATOR_CONFIG (config));

  if (config->orientation != orientation)
    {
      config->orientation = orientation;
      needs_update = TRUE;
    }

  if (config->panel_orientation != panel_orientation)
    {
      config->panel_orientation = panel_orientation;
      needs_update = TRUE;
    }

  if (needs_update)
    g_signal_emit (G_OBJECT (config), indicator_config_signals[CONFIGURATION_CHANGED], 0);
}



GtkOrientation
indicator_config_get_orientation (IndicatorConfig *config)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), DEFAULT_ORIENTATION);

  return config->orientation;
}




GtkOrientation
indicator_config_get_panel_orientation (IndicatorConfig *config)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), DEFAULT_PANEL_ORIENTATION);

  return config->panel_orientation;
}




void
indicator_config_set_size (IndicatorConfig  *config,
                           gint              panel_size,
                           gint              nrows)
{
  gboolean              needs_update = FALSE;

  g_return_if_fail (XFCE_IS_INDICATOR_CONFIG (config));

  if (config->nrows != nrows)
    {
      config->nrows = nrows;
      needs_update = TRUE;
    }

  if (config->panel_size != panel_size)
    {
      config->panel_size = panel_size;
      needs_update = TRUE;
    }

  if (needs_update)
    g_signal_emit (G_OBJECT (config), indicator_config_signals[CONFIGURATION_CHANGED], 0);
}



gint
indicator_config_get_nrows (IndicatorConfig *config)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), 1);

  return config->nrows;
}




gint
indicator_config_get_panel_size (IndicatorConfig *config)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), DEFAULT_PANEL_SIZE);

  return config->panel_size;
}




gchar**
indicator_config_get_excluded_modules (IndicatorConfig *config)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), NULL);

  return config->excluded_modules;
}




gboolean
indicator_config_get_mode_whitelist (IndicatorConfig *config)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), FALSE);

  return config->mode_whitelist;
}




gboolean
indicator_config_is_blacklisted (IndicatorConfig *config,
                                 const gchar     *name)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), FALSE);

  return g_hash_table_lookup_extended (config->blacklist, name, NULL, NULL);
}




void
indicator_config_blacklist_set (IndicatorConfig *config,
                                const gchar     *name,
                                gboolean         add)
{
  gchar *name_copy;

  g_return_if_fail (XFCE_IS_INDICATOR_CONFIG (config));
  //g_return_if_fail (!exo_str_is_empty (name));

  if (add)
    {
      name_copy = g_strdup (name);
      g_hash_table_replace (config->blacklist, name_copy, name_copy);
    }
  else
    {
      g_hash_table_remove (config->blacklist, name);
    }
  g_object_notify (G_OBJECT (config), "blacklist");
  g_signal_emit (G_OBJECT (config), indicator_config_signals [INDICATOR_LIST_CHANGED], 0);
}





void
indicator_config_whitelist_set (IndicatorConfig *config,
                                const gchar     *name,
                                gboolean         add)
{
  gchar *name_copy;

  g_return_if_fail (XFCE_IS_INDICATOR_CONFIG (config));
  //g_return_if_fail (!exo_str_is_empty (name));

  if (add)
    {
      name_copy = g_strdup (name);
      g_hash_table_replace (config->whitelist, name_copy, name_copy);
    }
  else
    {
      g_hash_table_remove (config->whitelist, name);
    }
  g_object_notify (G_OBJECT (config), "whitelist");
  g_signal_emit (G_OBJECT (config), indicator_config_signals [INDICATOR_LIST_CHANGED], 0);
}





gboolean
indicator_config_is_whitelisted (IndicatorConfig *config,
                                 const gchar     *name)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), FALSE);

  return g_hash_table_lookup_extended (config->whitelist, name, NULL, NULL);
}




GList*
indicator_config_get_known_indicators (IndicatorConfig *config)
{
  g_return_val_if_fail (XFCE_IS_INDICATOR_CONFIG (config), NULL);

  return config->known_indicators;
}




void
indicator_config_add_known_indicator (IndicatorConfig *config,
                                      const gchar     *name)
{
  GList    *li;

  g_return_if_fail (XFCE_IS_INDICATOR_CONFIG (config));
  //g_return_if_fail (!exo_str_is_empty (name));

  /* check if the indicator is already known */
  for(li = config->known_indicators; li != NULL; li = li->next)
    if (g_strcmp0 (li->data, name) == 0)
      return;

  config->known_indicators = g_list_append (config->known_indicators, g_strdup (name));

  g_object_notify (G_OBJECT (config), "known-indicators");
  g_signal_emit (G_OBJECT (config), indicator_config_signals [INDICATOR_LIST_CHANGED], 0);
}




void
indicator_config_swap_known_indicators (IndicatorConfig *config,
                                        const gchar     *name1,
                                        const gchar     *name2)
{
  GList       *li, *li_tmp;

  g_return_if_fail (XFCE_IS_INDICATOR_CONFIG (config));
  //g_return_if_fail (!exo_str_is_empty (name1));
  //g_return_if_fail (!exo_str_is_empty (name2));

  for(li = config->known_indicators; li != NULL; li = li->next)
    if (g_strcmp0 (li->data, name1) == 0)
      break;

  /* make sure that the list contains name1 followed by name2 */
  if (li == NULL || li->next == NULL || g_strcmp0 (li->next->data, name2) != 0)
    {
      g_debug("Couldn't swap indicators: %s and %s", name1, name2);
      return;
    }

  /* li_tmp will contain only the removed element (name2) */
  li_tmp = li->next;
  config->known_indicators = g_list_remove_link (config->known_indicators, li_tmp);

  /* not strictly necessary (in testing the list contents was preserved)
   * but searching for the index again should be safer */
  for(li = config->known_indicators; li != NULL; li = li->next)
    if (g_strcmp0 (li->data, name1) == 0)
      break;

  config->known_indicators = g_list_insert_before (config->known_indicators, li, li_tmp->data);
  g_list_free (li_tmp);

  g_object_notify (G_OBJECT (config), "known-indicators");
  g_signal_emit (G_OBJECT (config), indicator_config_signals [INDICATOR_LIST_CHANGED], 0);
}




void
indicator_config_names_clear (IndicatorConfig *config)
{
  g_list_foreach (config->known_indicators, (GFunc) g_free, NULL);
  g_list_free (config->known_indicators);
  config->known_indicators = NULL;
  g_object_notify (G_OBJECT (config), "known-indicators");

  g_hash_table_remove_all (config->blacklist);
  g_object_notify (G_OBJECT (config), "blacklist");

  g_hash_table_remove_all (config->whitelist);
  g_object_notify (G_OBJECT (config), "whitelist");
  g_signal_emit (G_OBJECT (config), indicator_config_signals [INDICATOR_LIST_CHANGED], 0);
}




IndicatorConfig *
indicator_config_new (const gchar     *property_base)
{
  IndicatorConfig    *config;
  XfconfChannel      *channel;
  gchar              *property;

  config = g_object_new (XFCE_TYPE_INDICATOR_CONFIG, NULL);

  if (xfconf_init (NULL))
    {
      channel = xfconf_channel_get ("xfce4-panel");

      property = g_strconcat (property_base, "/single-row", NULL);
      xfconf_g_property_bind (channel, property, G_TYPE_BOOLEAN, config, "single-row");
      g_free (property);

      property = g_strconcat (property_base, "/align-left", NULL);
      xfconf_g_property_bind (channel, property, G_TYPE_BOOLEAN, config, "align-left");
      g_free (property);

      property = g_strconcat (property_base, "/square-icons", NULL);
      xfconf_g_property_bind (channel, property, G_TYPE_BOOLEAN, config, "square-icons");
      g_free (property);

      property = g_strconcat (property_base, "/mode-whitelist", NULL);
      xfconf_g_property_bind (channel, property, G_TYPE_BOOLEAN, config, "mode-whitelist");
      g_free (property);

      property = g_strconcat (property_base, "/blacklist", NULL);
      xfconf_g_property_bind (channel, property, XFCE_TYPE_INDICATOR_CONFIG_VALUE_ARRAY, config, "blacklist");
      g_free (property);

      property = g_strconcat (property_base, "/whitelist", NULL);
      xfconf_g_property_bind (channel, property, XFCE_TYPE_INDICATOR_CONFIG_VALUE_ARRAY, config, "whitelist");
      g_free (property);

      property = g_strconcat (property_base, "/known-indicators", NULL);
      xfconf_g_property_bind (channel, property, XFCE_TYPE_INDICATOR_CONFIG_VALUE_ARRAY, config, "known-indicators");
      g_free (property);

      g_signal_emit (G_OBJECT (config), indicator_config_signals[CONFIGURATION_CHANGED], 0);
    }

  return config;
}
