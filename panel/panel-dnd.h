/* vim: set expandtab ts=8 sw=4: */

/*  $Id$
 *
 *  Copyright © 2005 Jasper Huijsmans <jasper@xfce.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published 
 *  by the Free Software Foundation; either version 2 of the License, or
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

#ifndef _PANEL_DND_H
#define _PANEL_DND_H

#include <gtk/gtk.h>

enum 
{
    TARGET_PLUGIN_NAME,
    TARGET_PLUGIN_WIDGET,
    TARGET_FILE
};

void panel_dnd_set_dest (GtkWidget *widget);

void panel_dnd_set_widget_delete_dest (GtkWidget *widget);

void panel_dnd_unset_dest (GtkWidget *widget);

GtkWidget *panel_dnd_get_plugin_from_data (GtkSelectionData *data);


void panel_dnd_set_name_source (GtkWidget *widget);

void panel_dnd_set_widget_source (GtkWidget *widget);

void panel_dnd_unset_source (GtkWidget *widget);

void panel_dnd_set_widget_data (GtkSelectionData *data, GtkWidget *plugin);

#endif /* _PANEL_DND_H */
