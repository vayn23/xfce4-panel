/*
 * Copyright (C) 2008-2009 Nick Schermer <nick@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __XFCE_TASKLIST_H__
#define __XFCE_TASKLIST_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _XfceTasklistClass     XfceTasklistClass;
typedef struct _XfceTasklist          XfceTasklist;
typedef enum   _XfceTasklistGrouping  XfceTasklistGrouping;
typedef enum   _XfceTasklistSortOrder XfceTasklistSortOrder;

#define XFCE_TYPE_TASKLIST            (xfce_tasklist_get_type ())
#define XFCE_TASKLIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), XFCE_TYPE_TASKLIST, XfceTasklist))
#define XFCE_TASKLIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_TASKLIST, XfceTasklistClass))
#define XFCE_IS_TASKLIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), XFCE_TYPE_TASKLIST))
#define XFCE_IS_TASKLIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_TASKLIST))
#define XFCE_TASKLIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_TASKLIST, XfceTasklistClass))



enum _XfceTasklistGrouping
{
  XFCE_TASKLIST_GROUPING_NEVER,
  XFCE_TASKLIST_GROUPING_AUTO, /* when space is limited */
  XFCE_TASKLIST_GROUPING_ALWAYS,

  XFCE_TASKLIST_GROUPING_MIN = XFCE_TASKLIST_GROUPING_NEVER,
  XFCE_TASKLIST_GROUPING_MAX = XFCE_TASKLIST_GROUPING_ALWAYS,
  XFCE_TASKLIST_GROUPING_DEFAULT = XFCE_TASKLIST_GROUPING_AUTO
};

enum _XfceTasklistSortOrder
{
  XFCE_TASKLIST_SORT_ORDER_TIMESTAMP, /* sort by unique_id */
  XFCE_TASKLIST_SORT_ORDER_GROUP_TIMESTAMP, /* sort by group and then by timestamp */
  XFCE_TASKLIST_SORT_ORDER_TITLE, /* sort by window title */
  XFCE_TASKLIST_SORT_ORDER_GROUP_TITLE, /* sort by group and then by title */

  XFCE_TASKLIST_SORT_ORDER_MIN = XFCE_TASKLIST_SORT_ORDER_TIMESTAMP,
  XFCE_TASKLIST_SORT_ORDER_MAX = XFCE_TASKLIST_SORT_ORDER_GROUP_TITLE,
  XFCE_TASKLIST_SORT_ORDER_DEFAULT = XFCE_TASKLIST_SORT_ORDER_GROUP_TIMESTAMP
};



GType xfce_tasklist_get_type        (void) G_GNUC_CONST;

void  xfce_tasklist_set_orientation (XfceTasklist   *tasklist,
                                     GtkOrientation  orientation);

void  xfce_tasklist_set_size        (XfceTasklist   *tasklist,
                                     gint            size);

G_END_DECLS

#endif /* !__XFCE_TASKLIST_H__ */