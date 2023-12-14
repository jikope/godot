#ifndef JIKOPE_AUDIO_H
#define JIKOPE_AUDIO_H

#include "core/object.h"

#include "audio_engine.h"
#include "core/string_name.h"
#include <map>

// SoundFont is formatted into instrument
// Instrument == Soundfont

class JikopeAudio : public Object {
	GDCLASS(JikopeAudio, Object);
public:
	// Init and Setup
	int initialize();
	int add_instrument(String sf_path, String name);
	bool has_instrument(String name);

	// Operating
	void midi_note_on(StringName instrument, int key, float velocity);
	void midi_note_off(StringName instrument, int key);
	void midi_note_off_all(StringName instrument);

	// Timings
	void start_clock();
	double get_current_clock() { return m_engine->get_current_clock(); }

	static JikopeAudio* get_singleton() { return singleton; }
	JikopeAudio();
	~JikopeAudio();
	String hello() { return String("Hello Jikope audio"); } /* Just for sanity check */
private:
	static JikopeAudio* singleton;
	AudioEngine* m_engine;
	std::map<StringName, tsf*> m_instruments;
protected:
    static void _bind_methods();
};


#endif /* JIKOPE_AUDIO_H */
