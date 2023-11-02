#include "jikope_audio.h"
#include "core/io/resource_loader.h"
#include "core/os/memory.h"
#include "core/pool_vector.h"
#include "modules/jikope_audio/sound_font.h"
#include <cstdint>


JikopeAudio* JikopeAudio::singleton;

void JikopeAudio::_bind_methods() {
    ClassDB::bind_method(D_METHOD("hello"), &JikopeAudio::hello);
    ClassDB::bind_method(D_METHOD("initialize"), &JikopeAudio::initialize);
    ClassDB::bind_method(D_METHOD("add_instrument", "soundfont", "name"), &JikopeAudio::add_instrument);
    ClassDB::bind_method(D_METHOD("note_on", "name", "key", "velocity"), &JikopeAudio::midi_note_on);
    ClassDB::bind_method(D_METHOD("note_off", "name", "key"), &JikopeAudio::midi_note_off);
    ClassDB::bind_method(D_METHOD("note_off_all", "name"), &JikopeAudio::midi_note_off_all);

    ClassDB::bind_method(D_METHOD("start_clock"), &JikopeAudio::start_clock);
    ClassDB::bind_method(D_METHOD("get_current_clock"), &JikopeAudio::get_current_clock);
}

int JikopeAudio::initialize() {
	return m_engine->initialize();
}

void JikopeAudio::start_clock() {
	m_engine->start_clock();
}


int JikopeAudio::add_instrument(String sf_path, String name) {
	Ref<SoundFont> sf_resource = ResourceLoader::load(sf_path);

	int sf_len = sf_resource->get_data().size();
	PoolVector<uint8_t> sf_data = sf_resource->get_data();

	tsf *sf = tsf_load_memory(sf_data.read().ptr(), sf_len);
    if (sf == nullptr) {
        print_line("Failed init tsf");
        return -1;
    }
    tsf_set_output(sf, TSF_STEREO_INTERLEAVED, SAMPLE_RATE, 5); // sample rate
	m_engine->add_soundfont(sf, m_instruments.size());
    m_instruments[name] = sf;

	print_line("Added instrument " + name);
    return 0;
}


void JikopeAudio::midi_note_on(StringName instrument, int key, float velocity) {
    int exists = m_instruments.count(instrument);
    if (exists == 0) {
        return;
    }
    tsf_note_on(m_instruments[instrument], 0, key, 1.0f); // preset 0, middle C
}

void JikopeAudio::midi_note_off(StringName instrument, int key) {
    int exists = m_instruments.count(instrument);
    if (exists == 0) {
        return;
    }
    tsf_note_off_fade(m_instruments[instrument], 0, key); // preset 0, middle C
}

void JikopeAudio::midi_note_off_all(StringName instrument) {
    int exists = m_instruments.count(instrument);
    if (exists == 0) {
        return;
    }
    tsf_note_off_all(m_instruments[instrument]);

}

JikopeAudio::JikopeAudio() {
	singleton = this;
	m_engine = memnew(AudioEngine);
}

JikopeAudio::~JikopeAudio() {
	memdelete(m_engine);
}
