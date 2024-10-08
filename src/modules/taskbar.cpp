#include "../config_parser.hpp"
#include "taskbar.hpp"

#include <gdk/wayland/gdkwayland.h>
#include <iostream>

uint text_length = 14;

// Placeholder functions
void handle_toplevel_title(void *data, zwlr_foreign_toplevel_handle_v1*, const char *title) {
	auto toplevel_entry = static_cast<taskbar_item*>(data);

	std::string text = title;

	if (text == "")
		text = "Untitled Window";

	if (text.length() > text_length)
		text = text.substr(0, text_length - 2) + "..";

	toplevel_entry->toplevel_label.set_text(text);
}

void handle_toplevel_app_id(void *data, zwlr_foreign_toplevel_handle_v1*, const char *app_id) {}

void handle_toplevel_output_enter(void *data, zwlr_foreign_toplevel_handle_v1*, wl_output *output) {}

void handle_toplevel_output_leave(void *data, zwlr_foreign_toplevel_handle_v1*, wl_output *output) {}

void handle_toplevel_state(void *data, zwlr_foreign_toplevel_handle_v1*, wl_array *state) {
	auto toplevel_entry = static_cast<Gtk::Box*>(data);
	auto flowbox_child = static_cast<Gtk::FlowBoxChild*>(toplevel_entry->get_parent());
	auto flowbox = static_cast<Gtk::FlowBox*>(flowbox_child->get_parent());

	uint32_t *state_array = (uint32_t *)state->data;
	size_t count = state->size / sizeof(uint32_t);

	for (size_t i = 0; i < count; ++i) {
		switch (state_array[i]) {
			case ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MAXIMIZED:
				break;
			case ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_MINIMIZED:
			break;
			case ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_ACTIVATED:
				flowbox->select_child(*flowbox_child);
				break;
			case ZWLR_FOREIGN_TOPLEVEL_HANDLE_V1_STATE_FULLSCREEN:
				break;
		}
	}
}

void handle_toplevel_done(void *data, zwlr_foreign_toplevel_handle_v1*) {}

void handle_toplevel_closed(void *data, zwlr_foreign_toplevel_handle_v1* handle) {
	auto toplevel_entry = static_cast<taskbar_item*>(data);
	auto flowbox_child = static_cast<Gtk::FlowBoxChild*>(toplevel_entry->get_parent());
	auto flowbox = static_cast<Gtk::FlowBox*>(flowbox_child->get_parent());
	flowbox->remove(*toplevel_entry);

	// Does this cause a memory leak?
	// Future me will find out!
}

void handle_toplevel_parent(void *data, zwlr_foreign_toplevel_handle_v1* handle, zwlr_foreign_toplevel_handle_v1* parent) {}

zwlr_foreign_toplevel_handle_v1_listener toplevel_handle_v1_impl = {
	.title = handle_toplevel_title,
	.app_id = handle_toplevel_app_id,
	.output_enter = handle_toplevel_output_enter,
	.output_leave = handle_toplevel_output_leave,
	.state = handle_toplevel_state,
	.done = handle_toplevel_done,
	.closed = handle_toplevel_closed,
	.parent = handle_toplevel_parent
};

void handle_manager_toplevel(void *data, zwlr_foreign_toplevel_manager_v1 *manager,
	zwlr_foreign_toplevel_handle_v1 *toplevel) {
	auto self = static_cast<module_taskbar*>(data);

	taskbar_item *toplevel_entry = Gtk::make_managed<taskbar_item>(self->flowbox_main);
	
	zwlr_foreign_toplevel_handle_v1_add_listener(toplevel,
		&toplevel_handle_v1_impl, toplevel_entry);

	self->flowbox_main.append(*toplevel_entry);
}

zwlr_foreign_toplevel_manager_v1_listener toplevel_manager_v1_impl = {
	.toplevel = handle_manager_toplevel,
};

void registry_handler(void *data, struct wl_registry *registry,
							 uint32_t id, const char *interface, uint32_t version) {

	auto self = static_cast<module_taskbar*>(data);
	if (strcmp(interface, zwlr_foreign_toplevel_manager_v1_interface.name) == 0) {
		auto zwlr_toplevel_manager = (zwlr_foreign_toplevel_manager_v1*)
			wl_registry_bind(registry, id,
				&zwlr_foreign_toplevel_manager_v1_interface,
				std::min(version, 3u));
		zwlr_foreign_toplevel_manager_v1_add_listener(zwlr_toplevel_manager,
			&toplevel_manager_v1_impl, self);
	}
}

wl_registry_listener registry_listener = {
	&registry_handler
};

module_taskbar::module_taskbar(sysbar *window, const bool &icon_on_start) : module(window, icon_on_start) {
	// Undo normal widget stuff
	get_style_context()->remove_class("module");
	set_cursor(Gdk::Cursor::create("default"));
	image_icon.hide();
	label_info.hide();

	if (config_main.position %2 == 0) {
		box_container.set_halign(Gtk::Align::CENTER);
		flowbox_main.set_min_children_per_line(25);
		flowbox_main.set_max_children_per_line(25);
	}
	else
		box_container.set_valign(Gtk::Align::CENTER);

	box_container.get_style_context()->add_class("module_taskbar");
	box_container.append(flowbox_main);

	scrolledwindow.set_child(box_container);
	scrolledwindow.set_vexpand(true);
	scrolledwindow.set_hexpand(true);
	append(scrolledwindow);
	setup_proto();
}

void module_taskbar::setup_proto() {
	auto gdk_display = gdk_display_get_default();
	auto display = gdk_wayland_display_get_wl_display(gdk_display);
	auto registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, this);
	wl_display_roundtrip(display);
}

taskbar_item::taskbar_item(const Gtk::FlowBox& container) {
	append(toplevel_label);

	if (container.get_min_children_per_line() == 25)
		set_size_request(100, -1);
	else
		set_size_request(-1, 100);
}
