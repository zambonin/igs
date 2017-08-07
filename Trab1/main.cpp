#include <gtk/gtk.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <assert.h>
#include <math.h>

static cairo_surface_t *surface = NULL;
GtkWidget *drawing_area;
GtkWidget *window_widget;
GtkWidget *window2_widget;
GtkWidget *viewport;
GtkWidget *w2ok;

struct Entry {
    GtkWidget *arg1;
    GtkWidget *arg2;
    GtkWidget *arg3;
    GtkWidget *arg4;
};

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

extern "C" G_MODULE_EXPORT void btn_w2okDot_clk(){

}

extern "C" G_MODULE_EXPORT void btn_w2okLine_clk(GtkWidget *widget, GtkWidget *ud){

    std::vector<float> vect;
    const gchar *strx = gtk_entry_get_text(GTK_ENTRY(ud));
    //CALLFUNCTOPRINT
    std::stringstream ss;
    ss << strx;

    int i;

    while (ss >> i)
    {
        vect.push_back(i);

        if (ss.peek() == ';')
            ss.ignore();
    }
    cairo_t *cr;
    cr = cairo_create (surface);


    for (i=0; i< vect.size(); i++)
        std::cout << vect.at(i)<<std::endl;

    if (vect.size() == 2) {
        cairo_translate(cr, vect.at(0), vect.at(1));
        cairo_arc(cr, 0, 0, 1, 0, 2 * M_PI);
        cairo_fill(cr);
    }
    if (vect.size() == 4) {
        // cairo_move_to(cr, coordStrx, 150);
        cairo_move_to(cr, vect.at(0), vect.at(1));
        cairo_line_to(cr, vect.at(2), vect.at(3));
        cairo_stroke(cr);
    }
    gtk_widget_queue_draw (window_widget);


    gtk_widget_destroy(GTK_WIDGET(window2_widget));
}

extern "C" G_MODULE_EXPORT void btn_w2okPolygon_clk(){

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
    gtk_main_quit();

}

int main(int argc, char *argv[]){
    GtkBuilder  *gtkBuilder;
    gtk_init(&argc, &argv);

    GtkWidget *txtStrCoordX;
    GtkWidget *txtStrCoordY;
    GtkWidget *txtEndCoordX;
    GtkWidget *txtEndCoordY;

    gtkBuilder = gtk_builder_new();
    gtk_builder_add_from_file(gtkBuilder, "windowTrab.glade", NULL);
    txtStrCoordX = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "entry3") );

    // float n = 2;
    // GtkWidget *entry = txtStrCoordX;
    // Entry *entry;
    // entry->arg1=txtStrCoordX;
    // entry[0] = txtStrCoordX;
    // entry[1] = txtStrCoordY;
    // entry[2] = txtEndCoordX;
    // entry[3] = txtEndCoordY;
    // entry[4] = NULL;
    window_widget = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "main_window") );
    viewport = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "viewport1") );
    drawing_area = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "drawing_area") );
    w2ok = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "button4") );
    g_signal_connect (drawing_area, "draw", G_CALLBACK (draw_cb), NULL);
    g_signal_connect (drawing_area,"configure-event", G_CALLBACK (configure_event_cb), NULL);
    // g_signal_connect(w2ok,"clicked",G_CALLBACK(btn_w2okLine_clk), &n);
    gtk_builder_connect_signals(gtkBuilder, NULL);
    gtk_widget_show_all(window_widget);
    gtk_main ();
    return 0;
}


