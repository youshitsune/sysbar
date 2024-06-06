#include "module.hpp"

module::module(const bool &icon_on_start, const bool &clickable) {
	// Initialization
	get_style_context()->add_class("module");
	append(label_info);
	auto cursor = Gdk::Cursor::create("pointer");
	set_cursor(cursor);

	label_info.set_margin_start(5);
	label_info.set_margin_end(5);

	if (icon_on_start)
		prepend(image_icon);
	else
		append(image_icon);

	popover_popout.set_child(box_popout);
	popover_popout.set_has_arrow(false);
	popover_popout.set_parent(*this);

	// Life would be simpler if buttons would not get stuck in a :hover
	// or :active state permenantly..
	if (clickable) {
		m_click_gesture = Gtk::GestureClick::create();
		m_click_gesture->set_button(GDK_BUTTON_PRIMARY);
		m_click_gesture->signal_pressed().connect(sigc::mem_fun(*this, &module::on_clicked));
		add_controller(m_click_gesture);
		box_popout.set_size_request(300,400);
		popover_popout.set_offset(-150,0);
	}
}

void module::on_clicked(int n_press, double x, double y) {
	popover_popout.popup();
}
