#include "gendhing_editor.h"
#include "core/dictionary.h"
#include "core/math/rect2.h"
#include "core/os/keyboard.h"
#include "core/print_string.h"
#include "core/ustring.h"
#include "scene/2d/canvas_item.h"
#include "scene/gui/control.h"
#include "scene/gui/label.h"
#include "scene/gui/popup.h"
#include "scene/gui/scroll_container.h"
#include "scene/resources/dynamic_font.h"


String ms_to_str(int number) {
	return String::num((float)number / 1000, 2);
}


int GendhingEditor::calculate_rows_and_cols(int len) {
	int n_row = (len / 16);
	if ((len - (16 * n_row)) > 0) {
		n_row += 1;
	}
	return n_row;
}

int GendhingEditor::scan_note_position(const Vector2 pos) {
	for (int i = 0; i < m_cols.size(); i++) {
		Col boundary = m_cols[i];
		int start_x = int(boundary.pos.x);
		int start_y = int(boundary.pos.y);
		int end_x = int(boundary.pos.x + boundary.size.x);
		int end_y = int(boundary.pos.y + boundary.size.y);

		if (
				pos.x > start_x && pos.x < end_x &&
				pos.y > start_y && pos.y < end_y) {
			return boundary.index;
			// printf("Found index %d with boundaries x: %d %d - %d %d\n", boundary.index, start_x, start_y, end_x, end_y);
			break;
		}
	}

	return -1;
}

void GendhingEditor::_gui_input(const Ref<InputEvent> &p_event) {
	Ref<InputEventMouseButton> btn = p_event;

	if (btn.is_valid()) {
		int mouse_btn = btn->get_button_index();

		if (mouse_btn == BUTTON_LEFT && btn->is_doubleclick()) {
			// printf("Gendhaga selected: %d - %d\n", m_selected, (int)m_selected / 4);
			Vector2 cursor_pos = btn->get_position();
			m_selected = scan_note_position(cursor_pos);
			if (m_selected != -1) {
				_toggle_popup();
				update();
			}
		}

		if (mouse_btn == BUTTON_LEFT && btn->is_pressed() ) {
			Vector2 cursor_pos = btn->get_position();
			m_selected = scan_note_position(cursor_pos);

			update();
			return;
		}
	}

	Ref<InputEventKey> key_event = p_event;
	if (key_event.is_valid()) {
		if (m_selected == -1) {
			return;
		}

		if (key_event->is_pressed()) {
			switch (key_event->get_scancode()) {
				case KEY_BACKSPACE:
					balungan_notes[m_selected] = '.';
					break;
				case KEY_TAB:
					if (m_selected == balungan_notes.length() - 1) {
						add_gendhaga();
					}

					if (m_selected != -1) {
						m_selected++;
					}
					break;
				default:
					if (key_event->get_unicode() > 32) {
						balungan_notes[m_selected] = key_event->get_unicode();
						if (m_selected == balungan_notes.length() - 1) {
							add_gendhaga();
						}
						if (m_selected != -1) {
							m_selected++;
						}
					}
					break;
					// case KEY_1:
					// 	balungan_notes[m_selected] = '1';
					// 	break;
					// case KEY_2:
					// 	balungan_notes[m_selected] = '2';
					// 	break;
					// case KEY_3:
					// 	balungan_notes[m_selected] = '3';
					// 	break;
					// case KEY_4:
					// 	balungan_notes[m_selected] = '4';
					// 	break;
					// case KEY_5:
					// 	balungan_notes[m_selected] = '5';
					// 	break;
					// case KEY_6:
					// 	balungan_notes[m_selected] = '6';
					// 	break;
			}
			accept_event();
			update();
		}
	}
}

/**
 * DRAWING
 */
void GendhingEditor::_notification(int p_what) {
	switch (p_what) {
	case NOTIFICATION_ENTER_TREE: {
		ScrollContainer* container = Object::cast_to<ScrollContainer>(get_parent());
		if (container) {
			m_container = container;
		} else {
			print_line("Scroll container is null");
		}
	} break;
	case NOTIFICATION_DRAW: {
		Size2 size = get_size();
		int width = size.width;

		RID ci = get_canvas_item();

		int font_ascent = m_font->get_ascent();
		Color font_color = Color(1, 1, 1);
		FontDrawer drawer(m_font, Color(1, 1, 1));

		int char_ofs = 0;
		int x_ofs = 0 + h_margin;
		int y_ofs = 0 + v_margin + font_ascent;

		if (m_selected != -1) {
			Col col = m_cols[m_selected];
			draw_rect(Rect2(col.pos, col.size), Color(0.25, 0.5, 0.25));
		}

		while (true) {
			if (balungan_timing.size() != balungan_notes.length() ||
				m_cols.size() != balungan_notes.length())
			{
				break;
			}

			if (char_ofs >= balungan_notes.length()) {
				break;
			}

			// Note
			CharType cchar = balungan_notes[char_ofs];
			CharType next = balungan_notes[char_ofs + 1];
			int char_width = m_font->get_char_size(cchar, next).width;
			int char_height = m_font->get_char_size(cchar, next).height;
			drawer.draw_char(ci, Point2(x_ofs, y_ofs), cchar, next, font_color);
			Col i_col = { char_ofs,
				Point2(x_ofs, y_ofs - font_ascent / 1.5),
				Size2(char_width, font_ascent)
			};
			m_cols.set(char_ofs, i_col);

			String time = String::num((float)balungan_timing[char_ofs] / 1000.0);
			draw_string(m_regular_font, Point2(x_ofs, y_ofs - 50), time, font_color);

			if ((char_ofs + 1) % 4 == 0) {
				draw_rect(Rect2(Point2(), Size2()), Color(0.3, 0.5, 0.3));

				x_ofs += GENDHAGA_INTERVAL; // Add space every gendhaga | every 4 beats
			}

			x_ofs += char_width + m_char_interval;

			if ((char_ofs + 1) % 16 == 0) {
				x_ofs = 0 + h_margin; // Reset x position
				y_ofs += char_height; // New row every 16 notes
			}

			char_ofs++;
		}

		if (m_container != nullptr) {
			// printf("Scroll min: %f - max: %d - page: %d\n", 0.1, m_container->get_v_scroll(), m_container->get_v_scrollbar()->);
		}

		m_add_gendhaga_btn->set_position(Size2(x_ofs, y_ofs - font_ascent / 2));
		set_custom_minimum_size(Size2(get_custom_minimum_size().width, y_ofs + v_margin));
	} break;
	};
}


void GendhingEditor::_toggle_popup() {
	int n_gendhaga = ((int)m_selected / 4) * 4;
	printf("n_gendhaga: %d\n", n_gendhaga);
	String balungan_split = balungan_notes.substr(n_gendhaga, 4);
	Dictionary gendhaga_note;
	gendhaga_note["balungan_note"] = balungan_split;
	gendhaga_note["balungan_timing"] = balungan_timing.slice(n_gendhaga, n_gendhaga + 3);

	m_gendhaga_edit->set_gendhaga(gendhaga_note, balungan_timing[m_selected]);
	m_popup_panel->popup_centered(Size2(1000, 500));

	// printf("Gendhaga selected: %d - %d\n", m_selected, (int)m_selected / 4);
	accept_event();
}

void GendhingEditor::_add_gendhaga_btn() {
	add_gendhaga();
	accept_event();
}

void GendhingEditor::add_gendhaga() {
	int len = balungan_notes.length();

	balungan_notes = balungan_notes + "....";

	balungan_timing.resize(balungan_notes.length());
	balungan_timing[len]     = (int)balungan_timing[len - 1] + balungan_interval_ms;
	balungan_timing[len + 1] = (int)balungan_timing[len + 0] + balungan_interval_ms;
	balungan_timing[len + 2] = (int)balungan_timing[len + 1] + balungan_interval_ms;
	balungan_timing[len + 3] = (int)balungan_timing[len + 2] + balungan_interval_ms;
	m_cols.resize(balungan_notes.length());

	update();

	if (m_container != nullptr && len % 16 == 0) {
		m_container->set_v_scroll(m_container->get_v_scroll() + 100);
	}
}

void GendhingEditor::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_gui_input"), &GendhingEditor::_gui_input);
	ClassDB::bind_method(D_METHOD("_add_gendhaga_btn"), &GendhingEditor::_add_gendhaga_btn);
}

GendhingEditor::GendhingEditor() {
	// Control's Class settings
	set_focus_mode(FOCUS_ALL);

	m_selected = -1;
	h_margin = 40;
	v_margin = 50;

	// Read from file format
	// For this development we feed some toy data
	balungan_interval_ms = 800;
	balungan_notes =
			"112621656!213216"
			"2321653232162165"
			"312621656!213216"
			"43216532";
	balungan_timing.resize(balungan_notes.length());
	int timing = 0;
	for (int i = 0; i < balungan_timing.size(); i++) {
		timing += balungan_interval_ms;
		balungan_timing[i] = timing;
	}
	// End of Reading

	int total_notes = balungan_notes.length();
	int n_row = calculate_rows_and_cols(total_notes);
	// int remain_note = abs(total_notes - (16 * (n_row - 1)));
	// printf("n_row: %d, remain_note: %d\n", n_row, remain_note);

	m_cols.resize(balungan_notes.length());

	Ref<DynamicFontData> kridhamardawa_data = ResourceLoader::load(KRIDHAMARDAWA_FONT);
	Ref<DynamicFont> kridhamardawa(memnew(DynamicFont));
	kridhamardawa->set_font_data(kridhamardawa_data);
	kridhamardawa->set_size(36);
	m_font = kridhamardawa;

	Ref<DynamicFontData> regular_font_data = ResourceLoader::load("res://Resources/Font/RedHatMono-SemiBold.ttf");
	Ref<DynamicFont> regular_font(memnew(DynamicFont));
	regular_font->set_font_data(regular_font_data);
	regular_font->set_size(11);
	m_regular_font = regular_font;

	Ref<StyleBoxFlat> style_box(memnew(StyleBoxFlat));
	style_box->set_border_color(Color(1, 0, 0));
	m_style_box = style_box;

	m_add_gendhaga_btn = memnew(Button);
	m_add_gendhaga_btn->set_text("+");
	m_add_gendhaga_btn->set_flat(true);
	m_add_gendhaga_btn->set_visible(true);
	m_add_gendhaga_btn->connect("pressed", this, "_add_gendhaga_btn");
	add_child(m_add_gendhaga_btn);

	// Setup Gendhaga Popup Editor
	m_popup_panel = memnew(PopupPanel);
	m_gendhaga_edit = memnew(GendhagaPopup);
	ScrollContainer* popup_container = memnew(ScrollContainer);
	popup_container->set_enable_h_scroll(false);
	popup_container->set_enable_v_scroll(true);
	popup_container->set_follow_focus(true);
	popup_container->set_clip_contents(true);

	Label* label = memnew(Label);
	label->set_text("Label parent");

	popup_container->add_child(m_gendhaga_edit);
	// popup_container->add_child(label);
	m_popup_panel->add_child(popup_container);
	add_child(m_popup_panel);
}

GendhingEditor::~GendhingEditor() {
	// memdelete(m_add_gendhaga_btn);
	// memdelete(m_scroll);
}


//------------------------------------------------------------------------------
// GENDHAGA POPUP SECTION
//------------------------------------------------------------------------------


void GendhagaPopup::set_gendhaga(Dictionary notes, int current_interval) {
	m_notes = notes;
	m_current_interval = current_interval;
}

void GendhagaPopup::_notification(int p_what) {
	switch (p_what) {
	case NOTIFICATION_DRAW: {
		if (is_showing) {
			String str = m_notes["balungan_note"];
			Array note_timing = m_notes["balungan_timing"];
			if (str.empty() || note_timing.empty()) {
				print_error("GendhagaEditor: Balungan is empty");
				break;
			}

			Size2 size = get_size();
			int h_margin = 50;

			RID ci = get_canvas_item();

			int font_ascent = m_font->get_ascent();
			Color font_color = Color(1, 1, 1);
			FontDrawer drawer(m_font, Color(1, 1, 1));

			int left_x_ofs = 0 + 20;
			int left_y_ofs = 0 + 100;

			int right_x_ofs = left_x_ofs + 200;
			// int right_y_ofs = 0 + 100;

			// Time navigation

			int x = right_x_ofs;
			int y = 30;
			int w = (size.width - right_x_ofs) - 50;
			draw_line(Point2(x, y), Point2(w, y), Color(1, 1, 1), 3);
			draw_string(m_regular_font, Point2(x, y - 10), ms_to_str(note_timing[0]), Color(1, 1, 1));
			draw_string(m_regular_font, Point2(w, y - 10), ms_to_str(note_timing[3]), Color(1, 1, 1));

			// Balungan
			draw_string(get_font(""), Point2(left_x_ofs, left_y_ofs), "Balungan", font_color);
			int x_interval = (size.width - right_x_ofs) / 4;
			int x_ofs = right_x_ofs;
			for (int i = 0; i < 4; i++) {
				drawer.draw_char(ci, Point2(x_ofs, left_y_ofs), str[i], str[i+1], font_color);
				// draw_string(m_regular_font, Point2(x_ofs, left_y_ofs - font_ascent), String::num((float)note_timing[i] / 1000, 2), font_color);
				// print_line(String::num((float)note_timing[i] / 1000, 2));
				x_ofs += x_interval;
			}

			// Bonang
			x_interval = (size.width - right_x_ofs) / 4;

			draw_string(get_font(""), Point2(left_x_ofs, left_y_ofs + 75), "Bonang", font_color);

			String bonang_note = m_notes["bonang_note"];
			Array bonang_timing = m_notes["bonang_timing"];
			draw_rect(Rect2(Point2(right_x_ofs, left_y_ofs + 30), Size2(size.width - right_x_ofs, 75)), Color(0.25, 0.25, 0.25));
			for (int i = 0; i < bonang_note.length(); i ++) {

			}


			set_custom_minimum_size(Size2(800, 400));
		}
	}
	}
}

void GendhagaPopup::_bind_methods() {

}


GendhagaPopup::GendhagaPopup() {
	Ref<DynamicFontData> kridhamardawa_data = ResourceLoader::load(KRIDHAMARDAWA_FONT);
	Ref<DynamicFont> kridhamardawa(memnew(DynamicFont));
	kridhamardawa->set_font_data(kridhamardawa_data);
	kridhamardawa->set_size(36);
	m_font = kridhamardawa;

	Ref<DynamicFontData> regular_font_data = ResourceLoader::load("res://Resources/Font/RedHatMono-SemiBold.ttf");
	Ref<DynamicFont> regular_font(memnew(DynamicFont));
	regular_font->set_font_data(regular_font_data);
	regular_font->set_size(11);
	m_regular_font = regular_font;

	set_h_size_flags(SIZE_EXPAND_FILL);
	set_v_size_flags(SIZE_FILL);
}

GendhagaPopup::~GendhagaPopup() {}
