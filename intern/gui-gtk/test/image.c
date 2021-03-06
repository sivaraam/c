/**
 * A simple test application that creates an
 * window that displays images and listens to
 * certain evnets realted to images.
 */

#include <math.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

/**
 * Grid cell constants
 */
static gint grid_cell_side = 106;
static gint total_cells = 2;
static gint grid_cells_horizontal = 1;
static gint grid_cells_vertical = 1;

static
gboolean motion_notify_event_cb (GtkWidget *widget,
                                 GdkEventMotion *event,
                                 GtkWidget *window)
{
	gint grid_x = 0, grid_y = 0;
	int round_x = round (event->x), round_y = round (event->y);

	g_print ("Got x: %d, y: %d\n", round_x, round_y);

	gtk_widget_translate_coordinates (GTK_WIDGET(widget), window,
	                                  round (event->x), round (event->y), &grid_x, &grid_y);

	g_print ("grid_event_box: translated coordiantes: x: %d, y: %d\n", grid_x, grid_y);

	return FALSE;
}

/**
 * Given the grid window relative (x, y) coordinates,
 * finds the corresonding (left, top) values which can
 * be used to index the widget found at (x, y) in the grid.
 *
 * Assumes that each widget are squares and have a constant
 * 'length' in the sides which is pre-determined.
 */
static
void get_grid_left_top(gint x, gint y,
                       gint *left, gint *top)
{
	*left = x/grid_cell_side;
	*top = y/grid_cell_side;
}

static
void drag_begin_cb (GtkGestureDrag *drag,
                    gdouble offset_x,
                    gdouble offset_y,
                    gpointer user_data)
{
	gint drag_start_x = round (offset_x), drag_start_y = round (offset_y);

	g_print ("drag_begin_cb: Start point: Got        x: %lf\t y:%lf\n", offset_x, offset_y);
	g_print ("drag_begin_cb: Start point: Rounded to x: %d\ty: %d\n", drag_start_x, drag_start_y);
}

/**
 * Replace the image in the grid cell corresponding to the destination
 * with the image in the grid cell corresponding to the source.
 *
 * Ignore in case the grid cell is out of bounds.
 */
static
void drag_end_cb (GtkGestureDrag *drag,
                  gdouble offset_x,
                  gdouble offset_y,
                  GtkGrid *grid)
{
	gint drag_end_offset_x = round (offset_x), drag_end_offset_y = round (offset_y);
	gdouble d_drag_start_x = 0.0, d_drag_start_y = 0.0;

	g_print ("drag_end_cb: End point: Got        x: %lf\t y: %lf\n", offset_x, offset_y);
	g_print ("drag_end_cb: End point: Rounded to x: %d\t y: %d\n", drag_end_offset_x, drag_end_offset_y);

	if (gtk_gesture_drag_get_start_point (drag, &d_drag_start_x, &d_drag_start_y))
	{
		gint drag_end_x= 0, drag_end_y = 0,
		     drag_start_x = 0, drag_start_y = 0;

		/* Coordinates to manipulate images in the grid */
		gint source_left = 0, source_top = 0,
		     dest_left = 0, dest_top = 0;

		drag_start_x = round (d_drag_start_x), drag_start_y = round (d_drag_start_y);

		g_print ("drag_end_cb: Start point: Got        x: %lf\t y:%lf\n", d_drag_start_x, d_drag_start_y);
		g_print ("drag_end_cb: Start point: Rounded to x: %d\ty: %d\n", drag_start_x, drag_start_y);

		drag_end_x = drag_start_x + drag_end_offset_x;
		drag_end_y = drag_start_y + drag_end_offset_y;
		g_print ("drag_end_cb: End point: Actual x: %d, y: %d\n", drag_end_x,
		                                                          drag_end_y);

		get_grid_left_top (drag_start_x, drag_start_y, &source_left, &source_top);
		get_grid_left_top (drag_end_x, drag_end_y, &dest_left, &dest_top);

		g_print ("drag_end_cb: source_left: %d, source_top: %d\n",
		         source_left, source_top);
		g_print ("drag_end_cb: dest_left: %d, dest_top: %d\n",
		         dest_left, dest_top);

		/* Do nothing the source and dest are the same */
		if (source_left == dest_left &&
		    source_top == dest_top)
		{
			return;
		}

		/* Swap the image in the squares */
		/* Update only if (left, top) is within bounds */
		if (source_left < total_cells &&
		    source_top < total_cells &&
		    dest_top < total_cells &&
		    dest_left < total_cells)
		{
			GtkWidget *image_at_dest;
			GtkWidget *image_at_start;
			GdkPixbuf *image_buf_at_start;

			image_at_start = gtk_grid_get_child_at (grid, source_left, source_top);
			image_at_dest = gtk_grid_get_child_at (grid, dest_left, dest_top);

			g_assert (image_at_dest != NULL && image_at_start != NULL);

			image_buf_at_start = gtk_image_get_pixbuf(GTK_IMAGE (image_at_start));

			g_assert (image_buf_at_start != NULL);

			gtk_image_set_from_pixbuf (GTK_IMAGE (image_at_dest), image_buf_at_start);
		}
	}
	else
	{
		g_print ("drag_end_cb: Could not get start point!\n");
	}
}

static
void activate(GtkApplication *app,
              gpointer user_data)
{
	GtkWidget *window;
	GtkWidget *grid;
	GtkWidget *grid_event_box;
	GtkGesture *grid_drag_gesture;
	GtkWidget *wq_image;
	GtkWidget *bh_image;
	GdkPixbuf *wq_image_buf;
	GdkPixbuf *bh_image_buf;
	GError *wq_image_buf_load_error = NULL, *bh_image_buf_load_error = NULL;
	static const char *white_queen_path = "images/white_queen(106x106).bmp",
	                  *black_horse_path = "images/black_horse(106x106).bmp";

	/* Create the window */
	window = gtk_application_window_new (app);
	gtk_window_set_title (GTK_WINDOW (window), "Image window");
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_resizable (GTK_WINDOW (window), FALSE);

	/* Create the grid to hold multiple images */
	grid = gtk_grid_new();
	gtk_grid_set_row_homogeneous (GTK_GRID (grid), TRUE);
	gtk_grid_set_column_homogeneous (GTK_GRID (grid), TRUE);

	/* Create the GdkPixbuf object for the images that are to be loaded */
	wq_image_buf = gdk_pixbuf_new_from_file (white_queen_path, &wq_image_buf_load_error);
	bh_image_buf = gdk_pixbuf_new_from_file (black_horse_path, &bh_image_buf_load_error);

	g_assert ((wq_image_buf == NULL && wq_image_buf_load_error != NULL) ||
	          (wq_image_buf != NULL && wq_image_buf_load_error == NULL));

	g_assert ((bh_image_buf == NULL && bh_image_buf_load_error != NULL) ||
	          (bh_image_buf != NULL && bh_image_buf_load_error == NULL));

	/* Attach images to the grid if successful */
	if (wq_image_buf_load_error == NULL || bh_image_buf_load_error == NULL)
	{
		wq_image = gtk_image_new_from_pixbuf (wq_image_buf);
		bh_image = gtk_image_new_from_pixbuf (bh_image_buf);

		gtk_grid_attach (GTK_GRID (grid), wq_image, 0, 0, grid_cells_horizontal, grid_cells_vertical);
		gtk_grid_attach (GTK_GRID (grid), bh_image, 1, 0, grid_cells_horizontal, grid_cells_vertical);

		// ensure gtk_grid_get_child_at() works as intended
		g_assert (bh_image == gtk_grid_get_child_at (GTK_GRID (grid), 1, 0));

		/* Attach an event box to the grid to receive events */
		grid_event_box = gtk_event_box_new ();
		gtk_container_add (GTK_CONTAINER (grid_event_box), grid);

		/* Attach the event box to the window */
		gtk_container_add (GTK_CONTAINER (window), grid_event_box);

		/* Listen to the motion events over the grid to analyse what we get */
		g_signal_connect (grid_event_box, "motion-notify-event", G_CALLBACK (motion_notify_event_cb), window);

		/* Attach a drag gesture to the event box */
		grid_drag_gesture = gtk_gesture_drag_new (grid_event_box);
		g_signal_connect (grid_drag_gesture, "drag-begin", G_CALLBACK (drag_begin_cb), NULL);
		g_signal_connect (grid_drag_gesture, "drag-end", G_CALLBACK (drag_end_cb), GTK_GRID (grid));

		gtk_widget_set_events (grid_event_box, gtk_widget_get_events (grid_event_box)
	                                     | GDK_POINTER_MOTION_MASK);
	}
	else if (wq_image_buf_load_error != NULL)
	{
		g_print("Loading image failed with error : %s\n", wq_image_buf_load_error->message);
		g_error_free (wq_image_buf_load_error);
	}
	else
	{
		g_print("Loading image failed with error : %s\n", bh_image_buf_load_error->message);
		g_error_free (bh_image_buf_load_error);
	}

	// unref the pixbuf buffers
	g_object_unref (wq_image_buf);
	g_object_unref (bh_image_buf);

	gtk_widget_show_all (window);
}

int main(int argc, char *argv[])
{
	GtkApplication *app;
	int status = 0;

	app = gtk_application_new ("com.ks.example", G_APPLICATION_FLAGS_NONE);
	g_signal_connect (app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run (G_APPLICATION (app), argc, argv);
	g_object_unref (app);

	return status;
}
