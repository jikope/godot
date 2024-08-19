#ifndef GENDHING_EDITOR_H
#define GENDHING_EDITOR_H

#include "core/string/node_path.h"
#include "sound_source.h"
#include "core/object/object.h"
#include "modules/jikope_audio/jikope_audio.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/color_rect.h"
#include "scene/gui/control.h"
#include "scene/gui/file_dialog.h"
#include <cstdint>
#include <map>


typedef std::map<uint32_t, char> BonangNote_t;
enum Laras { Slendro, Pelog };
enum SelectedLane { None, Left, Right };
enum ModalMode { NORMAL, RECORDING, EDITING };

static const char* MODAL_TEXT[] = { "NORMAL", "RECORDING", "EDITING"};

class GendhingEditor : public Control
{
	GDCLASS(GendhingEditor, Control)
public:
    GendhingEditor();
    ~GendhingEditor();

	void switch_laras();
	Laras get_laras();
	void set_song_picker(const NodePath& fd);
	NodePath get_song_picker();

	Error load_audio_file();
	void set_audio_file_path(const String& path);
	String get_audio_file_path();

	Error save_gendhing();

	void update();

protected:
	static void _bind_methods();
	void _notification(int p_what);
	void input(const Ref<InputEvent> &p_event) override;

	void _draw_preview_panel();
	void _draw_editor_panel();
	void _play_bonang_notes(uint64_t current_time_ms);

	void on_song_picker_btn_pressed();
	void on_song_picker_btn_selected(String path);

	void on_play_btn_pressed();
	void on_stop_btn_pressed();
	void on_pause_btn_pressed();

	void preview_input(const Ref<InputEvent> &p_event);
	void editor_input(const Ref<InputEvent> &p_event);

	void switch_modal_mode();

private:
	void _move_note(int direction);
	void _remove_note();
	void _select_next_note();
	void _select_prev_note();
	void _move_note_lane();
	void _insert_note_at_timeline(int time, char note);
	void _copy_notes(uint64_t time_ms);

	Ref<Font> _default_font;
	Ref<Font> _note_font;

	Ref<SoundSource> _audio_file;
	String _audio_file_path;
	short _duration = 0;
	Laras _laras = Laras::Slendro;
	ModalMode _mode = ModalMode::NORMAL;

	// How do we store bonang notes ??
	// Since bonang can hit 2 notes at the same time.
	// We need to store these 2 possible note. Also we
	// need think how to display this as replayable note
	// in the gameplay.

	// In RECORDING MODE, mipil produces L and R notes at same lane
	// cause of the time difference.
	// Ex:
	//  L:  -  1  -  1
	//  R:  2  -  2  -
	// Becomes
	//  L:  2  1  2  1
	//  R:
	// To solve this we need to postprocess the result of recording
	// by editing the notes in EDITING MODE.
	BonangNote_t _left_notes;
	BonangNote_t _right_notes;

	// Store copy for playback purpose
	BonangNote_t _left_notes_copy;
	BonangNote_t _right_notes_copy;

	// Components
	ColorRect* _control_panel;
	VBoxContainer* _menu_container;
	FileDialog* _song_picker = nullptr;
	NodePath _song_picker_path;
	Button* _song_picker_btn;
	Button* _play_btn;
	Button* _stop_btn;
	Button* _pause_btn;
	Timer _song_time_timer;
	Label* _curr_time;

	ColorRect* _editor_panel;
	SelectedLane _selected_note_lane = SelectedLane::None;
	int _selected_note_time; // Ms
	int _scroll_offset = 0; // Pixel
	int _page_width = 0; // Pixel
	int _interval_second = 200; // Pixel
	int _note_resolution = 100; // Ms

	ColorRect* _preview_panel;

	JikopeAudio* jikope;
};


#endif /* GENDHING_EDITOR_H */
