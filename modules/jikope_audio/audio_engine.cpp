#include <cstdio>
#include <memory>
#include "audio_engine.h"

// All Preprocessor directly coded in libs/miniaudio_split/miniaudio.c
// #define MA_NO_VORBIS
// #define MA_NO_PULSEAUDIO
// #define MA_NO_JACK
// #define MA_NO_VORBIS
// #define MA_DEBUG_OUTPUT
// #define MINIAUDIO_IMPLEMENTATION
// #include "libs/miniaudio/miniaudio.h"
#include "libs/miniaudio_split/miniaudio.h"
// #include "libs/miniaudio_split/miniaudio_libvorbis.h"


// #include "libs/stb_vorbis.c"
// #define STB_VORBIS_INCLUDE_STB_VORBIS_H
#define TSF_IMPLEMENTATION
#include "libs/TinySoundFont/tsf.h"


ma_result AudioEngine::initialize() {
	ma_result result;

	this->_m_instrument_soundfonts.instrument_count = 0;
	// this->_m_instrument_soundfonts.instrument_sf = nullptr;

	result = ma_context_init(nullptr, 1, nullptr, &_context);
	MA_ERR(result, "Unable to init context");

	ma_device_config device_config = ma_device_config_init(ma_device_type_playback);
	// device_config.sampleRate = 44100;
	device_config.pUserData = this;
	device_config.playback.format = ma_format_f32;
	device_config.playback.channels = 2;
	device_config.dataCallback = ma_data_callback;
	device_config.performanceProfile = ma_performance_profile_low_latency;

	device_config.wasapi.noAutoConvertSRC = true;

	result = ma_device_init(nullptr, &device_config, &_device);
	MA_ERR(result, "Unable to init device ");

	ma_engine_config engine_config = ma_engine_config_init();
	engine_config.pDevice = &_device;
	engine_config.channels = 2;
	// engine_config.sampleRate = 48000;
	engine_config.pContext = &_context;
	engine_config.periodSizeInMilliseconds = 10;

	ma_resource_manager_config resource_config = ma_resource_manager_config_init();
	resource_config.decodedFormat = ma_format_f32;
	resource_config.decodedChannels = 0;
	// resource_config.decodedSampleRate = 44100;

	result = ma_resource_manager_init(&resource_config, &_resources_manager);
	MA_ERR(result, "Unable to init resource manager");
	engine_config.pResourceManager = &_resources_manager;

	result = ma_engine_init(&engine_config, &_engine);
	MA_ERR(result, "Unable to init engine");
	printf("Engine is %p\n", &_engine);

	// ma_engine_start(&_engine);
	// MA_ERR(result, "Unable to start engine");

	_initialized = true;

	return MA_SUCCESS;
}

void AudioEngine::ma_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	(void)pInput;
	AudioEngine* jikope = (AudioEngine*)pDevice->pUserData;

	if (jikope != nullptr) {

		// printf("Engine is %p\n", &jikope->_engine);
		ma_result res = ma_engine_read_pcm_frames(&jikope->_engine, pOutput, frameCount, nullptr);
		if (res != MA_SUCCESS) {
			printf("Failed to read pcm frames %d\n", res);
		}
		InstrumentSoundFonts* instruments = &jikope->_m_instrument_soundfonts;

		if (instruments->instrument_count > 0) {
			float* out = (float*)pOutput;

			for (int i = 0; i < instruments->instrument_count; i++) {
				tsf* sf = instruments->instrument_sf[i];
				float temp_buf[frameCount * NUM_CH];
				int sample_count = frameCount * NUM_CH;
				tsf_render_float(sf, temp_buf, frameCount, 0);
				// tsf_render_short(sf, temp_buf, frameCount);

				for (int s = 0; s < sample_count; s++) {
					out[s] += temp_buf[s];
				}
			}
		}

	}
}


int AudioEngine::add_soundfont(tsf* soundfont, int count) {
	_m_instrument_soundfonts.instrument_sf[count] = soundfont;
	_m_instrument_soundfonts.instrument_count = count + 1;

	return 0;
}


ma_engine* AudioEngine::get_ma_engine() {
	if (_initialized == true) {
		return &_engine;
	} else {
		initialize();
		return &_engine;
	}
}

AudioEngine::AudioEngine() {}
AudioEngine::~AudioEngine() {}
