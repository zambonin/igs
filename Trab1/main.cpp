#include "structures.hpp"

#include <map>
#include <sstream>
#include <math.h>
#define PI 3.14159265
static cairo_surface_t *surface = nullptr;

GtkWidget *drawing_area, *window_widget, *combo;
GtkListStore *glist;
GtkBuilder *builder;
GtkTreeIter iter;
GtkTreePath *path;
char* selectedObj;

std::map<std::string, drawable> objects;
window w;

std::list<coord> split(const char* input) {

    std::list<coord> c;
    std::list<double> tmp;

    std::istringstream iss1(input);
    std::string s1, s2;

    while(getline(iss1, s1, ' ')) {
        std::istringstream iss2(s1);
        while(getline(iss2, s2, ';')) {
            try {
                tmp.push_back(std::stod(s2));
            } catch (const std::invalid_argument& ia) {}
        }
        c.emplace_back(coord(tmp.front(), tmp.back()));
        if (tmp.size() != 2) {
            c.clear();
            return c;
        }
        tmp.clear();
    }

    return c;

}

static void clear_surface() {

    cairo_t *cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 1, 1, 1);
    cairo_paint(cr);
    cairo_destroy(cr);

}

static gboolean configure_event_cb(GtkWidget *widget,
        GdkEventConfigure *event, gpointer data) {

    if (surface != nullptr) {
        cairo_surface_destroy(surface);
    }

    surface = gdk_window_create_similar_surface(gtk_widget_get_window(widget),
            CAIRO_CONTENT_COLOR, gtk_widget_get_allocated_width(widget),
            gtk_widget_get_allocated_height(widget));
    clear_surface();

    return TRUE;

}

static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, gpointer data) {

    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);

    return FALSE;

}

void update() {

    clear_surface();
    cairo_t *cr = cairo_create(surface);
    for (auto& obj : objects) {
        obj.second.viewport(drawing_area, w);
        obj.second.draw(cr);
    }
    gtk_widget_queue_draw(window_widget);
    cairo_destroy(cr);

}

extern "C" G_MODULE_EXPORT void change_selection() {
    gtk_combo_box_get_active_iter (GTK_COMBO_BOX(combo), &iter);
    // path = gtk_tree_path_new_from_string ("");
    // gtk_tree_model_get_iter(GTK_TREE_MODEL (glist), &iter, path);
    char* value;
    gtk_tree_model_get(GTK_TREE_MODEL(glist), &iter, 0, &value, -1);
    selectedObj = value;
    update();
    // std::cout << value << std::endl;

}

extern "C" G_MODULE_EXPORT void btn_trans_clk(GtkWidget *widget, GtkWidget *entry) {
    GtkEntry *transferVect = GTK_ENTRY(gtk_builder_get_object(builder, "transferVector"));
    std::list<coord> c = split(gtk_entry_get_text(transferVect));

    //TODO Create matrix method for creation of base.
    matrix<double> base(3,3);
    base(0,0) = 1;
    base(0,1) = 0;
    base(0,2) = 0;
    base(1,0) = 0;
    base(1,1) = 1;
    base(1,2) = 0;

    auto it = std::begin(c);
    base(2,0) = (*it).x;
    base(2,1) = (*it).y;
    base(2,2) = 1;

    std::cout << "Name: " << selectedObj << objects.find(selectedObj)->first << std::endl;
    drawable *obj = &(objects.find(selectedObj)->second);

    obj->transform(base);
    update();
}

extern "C" G_MODULE_EXPORT void btn_scale_clk(GtkWidget *widget, GtkWidget *entry) {
    GtkEntry *scaleVect = GTK_ENTRY(gtk_builder_get_object(builder, "scaleFactor"));
    std::list<coord> c = split(gtk_entry_get_text(scaleVect));


    //TODO Create matrix method for creation of base.
    matrix<double> baseScale(3,3);
    baseScale(0,1) = 0;
    baseScale(0,2) = 0;
    baseScale(1,0) = 0;
    baseScale(1,2) = 0;
    baseScale(2,0) = 0;
    baseScale(2,1) = 0;
    auto it = c.begin();
    baseScale(0,0) = (*it).x;
    std::cout << (*it).x << " ";
    baseScale(1,1) = (*it).y;
    std::cout << (*it).y << std::endl;
    baseScale(2,2) = 1;
    //TODO Get center of object, execute translation for center and back.
    drawable *obj = &(objects.find(selectedObj)->second);
    matrix<double> baseTrans(3,3);
    baseTrans(0,0) = 1;
    baseTrans(0,1) = 0;
    baseTrans(0,2) = 0;
    baseTrans(1,0) = 0;
    baseTrans(1,1) = 1;
    baseTrans(1,2) = 0;
    baseTrans(2,0) = obj->centerX;
    baseTrans(2,1) = obj->centerY;
    baseTrans(2,2) = 1;

    matrix<double> baseTransOrig(3,3);
    baseTransOrig(0,0) = 1;
    baseTransOrig(0,1) = 0;
    baseTransOrig(0,2) = 0;
    baseTransOrig(1,0) = 0;
    baseTransOrig(1,1) = 1;
    baseTransOrig(1,2) = 0;
    baseTransOrig(2,0) = -1*obj->centerX;
    baseTransOrig(2,1) = -1*obj->centerY;
    baseTransOrig(2,2) = 1;
    //TODO Get obj from list of objects with selectedObjValue;
    baseScale.display();
    baseTrans.display();
    baseTransOrig.display();
    obj->transform(baseTransOrig*baseScale*baseTrans);
    update();
}

extern "C" G_MODULE_EXPORT void btn_rotate_clk(GtkWidget *widget, GtkWidget *entry) {
    GtkEntry *rotateVect = GTK_ENTRY(gtk_builder_get_object(builder, "rotationDegree"));
    const double degree = (PI*(std::atof(gtk_entry_get_text(rotateVect))))/180;
    std::cout << degree << std::endl;
    matrix<double> baseRot(3,3);
    baseRot(0,0) = cos(degree);
    baseRot(0,1) = -sin(degree);
    baseRot(0,2) = 0;
    baseRot(1,0) = sin(degree);
    baseRot(1,1) = cos(degree);
    baseRot(1,2) = 0;
    baseRot(2,0) = 0;
    baseRot(2,1) = 0;
    baseRot(2,2) = 1;

    drawable *obj = &(objects.find(selectedObj)->second);

    matrix<double> baseTrans(3,3);
    baseTrans(0,0) = 1;
    baseTrans(0,1) = 0;
    baseTrans(0,2) = 0;
    baseTrans(1,0) = 0;
    baseTrans(1,1) = 1;
    baseTrans(1,2) = 0;
    baseTrans(2,0) = obj->centerX;
    baseTrans(2,1) = obj->centerY;
    baseTrans(2,2) = 1;

    matrix<double> baseTransOrig(3,3);
    baseTransOrig(0,0) = 1;
    baseTransOrig(0,1) = 0;
    baseTransOrig(0,2) = 0;
    baseTransOrig(1,0) = 0;
    baseTransOrig(1,1) = 1;
    baseTransOrig(1,2) = 0;
    baseTransOrig(2,0) = -1*obj->centerX;
    baseTransOrig(2,1) = -1*obj->centerY;
    baseTransOrig(2,2) = 1;
    // baseRot.display();
    // baseTrans.display();
    // baseTransOrig.display();
    obj->transform(baseTransOrig*baseRot*baseTrans);
    update();

    //TODO Create matrix method for creation of base.
    //TODO Get center of object and world. Rotate based on that point.
    //TODO Rotate based on any point.
    //TODO Get obj from list of objects with selectedObjValue;
    //transform();
    // update();
}

extern "C" G_MODULE_EXPORT void btn_draw_figure_clk() {

    GtkEntry *name = GTK_ENTRY(gtk_builder_get_object(builder, "name")),
             *coor = GTK_ENTRY(gtk_builder_get_object(builder, "coord"));

    std::string name_entry = std::string(gtk_entry_get_text(name));

    gtk_list_store_append(glist, &iter);
    gtk_list_store_set(glist, &iter, 0, gtk_entry_get_text(name), -1);

    const char *coord_entry = gtk_entry_get_text(coor);
    std::list<coord> c = split(coord_entry);

    if (!name_entry.empty() && !c.empty()) {
        std::string s(name_entry);
        objects.insert(std::pair<std::string, drawable>(s, drawable(s, c)));
        update();
    }

    gtk_entry_set_text(name, "");
    gtk_entry_set_text(coor, "");


}

extern "C" G_MODULE_EXPORT void btn_pan_up_clk(
        GtkWidget *widget, GtkWidget *entry) {

    const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
    w.set_limits(0, 0, rate, rate);
    update();

}

extern "C" G_MODULE_EXPORT void btn_pan_left_clk(
        GtkWidget *widget, GtkWidget *entry) {

    const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
    w.set_limits(-1 * rate, -1 * rate, 0, 0);
    update();

}

extern "C" G_MODULE_EXPORT void btn_pan_right_clk(
        GtkWidget *widget, GtkWidget *entry) {

    const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
    w.set_limits(rate, rate, 0, 0);
    update();

}

extern "C" G_MODULE_EXPORT void btn_pan_down_clk(
        GtkWidget *widget, GtkWidget *entry) {

    const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
    w.set_limits(0, 0, -1 * rate, -1 * rate);
    update();

}

extern "C" G_MODULE_EXPORT void btn_zoom_out_clk(
        GtkWidget *widget, GtkWidget *entry) {

    const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
    w.zoom(1 + rate);
    update();

}

extern "C" G_MODULE_EXPORT void btn_zoom_in_clk(
        GtkWidget *widget, GtkWidget *entry) {

    const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
    w.zoom(1 - rate);
    update();

}

extern "C" G_MODULE_EXPORT void btn_exit_clk() {
    gtk_main_quit();
}

extern "C" G_MODULE_EXPORT void btn_clear_clk() {
    clear_surface();
    objects.clear();
    w.reset();
    gtk_widget_queue_draw(window_widget);
    // gtk_list_store_clear(glist);
}

extern "C" G_MODULE_EXPORT void btn_center_clk() {
    w.reset();
    update();
}

int main(int argc, char *argv[]) {

    gtk_init(&argc, &argv);

    builder = gtk_builder_new_from_file("src/window.glade");
    window_widget = GTK_WIDGET(gtk_builder_get_object(builder, "main_window"));
    drawing_area = GTK_WIDGET(gtk_builder_get_object(builder, "drawing_area"));

    glist = GTK_LIST_STORE(gtk_builder_get_object(builder, "liststore1"));
    combo = GTK_WIDGET(gtk_builder_get_object(builder, "comboBox"));

    GType types = G_TYPE_STRING;

    gtk_list_store_set_column_types(glist, 1, &types);

    // gtk_list_store_append(glist, &iter);
    // gtk_list_store_set(glist, &iter, 0, "foo", -1);

    // gtk_list_store_append(glist, &iter);
    // gtk_list_store_set(glist, &iter, 0, "test", -1);


    // gtk_list_store_append(glist, &iter);
    // gtk_list_store_set(glist, &iter, 0, "test1", -1);


    gtk_combo_box_set_model (GTK_COMBO_BOX (combo), GTK_TREE_MODEL(glist));

    GtkCellRenderer *cell = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( combo ), cell, TRUE );
    gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( combo ), cell, "text", 0, NULL );

    g_signal_connect(drawing_area, "draw",
            G_CALLBACK(draw_cb), nullptr);
    g_signal_connect(drawing_area, "configure-event",
            G_CALLBACK(configure_event_cb), nullptr);

    gtk_builder_connect_signals(builder, nullptr);
    gtk_widget_show_all(window_widget);
    gtk_main();

    return 0;

}
