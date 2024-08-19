#include "gendhing_editor.h"
#include "core/error/error_list.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/os/keyboard.h"
#include "core/string/print_string.h"
#include "core/variant/dictionary.h"
#include "modules/jikope_audio/gendhing.h"
#include "modules/jikope_audio/jikope_audio.h"
#include <cstdint>
#include <unordered_map>
#include <utility>

int last_insert_time = 0;
int separator_x = 0;
const std::unordered_map<char, char> note_pelog = {
	{ 'w', '$' },
	{ 'e', '^' },
	{ 'r', '%' },
	{ 't', '#' },
	{ 'y', '@' },
	{ 'u', '!' },
	{ 'i', '&' },
	{ 's', '1' },
	{ 'd', '7' },
	{ 'f', '2' },
	{ 'g', '3' },
	{ 'h', '5' },
	{ 'j', '6' },
	{ 'k', '4' }
};
const std::unordered_map<char, char> note_slendro = {
	{ 'e', '^'},
	{ 'r', '%'},
	{ 't', '#'},
	{ 'y', '@'},
	{ 'u', 'x'},
	{ 'd', '!'},
	{ 'f', '2'},
	{ 'g', '3'},
	{ 'h', '5'},
	{ 'j', '6'}
};


Error GendhingEditor::save_gendhing() {
	Dictionary left_notes;
	Dictionary right_notes;

	if (!_left_notes.empty()) {
		for (auto i : _left_notes) {
			left_notes[i.first] = i.second;
		}
	}

	if (!_right_notes.empty()) {
		for (auto i : _right_notes) {
			right_notes[i.first] = i.second;
		}
	}

	Ref<Gendhing> gendhing = memnew(Gendhing);
	gendhing->set_title("Ldr. Pangkur Sl 9");
	gendhing->set_laras((int)Laras::Slendro);
	gendhing->set_left_notes(left_notes);
	gendhing->set_right_notes(right_notes);
	// gendhing->set_notes(left_notes, right_notes);

	String path("user://pangkur");
	Error err = gendhing->save(path);

	// ResourceSaver::save(left_notes, "user://Notes.tres");
	ERR_FAIL_COND_V_MSG(err != OK, err, "Failed saving Gendhing file " + path + ".");

	return OK;
}

void GendhingEditor::_move_note(int direction) {
	if (_selected_note_time != 0 && _selected_note_lane != SelectedLane::None) {
		auto notes = &_left_notes;
		if (_selected_note_lane == SelectedLane::Right) {
			notes = &_right_notes;
		}

		if (!notes->empty()) {
			auto found = notes->find(_selected_note_time);
			if (found != notes->end()) {
				auto next_ms = notes->find(_selected_note_time + (direction * _note_resolution));

				if (next_ms == notes->end()) {
					// print_line("Current " + String::num(found->first) + " Note: " + String::chr(found->second));
					// print_line("Next " + String::num(_selected_note_time + (direction * _note_resolution)));
					// print_line("Inserting " + String::num(_selected_note_time + (direction * _note_resolution)) + " Note: " + String::chr(found->second));
					notes->insert(std::pair<int, char>(_selected_note_time + (direction * _note_resolution), found->second));
					notes->erase(found);
					_selected_note_time += (direction * _note_resolution);
				}
			}
		}
	}
}

void GendhingEditor::_remove_note() {
	if (_selected_note_time != 0 && _selected_note_lane != SelectedLane::None) {
		auto notes = &_left_notes;
		if (_selected_note_lane == SelectedLane::Right) {
			notes = &_right_notes;
		}

		if (!notes->empty()) {
			auto found = notes->find(_selected_note_time);
			if (found != notes->end()) {
				notes->erase(found);
			}
		}
	}
}

void GendhingEditor::_select_next_note() {
	if (_selected_note_time != 0 && _selected_note_lane != SelectedLane::None) {
		BonangNote_t::iterator iter = _left_notes.lower_bound(_selected_note_time);

		if (iter->first == _selected_note_time) {
			iter = std::next(iter);
		}

		BonangNote_t::iterator iter_right = _right_notes.lower_bound(_selected_note_time);
		if (iter_right->first == _selected_note_time) {
			iter_right = std::next(iter_right);
		}

		auto selected_iter = iter;
		if (iter_right != _right_notes.end()) {
			if (iter->first > iter_right->first) {
				selected_iter = iter_right;
			} else if (iter == _left_notes.end()) {
				selected_iter = iter_right;
			}
		}

		if (selected_iter != _left_notes.end() && selected_iter != _right_notes.end()) {
			_selected_note_time = selected_iter->first;
		}
	}
}


void GendhingEditor::_select_prev_note() {
	if (_selected_note_time != 0 && _selected_note_lane != SelectedLane::None) {
		BonangNote_t::iterator iter = _left_notes.lower_bound(_selected_note_time);
		BonangNote_t::iterator iter_right = _right_notes.lower_bound(_selected_note_time);

		// if (iter == _left_notes.begin() && iter_right == _right_notes.begin()) {

		// }


		if (!_left_notes.empty()) {
			iter = std::prev(iter);
		}

		if (iter_right != _right_notes.end() && !_right_notes.empty()) {
			iter_right = std::prev(iter_right);
		}

		print_line("Left " + String::num(iter->first) + " Note: " + String::chr(iter->second));
		print_line("Right " + String::num(iter_right->first) + " Note: " + String::chr(iter_right->second));

		auto selected_iter = iter;
		if (iter->first < iter_right->first && iter_right->first < _selected_note_time) {
			selected_iter = iter_right;
		}
			// else if (iter == _left_notes.end()) {
			// 	selected_iter = iter_right;
			// }

		print_line("Selected " + String::num(selected_iter->first) + " Note: " + String::chr(selected_iter->second));

		if (selected_iter != _left_notes.end() && selected_iter != _right_notes.end()) {
			_selected_note_time = selected_iter->first;
		}
	}
}

void GendhingEditor::_move_note_lane() {
	if (_selected_note_time != 0 && _selected_note_lane != SelectedLane::None) {
		auto iter = _left_notes.find(_selected_note_time);

		if (iter != _left_notes.end()) {
			auto right_note = _right_notes.find(_selected_note_time);

			if (right_note == _right_notes.end()) {
				_right_notes.insert(std::pair<int, char>(iter->first, iter->second));
				_left_notes.erase(iter);
			}
		} else {
			auto iter = _right_notes.find(_selected_note_time);

			if (iter != _right_notes.end()) {
				auto left_note = _left_notes.find(_selected_note_time);

				if (left_note == _left_notes.end()) {
					_left_notes.insert(std::pair<int, char>(iter->first, iter->second));
					_right_notes.erase(iter);
				}
			}
		}
	}
}

void GendhingEditor::_insert_note_at_timeline(int time, char note) {
	// if (_selected_note_lane == SelectedLane::Left) {
	// 	_left_notes.insert(std::pair<int, char>(time, note));
	// } else if (_selected_note_lane == SelectedLane::Right) {
	// 	_right_notes.insert(std::pair<int, char>(time, note));
	// }

	int time_floored = (time / _note_resolution) * _note_resolution;

	if (time_floored - last_insert_time < _note_resolution) {
		_right_notes.insert(std::pair<int, char>(time_floored, note));
	} else {
		if (_left_notes.count(time_floored) == 0) {
			_left_notes.insert(std::pair<int, char>(time_floored, note));
		} else {
			_right_notes.insert(std::pair<int, char>(time_floored, note));
		}
	}

	last_insert_time = time_floored;
}

void GendhingEditor::_copy_notes(uint64_t time_ms) {
	_left_notes_copy.clear();
	BonangNote_t::iterator begin_time_left = _left_notes.lower_bound(time_ms);
	for (auto it = begin_time_left; it != _left_notes.end(); it++) {
		_left_notes_copy.insert(*it);
	}

	_right_notes_copy.clear();
	BonangNote_t::iterator begin_time_right = _right_notes.lower_bound(time_ms);
	for (auto it = begin_time_right; it != _right_notes.end(); it++) {
		_right_notes_copy.insert(*it);
	}
}

void GendhingEditor::_play_bonang_notes(uint64_t current_time_ms) {
	BonangNote_t::iterator left_note = _left_notes_copy.begin();
	if (left_note != _left_notes_copy.end() && current_time_ms >= left_note->first) {
		print_line("[LEFT] Time: " + String::num(left_note->first) + " Note: " + String::chr(left_note->second));
		_left_notes_copy.erase(left_note);
	}

	BonangNote_t::iterator right_note = _right_notes_copy.begin();
	if (right_note != _right_notes_copy.end() && current_time_ms >= right_note->first) {
		print_line("[RIGHT] Time: " + String::num(right_note->first) + " Note: " + String::chr(right_note->second));
		_right_notes_copy.erase(right_note);
	}
}

//------------------------------------------------------------------------------
// DRAWING METHODS
//------------------------------------------------------------------------------
void GendhingEditor::_draw_editor_panel() {
	Size2 viewport = _editor_panel->get_size();
	int width = viewport.width;
	int height = viewport.height;

	static int timeline_margin = 30;
	int row_height = height / 2;
	separator_x = width * 0.10;
	_page_width = width - separator_x;

	// _editor_panel->draw_line(Point2(0, timeline_margin), Point2(width, timeline_margin), Color::hex(0xFFFFFFFF), 1);
	// _editor_panel->draw_string(_default_font, Point2(50, timeline_margin + ((height * 0.33) / 2)), String("RAW"));

	// Vertical Separator
	_editor_panel->draw_line(Point2(separator_x, 0), Point2(separator_x, height), Color::hex(0xFFFFFFFF), 1);
	_editor_panel->draw_string(_default_font, Point2(separator_x / 3.5, 20), String::utf8(MODAL_TEXT[_mode]));

	if(_audio_file.is_null()) {
		return;
	}

	int current_time_ms = _audio_file->get_current_time();
	int x = 0;
	int first_time_pos = 0;

	while (true) {
		if ((_scroll_offset + x) % (_interval_second / 2) == 0) {
			first_time_pos = _scroll_offset + x;
			break;
		}
		x++;
	}

	x = first_time_pos;
	while (true) {
		int position = x + separator_x - _scroll_offset;
		if (position > width) {
			break;
		}

		int half = (_interval_second / 2);
		float ms = x / (float)_interval_second;

		char current_min = ms / 60;
		char current_sec = (int)ms % 60;
		String playback_time = vformat("%d:%02d", current_min, current_sec);
		_editor_panel->draw_string(_default_font, Point2(position, 20), playback_time);
		_editor_panel->draw_line(Point2(position, timeline_margin), Point2(position, height), Color(0.5, 0.5, 0.5), 2);
		_editor_panel->draw_line(Point2(position + half, timeline_margin), Point2(position + half, height), Color(0.5, 0.5, 0.5), 2);
		x += _interval_second;
	}

	// Draw relative time progress bar
	float current_secs = (float)current_time_ms / 1000.0;
	float progress_bar_value = (float)current_secs / ((float)_page_width / (float)_interval_second);
	int progress_bar_position = (progress_bar_value * _page_width);
	int rendered_bar_position = progress_bar_position + separator_x - _scroll_offset;

	if (rendered_bar_position < 0) {
		// _scroll_offset = current_secs / (_page_width / _interval_second);
		_scroll_offset -= _page_width;
	}

	_editor_panel->draw_line(Point2(rendered_bar_position, 0), Point2(rendered_bar_position, height), Color::hex(0xFF0000FF), 3);

	int y_offset = timeline_margin;
	// int left_offset = y_offset;
	_editor_panel->draw_string(_default_font, Point2(50, y_offset + (row_height / 2)), String("LEFT"));
	_editor_panel->draw_line(Point2(0, y_offset), Point2(width, y_offset), Color::hex(0xFFFFFFFF), 1);

	if (_selected_note_lane == SelectedLane::Left) {
		_editor_panel->draw_rect(Rect2(separator_x, y_offset, width - separator_x, row_height), Color::hex(0x4A5566));
	} else if (_selected_note_lane == SelectedLane::Right) {
		_editor_panel->draw_rect(Rect2(separator_x, y_offset + row_height, width - separator_x, row_height), Color::hex(0x4A5566));
	}

	// Draw rect for note selection
	if (_selected_note_time != 0 && _selected_note_lane != SelectedLane::None) {
		int selected_note_pos = (((float)_selected_note_time / 1000.0) * (float)_interval_second) + separator_x - _scroll_offset;

		int y_pos = y_offset;

		if (_selected_note_lane == SelectedLane::Right) {
			y_pos = y_offset + row_height;
		}

		_editor_panel->draw_rect(Rect2(selected_note_pos - 13, y_pos + (row_height / 3), 25, 50), Color::hex(0xF0F0F099));
	}

	// Left Notes
	for (auto note = _left_notes.begin(); note != _left_notes.end(); note++) {
		if (note->first > (((float)_scroll_offset / (float)_interval_second) * 1000) && note->first < (((float)(_scroll_offset + _page_width) / (float)_interval_second) * 1000)) {
			int pos_x = (((float)note->first / 1000.0) * (float)_interval_second) + separator_x - _scroll_offset;
			_editor_panel->draw_string(_note_font, Point2(pos_x - 8, y_offset + (row_height / 2)), String::chr(note->second), HorizontalAlignment::HORIZONTAL_ALIGNMENT_LEFT, 24, 24);
		}
	}

	y_offset = y_offset + row_height;
	_editor_panel->draw_string(_default_font, Point2(50, y_offset + (row_height / 2)), String("RIGHT"));
	_editor_panel->draw_line(Point2(0, y_offset), Point2(width, y_offset), Color::hex(0xFFFFFFFF), 1);

	// Right Notes
	for (auto note = _right_notes.begin(); note != _right_notes.end(); note++) {
		if (note->first > (((float)_scroll_offset / (float)_interval_second) * 1000) && (((float)(_scroll_offset + _page_width) / (float)_interval_second) * 1000)) {
			int pos_x = (((float)note->first / 1000.0) * (float)_interval_second) + separator_x - _scroll_offset;
			// _editor_panel->draw_string(_note_font, Point2(pos_x, y_offset + (row_height / 2)), String::chr(note->second));
			_editor_panel->draw_string(_note_font, Point2(pos_x - 8, y_offset + (row_height / 2)), String::chr(note->second), HorizontalAlignment::HORIZONTAL_ALIGNMENT_LEFT, 24, 24);
		}
	}

	if (progress_bar_position > (_scroll_offset + _page_width)) {
		_scroll_offset += _page_width;
	}
}

void GendhingEditor::_draw_preview_panel() {
	Size2 viewport = _preview_panel->get_size();
	int width = viewport.width;
	int height = viewport.height;

	_preview_panel->draw_rect(Rect2(0, 0, width, height), Color::hex(0x222222));

	if (_audio_file.is_valid() && _audio_file->is_loaded()) {
		short duration_min = _duration / 60;
		short duration_sec = _duration % 60;
		float current_secs = (float)_audio_file->get_current_time() / 1000.0;
		// print_line("AUdio Length " + String::num(current_secs));
		short current_min = current_secs / 60;
		short current_sec = (short)current_secs % 60;

		String playback_time = vformat("%d:%02d / %d:%02d", current_min, current_sec, duration_min, duration_sec);
		float progress_bar_value = ((float)current_secs / (float)_duration);
		int progress_bar_position = progress_bar_value * width;

		_preview_panel->draw_string(_default_font, Point2(width - (width * 0.08), 20), playback_time);
		_preview_panel->draw_line(Point2(progress_bar_position, 0), Point2(progress_bar_position, height), Color::hex(0xFFFFFFFF), 3);

		for (auto note : _left_notes) {
			int time_ms = note.first;
			float progress_bar_value = ((float)time_ms / (float)(_duration * 1000));
			int note_preview_x = progress_bar_value * width;

			_preview_panel->draw_rect(Rect2(note_preview_x, height / 3, 1, 1), Color::hex(0xFFFFFF));
		}

		for (auto note : _right_notes) {
			int time_ms = note.first;
			float progress_bar_value = ((float)time_ms / (float)(_duration * 1000));
			int note_preview_x = progress_bar_value * width;

			_preview_panel->draw_rect(Rect2(note_preview_x, height / 3, 1, 1), Color::hex(0xFFFFFF));
		}
	}
}
//------------------------------------------------------------------------------
// END DRAWING METHODS
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// INPUT HANDLES
//------------------------------------------------------------------------------
void GendhingEditor::editor_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> mb = p_event;

	if (mb.is_valid() && mb->is_pressed()) {
		if (mb->get_button_index() == MouseButton::LEFT || mb->get_button_index() == MouseButton::RIGHT) {
			if (_duration < 1) {
				return;
			}

			Vector2 mouse_position = mb->get_position();
			mouse_position.x += 10;
			if (mouse_position.x < separator_x) {
				return;
			}

			if (mb->get_button_index() == MouseButton::LEFT) {
				if (mouse_position.y < _editor_panel->get_size().height / 2) {
					_selected_note_lane = SelectedLane::Left;
				} else {
					_selected_note_lane = SelectedLane::Right;
				}
			} else {
				_selected_note_lane = SelectedLane::None;
			}

			int scroll_time = ((float)_scroll_offset / (float)_interval_second) * 1000;
			int mouse_ms = (int)((mouse_position.x - separator_x) / (float)_interval_second * 1000);

			int time_floored = ((mouse_ms + scroll_time) / _note_resolution) * _note_resolution;
			_selected_note_time = time_floored;
			_audio_file->seek(time_floored);

			if (_audio_file->is_playing()) {
				this->_copy_notes(time_floored);
			}
			this->update();
		}
	}
}

void GendhingEditor::preview_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> mb = p_event;

	if (mb.is_valid() && mb->is_pressed()) {
		if (mb->get_button_index() == MouseButton::LEFT) {
			float width = _preview_panel->get_size().x;
			Vector2 mouse_position = mb->get_position();

			if (_duration < 1) {
				return;
			}

			int millisecond_to_seek = (mouse_position.x / width) * _duration;
			_audio_file->seek(millisecond_to_seek * 1000);

			int page_width = _page_width;
			float page_width_time = ((float)page_width / (float)_interval_second);
			int page_n = (millisecond_to_seek / page_width_time);
			_scroll_offset = (int)page_n * page_width;

			if (_audio_file->is_playing()) {
				this->_copy_notes(millisecond_to_seek * 1000);
			}
			this->update();
		}
	}
}

void GendhingEditor::input(const Ref<InputEvent> &p_event) {
	Ref<InputEventKey> key_press = p_event;
	if (key_press.is_valid() && key_press->is_pressed()) {

		switch(key_press->get_keycode()) {
		case Key::SPACE: {
			this->on_play_btn_pressed();
			this->update();
			return;
		}
		case Key::F1: {
			this->switch_modal_mode();
			this->update();
			return;
		}
		case Key::S: {
			if (key_press->is_command_or_control_pressed()) {
				this->save_gendhing();
			}
			this->update();
			return;
		}
		default: {
			break;
		}
		}


		if (_mode == ModalMode::RECORDING) {
			char note = key_press->get_unicode();
			const std::unordered_map<char, char>* note_map;

			if (_laras == Laras::Pelog) {
				note_map = &note_pelog;
			} else {
				note_map = &note_slendro;
			}

			auto found = note_map->find(note);
			if (found != note_map->end()) {
				if (_audio_file->is_playing()) {
					int time_floored = (_audio_file->get_current_time() / _note_resolution) * _note_resolution;

					// if (time_floored - last_insert_time < _note_resolution) {
					// 	_selected_note_lane = SelectedLane::Left;
					// } else {
					// 	if (_left_notes.count(time_floored) == 0) {
					// 		_selected_note_lane = SelectedLane::Left;
					// 	} else {
					// 		_selected_note_lane = SelectedLane::Left;;
					// 	}
					// }
					_insert_note_at_timeline(time_floored, found->second);
				} else {
					_insert_note_at_timeline(_selected_note_time, found->second);
					this->update();
				}
			}
		} else if (_mode == ModalMode::EDITING) {
			switch(key_press->get_keycode()) {
			case Key::TAB: {
				if (key_press->is_shift_pressed()) {
					this->_select_prev_note();
				} else {
					this->_select_next_note();
				}

				break;
			}
			case Key::ENTER: {
				this->_move_note_lane();
				break;
			}
			case Key::BACKSPACE: {
				this->_remove_note();
				break;
			}
			case Key::LEFT: {
				this->_move_note(-1);
				break;
			}
			case Key::RIGHT: {
				this->_move_note(1);
				break;
			}
			default: {
				break;
			}
			};
			this->update();
		}
	}
}

//------------------------------------------------------------------------------
// END INPUT HANDLES
//------------------------------------------------------------------------------
void GendhingEditor::_notification(int p_what) {
	switch(p_what) {
	case NOTIFICATION_ENTER_TREE: {
		grab_focus();
		if (!_song_picker_path.is_empty()) {
			_song_picker = Object::cast_to<FileDialog>(get_node(NodePath(_song_picker_path)));
			if (_song_picker) {
				_song_picker->connect("file_selected", callable_mp(this, &GendhingEditor::on_song_picker_btn_selected));
			}

			if (!_audio_file_path.is_empty()) {
				load_audio_file();
			}
		}

		_default_font = get_theme_font("default");
		break;
	}
	case NOTIFICATION_THEME_CHANGED: {
		// _preview_panel->set_color(get_theme_color(SNAME("dark_color_2"), EditorStringName(Editor)));
		// _control_panel->set_color(get_theme_color(SNAME("dark_color_1"), EditorStringName(Editor)));
		_default_font = get_theme_font("default");
		break;
	}
	case NOTIFICATION_DRAW: {
		Size2 vp = get_size();
		int width = vp.width;
		int height = vp.height;
		float editor_height = 0.55 * height;
		float preview_height = 0.20 * height;
		float control_height = 0.25 * height;

		draw_rect(Rect2(0, 0, width, height), Color::hex(0x282d35FF));
		_editor_panel->set_position(Point2(0, 0));
		_editor_panel->set_size(Size2(width, editor_height));

		_preview_panel->set_position(Point2(0, editor_height));
		_preview_panel->set_size(Size2(width, preview_height));

		_control_panel->set_position(Point2(0 + 50, preview_height + editor_height));
		_control_panel->set_size(Size2(width - 100, control_height));
		break;
	}
	}
}

void GendhingEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_song_picker", "song_picker"), &GendhingEditor::set_song_picker);
	ClassDB::bind_method(D_METHOD("get_song_picker"), &GendhingEditor::get_song_picker);

	ClassDB::bind_method(D_METHOD("set_audio_file_path", "path"), &GendhingEditor::set_audio_file_path);
	ClassDB::bind_method(D_METHOD("get_audio_file_path"), &GendhingEditor::get_audio_file_path);

	ClassDB::bind_method(D_METHOD("input"), &GendhingEditor::input);

	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "_song_picker"), "set_song_picker", "get_song_picker");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "_audio_file_path", PROPERTY_HINT_FILE, ".ogg"), "set_audio_file_path", "get_audio_file_path");
}

void GendhingEditor::update() {
	uint64_t current_time = _audio_file->get_current_time();
	// print_line("Time: " + String::num(current_time));
	// Play Note
	_play_bonang_notes(current_time);

	_preview_panel->call_deferred("queue_redraw");
	_editor_panel->call_deferred("queue_redraw");
}

Error GendhingEditor::load_audio_file() {
	ERR_FAIL_COND_V_MSG(this->_audio_file_path.is_empty(), ERR_FILE_NOT_FOUND, "Audio file path is empty.");

	if (!_audio_file.is_null()) {
		_audio_file.unref();
	}
	_audio_file.instantiate();
	Error err = _audio_file->load_file(this->_audio_file_path, "Audio Song File");

	ERR_FAIL_COND_V_MSG(err != OK, err, "Error loading SoundSource: " + String::num(err));
	_audio_file->connect("time_update", callable_mp(this, &GendhingEditor::update));
	_duration = _audio_file->get_duration();

	return OK;
}

// ------------------------------------------------------------------------------
// GUI Callback
// ------------------------------------------------------------------------------
void GendhingEditor::on_song_picker_btn_pressed() {
	if (_song_picker != nullptr && !_song_picker_path.is_empty()) {
		_song_picker->popup_centered();
	} else {
		print_line("Song picker null");
	}
}

void GendhingEditor::on_song_picker_btn_selected(String path) {
	this->set_audio_file_path(path);
	this->load_audio_file();
	this->update();
}

void GendhingEditor::on_play_btn_pressed() {
	if (_audio_file.is_null() || !_audio_file->is_loaded()) {
		return;
	}

	if (_audio_file->is_playing()) {
		_play_btn->set_text(U"▶");
		_mode = NORMAL;
		_audio_file->pause();
	} else {
		this->_copy_notes(_audio_file->get_current_time());
		_play_btn->set_text(U"⏸");
		_mode = RECORDING;
		_selected_note_time = 0;
		_selected_note_lane = SelectedLane::None;
		_audio_file->play();
	}
}
void GendhingEditor::on_stop_btn_pressed() {
	_audio_file->stop();
}
void GendhingEditor::on_pause_btn_pressed() {
	_audio_file->pause();
}

void GendhingEditor::switch_modal_mode() {
	switch (_mode) {
	case ModalMode::NORMAL: {
		_mode = EDITING;
		break;
	}
	case ModalMode::EDITING: {
		_mode = NORMAL;
		break;
	}
	default:
		break;
	}
}

void GendhingEditor::switch_laras() {
	switch (_laras) {
	case Laras::Slendro: {
		_laras = Laras::Pelog;
		break;
	}
	case Laras::Pelog: {
		_laras = Laras::Slendro;
		break;
	}
	default:
		break;
	}
}

// ------------------------------------------------------------------------------
// Getters & Setters
// ------------------------------------------------------------------------------
void GendhingEditor::set_audio_file_path(const String &path) {
	_audio_file_path = path;
}

String GendhingEditor::get_audio_file_path() {
	return _audio_file_path;
}

void GendhingEditor::set_song_picker(const NodePath& fd) {
	_song_picker_path = fd;
}

Laras GendhingEditor::get_laras() {
	return _laras;
}

NodePath GendhingEditor::get_song_picker() {
	if (!_song_picker_path.is_empty()) {
		return _song_picker_path;
	}

	return NodePath();
}

GendhingEditor::GendhingEditor() {
	// Prepopulated notes for testing
	// _left_notes = {{ 750, '7' }, { 1250, '7' }, { 1750, '#' }, { 2250, '3' }};
	// _right_notes = {{ 500, '@' }, { 1000, '@' }, { 1500, '@' }, { 2000, '@' }};
	Ref<Gendhing> gendhing = ResourceLoader::load("user://pangkur.tres");
	Dictionary left_notes = gendhing->get_left_notes();
	if (!left_notes.is_empty()) {
		Array keys = left_notes.keys();
		for (int i = 0; i < keys.size(); i++) {
			int key = keys[i];
			_left_notes.insert(std::pair<int, char>(key, (int)left_notes[key]));
		}
	}

	Dictionary right_notes = gendhing->get_right_notes();
	if (!right_notes.is_empty()) {
		Array keys = right_notes.keys();
		for (int i = 0; i < keys.size(); i++) {
			int key = keys[i];
			_right_notes.insert(std::pair<int, char>(key, (int)right_notes[key]));
		}
	}

	_note_font = ResourceLoader::load("res://assets/fonts/custom-kridhamardawa-bold.ttf");
	// _note_font->add_theme_font_size_override("font_size", 24);

	jikope = JikopeAudio::get_singleton();

	set_focus_mode(FOCUS_ALL);
	this->connect("gui_input", callable_mp(this, &GendhingEditor::input));

	// EDITOR PANEL
	_editor_panel = memnew(ColorRect);
    _editor_panel->connect("draw", callable_mp(this, &GendhingEditor::_draw_editor_panel));
	_editor_panel->set_color(Color::hex(0x282d35FF));
	_editor_panel->connect("gui_input", callable_mp(this, &GendhingEditor::editor_input));

	// PREVIEW PANEL
	_preview_panel = memnew(ColorRect);
	_preview_panel->set_color(Color::hex(0x21262eFF));
	_preview_panel->connect("draw", callable_mp(this, &GendhingEditor::_draw_preview_panel));
	_preview_panel->connect("gui_input", callable_mp(this, &GendhingEditor::preview_input));

	// CONTROL PANEL
	_control_panel = memnew(ColorRect);
	_control_panel->set_color(Color::hex(0x282d35FF));
	{
		HBoxContainer *control_box = memnew(HBoxContainer);
		control_box->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
		_control_panel->add_child(control_box);

		VBoxContainer *song_info_box = memnew(VBoxContainer);
		control_box->add_child(song_info_box);

		// OPEN AN OGG FILE
		_song_picker_btn = memnew(Button);
		_song_picker_btn->set_focus_mode(FOCUS_NONE);
		_song_picker_btn->set_text("Open OGG File");
		_song_picker_btn->add_theme_font_size_override("font_size", 24);
		_song_picker_btn->connect("pressed", callable_mp(this, &GendhingEditor::on_song_picker_btn_pressed));
		song_info_box->add_child(_song_picker_btn);

		// PLAYER CONTROLS
		HBoxContainer *audio_control_box = memnew(HBoxContainer);
		audio_control_box->set_h_size_flags(Control::SIZE_EXPAND_FILL);
		audio_control_box->set_alignment(BoxContainer::ALIGNMENT_CENTER);
		song_info_box->add_child(audio_control_box);

		_play_btn = memnew(Button);
		_stop_btn = memnew(Button);

		_play_btn->set_focus_mode(FOCUS_NONE);
		_stop_btn->set_focus_mode(FOCUS_NONE);
		// _pause_btn = memnew(Button);
		_play_btn->connect("pressed", callable_mp(this, &GendhingEditor::on_play_btn_pressed));
		_stop_btn->connect("pressed", callable_mp(this, &GendhingEditor::on_stop_btn_pressed));
		// _pause_btn->connect("pressed", callable_mp(this, &GendhingEditor::on_pause_btn_pressed));
		_play_btn->set_text(U"▶");
		_play_btn->add_theme_font_size_override("font_size", 30);
		_stop_btn->set_text(U"⏹");
		_stop_btn->add_theme_font_size_override("font_size", 30);
		// _pause_btn->set_text(U"⏸");
		audio_control_box->add_child(_play_btn);
		// audio_control_box->add_child(_pause_btn);
		audio_control_box->add_child(_stop_btn);
	}

	add_child(_editor_panel);
	add_child(_preview_panel);
	add_child(_control_panel);
}

GendhingEditor::~GendhingEditor() {
	// memdelete(_control_panel);
	// memdelete(_editor_panel);
	// memdelete(_preview_panel);
}
