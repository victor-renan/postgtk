#include <gtk/gtk.h>

#include "services/request.h"

#include "app.h"
#include "window.h"


struct _MainWindow {
    GtkApplicationWindow parent;
    GtkWidget *reqlist;
    GtkWidget *reqplace;
    GtkWidget *reqplace_methods;
    GtkWidget *reqplace_body;
    GtkWidget *reqplace_headers;
    GtkWidget *reqplace_url;
    GtkWidget *reqplace_submit;
    GtkWidget *resplace_switcher;
    GtkWidget *resplace_stack;
    GtkWidget *resplace_preview;
    GtkWidget *resplace_headers;
    GtkWidget *resplace_cookies;
    GtkWidget *resplace_size;
    GtkWidget *resplace_time;
    GtkWidget *resplace_status;
    GtkWidget *reqplace_body_types;
};

G_DEFINE_TYPE(MainWindow, main_window, GTK_TYPE_APPLICATION_WINDOW);

static void
main_window_init(MainWindow *window)
{
    gtk_widget_init_template(GTK_WIDGET(window));
}

static void
main_window_class_init(MainWindowClass *class)
{
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class), "/org/postgtk/window.ui");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, reqlist);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, reqplace);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, reqplace_body);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, reqplace_methods);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, reqplace_headers);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, reqplace_url);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, reqplace_submit);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, resplace_switcher);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, resplace_stack);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, resplace_preview);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, resplace_headers);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, resplace_cookies);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, resplace_size);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, resplace_time);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, resplace_status);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), MainWindow, reqplace_body_types);
}

MainWindow
*main_window_new(MainApp *app)
{
    return g_object_new(MAIN_WINDOW_TYPE, "application", app, NULL);
}

GtkTextView
*ui_add_reqplace_textview(GtkBox *box, bool editable, gchar *content)
{
    GtkWidget *scroller, *textview;
    GtkTextBuffer *buffer;

    scroller = gtk_scrolled_window_new();
    textview = gtk_text_view_new();

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, content, -1);

    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(textview), 8);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(textview), 8);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(textview), 8);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(textview), 8);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(textview), TRUE);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), editable);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scroller), textview);

    gtk_widget_set_hexpand(scroller, TRUE);
    gtk_widget_set_vexpand(scroller, TRUE);

    gtk_box_append(box, scroller);

    return GTK_TEXT_VIEW(textview);
}

typedef struct RequestForm {
    GtkBox *reqplace;
    GtkDropDown *method;
    GtkEntry *url;
    GtkTextView *body;
    GtkTextView *headers;
    GtkWidget *res_switcher;
    GtkWidget *res_stack;
    GtkTextView *res_preview;
    GtkTextView *res_headers;
    GtkTextView *res_cookies;
    GtkLabel *res_size;
    GtkLabel *res_time;
    GtkLabel *res_status;
    GtkDropDown *body_type;
} RequestForm;

void
send_request(GtkWidget *widget, RequestForm *form)
{
    Request req;

    uint method = (uint) gtk_drop_down_get_selected(form->method);
    uint body_type = (uint) gtk_drop_down_get_selected(form->body_type);
    char *url = (char*) gtk_entry_buffer_get_text(gtk_entry_get_buffer(form->url));

    GtkTextBuffer *body_buf, *header_buf;
    GtkTextIter bl, br, hl, hr;

    body_buf = gtk_text_view_get_buffer(form->body);
    header_buf = gtk_text_view_get_buffer(form->headers);

    gtk_text_buffer_get_bounds(body_buf, &bl, &br);
    gtk_text_buffer_get_bounds(header_buf, &hl, &hr);

    char *body = gtk_text_buffer_get_text(body_buf, &bl, &br, FALSE);
    char *headers = gtk_text_buffer_get_text(header_buf, &hl, &hr, FALSE);

    req.method = method;
    req.url = url;
    req.body = body;
    req.headers = headers;
    req.body_type = body_type;

    Response res = perform_request(req);

    gtk_text_buffer_set_text(gtk_text_view_get_buffer(form->res_preview), res.preview, -1);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(form->res_headers), res.headers, -1);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(form->res_cookies), res.cookies, -1);

    gtk_label_set_text(form->res_size, g_strdup_printf("%ldB", res.size));
    gtk_label_set_text(form->res_time, g_strdup_printf("%.2fms", res.time * 1000));
    gtk_label_set_text(form->res_status, g_strdup_printf("Code %ld", res.status));
}

void
main_window_open(MainWindow *window, GFile *file)
{
    GtkTextView
    *body,
    *headers,
    *res_preview,
    *res_headers,
    *res_cookies;

    body = ui_add_reqplace_textview(GTK_BOX(window->reqplace_body), TRUE, "{}");
    headers = ui_add_reqplace_textview(GTK_BOX(window->reqplace_headers), TRUE, "{}");
    res_preview = ui_add_reqplace_textview(GTK_BOX(window->resplace_preview), FALSE, "No Preview");
    res_headers = ui_add_reqplace_textview(GTK_BOX(window->resplace_headers), FALSE, "No Headers");
    res_cookies = ui_add_reqplace_textview(GTK_BOX(window->resplace_cookies), FALSE, "No Cookies");

    GtkLabel *res_size, *res_time, *res_status;

    res_size = GTK_LABEL(window->resplace_size);
    res_time = GTK_LABEL(window->resplace_time);
    res_status = GTK_LABEL(window->resplace_status);

    RequestForm *form = g_new(RequestForm, 1);

    form->reqplace = GTK_BOX(window->reqplace);
    form->method = GTK_DROP_DOWN(window->reqplace_methods);
    form->body_type = GTK_DROP_DOWN(window->reqplace_body_types);
    form->url = GTK_ENTRY(window->reqplace_url);
    form->res_stack = window->resplace_stack;
    form->res_switcher = window->resplace_switcher;
    form->body = body;
    form->headers = headers;
    form->res_preview = res_preview;
    form->res_headers = res_headers;
    form->res_cookies = res_cookies;
    form->res_size = res_size;
    form->res_time = res_time;
    form->res_status = res_status;

    g_signal_connect(
        GTK_BUTTON(window->reqplace_submit), "clicked",
        G_CALLBACK(send_request), form);
}