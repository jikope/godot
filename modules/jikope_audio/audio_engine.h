#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include "libs/miniaudio_split/miniaudio.h"

// Added fade out function
// void tsf_note_off_fade(tsf* f, int preset_index, int key)
// https://github.com/schellingb/TinySoundFont/issues/56
#include "libs/TinySoundFont/tsf.h"

#include <chrono>
#include <memory>
#define NUM_CH 2
#define LATENCY_MS 10
#define SAMPLE_RATE 44100
#define MAX_SOUNDS 32

#define MA_ERR(res, msg) ({\
			if(res != MA_SUCCESS) {\
				perror(msg); \
				return res; \
			}\
		})

typedef struct {
	int instrument_count;
	tsf* instrument_sf[];
} InstrumentSoundFonts;

typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint_t;

class AudioEngine {
public:
	void start_clock();
	double get_current_clock();

	ma_engine* get_ma_engine();

	int add_soundfont(tsf* soundfont, int count);

    ma_result initialize();
	AudioEngine();
	~AudioEngine();

	bool _initialized = false;
private:
	std::chrono::high_resolution_clock m_clock;
	timepoint_t m_start_time;

	ma_context _context;
	ma_engine _engine;
	ma_device _device;
	ma_device_config deviceConfig;
	ma_resource_manager _resources_manager;

	ma_sound _sound[MAX_SOUNDS];
	InstrumentSoundFonts _m_instrument_soundfonts;

	static void ma_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
};

#endif /* AUDIO_ENGINE_H */
