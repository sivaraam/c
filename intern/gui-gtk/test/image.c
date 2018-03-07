/**
 * A simple test application that creates an
 * window that display an image.
 */

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>


static
void activate(GtkApplication *app,
              gpointer user_data)
{
	GtkWidget *window;
	GtkWidget *grid;
	GtkWidget *image;
	GtkWidget *image_1;
	GdkPixbuf *image_buf;
	GdkPixbuf *image_buf_copy;
	GError *image_buf_load_error = NULL;
	static const char *image_path = "../../files/maze/test_inputs.bak/BMP1.bmp";

	/* Create the window */
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Image window");
	gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

	/* Create the grid to hold multiple images and attach it to the window */
	grid = gtk_grid_new();
	gtk_container_add (GTK_CONTAINER (window), grid);

	/* Create the GdkPixbuf object for the images that are to be loaded */
	image_buf = gdk_pixbuf_new_from_file (image_path, &image_buf_load_error);

	g_assert ((image_buf == NULL && image_buf_load_error != NULL) ||
	          (image_buf != NULL && image_buf_load_error == NULL));

	/* The copy is not necessary. This is just to try reference handling. */
	image_buf_copy = g_object_ref (image_buf);

	/* Attach images to the grid if successful */
	if (image_buf_load_error == NULL)
	{
		g_assert (image_buf != NULL);

		image = gtk_image_new_from_pixbuf (image_buf);
		image_1 = gtk_image_new_from_pixbuf (image_buf_copy);

		gtk_grid_attach (GTK_GRID (grid), image, 0, 0, 1, 1);
		gtk_grid_attach (GTK_GRID (grid), image_1, 1, 0, 1, 1);

		g_assert (image_1 == gtk_grid_get_child_at (GTK_GRID (grid), 1, 0));
	}
	else
	{
		// Report error to user, and free error
		g_assert (image_buf == NULL);

		g_print("Loading image failed with error : %s\n", image_buf_load_error->message);
		g_error_free (image_buf_load_error);
	}

	g_object_unref (image_buf_copy);
	gtk_widget_show_all (window);
}

int main(int argc, char *argv[])
{
	GtkApplication *app;
	int status;

	app = gtk_application_new ("com.ks.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}
