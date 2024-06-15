#include "main.hpp"
#include "window.hpp"
#include "css.hpp"
#include "config.hpp"
#include "module.hpp"

#include "modules/clock.hpp"
#include "modules/weather.hpp"
#include "modules/tray.hpp"
#include "modules/hyprland.hpp"
#include "modules/volume.hpp"
#include "modules/network.hpp"

#include <gtk4-layer-shell.h>
#include <filesystem>
#include <iostream>

sysbar::sysbar() {
	// Initialize layer shell
	gtk_layer_init_for_window(gobj());
	gtk_layer_set_namespace(gobj(), "sysbar");
	gtk_layer_set_layer(gobj(), GTK_LAYER_SHELL_LAYER_TOP);
	gtk_layer_set_exclusive_zone(gobj(), size);

	gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_TOP, true);
	gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_RIGHT, true);
	gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_BOTTOM, true);
	gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_LEFT, true);

	switch (position) {
		case 0:
			gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_BOTTOM, false);
			set_default_size(-1, size);
			break;
		case 1:
			gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_LEFT, false);
			set_default_size(size, -1);
			break;
		case 2:
			gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_TOP, false);
			set_default_size(-1, size);
			break;
		case 3:
			gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_RIGHT, false);
			set_default_size(size, -1);
			break;
	}

	// Initialize
	set_hide_on_close(true);
	set_child(centerbox_main);
	centerbox_main.set_start_widget(box_start);
	centerbox_main.set_center_widget(box_center);
	centerbox_main.set_end_widget(box_end);
	show();

	// Load custom css
	std::string home_dir = getenv("HOME");
	std::string css_path = home_dir + "/.config/sys64/bar.css";
	css_loader css(css_path, this);

	// Load modules
	std::cout << "Loading modules" << std::endl;
	load_modules(m_start, box_start);
	load_modules(m_center, box_center);
	load_modules(m_end, box_end);
}

void sysbar::load_modules(std::string modules, Gtk::Box &box) {
	std::istringstream iss(modules);
	std::string module_name;

	while (std::getline(iss, module_name, ',')) {
		module *my_module;

		if (false)
			std::cout << "You're not supposed to see this" << std::endl;

		#ifdef MODULE_CLOCK
		else if (module_name == "clock")
			my_module = new module_clock(true, false);
		#endif

		#ifdef MODULE_WEATHER
		else if (module_name == "weather")
			my_module = new module_weather(true, false);
		#endif

		#ifdef MODULE_TRAY
		else if (module_name == "tray")
			my_module = new module_tray(true, false);
		#endif

		#ifdef MODULE_HYPRLAND
		else if (module_name == "hyprland")
			my_module = new module_hyprland(false, false);
		#endif

		#ifdef MODULE_VOLUME
		else if (module_name == "volume")
			my_module = new module_volume(false, false);
		#endif

		#ifdef MODULE_NETWORK
		else if (module_name == "network")
			my_module = new module_network(false, false);
		#endif

		else {
			std::cout << "Unknown module: " << module_name << std::endl;
			return;
		}

		box.append(*my_module);
	}
}
