#include <gtk/gtk.h>
#include <iostream>
#include <vector>
#include <assert.h>
static cairo_surface_t *surface = NULL;
GtkWidget *drawing_area;
GtkWidget *window_widget;
GtkWidget *window2_widget;
GtkWidget *viewport;
GtkWidget *txtCoordX;
GtkWidget *txtCoordY;
GtkWidget *w2ok;
/*Clear the surface, removing the scribbles*/
static void clear_surface (){
    cairo_t *cr;

    cr = cairo_create (surface);

    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_paint (cr);

    cairo_destroy (cr);
}

/*Creates the surface*/
static gboolean configure_event_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer data){
    if (surface)
        cairo_surface_destroy (surface);

    surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
            CAIRO_CONTENT_COLOR,
            gtk_widget_get_allocated_width (widget),
            gtk_widget_get_allocated_height (widget));
    clear_surface ();
    return TRUE;
}

/* Redraw the screen from the surface */
static gboolean draw_cb (GtkWidget *widget, cairo_t   *cr,  gpointer   data){
    cairo_set_source_surface (cr, surface, 0, 0);
    cairo_paint (cr);

    return FALSE;
}

/*Function that will be called when the ok button is pressed*/
extern "C" G_MODULE_EXPORT void btn_ok_clicked_cb(){
    cairo_t *cr;
    cr = cairo_create (surface);
    cairo_move_to(cr, 200, 100);
    cairo_line_to(cr, 300, 50);
    cairo_stroke(cr);
    gtk_widget_queue_draw (window_widget);
}

extern "C" G_MODULE_EXPORT void btn_addFigure_clk(){
    GtkBuilder *gtkBuilderAux;
    gtkBuilderAux = gtk_builder_new();
    gtk_builder_add_from_file(gtkBuilderAux, "windowTrab.glade", NULL);


    window2_widget = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilderAux), "windowAddFigure") );
    gtk_builder_connect_signals(gtkBuilderAux, NULL);
    gtk_widget_show(window2_widget);
}

extern "C" G_MODULE_EXPORT void btn_w2cancel_clk(){
    gtk_widget_destroy(GTK_WIDGET(window2_widget));
    // GtkBuilder *gtkBuilder;
    // window2_widget = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "windowAddFigure") );

    // gtk_widget_show_all(window2_widget);
}

extern "C" G_MODULE_EXPORT void btn_w2ok_clk(GtkWidget *widget, GtkWidget **entry){
    GtkEntry *entry_ptr_x = (GtkEntry*)entry[0];
    GtkEntry *entry_ptr_y = (GtkEntry*)entry[1];

    const char *x, *y;


    x = (char*)gtk_entry_get_text(entry_ptr_x);
    y = (char*)gtk_entry_get_text(entry_ptr_y);
    float coordx = atof(x);
    float coordy = atof(y);
    g_debug("%.2f, %.2f", coordx, coordy);
    int i;
    for (i = 0; i < 2; i++)
        free(entry[i]);

    free(entry);
}

extern "C" G_MODULE_EXPORT void btn_w2addCoord_clk(){

}

extern "C" G_MODULE_EXPORT void btn_left_clk(){
}

extern "C" G_MODULE_EXPORT void btn_zoomOut_clk(){

}

extern "C" G_MODULE_EXPORT void btn_zoomIn_clk(){

}

extern "C" G_MODULE_EXPORT void btn_up_clk(){

}

extern "C" G_MODULE_EXPORT void btn_exit_clk(){
    gtk_widget_destroy(GTK_WIDGET(window_widget));

}

int main(int argc, char *argv[]){
    GtkBuilder  *gtkBuilder;
    gtk_init(&argc, &argv);

    gtkBuilder = gtk_builder_new();
    gtk_builder_add_from_file(gtkBuilder, "windowTrab.glade", NULL);

    window_widget = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "main_window") );
    viewport = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "viewport1") );
    drawing_area = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "drawing_area") );
    w2ok = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "button1") );
    txtCoordX = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "entry1") );
    txtCoordY = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "entry2") );
    // txtCoordX = gtk_entry_new();
    // txtCoordY = gtk_entry_new();
    GtkWidget *entry[2];
    entry[0] = txtCoordX;
    entry[1] = txtCoordY;
    printf("Hello");
    g_signal_connect (drawing_area, "draw", G_CALLBACK (draw_cb), NULL);
    g_signal_connect (drawing_area,"configure-event", G_CALLBACK (configure_event_cb), NULL);
    g_signal_connect(w2ok,"clicked",G_CALLBACK(btn_w2ok_clk), entry);
    gtk_builder_connect_signals(gtkBuilder, NULL);
    gtk_widget_show_all(window_widget);
    gtk_main ();
    return 0;
}


