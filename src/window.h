#pragma once

#include <gtk/gtk.h>
#include "app.h"

#define MAIN_WINDOW_TYPE (main_window_get_type())

G_DECLARE_FINAL_TYPE(MainWindow, main_window, MAIN, WINDOW, GtkApplicationWindow)

MainWindow *main_window_new (MainApp *app);
void main_window_open(MainWindow *window, GFile *file);
