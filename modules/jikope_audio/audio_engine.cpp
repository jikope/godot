#include "audio_engine.h"
#include <chrono>

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_PULSEAUDIO
#define MA_NO_JACK
#include "miniaudio/miniaudio.h"


#include "stb_vorbis.c"
#define STB_VORBIS_INCLUDE_STB_VORBIS_H
#define TSF_IMPLEMENTATION
#include "TinySoundFont/tsf.h"



static void audio_callback_fn(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    DataCallback* data = (DataCallback*)pDevice->pUserData;
    if (data == nullptr) {
        return;
    }
    // // tsf_render_short(data->sf[0], (short*)pOutput, frameCount, 0);

    short* out = (short*)pOutput;

	if (data->instrument_count != 0) {
		for (int i = 0; i < data->instrument_count; i++) {
			tsf* sf = data->instrument_sf[i];
			short temp_buf[4096];
			int sample_count = frameCount * NUM_CH;
			tsf_render_short(sf, (short*)temp_buf, frameCount, 0);

			for (int s = 0; s < sample_count; s++) {
				out[s] = (out[s] + temp_buf[s]) / 2;
			}
		}
	}

    (void)pInput;
}


ma_result AudioEngine::initialize() {
    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format          = ma_format_s16;
    deviceConfig.playback.channels        = NUM_CH;
    deviceConfig.sampleRate               = SAMPLE_RATE;
    deviceConfig.periodSizeInMilliseconds = LATENCY_MS;
    // deviceConfig.periodSizeInFrames       = 256;
    deviceConfig.dataCallback             = audio_callback_fn;

    deviceConfig.pUserData = &m_data_cb;

    if (ma_device_init(nullptr, &deviceConfig, &m_device) != MA_SUCCESS) {
        return MA_FAILED_TO_INIT_BACKEND;
    }

    if (ma_device_start(&m_device) != MA_SUCCESS) {
		return MA_FAILED_TO_START_BACKEND_DEVICE;
    }

	_initialized = true;
	return MA_SUCCESS;
}

void AudioEngine::start_clock() {
	m_start_time = std::chrono::high_resolution_clock::now();
}

double AudioEngine::get_current_clock() {
	std::chrono::duration<double, std::milli> now = std::chrono::high_resolution_clock::now() - m_start_time;
	return now.count();
}

int AudioEngine::add_soundfont(tsf* soundfont, int index) {
	m_data_cb.instrument_sf[index] = soundfont;
	m_data_cb.instrument_count = index + 1;

	return 0;
}

AudioEngine::AudioEngine() {
	// m_device = new ma_device;
	// m_data_cb = new DataCallback;
}

AudioEngine::~AudioEngine() {
	ma_device_uninit(&m_device);
	// delete m_device;
	// delete m_data_cb;
}
