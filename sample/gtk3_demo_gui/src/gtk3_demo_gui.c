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
char g_src_path[PATH_LEN];

GtkBuilder *builder;
static GtkWidget *main_stack;
GtkImage *img_2_0, *img_2_1, *img_2_2, *img_2_3;
GtkWidget *main_win;

static void switch_to_page_1(GtkEventBox *page_0, GtkBox *page_1)
{
	gtk_stack_set_visible_child_name(GTK_STACK(main_stack), "page_1");

	return;
}

static void help()
{
	printf("[Usage]:\r\n"
	       "\t -s <src path>/number/ for number image .jpg\n"
	       "\t\t <src path>/Augentix_Logo_282x45.png\n"
	       "\t-h help()\n"
	       "please install font at $HOME/.font\n");
}

static void destroy_main_win(GtkEventBox *page_3, GtkWidget *win)
{
	gtk_window_close(GTK_WINDOW(main_win));
}

static void switch_to_page_3(GtkEventBox *page_2, GtkBox *page_3)
{
	gtk_stack_set_visible_child_name(GTK_STACK(main_stack), "page_3");
	return;
}

static void handle_summit(GtkWidget *button, GtkWidget *textview)
{
	GtkTextBuffer *buffer;
	GtkTextIter start, end, start_offset, end_offset;
	GtkImage *img_list[4] = { img_2_0, img_2_1, img_2_2, img_2_3 };
	char number_img_name[PATH_LEN];
	int char_num = -1;
	char *p_license = NULL;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);

	char_num = gtk_text_buffer_get_char_count(buffer);
	if (char_num > LICENSE_LEN) {
		ERR("Input more then %d: %d", LICENSE_LEN, char_num);
		return;
	} else if (char_num < LICENSE_LEN) {
		ERR("Input less then %d: %d", LICENSE_LEN, char_num);
		return;
	}

	LOG("summit : %s\n", gtk_text_buffer_get_text(buffer, &start, &end, FALSE));
	for (int i = 0; i < LICENSE_LEN; i++) {
		gtk_text_buffer_get_iter_at_offset(buffer, &start_offset, i);
		gtk_text_buffer_get_iter_at_offset(buffer, &end_offset, i + 1);
		p_license = gtk_text_buffer_get_text(buffer, &start_offset, &end_offset, FALSE);
		snprintf(&number_img_name[0], PATH_LEN, "%s/image/number/%s.jpg", &g_src_path[0], p_license);
		gtk_image_set_from_file(img_list[i], number_img_name);

		LOG("show %s\n", &number_img_name[0]);
	}
	/*switch to page 2*/
	gtk_stack_set_visible_child_name(GTK_STACK(main_stack), "page_2");
}

static void handle_clean(GtkWidget *button, GtkWidget *textview)
{
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	gtk_text_buffer_get_start_iter(buffer, &start);
	gtk_text_buffer_get_end_iter(buffer, &end);

	LOG("clean : %s\n", gtk_text_buffer_get_text(buffer, &start, &end, FALSE));

	gtk_text_buffer_delete(buffer, &start, &end);

	LOG("clean textview\n");
}

static void handle_insert(GtkWidget *button, GtkWidget *textview)
{
	GtkTextBuffer *buffer;
	const gchar *id;
	const gchar *text;
	int char_num = -1;

	id = gtk_buildable_get_name(GTK_BUILDABLE(button));

	if (strcmp(id, "bt_1_0") == 0) {
		text = "0";
	} else if (strcmp(id, "bt_1_1") == 0) {
		text = "1";
	} else if (strcmp(id, "bt_1_2") == 0) {
		text = "2";
	} else if (strcmp(id, "bt_1_3") == 0) {
		text = "3";
	} else if (strcmp(id, "bt_1_4") == 0) {
		text = "4";
	} else if (strcmp(id, "bt_1_5") == 0) {
		text = "5";
	} else if (strcmp(id, "bt_1_6") == 0) {
		text = "6";
	} else if (strcmp(id, "bt_1_7") == 0) {
		text = "7";
	} else if (strcmp(id, "bt_1_8") == 0) {
		text = "8";
	} else if (strcmp(id, "bt_1_9") == 0) {
		text = "9";
	} else {
		text = "";
	}

	LOG("insert text: %s\n", text);

	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
	char_num = gtk_text_buffer_get_char_count(buffer);
	if (char_num > 4) {
		ERR("Input more then 4: %d, please clean end re-edit\n", char_num);
		return;
	}

	gtk_text_buffer_insert_at_cursor(buffer, text, -1);

	return;
}

int main(int argc, char **argv)
{
	snprintf(&g_src_path[0], PATH_LEN, "/system/bin/gtk3_demo");

	char logo_path[PATH_LEN];
	char ui_name[PATH_LEN];
	GdkPixbuf *buf;

	GtkWidget *img_0_0;
	GError *err = NULL;

	int c = 0;
	while ((c = getopt(argc, argv, "hs:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 's':
			printf("src path : %s\r\n", argv[optind - 1]);
			snprintf(&g_src_path[0], PATH_LEN, "%s", argv[optind - 1]);
			break;
		}
	}

	if (access((const char *)&g_src_path[0], F_OK) == -1) {
		ERR("%s path not exist!\n", &g_src_path[0]);
		help();
		return -ENODATA;
	}

	gtk_init(NULL, NULL);

	snprintf(&ui_name[0], PATH_LEN, "%s/gtk3_demo_gui.ui", &g_src_path[0]);
	snprintf(&logo_path[0], PATH_LEN, "%s/image/Augentix_Logo_282x45.png", &g_src_path[0]);

	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, &ui_name[0], NULL);
	/*page 0 signals*/
	gtk_builder_add_callback_symbol(builder, "on_page_0_button_press_event", G_CALLBACK(switch_to_page_1));
	/*page 1 signals*/
	gtk_builder_add_callback_symbol(builder, "on_bt_1_0_clicked", G_CALLBACK(handle_insert));
	gtk_builder_add_callback_symbol(builder, "on_bt_1_1_clicked", G_CALLBACK(handle_insert));
	gtk_builder_add_callback_symbol(builder, "on_bt_1_2_clicked", G_CALLBACK(handle_insert));
	gtk_builder_add_callback_symbol(builder, "on_bt_1_3_clicked", G_CALLBACK(handle_insert));
	gtk_builder_add_callback_symbol(builder, "on_bt_1_4_clicked", G_CALLBACK(handle_insert));
	gtk_builder_add_callback_symbol(builder, "on_bt_1_5_clicked", G_CALLBACK(handle_insert));
	gtk_builder_add_callback_symbol(builder, "on_bt_1_6_clicked", G_CALLBACK(handle_insert));
	gtk_builder_add_callback_symbol(builder, "on_bt_1_7_clicked", G_CALLBACK(handle_insert));
	gtk_builder_add_callback_symbol(builder, "on_bt_1_8_clicked", G_CALLBACK(handle_insert));
	gtk_builder_add_callback_symbol(builder, "on_bt_1_9_clicked", G_CALLBACK(handle_insert));
	gtk_builder_add_callback_symbol(builder, "on_bt_1_clean_clicked", G_CALLBACK(handle_clean));
	gtk_builder_add_callback_symbol(builder, "on_bt_1_summit_clicked", G_CALLBACK(handle_summit));
	/*page 2 signals*/
	gtk_builder_add_callback_symbol(builder, "on_evt_2_0_button_press_event", G_CALLBACK(switch_to_page_3));
	/*page 3 signals*/
	gtk_builder_add_callback_symbol(builder, "on_page_3_button_press_event", G_CALLBACK(destroy_main_win));

	gtk_builder_connect_signals(builder, NULL);

	main_win = (GtkWidget *)gtk_builder_get_object(builder, "main_win");
	main_stack = (GtkWidget *)gtk_builder_get_object(builder, "main_stack");
	img_0_0 = (GtkWidget *)gtk_builder_get_object(builder, "img_0_0");
	buf = gdk_pixbuf_new_from_file(&logo_path[0], &err);
	gtk_image_set_from_pixbuf(GTK_IMAGE(img_0_0), buf);

	img_2_0 = (GtkImage *)gtk_builder_get_object(builder, "img_2_0");
	img_2_1 = (GtkImage *)gtk_builder_get_object(builder, "img_2_1");
	img_2_2 = (GtkImage *)gtk_builder_get_object(builder, "img_2_2");
	img_2_3 = (GtkImage *)gtk_builder_get_object(builder, "img_2_3");

	gtk_window_present(GTK_WINDOW(main_win));

	gtk_main();

	g_object_unref(builder);

	return 0;
}
