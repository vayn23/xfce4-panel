/*  xfce4
 *
 *  Copyright (C) 2002 Jasper Huijsmans <huysmans@users.sourceforge.net>
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

#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>
#include <libxfcegui4/dialogs.h>

#include "xfce_settings.h"
#include "xfce_settings_plugin.h"
#include "xfce_settings_dialog.h"

#define _(String) String
#define N_(String) String

#define strequals(s1,s2) !strcmp(s1, s2)

#ifndef DATADIR
#define DATADIR "/usr/local/share/xfce4"
#endif

/* panel sides / popup orientation */
enum
{ LEFT, RIGHT, TOP, BOTTOM };

/* panel styles */
enum
{ OLD_STYLE, NEW_STYLE };

/* panel orientation */
enum
{ HORIZONTAL, VERTICAL };

static McsManager *mcs_manager;

enum
{ RESPONSE_REMOVE, RESPONSE_CHANGE, RESPONSE_CANCEL, RESPONSE_REVERT };

/*  Global settings
 *  ---------------
 *  size: option menu
 *  popup position: option menu
 *  style: option menu (should be radio buttons according to GNOME HIG)
 *  panel orientation: option menu
 *  icon theme: option menu
 *  num groups : spinbutton
 *  position: button (restore default)
*/
static GtkWidget *orientation_menu;
static GtkWidget *size_menu;
static GtkWidget *popup_position_menu;
static GtkWidget *style_menu;
static GtkWidget *theme_menu;
/*static GtkWidget *groups_spin;*/

static GtkWidget *layer_menu;
static GtkWidget *pos_button;

static GtkSizeGroup *sg = NULL;
static GtkWidget *revert;

static int backup_theme_index = 0;

GtkShadowType main_shadow = GTK_SHADOW_NONE;
GtkShadowType header_shadow = GTK_SHADOW_NONE;
GtkShadowType option_shadow = GTK_SHADOW_NONE;

static gboolean is_running = FALSE;
static GtkWidget *dialog = NULL;

/* stop gcc from complaining when using -Wall:
 * this variable will not be used, but I want the 
 * definition of names to be available in the 
 * xfce-settings.h header for other modules */
char **names = xfce_settings_names;

/* useful widgets */
static void add_header(const char *text, GtkBox * box)
{
    GtkWidget *frame, *label;
    char *markup;

    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), header_shadow);
    gtk_widget_show(frame);
    gtk_box_pack_start(box, frame, FALSE, TRUE, 0);

    label = gtk_label_new(NULL);
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    markup = g_strconcat("<b>", text, "</b>", NULL);
    gtk_label_set_markup(GTK_LABEL(label), markup);
    g_free(markup);
    gtk_widget_show(label);
    gtk_container_add(GTK_CONTAINER(frame), label);
}

#define SKIP 12

static void add_spacer(GtkBox * box)
{
    GtkWidget *eventbox = gtk_alignment_new(0,0,0,0);

    gtk_widget_set_size_request(eventbox, SKIP, SKIP);
    gtk_widget_show(eventbox);
    gtk_box_pack_start(box, eventbox, FALSE, TRUE, 0);
}

/* backup values */
static int bu_orientation;
static int bu_layer;
static int bu_size;
static int bu_popup_position;
static int bu_style;
static char *bu_theme;

static void xfce_create_backup(void)
{
    McsSetting *setting;

    setting = &xfce_options[XFCE_ORIENTATION];
    bu_orientation = setting->data.v_int;

    setting = &xfce_options[XFCE_LAYER];
    bu_layer = setting->data.v_int;

    setting = &xfce_options[XFCE_SIZE];
    bu_size = setting->data.v_int;

    setting = &xfce_options[XFCE_POPUPPOSITION];
    bu_popup_position = setting->data.v_int;

    setting = &xfce_options[XFCE_STYLE];
    bu_style = setting->data.v_int;

    setting = &xfce_options[XFCE_THEME];
    bu_theme = g_strdup(setting->data.v_string);

    xfce_options[XFCE_POSITION].data.v_int = XFCE_POSITION_SAVE;
    mcs_manager_set_setting(mcs_manager, &xfce_options[XFCE_POSITION], 
	    		    CHANNEL);
    mcs_manager_notify(mcs_manager, CHANNEL);
}

static void xfce_restore_backup(void)
{
    /* we just let the calbacks of our dialog do all the work */

    /* this must be first */
    gtk_option_menu_set_history(GTK_OPTION_MENU(orientation_menu),
    				bu_orientation);

    gtk_option_menu_set_history(GTK_OPTION_MENU(size_menu), bu_size);

    gtk_option_menu_set_history(GTK_OPTION_MENU(popup_position_menu),
    				bu_popup_position);

    gtk_option_menu_set_history(GTK_OPTION_MENU(style_menu), bu_style);
    gtk_option_menu_set_history(GTK_OPTION_MENU(theme_menu),
                                backup_theme_index);

/*    gtk_spin_button_set_value(GTK_SPIN_BUTTON(groups_spin), bu_num_groups);*/
    
    gtk_option_menu_set_history(GTK_OPTION_MENU(layer_menu), bu_layer);

    xfce_options[XFCE_POSITION].data.v_int = XFCE_POSITION_RESTORE;
    mcs_manager_set_setting(mcs_manager, &xfce_options[XFCE_POSITION], CHANNEL);
    mcs_manager_notify(mcs_manager, CHANNEL);
}

static void xfce_free_backup(void)
{
    g_free(bu_theme);
}

/* size */
static void size_menu_changed(GtkOptionMenu * menu)
{
    int n = gtk_option_menu_get_history(menu);
    McsSetting *setting = &xfce_options[XFCE_SIZE];

    if (n == setting->data.v_int)
	return;

    setting->data.v_int = n;
    mcs_manager_set_setting(mcs_manager, setting, CHANNEL);
    mcs_manager_notify(mcs_manager, CHANNEL);
    
    gtk_widget_set_sensitive(revert, TRUE);
}

static void add_size_menu(GtkWidget * option_menu, int size)
{
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *item;

    item = gtk_menu_item_new_with_label(_("Tiny"));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    item = gtk_menu_item_new_with_label(_("Small"));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    item = gtk_menu_item_new_with_label(_("Medium"));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    item = gtk_menu_item_new_with_label(_("Large"));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    gtk_option_menu_set_menu(GTK_OPTION_MENU(option_menu), menu);
    gtk_option_menu_set_history(GTK_OPTION_MENU(option_menu), size);

    g_signal_connect(option_menu, "changed", G_CALLBACK(size_menu_changed),
                     NULL);
}

/* style */
static void style_changed(GtkOptionMenu * menu)
{
    int n = gtk_option_menu_get_history(menu);
    McsSetting *setting = &xfce_options[XFCE_STYLE];

    if(n == setting->data.v_int)
        return;

    setting->data.v_int = n;
    mcs_manager_set_setting(mcs_manager, setting, CHANNEL);
    mcs_manager_notify(mcs_manager, CHANNEL);

    gtk_widget_set_sensitive(revert, TRUE);
}

static void add_style_menu(GtkWidget * option_menu, int style)
{
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *item;
    int n, pos;

    item = gtk_menu_item_new_with_label(_("Traditional"));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    item = gtk_menu_item_new_with_label(_("Modern"));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    gtk_option_menu_set_menu(GTK_OPTION_MENU(option_menu), menu);
    gtk_option_menu_set_history(GTK_OPTION_MENU(option_menu), style);

    g_signal_connect(option_menu, "changed", G_CALLBACK(style_changed), NULL);

    n = xfce_options[XFCE_ORIENTATION].data.v_int;
    pos = xfce_options[XFCE_POPUPPOSITION].data.v_int;
    
    if ((n == HORIZONTAL && (pos == LEFT || pos == RIGHT)) ||
        (n == VERTICAL && (pos == TOP || pos == BOTTOM)))
    {
        gtk_option_menu_set_history(GTK_OPTION_MENU(option_menu), NEW_STYLE);
        gtk_widget_set_sensitive(option_menu, FALSE);
    }
    else
    {
        gtk_widget_set_sensitive(option_menu, TRUE);
    }
}

/* Panel Orientation */
static void orientation_changed(GtkOptionMenu * menu)
{
    int n = gtk_option_menu_get_history(menu);
    int pos = xfce_options[XFCE_POPUPPOSITION].data.v_int;
    McsSetting *setting = &xfce_options[XFCE_ORIENTATION];

    if(n == setting->data.v_int)
        return;

    setting->data.v_int = n;
    mcs_manager_set_setting(mcs_manager, setting, CHANNEL);

    /* this seems more logical */
    switch (pos)
    {
	case LEFT:
	    pos = BOTTOM;
	    break;
	case RIGHT:
	    pos = TOP;
	    break;
	case TOP:
	    pos = RIGHT;
	    break;
	case BOTTOM:
	    pos = LEFT;
	    break;
    }
    
    gtk_option_menu_set_history(GTK_OPTION_MENU(popup_position_menu), pos);
	
    if ((n == HORIZONTAL && (pos == LEFT || pos == RIGHT)) ||
        (n == VERTICAL && (pos == TOP || pos == BOTTOM)))
    {
        gtk_option_menu_set_history(GTK_OPTION_MENU(style_menu), NEW_STYLE);
        gtk_widget_set_sensitive(style_menu, FALSE);
    }
    else
    {
        gtk_widget_set_sensitive(style_menu, TRUE);
    }
    
    gtk_widget_set_sensitive(revert, TRUE);
}

static void add_orientation_menu(GtkWidget * option_menu, int orientation)
{
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *item;

    item = gtk_menu_item_new_with_label(_("Horizontal"));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    item = gtk_menu_item_new_with_label(_("Vertical"));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    gtk_option_menu_set_menu(GTK_OPTION_MENU(option_menu), menu);
    gtk_option_menu_set_history(GTK_OPTION_MENU(option_menu), orientation);

    g_signal_connect(option_menu, "changed",
                     G_CALLBACK(orientation_changed), NULL);

}

/* popup position */
static void popup_position_changed(GtkOptionMenu * menu)
{
    int n = gtk_option_menu_get_history(menu);
    McsSetting *setting = &xfce_options[XFCE_POPUPPOSITION];

    if(n == setting->data.v_int)
        return;

    setting->data.v_int = n;
    mcs_manager_set_setting(mcs_manager, setting, CHANNEL);
    mcs_manager_notify(mcs_manager, CHANNEL);
    
    gtk_widget_set_sensitive(revert, TRUE);
}

static void add_popup_position_menu(GtkWidget * option_menu, int position)
{
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *item;

    item = gtk_menu_item_new_with_label(_("Left"));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    item = gtk_menu_item_new_with_label(_("Right"));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    item = gtk_menu_item_new_with_label(_("Top"));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    item = gtk_menu_item_new_with_label(_("Bottom"));
    gtk_widget_show(item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

    gtk_option_menu_set_menu(GTK_OPTION_MENU(option_menu), menu);
    gtk_option_menu_set_history(GTK_OPTION_MENU(option_menu), position);

    g_signal_connect(option_menu, "changed",
                     G_CALLBACK(popup_position_changed), NULL);
}

/*  theme
*/
static char **find_themes(void)
{
    char **themes = NULL;
    GList *list = NULL, *li;
    GDir *gdir;
    char **dirs, **d;
    const char *file;
    int i, len;

    /* Add default theme */
    dirs = g_new0(char *, 3);

    dirs[0] = g_build_filename(g_getenv("HOME"), ".xfce4", "themes", NULL);
    dirs[1] = g_build_filename(DATADIR, "themes", NULL);

    for(d = dirs; *d; d++)
    {
        gdir = g_dir_open(*d, 0, NULL);

        if(gdir)
        {
            while((file = g_dir_read_name(gdir)))
            {
                char *path = g_build_filename(*d, file, NULL);

                if(!g_list_find_custom(list, file, (GCompareFunc) strcmp) &&
                   g_file_test(path, G_FILE_TEST_IS_DIR))
                {
                    list = g_list_append(list, g_strdup(file));
                }

                g_free(path);
            }

            g_dir_close(gdir);
        }
    }

    len = g_list_length(list);

    themes = g_new0(char *, len + 1);

    for(i = 0, li = list; li; li = li->next, i++)
    {
	themes[i] = (char *)li->data;
    }

    g_list_free(list);
    g_strfreev(dirs);

    return themes;
}

static void theme_changed(GtkOptionMenu * option_menu)
{
    const char *theme;
    GtkWidget *label;
    McsSetting *setting = &xfce_options[XFCE_THEME];

    /* Right, this is weird, apparently the option menu
     * button reparents the label connected to the menuitem
     * that is selected. So to get to the label we have to go 
     * to the child of the button and not of the menu item!
     *
     * This took a while to find out :-)
     */
    label = gtk_bin_get_child(GTK_BIN(option_menu));

    theme = gtk_label_get_text(GTK_LABEL(label));

    if(strequals(theme, setting->data.v_string))
        return;

    g_free(setting->data.v_string);
    setting->data.v_string = g_strdup(theme);
    mcs_manager_set_setting(mcs_manager, setting, CHANNEL);
    mcs_manager_notify(mcs_manager, CHANNEL);

    gtk_widget_set_sensitive(revert, TRUE);
}

static void add_theme_menu(GtkWidget * option_menu, const char *theme)
{
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *item;
    int i = 0, n = 0;
    char **themes = find_themes();
    char **s;

    for(i = 0, s = themes; *s; s++, i++)
    {
	item = gtk_menu_item_new_with_label(*s);
	gtk_widget_show(item);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);

	if(theme && strequals(theme, *s))
	    n = backup_theme_index = i;
    }

    g_strfreev(themes);

    gtk_option_menu_set_menu(GTK_OPTION_MENU(option_menu), menu);
    gtk_option_menu_set_history(GTK_OPTION_MENU(option_menu), n);

    g_signal_connect(option_menu, "changed", G_CALLBACK(theme_changed), NULL);
}

static void add_style_box(GtkBox * box)
{
    GtkWidget *frame, *vbox, *hbox, *label;

    /* frame and vbox */
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), option_shadow);
    gtk_widget_show(frame);
    gtk_box_pack_start(box, frame, TRUE, TRUE, 0);

    vbox = gtk_vbox_new(FALSE, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(frame), vbox);

    /* size */
    hbox = gtk_hbox_new(FALSE, 4);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

    label = gtk_label_new(_("Panel size:"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_widget_show(label);
    gtk_size_group_add_widget(sg, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    size_menu = gtk_option_menu_new();
    gtk_widget_show(size_menu);
    add_size_menu(size_menu, xfce_options[XFCE_SIZE].data.v_int);
    gtk_box_pack_start(GTK_BOX(hbox), size_menu, TRUE, TRUE, 0);

    /* style */
    hbox = gtk_hbox_new(FALSE, 4);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

    label = gtk_label_new(_("Panel style:"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_widget_show(label);
    gtk_size_group_add_widget(sg, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    style_menu = gtk_option_menu_new();
    gtk_widget_show(style_menu);
    add_style_menu(style_menu, xfce_options[XFCE_STYLE].data.v_int);
    gtk_box_pack_start(GTK_BOX(hbox), style_menu, TRUE, TRUE, 0);

    /* panel orientation */
    hbox = gtk_hbox_new(FALSE, 4);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

    label = gtk_label_new(_("Panel Orientation:"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_widget_show(label);
    gtk_size_group_add_widget(sg, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    orientation_menu = gtk_option_menu_new();
    gtk_widget_show(orientation_menu);
    add_orientation_menu(orientation_menu, 
	    		 xfce_options[XFCE_ORIENTATION].data.v_int);
    gtk_box_pack_start(GTK_BOX(hbox), orientation_menu, TRUE, TRUE, 0);

    /* popup button */
    hbox = gtk_hbox_new(FALSE, 4);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

    label = gtk_label_new(_("Popup position:"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_widget_show(label);
    gtk_size_group_add_widget(sg, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    popup_position_menu = gtk_option_menu_new();
    gtk_widget_show(popup_position_menu);
    add_popup_position_menu(popup_position_menu, 
	    		    xfce_options[XFCE_POPUPPOSITION].data.v_int);
    gtk_box_pack_start(GTK_BOX(hbox), popup_position_menu, TRUE, TRUE, 0);

    /* icon theme */
    hbox = gtk_hbox_new(FALSE, 4);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

    label = gtk_label_new(_("Icon theme:"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_widget_show(label);
    gtk_size_group_add_widget(sg, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    theme_menu = gtk_option_menu_new();
    gtk_widget_show(theme_menu);
    add_theme_menu(theme_menu, xfce_options[XFCE_THEME].data.v_string);
    gtk_box_pack_start(GTK_BOX(hbox), theme_menu, TRUE, TRUE, 0);
}

#if 0
/* panel groups and screen buttons */
static void spin_changed(GtkWidget * spin)
{
    int n;
    gboolean changed = FALSE;
    n = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));


    if(n != settings.num_groups)
    {
        panel_set_num_groups(n);
        changed = TRUE;
    }

    if(changed)
        gtk_widget_set_sensitive(revert, TRUE);
}

static void add_controls_box(GtkBox * box)
{
    GtkWidget *frame, *vbox, *hbox, *label;

    /* frame and vbox */
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), option_shadow);
    gtk_widget_show(frame);
    gtk_box_pack_start(box, frame, TRUE, TRUE, 0);

    vbox = gtk_vbox_new(FALSE, 4);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(frame), vbox);

    /* groups */
    hbox = gtk_hbox_new(FALSE, 4);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

    label = gtk_label_new(_("Panel controls:"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_widget_show(label);
    gtk_size_group_add_widget(sg, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    groups_spin = gtk_spin_button_new_with_range(1, 2*NBGROUPS, 1);
    gtk_widget_show(groups_spin);
    gtk_box_pack_start(GTK_BOX(hbox), groups_spin, FALSE, FALSE, 0);

    gtk_spin_button_set_value(GTK_SPIN_BUTTON(groups_spin), settings.num_groups);
    g_signal_connect(groups_spin, "value-changed", G_CALLBACK(spin_changed),
                     NULL);
}
#endif

/* position */
static void layer_changed(GtkWidget * om, gpointer data)
{
    int layer;
    McsSetting *setting = &xfce_options[XFCE_LAYER];
    
    layer = gtk_option_menu_get_history(GTK_OPTION_MENU(om));

    if (setting->data.v_int == layer)
	return;

    setting->data.v_int = layer;
    mcs_manager_set_setting(mcs_manager, setting, CHANNEL);
    mcs_manager_notify(mcs_manager, CHANNEL);
    
    gtk_widget_set_sensitive(revert, TRUE);
}

static char *position_names[] = {
    N_("Bottom"),
    N_("Top"),
    N_("Left"),
    N_("Right"),
};

static void position_clicked(GtkWidget * button, GtkOptionMenu *om)
{
    int n;
    McsSetting *setting = &xfce_options[XFCE_POSITION];
    
    n = gtk_option_menu_get_history(om);

    /* make sure it gets changed */
    setting->data.v_int = XFCE_POSITION_NONE;
    mcs_manager_set_setting(mcs_manager, setting, CHANNEL);
    mcs_manager_notify(mcs_manager, CHANNEL);
    gdk_flush();
    
    setting->data.v_int = n;
    mcs_manager_set_setting(mcs_manager, setting, CHANNEL);
    mcs_manager_notify(mcs_manager, CHANNEL);
    
    gtk_widget_set_sensitive(revert, TRUE);
}

static void add_position_box(GtkBox * box)
{
    GtkWidget *frame, *vbox, *hbox, *label, *optionmenu, *menu;
    int i;

    /* frame and vbox */
    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), option_shadow);
    gtk_widget_show(frame);
    gtk_box_pack_start(box, frame, TRUE, TRUE, 0);

    vbox = gtk_vbox_new(FALSE, 6);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 4);
    gtk_widget_show(vbox);
    gtk_container_add(GTK_CONTAINER(frame), vbox);

    /* checkbutton */
    hbox = gtk_hbox_new(FALSE, 4);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

    label = gtk_label_new(_("Panel layer:"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_widget_show(label);
    gtk_size_group_add_widget(sg, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    layer_menu = gtk_option_menu_new();
    gtk_widget_show(layer_menu);
    gtk_box_pack_start(GTK_BOX(hbox), layer_menu, FALSE, FALSE, 0);

    menu = gtk_menu_new();
    gtk_option_menu_set_menu(GTK_OPTION_MENU(layer_menu), menu);

    {
	GtkWidget *mi;

	mi = gtk_menu_item_new_with_label(_("Top"));
	gtk_widget_show(mi);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

	mi = gtk_menu_item_new_with_label(_("Normal"));
	gtk_widget_show(mi);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);

	mi = gtk_menu_item_new_with_label(_("Bottom"));
	gtk_widget_show(mi);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
    }
    
    gtk_option_menu_set_history(GTK_OPTION_MENU(layer_menu), 
	    			xfce_options[XFCE_LAYER].data.v_int);

    g_signal_connect(layer_menu, "changed", G_CALLBACK(layer_changed), NULL);    

    /* centering */
    hbox = gtk_hbox_new(FALSE, 4);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

    label = gtk_label_new(_("Center the panel:"));
    gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
    gtk_widget_show(label);
    gtk_size_group_add_widget(sg, label);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    optionmenu = gtk_option_menu_new();
    gtk_widget_show(optionmenu);
    gtk_box_pack_start(GTK_BOX(hbox), optionmenu, FALSE, FALSE, 0);

    menu = gtk_menu_new();
    gtk_widget_show(menu);
    gtk_option_menu_set_menu(GTK_OPTION_MENU(optionmenu), menu);

    for (i = 0; i < 4; i++)
    {
	GtkWidget *mi = gtk_menu_item_new_with_label(_(position_names[i]));

	gtk_widget_show(mi);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
    }
    
    gtk_option_menu_set_history(GTK_OPTION_MENU(optionmenu), 0);
    
    pos_button = mixed_button_new(GTK_STOCK_APPLY, _("Set"));
    gtk_widget_show(pos_button);
    gtk_box_pack_start(GTK_BOX(hbox), pos_button, FALSE, FALSE, 0);

    g_signal_connect(pos_button, "clicked", G_CALLBACK(position_clicked), optionmenu);
}

/* the dialog */
/* static int lastpage = 0;*/

static void dialog_delete(GtkWidget *dialog)
{
/*    lastpage = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
*/
    gtk_widget_destroy(dialog);
    is_running = FALSE;
    dialog = NULL;

    xfce_free_backup();
    xfce_write_options(mcs_manager);
}

static void dialog_response(GtkWidget *dialog, int response)
{
    if(response == RESPONSE_REVERT)
    {
	xfce_restore_backup();
	xfce_set_options(mcs_manager);
	gtk_widget_set_sensitive(revert, FALSE);
    }
    else
	dialog_delete(dialog);
}

void run_xfce_settings_dialog(McsPlugin *mp)
{
    GtkWidget *button, *header, *hbox, *vbox, *sep;

    if(is_running)
    {
        gtk_window_present(GTK_WINDOW(dialog));
        return;
    }

    is_running = TRUE;

    mcs_manager = mp->manager;

    xfce_create_backup();

    dialog = gtk_dialog_new_with_buttons(_("XFce Panel Preferences"),
                                    	 NULL, GTK_DIALOG_NO_SEPARATOR, NULL);

    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

    gtk_container_set_border_width(GTK_CONTAINER(dialog), 4);

    revert = mixed_button_new(GTK_STOCK_UNDO, _("_Revert"));
    gtk_widget_show(revert);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), revert, RESPONSE_REVERT);
    gtk_widget_set_sensitive(revert, FALSE);

    button = mixed_button_new(GTK_STOCK_OK, _("_Done"));
    gtk_widget_show(button);
    gtk_dialog_add_action_widget(GTK_DIALOG(dialog), button, GTK_RESPONSE_OK);

    g_signal_connect(dialog, "response", G_CALLBACK(dialog_response), dialog);
    g_signal_connect_swapped(dialog, "delete_event", 
	    		     G_CALLBACK(dialog_delete), dialog);
    
    /* pretty header */
    vbox = GTK_DIALOG(dialog)->vbox;
    header = create_header(mp->icon, _("XFce Panel Settings"));
    gtk_box_pack_start(GTK_BOX(vbox), header, TRUE, TRUE, 0);
    add_spacer(GTK_BOX(vbox));
 
    /* hbox */
    hbox = gtk_hbox_new(FALSE, 8);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox,
                       TRUE, TRUE, 0);

    /* Appearance */
    vbox = gtk_vbox_new(FALSE, 8);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

    sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

    add_header(_("Appearance"), GTK_BOX(vbox));
    add_style_box(GTK_BOX(vbox));
    add_spacer(GTK_BOX(vbox));

    g_object_unref(sg);

    /* Separator */
    sep = gtk_vseparator_new();
    gtk_widget_show(sep);
    gtk_box_pack_start(GTK_BOX(hbox), sep, TRUE, TRUE, 0);
    
    /* Position */
    vbox = gtk_vbox_new(FALSE, 8);
    gtk_widget_show(vbox);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, FALSE, FALSE, 0);

    sg = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

    add_header(_("Position"), GTK_BOX(vbox));
    add_position_box(GTK_BOX(vbox));
/*    add_spacer(GTK_BOX(vbox));*/

    g_object_unref(sg);

    gtk_widget_show(dialog);
}

