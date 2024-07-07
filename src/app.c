#include <gtk/gtk.h>

#include "app.h"
#include "window.h"

struct _MainApp {
	GtkApplication parent;
};

G_DEFINE_TYPE(MainApp, main_app, GTK_TYPE_APPLICATION);

static void main_app_init(MainApp *app) {}

static void main_app_activate(GApplication *app) {
	MainWindow *window;

    window = main_window_new(MAIN_APP(app));
	gtk_window_present(GTK_WINDOW(window));
}

static void main_app_open(GApplication *app, GFile **files, int n_files, const char *hint) {
	GList *windows;
	MainWindow *window;

    windows = gtk_application_get_windows(GTK_APPLICATION(app));
	if (windows) {
		window = MAIN_WINDOW(windows->data);
	} else {
        window = main_window_new(MAIN_APP(app));
    }

    for (int i = 0; i < n_files; i++) {
        main_window_open(window, files[i]);
    }

    gtk_window_present(GTK_WINDOW(window));
}

static void main_app_class_init(MainAppClass *class) {
    G_APPLICATION_CLASS(class)->activate = main_app_activate;
    G_APPLICATION_CLASS(class)->open = main_app_open;
}

MainApp *main_app_new(void) {
    return g_object_new(MAIN_APP_TYPE, "app", "org.postgtk", "flags", G_APPLICATION_HANDLES_OPEN, NULL);
}