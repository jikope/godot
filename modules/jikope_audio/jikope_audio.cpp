#include "jikope_audio.h"
#include "core/config/engine.h"
#include "core/error/error_macros.h"
#include "core/variant/variant.h"
#include "modules/jikope_audio/sound_font.h"
// #include "core/io/resource_loader.h"
// #include "core/os/memory.h"
// #include "core/pool_vector.h"
// #include "modules/jikope_audio/sound_font.h"
#include <cstdint>

JikopeAudio* JikopeAudio::singleton;

void JikopeAudio::_bind_methods() {
    ClassDB::bind_method(D_METHOD("hello"), &JikopeAudio::hello);
    ClassDB::bind_method(D_METHOD("initialize"), &JikopeAudio::initialize);
    ClassDB::bind_method(D_METHOD("has_instrument", "name"), &JikopeAudio::has_instrument);
    ClassDB::bind_method(D_METHOD("add_instrument", "soundfont", "name"), &JikopeAudio::add_instrument);

    ClassDB::bind_method(D_METHOD("midi_note_on", "name", "key", "velocity"), &JikopeAudio::midi_note_on);
    ClassDB::bind_method(D_METHOD("midi_note_off", "name", "key"), &JikopeAudio::midi_note_off);
    ClassDB::bind_method(D_METHOD("midi_note_off_all", "name"), &JikopeAudio::midi_note_off_all);

    // ClassDB::bind_method(D_METHOD("load_song", "path"), &JikopeAudio::load_song);

    ClassDB::bind_method(D_METHOD("start_clock"), &JikopeAudio::start_clock);
    // ClassDB::bind_method(D_METHOD("get_current_clock"), &JikopeAudio::get_current_clock);
}

int JikopeAudio::initialize() {
	if (!_audio_engine._initialized) {
		return _audio_engine.initialize();
	}

	return 0;
}

ma_engine* JikopeAudio::get_engine() {
	ma_engine* engine = _audio_engine.get_ma_engine();
	return engine;
}

void JikopeAudio::start_clock() {
	// if (m_engine->_initialized) {
	// 	m_engine->start_clock();
	// } else {
	// 	printf("engine not initalized\n");
	// }
}


int JikopeAudio::add_instrument(const String &sf_path, const String &name) {
	Ref<SoundFont> sf_resource = ResourceLoader::load(sf_path);

	int sf_len = sf_resource->get_data().size();
	PackedByteArray sf_data = sf_resource->get_data();

	tsf *sf = tsf_load_memory(sf_data.ptr(), sf_len);
    if (sf == nullptr) {
        print_line("Failed init tsf");
        return -1;
    }
    tsf_set_output(sf, TSF_STEREO_INTERLEAVED, SAMPLE_RATE, 5); // sample rate
	_audio_engine.add_soundfont(sf, _instruments.size());
    _instruments[name] = sf;

	print_line("Added instrument " + name);

	return 0;
}

bool JikopeAudio::has_instrument(String name) {
	if (_instruments.count(name)) {
		return true;
	}
	return false;
}


void JikopeAudio::midi_note_on(StringName instrument, int key, float velocity) {
    int exists = _instruments.count(instrument);
    if (exists == 0) {
        return;
    }
    tsf_note_on(_instruments[instrument], 0, key, 1.0f); // preset 0, middle C
}

void JikopeAudio::midi_note_off(StringName instrument, int key) {
    int exists = _instruments.count(instrument);
    if (exists == 0) {
        return;
    }
    tsf_note_off_fade(_instruments[instrument], 0, key); // preset 0, middle C
}

void JikopeAudio::midi_note_off_all(StringName instrument) {
    int exists = _instruments.count(instrument);
    if (exists == 0) {
        return;
    }
    tsf_note_off_all(_instruments[instrument]);
}

JikopeAudio::JikopeAudio() {
	singleton = this;
	// m_engine = memnew(AudioEngine);
}

JikopeAudio::~JikopeAudio() {
	// memdelete(m_engine);
}
