#ifndef __BAR_H__
#define __BAR_H__

#include <gtk/gtk.h>

#define BAR_TYPE            (bar_get_type())
#define BAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), BAR_TYPE, Bar))
#define BAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), BAR_TYPE, BarClass))
#define IS_BAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), BAR_TYPE))
#define IS_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), BAR_TYPE))
#define BAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), BAR_TYPE, BarClass))

typedef struct _Bar Bar;
typedef struct _BarClass BarClass;

struct _Bar
{
  GtkWindow parent_class;
};

struct _BarClass
{
  GtkWindowClass parent_class;
  void (*old_hide)( GtkWidget * );
};

typedef struct _BarPrivate BarPrivate;

struct _BarPrivate {
  gchar *name;
  GtkWidget *start, *center, *end;
  gboolean hidden;
  gboolean jump;
  gboolean visible;
  gchar *output;
  gchar *bar_id;
};

GtkWidget *bar_new ( gchar * );
void bar_set_monitor ( gchar *, GtkWidget * );
void bar_set_layer ( gchar *, GtkWidget *);
void bar_set_size ( gchar *, GtkWidget * );
void bar_set_exclusive_zone ( gchar *, GtkWidget * );
gchar *bar_get_output ( GtkWidget * );
gint bar_get_toplevel_dir ( GtkWidget * );
void bar_set_id ( GtkWidget *self, gchar *id );
void bar_set_visibility ( GtkWidget *self, const gchar *id, gchar state );
void bar_visibility_toggle_all ( gpointer d );
void bar_monitor_added_cb ( GdkDisplay *, GdkMonitor * );
void bar_monitor_removed_cb ( GdkDisplay *, GdkMonitor * );
gboolean bar_update_monitor ( GtkWidget * );
void bar_save_monitor ( GtkWidget * );
GtkWidget *bar_from_name ( gchar *name );
GtkWidget *bar_grid_from_name ( gchar *addr );
void bar_set_theme ( gchar *new_theme );
GtkWidget *bar_mirror ( GtkWidget *src );

#endif
