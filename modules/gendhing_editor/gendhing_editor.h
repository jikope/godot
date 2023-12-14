#ifndef GENDHING_EDITOR_H
#define GENDHING_EDITOR_H

#include "core/dictionary.h"
#include "core/math/vector2.h"
#include "scene/gui/button.h"
#include "scene/gui/popup.h"
#include "scene/gui/scroll_bar.h"
#include "scene/gui/scroll_container.h"
#include "scene/resources/style_box.h"

#include "core/object.h"
#include "core/reference.h"
#include "core/ustring.h"
#include "core/variant.h"
#include "scene/gui/control.h"
#include <cstdint>

#define KRIDHAMARDAWA_FONT "res://Resources/Font/custom-kridhamardawa-bold.ttf"
#define N_COLS 4
#define GENDHAGA_INTERVAL 100 // HEIGHT in pixel

typedef struct bonang_note {
	int note;
	int timing;
} BonangNote;

class GendhagaPopup : public Control {
	GDCLASS(GendhagaPopup, Control)
public:
	void set_gendhaga(Dictionary notes, int current_interval);

	GendhagaPopup();
	~GendhagaPopup();
private:
	Ref<Font> m_font;
	Ref<Font> m_regular_font;
	bool is_showing;

	ScrollContainer* m_container;
	Control* m_canvas;

	Dictionary m_notes;
	int m_current_interval;

	// void _gui_input(const Ref<InputEvent> &p_event);
	void _notification(int p_what);
protected:
	static void _bind_methods();
};

class GendhingEditor : public Control {
	GDCLASS(GendhingEditor, Control);

	struct Col { // Selection boundaries for single note
		int index;
		Point2 pos;
		Size2 size;
	};
public:
	GendhingEditor();
	~GendhingEditor();
private:
	// User Interface
	Ref<Font> m_font;
	Ref<Font> m_regular_font;
	Ref<StyleBox> m_style_box;

	VScrollBar* m_scroll;
	ScrollContainer* m_container = nullptr;
	GendhagaPopup* m_gendhaga_edit;
	PopupPanel* m_popup_panel;
	Button* m_add_gendhaga_btn;

	int v_margin, h_margin;

	mutable Vector<Col> m_cols;
	// mutable Vector<Row> m_rows;

	int balungan_interval_ms;
	String balungan_notes;
	Array balungan_timing;

	// Interactions
	int m_selected;
	int m_char_interval = 20;
	int scan_note_position(const Vector2 pos);
	int calculate_rows_and_cols(int len);

	// Store array of gendhaga

	// Callbacks godot called
	void _gui_input(const Ref<InputEvent> &p_event);
	void _notification(int p_what);
	void _add_gendhaga_btn();
	void _toggle_popup();

	void add_gendhaga();
protected:
	static void _bind_methods();
};


#endif /* GENDHING_EDITOR_H */
