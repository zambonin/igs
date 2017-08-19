#ifndef HANDLER_HPP
#define HANDLER_HPP

#include "utils.hpp"

extern "C" G_MODULE_EXPORT void btn_draw_figure_clk() {
  GtkEntry *name = GTK_ENTRY(gtk_builder_get_object(builder, "name")),
           *coor = GTK_ENTRY(gtk_builder_get_object(builder, "coord"));
  GtkComboBoxText *combo =
      GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "combo"));

  std::string s(gtk_entry_get_text(name));
  std::list<coord> c = split(gtk_entry_get_text(coor));

  if (!s.empty() && !c.empty() && objects.count(s) == 0) {
    gtk_combo_box_text_append(combo, nullptr, s.c_str());
    objects.insert({s, drawable(s, c)});
    update();
  }

  gtk_entry_set_text(name, "");
  gtk_entry_set_text(coor, "");
}

extern "C" G_MODULE_EXPORT void btn_pan_up_clk(GtkWidget *widget,
                                               GtkWidget *entry) {
  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w.set_limits(0, 0, rate, rate);
  update();
}

extern "C" G_MODULE_EXPORT void btn_pan_left_clk(GtkWidget *widget,
                                                 GtkWidget *entry) {
  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w.set_limits(-1 * rate, -1 * rate, 0, 0);
  update();
}

extern "C" G_MODULE_EXPORT void btn_pan_right_clk(GtkWidget *widget,
                                                  GtkWidget *entry) {
  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w.set_limits(rate, rate, 0, 0);
  update();
}

extern "C" G_MODULE_EXPORT void btn_pan_down_clk(GtkWidget *widget,
                                                 GtkWidget *entry) {
  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w.set_limits(0, 0, -1 * rate, -1 * rate);
  update();
}

extern "C" G_MODULE_EXPORT void btn_zoom_out_clk(GtkWidget *widget,
                                                 GtkWidget *entry) {
  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w.zoom(1 + rate);
  update();
}

extern "C" G_MODULE_EXPORT void btn_zoom_in_clk(GtkWidget *widget,
                                                GtkWidget *entry) {
  const double rate = std::stod(gtk_entry_get_text(GTK_ENTRY(entry)));
  w.zoom(1 - rate);
  update();
}

extern "C" G_MODULE_EXPORT void btn_exit_clk() { gtk_main_quit(); }

extern "C" G_MODULE_EXPORT void btn_center_clk() {
  w.reset();
  update();
}

extern "C" G_MODULE_EXPORT void btn_clear_clk() {
  objects.clear();
  gtk_combo_box_text_remove_all(
      GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "combo")));
  btn_center_clk();
}

extern "C" G_MODULE_EXPORT void btn_trans_figure_clk(GtkWidget *widget,
                                                     GtkWidget *entry) {
  GtkComboBoxText *combo =
      GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "combo"));
  gchar *obj = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));

  GtkComboBoxText *ops =
      GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "operations"));
  int op_id = gtk_combo_box_get_active(GTK_COMBO_BOX(ops));

  GtkEntry *vector = GTK_ENTRY(gtk_builder_get_object(builder, "transfer"));
  std::list<coord> c = split(gtk_entry_get_text(vector));

  if (obj == nullptr || (c.empty() && op_id < 3)) {
    return;
  }

  const double angle =
      M_PI * std::stod(gtk_entry_get_text(GTK_ENTRY(entry))) / 180;

  drawable &d = objects.find(obj)->second;
  std::map<int, matrix> bases = {
      {0, m_transfer(c.front())},
      {1,
       m_transfer(-d.center()) * m_scale(c.front()) * m_transfer(d.center())},
      {2, m_transfer(-c.front()) * m_rotate(angle) * m_transfer(c.front())},
      {3, m_rotate(angle)},
      {4, m_transfer(-d.center()) * m_rotate(angle) * m_transfer(d.center())},
  };

  d.transform(bases.find(op_id)->second);
  update();
}

extern "C" G_MODULE_EXPORT void btn_delete_figure_clk() {
  GtkComboBoxText *combo =
      GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "combo"));
  gchar *obj = gtk_combo_box_text_get_active_text(combo);
  gint id = gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
  if (obj != nullptr) {
    gtk_combo_box_text_remove(combo, id);
    objects.erase(obj);
  }
  update();
}

#endif // HANDLER_HPP
