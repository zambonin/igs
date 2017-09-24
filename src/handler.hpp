#ifndef HANDLER_HPP
#define HANDLER_HPP

#include "utils.hpp"

extern "C" G_MODULE_EXPORT void btn_draw_figure_clk(GtkWidget *widget,
                                                    GtkWidget *entry) {
  GtkEntry *name = GTK_ENTRY(gtk_builder_get_object(builder, "name_obj_entry"));
  GtkEntry *coor =
      GTK_ENTRY(gtk_builder_get_object(builder, "coords_obj_entry"));
  GtkComboBoxText *combo =
      GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "object_list"));
  GtkToggleButton *cbtn =
      GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "curve_obj_check"));
  GtkToggleButton *sbtn =
      GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "spline_obj_radiobtn"));
  GtkToggleButton *lbtn =
      GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "lbclip_obj_radiobtn"));
  GtkRange *steps =
      GTK_RANGE(gtk_builder_get_object(builder, "curve_step_scale"));

  std::string s(gtk_entry_get_text(name));
  std::list<coord> c = read_coord(gtk_entry_get_text(coor));
  bool curve = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(cbtn));
  bool spline = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(sbtn));
  bool fill = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(entry));
  bool clip = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(lbtn));
  double step = gtk_range_get_value(steps);

  if (!s.empty() && !c.empty() && objects.count(s) == 0) {
    gtk_combo_box_text_append(combo, nullptr, s.c_str());
    if (curve && spline) {
      objects.insert(std::make_pair(
          s, std::unique_ptr<bspline>(new bspline(s, c, clip, step))));
    } else if (curve && !spline) {
      objects.insert(std::make_pair(
          s, std::unique_ptr<bezier>(new bezier(s, c, clip, step))));
    } else if (c.size() >= 3) {
      objects.insert(
          std::make_pair(s, std::unique_ptr<polygon>(new polygon(s, c, fill))));
    } else if (c.size() == 2) {
      objects.insert(
          std::make_pair(s, std::unique_ptr<line>(new line(s, c, clip))));
    } else {
      objects.insert(
          std::make_pair(s, std::unique_ptr<point>(new point(s, c))));
    }
    update();
  }

  gtk_entry_set_text(name, "");
  gtk_entry_set_text(coor, "");
}

extern "C" G_MODULE_EXPORT void btn_clip_clk(GtkWidget *widget,
                                             GtkWidget *btn) {
  lclip = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
}

extern "C" G_MODULE_EXPORT void btn_rotate_left_clk(GtkWidget *widget,
                                                    GtkWidget *scale) {
  rotate(gtk_range_get_value(GTK_RANGE(scale)));
}

extern "C" G_MODULE_EXPORT void btn_rotate_right_clk(GtkWidget *widget,
                                                     GtkWidget *scale) {
  rotate(-gtk_range_get_value(GTK_RANGE(scale)));
}

extern "C" G_MODULE_EXPORT void btn_pan_up_clk(GtkWidget *widget,
                                               GtkWidget *scale) {
  pan(coord(0, gtk_range_get_value(GTK_RANGE(scale))));
}

extern "C" G_MODULE_EXPORT void btn_pan_left_clk(GtkWidget *widget,
                                                 GtkWidget *scale) {
  pan(coord(-gtk_range_get_value(GTK_RANGE(scale)), 0));
}

extern "C" G_MODULE_EXPORT void btn_pan_right_clk(GtkWidget *widget,
                                                  GtkWidget *scale) {
  pan(coord(gtk_range_get_value(GTK_RANGE(scale)), 0));
}

extern "C" G_MODULE_EXPORT void btn_pan_down_clk(GtkWidget *widget,
                                                 GtkWidget *scale) {
  pan(coord(0, -gtk_range_get_value(GTK_RANGE(scale))));
}

extern "C" G_MODULE_EXPORT void btn_zoom_out_clk(GtkWidget *widget,
                                                 GtkWidget *scale) {
  zoom(1 + gtk_range_get_value(GTK_RANGE(scale)));
}

extern "C" G_MODULE_EXPORT void btn_zoom_in_clk(GtkWidget *widget,
                                                GtkWidget *scale) {
  zoom(1 - gtk_range_get_value(GTK_RANGE(scale)));
}

extern "C" G_MODULE_EXPORT void btn_exit_clk() { gtk_main_quit(); }

extern "C" G_MODULE_EXPORT void btn_center_clk() {
  w = window(vp_width, vp_height);
  update();
}

extern "C" G_MODULE_EXPORT void btn_clear_clk(GtkWidget *widget,
                                              GtkWidget *combo) {
  objects.clear();
  gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(combo));
  btn_center_clk();
}

extern "C" G_MODULE_EXPORT void btn_trans_figure_clk(GtkWidget *widget,
                                                     GtkWidget *entry) {
  GtkComboBoxText *combo =
      GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "object_list"));
  gchar *obj = gtk_combo_box_text_get_active_text(combo);

  if (obj == nullptr) {
    return;
  }

  GtkComboBoxText *ops =
      GTK_COMBO_BOX_TEXT(gtk_builder_get_object(builder, "operation_list"));
  GtkRange *xcoor =
      GTK_RANGE(gtk_builder_get_object(builder, "x_coord_param_scale"));
  GtkRange *ycoor =
      GTK_RANGE(gtk_builder_get_object(builder, "y_coord_param_scale"));

  const int op_id = gtk_combo_box_get_active(GTK_COMBO_BOX(ops));
  const double angle = M_PI * gtk_range_get_value(GTK_RANGE(entry)) / 180;
  coord vector(gtk_range_get_value(xcoor), gtk_range_get_value(ycoor));

  drawable &d = *(objects.find(obj)->second.get());
  std::unordered_map<int, matrix<double>> bases = {
      {0, m_transfer(vector)},
      {1, m_transfer(-d.center()) * m_scale(vector) * m_transfer(d.center())},
      {2, m_transfer(-vector) * m_rotate(angle) * m_transfer(vector)},
      {3, m_transfer(coord()) * m_rotate(angle)},
      {4, m_transfer(-d.center()) * m_rotate(angle) * m_transfer(d.center())},
  };

  transform(bases.find(op_id)->second, d.orig);
  update();
}

extern "C" G_MODULE_EXPORT void btn_delete_figure_clk(GtkWidget *widget,
                                                      GtkWidget *combo) {
  gchar *obj = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
  if (obj != nullptr) {
    gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(combo),
                              gtk_combo_box_get_active(GTK_COMBO_BOX(combo)));
    objects.erase(obj);
    update();
  }
}

extern "C" G_MODULE_EXPORT void btn_add_obj_figure(GtkWidget *widget,
                                                   GtkWidget *combo) {
  gchar *file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
  if (objects.count(file) == 0) {
    GtkToggleButton *fill_btn =
        GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "fill_obj_check"));
    polygon p = read_obj(file);
    p.fill = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(fill_btn));
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), nullptr,
                              p.name.c_str());
    objects.insert(
        std::make_pair(p.name, std::unique_ptr<polygon>(new polygon(p))));
    update();
  }
}

extern "C" G_MODULE_EXPORT void btn_save_obj(GtkWidget *widget,
                                             GtkWidget *combo) {
  gchar *obj = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
  if (obj != nullptr) {
    write_obj(*(objects.find(obj)->second.get()));
  }
}

#endif // HANDLER_HPP
