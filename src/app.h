#pragma once

#include <gtk/gtk.h>

#define MAIN_APP_TYPE (main_app_get_type())
G_DECLARE_FINAL_TYPE(MainApp, main_app, MAIN, APP, GtkApplication)

MainApp *main_app_new(void);