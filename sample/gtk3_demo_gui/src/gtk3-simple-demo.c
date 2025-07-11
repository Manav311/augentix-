#include <gtk/gtk.h>

GObject *button;

G_MODULE_EXPORT void on_window_main_button_press_event(GtkWidget *widget, GdkEventButton *event, GdkWindowEdge edge)
{
	printf("[%d] right mouse button\r\n", __LINE__);

	if (event->type == GDK_BUTTON_PRESS) {
		if (event->button == 1) {
			gtk_window_begin_move_drag(GTK_WINDOW(gtk_widget_get_toplevel(widget)), event->button,
			                           event->x_root, event->y_root, event->time);
		}
	}
}

G_MODULE_EXPORT void on_bt0_clicked()
{
	printf("[%d]press gui button\r\n", __LINE__);
	static int cnt = 0;
	cnt++;
	char label_str[32];
	snprintf(&label_str[0], 32, "press: %d", cnt);
	gtk_button_set_label(GTK_BUTTON(button), label_str);
}

G_MODULE_EXPORT void on_window_main_destroy()
{
	printf("End\r\n");
	gtk_main_quit();
}

static void activate(GtkApplication *app, gpointer data)
{
	GtkBuilder *builder;
	GtkWindow *window;

	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "/usrdata/example.ui", NULL);
	gtk_builder_add_callback_symbol(builder, "on_window_main_destroy", (GCallback)on_window_main_destroy);
	gtk_builder_add_callback_symbol(builder, "on_window_main_button_press_event",
	                                (GCallback)on_window_main_button_press_event);
	gtk_builder_add_callback_symbol(builder, "on_bt0_clicked", (GCallback)on_bt0_clicked);

	window = (GtkWindow *)gtk_builder_get_object(builder, "window_main");
	gtk_application_add_window(GTK_APPLICATION(app), window);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

	button = gtk_builder_get_object(builder, "bt0");

	gtk_builder_connect_signals(builder, NULL);

	gtk_widget_show_all(GTK_WIDGET(window));

	g_object_unref(builder);
}

int main(int argc, char **argv)
{
	GtkApplication *app;
	int app_status;

	app = gtk_application_new("org.main.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
	app_status = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);
	return app_status;
}
