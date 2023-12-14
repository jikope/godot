#include "gendhing_roll_editor.h"
#include "core/math/vector2.h"
#include "core/os/dir_access.h"
#include "core/os/file_access.h"
#include "modules/gendhing_editor/gendhing.h"
#include "nlohmann/json.hpp"
#include "core/color.h"
#include "core/error_list.h"
#include "core/os/input_event.h"
#include "core/os/keyboard.h"
#include "scene/gui/control.h"
#include "scene/gui/scroll_bar.h"
#include "scene/resources/dynamic_font.h"
#include <iostream>
#include <string>

#define MARGIN_H 50
#define MARGIN_V 50

String ms_to_str2(int number) {
	return String::num((float)number / 1000, 2);
}

String GendhingRollEditor::get_title() const {
	return _title;
}
void GendhingRollEditor::set_title(const String &title) {
	if (title == _title) {
		return;
	}

	_title = title;
}

int GendhingRollEditor::get_interval() {
	return _current_ms;
}

void GendhingRollEditor::set_interval(int ms) {
	_current_ms = ms;
}

String GendhingRollEditor::find_instrument_boundary(Vector2 position) {
	for (int i = 0; i < _displayed_instrument.size(); i++) {
		// Check instrument boundary
		auto name = _displayed_instrument[i];
		int top = _instruments_boundary[name].boundary_top;
		int bot = _instruments_boundary[name].boundary_bot;
		if (top < 1 || bot < 1) {
			return "";
		}
		if (position.y > top && position.y < bot) {
			return name;
		}
	}

	return "";
}

int GendhingRollEditor::find_timeline_from_position(Vector2 position) {
	int time_pos = ((position.x - _separator_x + _h_scroll->get_value()) / _interval_sec) * 1000;
	if (time_pos % _time_resolution > _time_resolution / 2) {
		time_pos = (time_pos / _time_resolution) * _time_resolution + _time_resolution;
	} else {
		time_pos = (time_pos / _time_resolution) * _time_resolution;
	}
	return time_pos;
}

void GendhingRollEditor::delete_selected_note_at_timeline() {
	if (_selected_ms > 0) {
		InstrumentNotes *instrument = &_instruments.at(_selected_instrument);

		if (instrument->count(_selected_ms)) {
			instrument->erase(_selected_ms);
		}
	}
}

void GendhingRollEditor::select_next_note_at_timeline() {
	if (_selected_ms > 0) {
		InstrumentNotes *instrument = &_instruments.at(_selected_instrument);

		if (!instrument->count(_selected_ms)) {
			int i = _selected_ms;
			while (true) {
				if (i >= _time_max) {
					if (_selected_ms + _current_ms >= _time_max) {
						_time_max += 5000;
					}
					_selected_ms += _current_ms;
					break;
				}
				int found = instrument->count(i);

				if (found) {
					_selected_ms = i;
					break;
				}

				i += _time_resolution;
			}
		} else {
			auto it = instrument->find(_selected_ms);
			auto last_element = instrument->end();
			--last_element;

			if (it->first == last_element->first) {
				if (_selected_ms + _current_ms >= _time_max) {
					_time_max += 5000;
				}
				_selected_ms += _current_ms;
			} else {
				_selected_ms = (++it)->first;
			}
		}

		int x = int((float)_selected_ms * (_interval_sec / 1000.0)) + _separator_x - _h_scroll->get_value();
		if (x > get_size().width) {
			_h_scroll->set_value(_h_scroll->get_value() + (get_size().width - _separator_x));
		}
	}
}

void GendhingRollEditor::select_prev_note_at_timeline() {
	if (_selected_ms > 0) {
		InstrumentNotes *instrument = &_instruments.at(_selected_instrument);

		if (!instrument->count(_selected_ms)) {
			int i = _selected_ms;
			while (true) {
				if (i <= 0) {
					break;
				}
				int found = instrument->count(i);

				if (found) {
					_selected_ms = i;
					break;
				}

				i -= _time_resolution;
			}
		} else {
			auto it = instrument->find(_selected_ms);
			auto first_element = instrument->begin();

			if (it->first != first_element->first) {
				_selected_ms = (--it)->first;
			}
		}

		int x = int((float)_selected_ms * (_interval_sec / 1000.0)) + _separator_x - _h_scroll->get_value();
		if (x < _separator_x + 50) {
			_h_scroll->set_value(_h_scroll->get_value() - _h_scroll->get_page());
		}
	}
}

void GendhingRollEditor::insert_note_at_timeline(char note) {
	InstrumentNotes *instrument = &_instruments.at(_selected_instrument);
	auto to_change = instrument->find(_selected_ms);
	if (to_change == instrument->end()) {
		instrument->insert(std::make_pair(_selected_ms, note));
	} else {
		to_change->second = note;
	}
}

void GendhingRollEditor::clear_instrument_note() {
	for (auto& i : _instruments) {
		i.second.clear();
	}
}

Error GendhingRollEditor::new_track(const String path) {
	_title = path;
	String new_track_path = USER_DIR + _title;

	DirAccess* dir = DirAccess::create(DirAccess::ACCESS_USERDATA);
	Error err = dir->make_dir(new_track_path);
	if (err != OK) {
		return err;
	}

	clear_instrument_note();

	return OK;
}

Error GendhingRollEditor::load_track(const String path) {
	String load_path = USER_DIR + path + "/track.json";
	FileAccess* file = FileAccess::open(load_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(!file, FAILED, "Couldn't open file " + load_path + " to load.");

	std::string string_json = file->get_as_utf8_string().utf8().get_data();

	nlohmann::ordered_json track_json;
	track_json = nlohmann::ordered_json::parse(string_json);
	std::map<std::string, InstrumentNotes> instrumentNote (track_json.at("instrument_note"));

	clear_instrument_note();
	for (auto i : instrumentNote) {
		std::map<String, InstrumentNotes>::iterator key = _instruments.find(i.first.c_str());
		if (key != _instruments.end()) {
			key->second = i.second;
		}
	}
	file->close();
	_title = path;
	if (_instruments.at("BALUNGAN").size() > 0) {
		auto last = _instruments.at("BALUNGAN").end();
		(--last);

		_time_max = last->first + 1500;
	}

	return OK;
}

Error GendhingRollEditor::save_track() {
	String save_path = USER_DIR + _title + "/track.json";
	nlohmann::ordered_json track_json;

	String title = get_title();
	track_json["title"] = title.utf8().get_data();
	std::map<std::string, InstrumentNotes> instrumentNote;

	for (auto instrument : _instruments) {
		if (instrument.second.empty()) {
			continue;
		}
		instrumentNote.insert(make_pair(instrument.first.utf8().get_data(), instrument.second));
	}
	track_json["instrument_note"] = instrumentNote;

	FileAccess* out_file = FileAccess::open(save_path, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(!out_file, FAILED, "Couldn't open file " + save_path + " to save.");

	auto string_js = nlohmann::to_string(track_json);
	out_file->store_string(string_js.c_str());
	out_file->close();
	emit_signal("saved");

	return OK;
}


void GendhingRollEditor::record_instrument() {
	_is_recording = true;
}

Error GendhingRollEditor::play_current_track() {
	_player.set_track(_time_max, _instruments);
	_player.seek(_selected_ms);
	_player.play_track();
	_is_playing = true;

	return OK;
}


void GendhingRollEditor::stop_current_track() {
	_player.stop_track();
}

void GendhingRollEditor::reset_current_track() {
	_is_playing = false;
	_is_recording = false;
	update();
}

// ------------------------------------------------------------------------------
// Event Handler CALLBACKS
// ------------------------------------------------------------------------------

void GendhingRollEditor::_gui_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> mb = p_event;

	if (mb.is_valid()) {
		if (mb->is_pressed()) {
			if (mb->get_button_index() == BUTTON_LEFT) {
				Vector2 mouse_position = mb->get_position();
				_selected_instrument = find_instrument_boundary(mouse_position);
				_selected_ms = find_timeline_from_position(mouse_position);

				update();
			}

			if (mb->get_button_index() == BUTTON_WHEEL_UP) {
				if (mb->get_shift()) {
					_h_scroll->set_value(_h_scroll->get_value() - (100 * mb->get_factor()));
				} else {
					_v_scroll->set_value(_v_scroll->get_value() - (25 * mb->get_factor()));
				}
			}
			if (mb->get_button_index() == BUTTON_WHEEL_DOWN) {
				if (mb->get_shift()) {
					_h_scroll->set_value(_h_scroll->get_value() + (100 * mb->get_factor()));
				} else {
					_v_scroll->set_value(_v_scroll->get_value() + (25 * mb->get_factor()));
				}
			}
		}
	}

	Ref<InputEventKey> key_event = p_event;
	if (key_event.is_valid()) {
		if (key_event->is_pressed()) {
			switch(key_event->get_scancode()) {
			case KEY_I: { // Display 'set interval' dialog
				if (key_event->get_shift()) {
					emit_signal("open_set_interval_dialog");
					accept_event();
				}
			} break;
			case KEY_O: { // Start playing from start
				if (key_event->get_shift()) {
					_selected_ms = 0;
					_h_scroll->set_value(0);
					accept_event();
					update();
					return;
				}
			} break;
			case KEY_P: {
				if (key_event->get_shift()) {
					// STOP
					if (_is_playing) {
						stop_current_track();
					} else {
						play_current_track();
					}
				}
				accept_event();
				return;
			} break;
			case KEY_S: {
				if (key_event->get_shift()) {
					save_track();
					accept_event();
					return;
				}
			} break;
			}
		}


		if (_is_recording && key_event->is_pressed()) {
			char note = key_event->get_unicode();
			if (note == 'e' || note == 'r' || note == 't' || note =='y' || note =='u' || note =='d' || note =='f' || note =='g' || note =='h' || note =='j') {
				switch (note) {
				case 'e': { note = '^'; } break;
				case 'r': { note = '%'; } break;
				case 't': { note = '#'; } break;
				case 'y': { note = '@'; } break;
				case 'u': { note = 'x'; } break;
				case 'd': { note = '!'; } break;
				case 'f': { note = '2'; } break;
				case 'g': { note = '3'; } break;
				case 'h': { note = '5'; } break;
				case 'j': { note = '6'; } break;
				}

				// InstrumentNotes *instrument = &_instruments.at(_selected_instrument);
				int time_pos = _player.get_clock();

				if (time_pos % _time_resolution > _time_resolution / 2) {
					time_pos = (time_pos / _time_resolution) * _time_resolution + _time_resolution;
				} else {
					time_pos = (time_pos / _time_resolution) * _time_resolution;
				}
				_selected_ms = time_pos;
				insert_note_at_timeline(note);
				accept_event();
			}
		}

		if (key_event->is_pressed() && _selected_ms >= 0 && !_selected_instrument.empty()) {
			switch (key_event->get_scancode()) {
			case KEY_BACKSPACE: {
				delete_selected_note_at_timeline();
				update();
			} break;
			case KEY_TAB: {
				if (key_event->get_shift()) {
					select_prev_note_at_timeline();
				} else {
					select_next_note_at_timeline();
				}

				accept_event();
				update();
			} break;
			case KEY_R: {
				if (key_event->get_shift() && !_is_recording && !_is_playing) {
					record_instrument();
					play_current_track();
					accept_event();
					update();
				}
			} break;
			default: {
				if (key_event->get_unicode() > 32 && !_is_recording && !_is_playing) {
					char note = key_event->get_unicode();
					insert_note_at_timeline(note);
					accept_event();
					update();
				}
			} break;
			}
		}
	}
}

void GendhingRollEditor::_notification(int p_what) {
	switch(p_what) {
	case NOTIFICATION_RESIZED: {
		printf("Resized\n");
	} break;
	case NOTIFICATION_DRAW: {
		Size2 size = get_size();
		int width = size.width;
		int height = size.height;

		Ref<Font> default_font = get_font("");
		Object::cast_to<DynamicFont>(*_regular_font)->set_size(20);

		int row_height = height / 6;
		int x_ofs = 0;
		int y_ofs = 0 + MARGIN_V + 10;

		// Title
		draw_string(_regular_font, Point2(50, MARGIN_V / 1.5), _title);

		// Draw left side
		_separator_x = (width * (25.0 / 100.0));
		draw_line(Point2(_separator_x, 0), Point2(_separator_x, height), Color(0.9, 0.9, 0.9), 2);

		// Iterate instruments to calculate boundaries
		// BALUNGAN ALWAYS VISIBLE
		draw_rect(Rect2(x_ofs, y_ofs + 5, _separator_x - 2, row_height - 10), Color(0.25, 0.25, 0.25));
		draw_string(_regular_font, Point2(x_ofs + MARGIN_H, y_ofs + row_height / 2), "BALUNGAN");

		_instruments_boundary["BALUNGAN"].boundary_top = y_ofs + 5;
		_instruments_boundary["BALUNGAN"].boundary_bot = y_ofs + 5 + row_height - 10;

		y_ofs += row_height;
		int v_scroll_area_start = y_ofs;
		int v_scroll_ofs = _v_scroll->get_value();

		if (v_scroll_ofs > 1) {
			draw_line(Point2(0, y_ofs - 5), Point2(size.width, y_ofs - 5), Color(0.2, 0.2, 0.2));
		}

        _displayed_instrument = _instrument_order;

        int y_calc = v_scroll_area_start;
		for (auto instrument = _instruments.begin(); instrument != _instruments.end(); instrument++) {
			String instrument_name = instrument->first;
			if (instrument_name == "BALUNGAN") {
				continue;
			}

			int y_start = y_calc;
			int y_end = y_calc + row_height;

            if (y_start - v_scroll_ofs < v_scroll_area_start) {
                _displayed_instrument.erase(instrument_name);
            }

            y_calc = y_end;
		}


		for (int i = 0; i < _displayed_instrument.size(); i++) {
			std::map<String, InstrumentNotes>::iterator instrument = _instruments.find(_displayed_instrument[i]);

			String instrument_name = instrument->first;
			if (instrument_name == "BALUNGAN") {
				continue;
			}

			draw_rect(Rect2(x_ofs, y_ofs + 5, _separator_x - 2, row_height - 10), Color(0.25, 0.25, 0.25));
			draw_string(_regular_font, Point2(x_ofs + MARGIN_H, y_ofs + row_height / 2), instrument_name);
			_instruments_boundary[instrument_name].boundary_top = y_ofs + 5;
			_instruments_boundary[instrument_name].boundary_bot = y_ofs + 5 + row_height - 10;

			if (_is_recording && instrument_name == _selected_instrument) {
				draw_circle(Point2(_separator_x - 30, y_ofs + row_height / 2), 5, Color(0.8, 0.1, 0.1));
			}


			y_ofs += row_height;
		}

		// Draw right side
		//   Scrollbar
		//      Time Position
		int scroll_ofs = _h_scroll->get_value();
		int first_time_pos = 0;

		// Selection Box
		if (_selected_ms > 0 && !_selected_instrument.empty()) {
			// MiliSec to position
			int x = int((float)_selected_ms * (_interval_sec / 1000.0)) + _separator_x - scroll_ofs;

			if (x > _separator_x) {
				int rect_width = _interval_sec / (1000 / _time_resolution);
				int center_y = _instruments_boundary.at(_selected_instrument).boundary_top + (row_height / 5);
				draw_rect(Rect2(x - rect_width / 2, 0 + MARGIN_V, rect_width, height), Color(0.35, 0.35, 0.35));
				draw_rect(Rect2(x - rect_width / 2, center_y, rect_width, row_height / 2), Color(0.25, 0.25, 0.25));
			}
		}

		// Scan first timing according to scroll value
		int i = 0;
		while (true) {
			if ((scroll_ofs + i) % (_interval_sec / 4) == 0) {
				first_time_pos = scroll_ofs + i;
				_first_line_in_timeline_position = first_time_pos;
				break;
			}
			i++;
		}

		// Object::cast_to<DynamicFont>(*_regular_font)->set_size(16);
		// DRAW LINE
		// Just increment x by interval_sec
		// x == time related
		// position == rendered position on screen
		int x = first_time_pos;
		while (true) {
			int position = x + _separator_x - scroll_ofs;
			if (position > size.width) {
				break;
			}

			if (x % _interval_sec == 0) {
				String time = String::num(x / (float)_interval_sec, 2);
				draw_string(_regular_font, Point2(position, 0 + MARGIN_V / 2), time);
				draw_line(Point2(position, 0 + MARGIN_V), Point2(position, height), Color(0.5, 0.5, 0.5), 2);
			} else {
				if (x % (_interval_sec / 2) == 0) {
					String time = String::num(x / (float)_interval_sec, 2);
					draw_string(_regular_font, Point2(position, 0 + MARGIN_V / 2), time, Color(0.5, 0.5, 0.5));
				}
				draw_line(Point2(position, 0 + MARGIN_V), Point2(position, height), Color(0.5, 0.5, 0.5), 2);
			}
			x += _interval_sec / 4;
		}
		draw_line(Point2(_separator_x, 0 + MARGIN_V), Point2(width, 0 + MARGIN_V), Color(0.9, 0.9, 0.9), 2);

		for (int i = 0; i < _displayed_instrument.size(); i++) {
			auto instrument = _instruments.find(_displayed_instrument[i]);
			int center_y = _instruments_boundary[instrument->first].boundary_top + (row_height / 2);

			for (auto note = instrument->second.begin(); note != instrument->second.end(); note++) {
				int x = ((float)note->first * (_interval_sec / 1000.0)) + _separator_x - scroll_ofs;

				if (x > _separator_x) {
					draw_string(_note_font, Point2(x - 10, center_y), String::chr(note->second));
				}
			}
		}

		// Draw playing line
		if (_player._is_playing) {
			double current_time = _player.get_clock();
			int time_position = current_time * (_interval_sec / 1000.0);
			int local_position = time_position + _separator_x - _h_scroll->get_value();

			if (local_position > width) {
				_h_scroll->set_value(_h_scroll->get_value() + _h_scroll->get_page());
			} else if (local_position < 0) {
				int drawing_area = (int)_h_scroll->get_page();
				_h_scroll->set_value(time_position % drawing_area * drawing_area);
			}
			// printf("Time %f - First: %d - Scroll: %d\n", current_time, local_position, scroll_ofs);
			int y_start = 0 + MARGIN_V;
			int y_end = height;
			draw_line(Point2(local_position, y_start), Point2(local_position, y_end), Color(0.1, 0.3, 0.7), 5);
		}

		_h_scroll->set_begin(Point2(_separator_x, size.height - 20));
		_h_scroll->set_end(Point2(size.width - 20, size.height));
		_v_scroll->set_begin(Point2(size.width - 20, v_scroll_area_start));
		_v_scroll->set_end(Point2(size.width, size.height));

		int total_width = ((_time_max / 1000) * _interval_sec) + _interval_sec;
		_h_scroll->set_max(total_width);
		_h_scroll->set_page(size.width - _separator_x);

		int total_v_scroll = (_instruments.size() - 1) * row_height;
		_v_scroll->set_max(total_v_scroll);
		_v_scroll->set_page(size.height - v_scroll_area_start);
		// printf("Total V Scroll: %d | Y_ofs: %d | Page: %f | Current : %f\n", total_v_scroll, y_ofs, size.height - v_scroll_area_start, _v_scroll->get_value());
	} break;
	}
}

void GendhingRollEditor::_scroll_moved(double p_to_val) {
	update();
}

void GendhingRollEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_gui_input"), &GendhingRollEditor::_gui_input);
	ClassDB::bind_method(D_METHOD("_scroll_moved"), &GendhingRollEditor::_scroll_moved);

	ClassDB::bind_method(D_METHOD("new_track", "path"), &GendhingRollEditor::new_track);
	ClassDB::bind_method(D_METHOD("load_track", "path"), &GendhingRollEditor::load_track);
	ClassDB::bind_method(D_METHOD("reset_current_track"), &GendhingRollEditor::reset_current_track);

	ClassDB::bind_method(D_METHOD("set_interval", "ms"), &GendhingRollEditor::set_interval);
	ClassDB::bind_method(D_METHOD("get_interval"), &GendhingRollEditor::get_interval);
	ClassDB::bind_method(D_METHOD("set_title", "title"), &GendhingRollEditor::set_title);
	ClassDB::bind_method(D_METHOD("get_title"), &GendhingRollEditor::get_title);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "title", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT_INTL), "set_title", "get_title");

	ADD_SIGNAL(MethodInfo("saved"));
	ADD_SIGNAL(MethodInfo("error"));
	ADD_SIGNAL(MethodInfo("open_set_interval_dialog"));
}

GendhingRollEditor::GendhingRollEditor() {
	// Load custom gendhing file
	InstrumentNotes bonang_barung;
	InstrumentNotes bonang_penerus;
	InstrumentNotes balungan;
	InstrumentNotes kendhang;
	InstrumentNotes peking;
	InstrumentNotes kempul;
	InstrumentNotes kenong;

	_instruments["BALUNGAN"] = balungan;
	_instruments["BONANG_BARUNG"] = bonang_barung;
	_instruments["BONANG_PENERUS"] = bonang_penerus;
	_instruments["PEKING"] = peking;
	_instruments["KENDHANG"] = kendhang;
	_instruments["KENONG"] = kenong;
	_instruments["KEMPUL"] = kempul;

	_instrument_order.push_back("BALUNGAN");
	_instrument_order.push_back("BONANG_BARUNG");
	_instrument_order.push_back("BONANG_PENERUS");
	_instrument_order.push_back("KENDHANG");
	_instrument_order.push_back("PEKING");
	_instrument_order.push_back("KEMPUL");
	_instrument_order.push_back("KENONG");

	_time_max = 8000; // parse from file
	_current_ms = 800;
	_selected_ms = 0;
	_title = "Untitled";

	set_clip_contents(true);
	set_focus_mode(FOCUS_ALL);

	Ref<DynamicFontData> note_font_data = ResourceLoader::load("res://Resources/Font/custom-kridhamardawa-bold.ttf");
	Ref<DynamicFont> note_font = memnew(DynamicFont);
	note_font->set_font_data(note_font_data);
	// note_font->set_size(36);
	_note_font = note_font;
	Object::cast_to<DynamicFont>(*_note_font)->set_size(36);

	Ref<DynamicFontData> regular_font_data = ResourceLoader::load("res://Resources/Font/RedHatMono-SemiBold.ttf");
	Ref<DynamicFont> regular_font = memnew(DynamicFont);
	regular_font->set_font_data(regular_font_data);
	regular_font->set_size(11);
	_regular_font = regular_font;

	_h_scroll = memnew(HScrollBar);
	_h_scroll->connect("value_changed", this, "_scroll_moved");
	_h_scroll->set_step(100);
	add_child(_h_scroll);

	_v_scroll = memnew(VScrollBar);
	_v_scroll->connect("value_changed", this, "_scroll_moved");
	_v_scroll->set_step(25);
	add_child(_v_scroll);

	_player.connect("time_update", this, "update");
	_player.connect("track_end", this, "reset_current_track");
	// bool has = Engine::get_singleton()->has_singleton("JikopeAudio");
}

GendhingRollEditor::~GendhingRollEditor() {
}
