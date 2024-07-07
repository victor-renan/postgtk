#include <gtk/gtk.h>

#include "app.h"
#include "window.h"

struct _MainWindow {
    GtkApplicationWindow parent;
};

G_DEFINE_TYPE(MainWindow, main_window, GTK_TYPE_APPLICATION_WINDOW);

static void main_window_init(MainWindow *window) {
    gtk_widget_init_template(GTK_WIDGET(window));
}

static void main_window_class_init(MainWindowClass *class) {
    gtk_widget_class_set_template_from_resource(
        GTK_WIDGET_CLASS(class), "/org/postgtk/window.ui");
}

MainWindow *main_window_new(MainApp *app) {
    return g_object_new(MAIN_WINDOW_TYPE, "application", app, NULL);
}

void main_window_open(MainWindow *window, GFile *file) {

}