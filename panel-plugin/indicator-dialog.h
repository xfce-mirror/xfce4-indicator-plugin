/*
 *  Copyright (C) 2012-2013 Andrzej <ndrwrdck@gmail.com>
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

#ifndef __INDICATOR_DIALOG_H__
#define __INDICATOR_DIALOG_H__

#include <gtk/gtk.h>
#include "indicator-config.h"

G_BEGIN_DECLS

#define XFCE_TYPE_INDICATOR_DIALOG (indicator_dialog_get_type ())
G_DECLARE_FINAL_TYPE (IndicatorDialog, indicator_dialog, XFCE, INDICATOR_DIALOG, GtkBuilder)

void               indicator_dialog_show     (IndicatorDialog  *dialog,
                                              GdkScreen        *screen);

IndicatorDialog   *indicator_dialog_new      (IndicatorConfig  *config);

G_END_DECLS

#endif /* !__INDICATOR_DIALOG_H__ */
