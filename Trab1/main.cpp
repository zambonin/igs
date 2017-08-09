#include <gtk/gtk.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <assert.h>
#include <math.h>
#include "structures.hpp"
static cairo_surface_t *surface = NULL;
GtkWidget *drawing_area;
GtkWidget *window_widget;
GtkWidget *window2_widget;
GtkWidget *viewport;
GtkWidget *w2ok;

std::list<drawable> master_list;
window w;

std::list<coord> split(const char* input) {

  std::list<coord> c;
  std::list<double> tmp;

  std::istringstream iss1(input);
  std::string s1, s2;

  while(getline(iss1, s1, ' ')) {
    std::istringstream iss2(s1);
    while(getline(iss2, s2, ';')) {
      tmp.push_back(std::stod(s2));
    }
    // TODO funciona com um elemento só
    c.push_back(coord(tmp.front(), tmp.back()));
    tmp.clear();
  }

  return c;

}

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

extern "C" G_MODULE_EXPORT void btn_add_figure_clk() {
  // TODO can't click twice
    GtkBuilder *gtkBuilderAux;
    gtkBuilderAux = gtk_builder_new();
    gtk_builder_add_from_file(gtkBuilderAux, "windowTrab.glade", NULL);

    window2_widget = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilderAux), "windowAddFigure") );
    gtk_builder_connect_signals(gtkBuilderAux, NULL);
    gtk_widget_show(window2_widget);
}

extern "C" G_MODULE_EXPORT void btn_w2cancel_clk() {
    gtk_widget_destroy(GTK_WIDGET(window2_widget));
}


coord viewport_tr(coord& c) {
  double xvp = ((c.x - w.xmin) / (w.xmax - w.xmin)) * (200 - -200);
  double yvp = (1 - ((c.y - w.ymin) / (w.ymax - w.ymin))) * (200 - -200);
  return coord(xvp, yvp);
}

void update_roleplay()  {
  clear_surface();
  cairo_t *cr = cairo_create(surface);
  cairo_set_line_width(cr, 2);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  for (auto& i : master_list) {

    std::list<coord> tmp;
    for (auto& j : i.clist) {
      tmp.push_back(viewport_tr(j));
    }
    drawable d(std::string("copy"), cr, tmp);
    d.draw();
    tmp.clear();
    gtk_widget_queue_draw(window_widget);
  }

}
extern "C" G_MODULE_EXPORT void btn_draw_figure_clk(GtkWidget *widget, GtkWidget *entry) {
  std::list<coord> c = split(gtk_entry_get_text(GTK_ENTRY(entry)));
    cairo_t *cr = cairo_create(surface);
  cairo_set_line_width(cr, 2);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  // TODO add entry for name
  drawable p(std::string("placeholder"), cr, c);

  master_list.push_back(p);

  update_roleplay();

  gtk_widget_destroy(GTK_WIDGET(window2_widget));
}

extern "C" G_MODULE_EXPORT void btn_pan_up_clk() {
  w.set_limits(0, 0, 10, 10);
  update_roleplay();
}

extern "C" G_MODULE_EXPORT void btn_pan_left_clk() {
  w.set_limits(-10, -10, 0, 0);
  update_roleplay();
}

extern "C" G_MODULE_EXPORT void btn_pan_right_clk() {
  w.set_limits(10, 10, 0, 0);
  update_roleplay();
}

extern "C" G_MODULE_EXPORT void btn_pan_down_clk() {
  w.set_limits(0, 0, -10, -10);
  update_roleplay();
}

extern "C" G_MODULE_EXPORT void btn_zoom_out_clk() {
  w.zoom(1.2, 1.2, 1.2, 1.2);
  update_roleplay();
}

extern "C" G_MODULE_EXPORT void btn_zoom_in_clk() {
  w.zoom(0.8, 0.8, 0.8, 0.8);
  update_roleplay();
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

    gtkBuilder = gtk_builder_new_from_file("windowTrab.glade");

    window_widget = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "main_window") );
    viewport = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "viewport1") );
    drawing_area = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "drawing_area") );
    w2ok = GTK_WIDGET( gtk_builder_get_object( GTK_BUILDER(gtkBuilder), "button4") );
    g_signal_connect (drawing_area, "draw", G_CALLBACK (draw_cb), NULL);
    g_signal_connect (drawing_area,"configure-event", G_CALLBACK (configure_event_cb), NULL);
    gtk_builder_connect_signals(gtkBuilder, NULL);
    gtk_widget_show_all(window_widget);
    gtk_main ();
    return 0;
}


