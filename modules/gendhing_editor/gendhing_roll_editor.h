#include "core/object.h"
#include "core/string_name.h"
#include "modules/gendhing_editor/gendhing.h"
#include "scene/gui/control.h"
#include "scene/gui/scroll_bar.h"
#include "player.h"


const String USER_DIR = "user://Gendhing/";


class GendhingRollEditor : public Control {
	GDCLASS(GendhingRollEditor, Control)
public:
    GendhingRollEditor();
    virtual ~GendhingRollEditor();
private:
	// How do we store instruments ?
	// Map/Dict of instrument
	//    instrument["BALUNGAN"]		| MAIN
	//    instrument["PEKING"]			| MAIN
	//    instrument["BONANG_BARUNG"]	| ELABORATE
	//    instrument["BONANG_PENERUS"]	| ELABORATE
	//    instrument["KENDHANG"]		| Structure
	//    instrument["KEMPUL"]			| Structure
	//    instrument["KENONG"]			| Structure

	struct InstrumentBoundary {
		int boundary_top;
		int boundary_bot;
	};

	Player _player;
	Vector<String> _instrument_order;
	Vector<String> _displayed_instrument;
	Instruments _instruments;
	std::map<String, InstrumentBoundary> _instruments_boundary;
	// Metadata
	String _title;
	String _laras;
	String _path;

	Ref<Font> _regular_font;
	Ref<Font> _note_font;

	HScrollBar* _h_scroll;
	VScrollBar* _v_scroll;

	bool _is_playing = false;
	bool _is_recording = false;
	String _selected_instrument;
	int _selected_ms;
	int _current_ms;

	int _time_max; // 20 s
	int _first_line_in_timeline_position;
	int _separator_x;
	int _time_resolution = 50;
	const int _interval_sec = 300; // 1 sec == N pixel
	const int _default_max_time = 20000; // 20 sec

	int get_interval();
	void set_interval(int ms);
	String get_title() const;
	void set_title(const String &title);
	void clear_instrument_note();
	Error new_track(const String path); // Assumes that new track is not already exists.
	Error load_track(const String path);
	Error save_track();

	void record_instrument();
	Error play_current_track();
	void stop_current_track();
	void reset_current_track();
	// void record_instrument_note();

	void insert_note_at_timeline(char note);
	void delete_selected_note_at_timeline();
	void select_next_note_at_timeline();
	void select_prev_note_at_timeline();
	String find_instrument_boundary(Vector2 position);
	int find_timeline_from_position(Vector2 position);

	void _gui_input(const Ref<InputEvent> &p_event);
	void _notification(int p_what);
	void _scroll_moved(double p_to_val);
protected:
	static void _bind_methods();
};
