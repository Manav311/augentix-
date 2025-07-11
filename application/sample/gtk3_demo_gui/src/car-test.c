

#include <gtk/gtk.h>
#include <stdio.h>
#include <errno.h>
#include <getopt.h>

#define DEBUG
#ifdef DEBUG
#define LOG(format, args...) fprintf(stderr, "[%s:%d]" format, __func__, __LINE__, ##args);
#else
#define LOG(format, args...)
#endif
#define ERR(format, args...) fprintf(stderr, "[%s:%d]" format, __func__, __LINE__, ##args);

#define PATH_LEN (64)
#define LICENSE_LEN (4)

GtkBuilder *builder;

static GtkWidget *main_stack;

static void switch_box1()
{
	gtk_stack_set_visible_child_name(GTK_STACK(main_stack), "box1");
	LOG("here\n");
}

int main(int argc, char *argv[])
{
	GtkWidget *main_win;

	gtk_init(NULL, NULL);

	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "/mnt/nfs/ethnfs/car-test.ui", NULL);
	gtk_builder_add_callback_symbol(builder, "on_bt0_clicked", G_CALLBACK(switch_box1));
	gtk_builder_add_callback_symbol(builder, "on_evt0_button_press_event", G_CALLBACK(switch_box1));
	gtk_builder_connect_signals(builder, NULL);

	main_win = (GtkWidget *)gtk_builder_get_object(builder, "main_win");
	main_stack = (GtkWidget *)gtk_builder_get_object(builder, "main_stack");

	gtk_window_present(GTK_WINDOW(main_win));

	gtk_main();

	return 0;
}