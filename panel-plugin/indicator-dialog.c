/*
 *  Copyright (C) 2012 Andrzej <ndrwrdck@gmail.com>
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

#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>
#include <xfconf/xfconf.h>
#include <exo/exo.h>
#include <libxfce4panel/xfce-panel-plugin.h>

#include "indicator.h"
#include "indicator-dialog.h"
#include "indicator-dialog_ui.h"



static void              indicator_dialog_build               (IndicatorDialog          *dialog,
                                                               IndicatorPlugin          *plugin);
static void              indicator_dialog_response            (GtkWidget                *window,
                                                               gint                      response_id,
                                                               IndicatorDialog          *dialog);



struct _IndicatorDialogClass
{
  GtkBuilderClass   __parent__;
};

struct _IndicatorDialog
{
  GtkBuilder        __parent__;

  GObject          *dialog;

  XfconfChannel    *channel;

  IndicatorPlugin  *plugin;

  //GtkTreeSelection *selection;

  //gulong            bindings[4];
  gulong            property_watch_id;
};



G_DEFINE_TYPE (IndicatorDialog, indicator_dialog, GTK_TYPE_BUILDER)



static void
indicator_dialog_class_init (IndicatorDialogClass *klass)
{
}



static void
indicator_dialog_init (IndicatorDialog *dialog)
{
}




static void
indicator_dialog_build (IndicatorDialog *dialog,
                        IndicatorPlugin *plugin)
{
  GObject     *object;
  GError      *error = NULL;

  dialog->channel = xfconf_channel_get ("xfce4-panel");
  dialog->plugin = plugin;

  if (xfce_titled_dialog_get_type () == 0)
    return;

  /* load the builder data into the object */
  if (gtk_builder_add_from_string (GTK_BUILDER (dialog), indicator_dialog_ui,
                                   indicator_dialog_ui_length, &error))
    {

      dialog->dialog = gtk_builder_get_object (GTK_BUILDER (dialog), "dialog");
      g_return_if_fail (XFCE_IS_TITLED_DIALOG (dialog->dialog));
      g_signal_connect (G_OBJECT (dialog->dialog), "response",
                        G_CALLBACK (indicator_dialog_response), dialog);

      object = gtk_builder_get_object (GTK_BUILDER (dialog), "size-max");
      g_return_if_fail (GTK_IS_WIDGET (object));
      exo_mutual_binding_new (G_OBJECT (indicator_get_buttonbox (plugin)), "icon-size-max",
                              G_OBJECT (object), "value");
    }
  else
    {
      g_critical ("Faild to construct the builder: %s.",
                  error->message);
      g_error_free (error);
    }
}


static void
indicator_dialog_response (GtkWidget                *window,
                           gint                      response_id,
                           IndicatorDialog          *dialog)
{
  g_return_if_fail (GTK_IS_DIALOG (window));
  g_return_if_fail (XFCE_IS_INDICATOR_DIALOG (dialog));

  if (response_id == GTK_RESPONSE_HELP)
    {
      xfce_dialog_show_help (GTK_WINDOW (window), "xfce4-indicator", "dialog", NULL);
    }
  else
    {
      g_signal_handler_disconnect (dialog->channel, dialog->property_watch_id);

      gtk_widget_destroy (window);

      g_object_unref (G_OBJECT (dialog));
    }
}



void
indicator_dialog_show (GdkScreen       *screen,
                       IndicatorPlugin *plugin)
{
  static IndicatorDialog *dialog = NULL;

  g_return_if_fail (GDK_IS_SCREEN (screen));
  g_return_if_fail (XFCE_IS_INDICATOR_PLUGIN (plugin));

  if (dialog == NULL)
    {
      dialog = g_object_new (XFCE_TYPE_INDICATOR_DIALOG, NULL);
      g_object_add_weak_pointer (G_OBJECT (dialog), (gpointer *) &dialog);
      indicator_dialog_build (XFCE_INDICATOR_DIALOG (dialog), plugin);
      gtk_widget_show (GTK_WIDGET (dialog->dialog));
    }
  else
    {
      gtk_window_present (GTK_WINDOW (dialog->dialog));
    }

  gtk_window_set_screen (GTK_WINDOW (dialog->dialog), screen);
}
