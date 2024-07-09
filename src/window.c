#include <gtk/gtk.h>

#include "app.h"
#include "window.h"

struct _MainWindow {
    GtkApplicationWindow parent;
    GtkWidget *reqlist;
    GtkWidget *reqplace;
    GtkWidget *reqplace_methods;
};

const int HTTP_METHODS_SIZE = 9;

const char *HTTP_METHODS[] = {
    "GET",
    "POST",
    "PUT",
    "PATCH",
    "DELETE",
    "OPTIONS",
    "HEAD",
    "CONNECT",
    "TRACE",
};

G_DEFINE_TYPE(MainWindow, main_window, GTK_TYPE_APPLICATION_WINDOW);

static void main_window_init(MainWindow *window)
{
    gtk_widget_init_template(GTK_WIDGET(window));
}

static void main_window_class_init(MainWindowClass *class)
{
    gtk_widget_class_set_template_from_resource(
        GTK_WIDGET_CLASS(class), "/org/postgtk/window.ui");
    gtk_widget_class_bind_template_child(
        GTK_WIDGET_CLASS(class), MainWindow, reqlist);
    gtk_widget_class_bind_template_child(
        GTK_WIDGET_CLASS(class), MainWindow, reqplace);
}

MainWindow *main_window_new(MainApp *app)
{
    return g_object_new(MAIN_WINDOW_TYPE, "application", app, NULL);
}

void set_reqplace_methods(MainWindow *window)
{
    
}

void main_window_open(MainWindow *window, GFile *file)
{
    if (file != NULL) {

    }
}