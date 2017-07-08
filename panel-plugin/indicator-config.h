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

#ifndef __INDICATOR_CONFIG_H__
#define __INDICATOR_CONFIG_H__

#include <glib.h>
#ifdef XFCONF_LEGACY
#include <dbus/dbus-glib.h>
#endif

G_BEGIN_DECLS

typedef struct _IndicatorConfigClass IndicatorConfigClass;
typedef struct _IndicatorConfig      IndicatorConfig;

#define XFCE_TYPE_INDICATOR_CONFIG             (indicator_config_get_type ())
#define XFCE_INDICATOR_CONFIG(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_INDICATOR_CONFIG, IndicatorConfig))
#define XFCE_INDICATOR_CONFIG_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_INDICATOR_CONFIG, IndicatorConfigClass))
#define XFCE_IS_INDICATOR_CONFIG(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_INDICATOR_CONFIG))
#define XFCE_IS_INDICATOR_CONFIG_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_INDICATOR_CONFIG))
#define XFCE_INDICATOR_CONFIG_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_INDICATOR_CONFIG, IndicatorConfigClass))

#ifdef XFCONF_LEGACY
#define XFCE_TYPE_INDICATOR_CONFIG_VALUE_ARRAY (indicator_config_value_array_get_type ())
#else
#define XFCE_TYPE_INDICATOR_CONFIG_VALUE_ARRAY G_TYPE_PTR_ARRAY
#endif

GType              indicator_config_value_array_get_type    (void) G_GNUC_CONST;

GType              indicator_config_get_type                (void) G_GNUC_CONST;

IndicatorConfig   *indicator_config_new                     (const gchar          *property_base);

void               indicator_config_set_orientation         (IndicatorConfig      *config,
                                                             GtkOrientation        panel_orientation,
                                                             GtkOrientation        orientation);

GtkOrientation     indicator_config_get_orientation         (IndicatorConfig      *config);

GtkOrientation     indicator_config_get_panel_orientation   (IndicatorConfig      *config);

void               indicator_config_set_size                (IndicatorConfig      *config,
                                                             gint                  panel_size,
                                                             gint                  nrows);

gint               indicator_config_get_nrows               (IndicatorConfig      *config);

gint               indicator_config_get_panel_size          (IndicatorConfig      *config);

gboolean           indicator_config_get_single_row          (IndicatorConfig      *config);

gboolean           indicator_config_get_align_left          (IndicatorConfig      *config);

gboolean           indicator_config_get_mode_whitelist      (IndicatorConfig      *config);

gboolean           indicator_config_is_blacklisted          (IndicatorConfig      *config,
                                                             const gchar          *name);

void               indicator_config_blacklist_set           (IndicatorConfig      *config,
                                                             const gchar          *name,
                                                             gboolean              add);

gboolean           indicator_config_is_whitelisted          (IndicatorConfig      *config,
                                                             const gchar          *name);

gboolean           indicator_config_get_square_icons        (IndicatorConfig      *config);


void               indicator_config_whitelist_set           (IndicatorConfig      *config,
                                                             const gchar          *name,
                                                             gboolean              add);

gchar            **indicator_config_get_excluded_modules    (IndicatorConfig      *config);

GList             *indicator_config_get_known_indicators    (IndicatorConfig      *config);

void               indicator_config_add_known_indicator     (IndicatorConfig      *config,
                                                             const gchar          *name);

void               indicator_config_swap_known_indicators   (IndicatorConfig      *config,
                                                             const gchar          *name1,
                                                             const gchar          *name2);

void               indicator_config_names_clear             (IndicatorConfig      *config);

G_END_DECLS

#endif /* !__INDICATOR_CONFIG_H__ */
