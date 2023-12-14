#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include "miniaudio/miniaudio.h"

// Added fade out function
// void tsf_note_off_fade(tsf* f, int preset_index, int key)
// https://github.com/schellingb/TinySoundFont/issues/56
#include "TinySoundFont/tsf.h"

#include <chrono>
#include <memory>
#define NUM_CH 2
#define LATENCY_MS 10
#define SAMPLE_RATE 44100


typedef struct {
    /* tsf* sf[INSTRUMENT_COUNT]; */

	int instrument_count;
	tsf* instrument_sf[];
} DataCallback;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint_t;

class AudioEngine {
public:
	void start_clock();
	double get_current_clock();

	int add_soundfont(tsf* soundfont, int count);

    ma_result initialize();
	AudioEngine();
	~AudioEngine();

	bool _initialized = false;
private:
	std::chrono::high_resolution_clock m_clock;
	timepoint_t m_start_time;
	ma_device m_device;
	ma_device_config deviceConfig;
	DataCallback m_data_cb;
};


#endif /* AUDIO_ENGINE_H */
