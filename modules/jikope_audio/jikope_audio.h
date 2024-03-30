#ifndef JIKOPE_AUDIO_H
#define JIKOPE_AUDIO_H

#include "core/object/class_db.h"
#include "audio_engine.h"
#include <map>

class JikopeAudio : public Object {
	GDCLASS(JikopeAudio, Object);
public:
	// Init and Setup
	int initialize();
	int add_instrument(const String &sf_path, const String &name);
	bool has_instrument(String name);

	// MIDI Operation
	void midi_note_on(StringName instrument, int key, float velocity);
	void midi_note_off(StringName instrument, int key);
	void midi_note_off_all(StringName instrument);

	// Timings
	void start_clock();
	double get_current_clock() { return _audio_engine.get_current_clock(); }

	ma_engine* get_engine();
	static JikopeAudio* get_singleton() { return singleton; }
	JikopeAudio();
	~JikopeAudio();
	String hello() { return String("Hello Jikope audio"); } /* Just for sanity check */
private:
	/* std::map<StringName, tsf*> m_instruments; */
	// std::map<StringName, int> _loaded_song;

	static JikopeAudio* singleton;
	AudioEngine _audio_engine;
protected:
    static void _bind_methods();
};

#endif /* JIKOPE_AUDIO_H */
