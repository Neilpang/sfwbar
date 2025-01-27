#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <gtk/gtk.h>

void window_ref ( GtkWidget *self, GtkWidget *ref );
void window_unref ( GtkWidget *ref, GtkWidget *self );
gboolean window_ref_check ( GtkWidget *self );

#endif
