
#include "libs/stb_vorbis.c"
#include <chrono>
#include <iostream>
#include <thread>

#define MINIAUDIO_IMPLEMENTATION
#define MA_NO_PULSEAUDIO
#define MA_NO_JACK
#include "libs/miniaudio/miniaudio.h"

#define MA_ERR(res, msg) ({\
			if(res != MA_SUCCESS) {\
				perror(msg); \
				return res; \
			}\
		})

static void ma_data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
	(void)pInput;

	ma_engine_read_pcm_frames((ma_engine*)pDevice->pUserData, pOutput, frameCount, nullptr);
}

int main(int argc, char *argv[])
{
	ma_result result;
	ma_context context;
	ma_device device;
	ma_engine engine;
	ma_resource_manager resource_manager;

	result = ma_context_init(nullptr, 1, nullptr, &context);
	MA_ERR(result, "Unable to init context");

	ma_device_config device_config = ma_device_config_init(ma_device_type_playback);
	device_config.sampleRate = 48000;
	device_config.pUserData = &engine;
	device_config.playback.format = ma_format_f32;
	device_config.playback.channels = 2;
	device_config.dataCallback = ma_data_callback;
	device_config.performanceProfile = ma_performance_profile_low_latency;

	device_config.wasapi.noAutoConvertSRC = true;

	result = ma_device_init(nullptr, &device_config, &device);
	MA_ERR(result, "Unable to init device ");

	ma_engine_config engine_config = ma_engine_config_init();
	engine_config.pDevice = &device;
	engine_config.channels = 2;
	engine_config.sampleRate = 44100;
	engine_config.pContext = &context;
	engine_config.periodSizeInMilliseconds = 10;

	ma_resource_manager_config resource_config = ma_resource_manager_config_init();
	resource_config.decodedFormat = ma_format_f32;
	resource_config.decodedChannels = 0;
	resource_config.decodedSampleRate = 48000;

	result = ma_resource_manager_init(&resource_config, &resource_manager);
	MA_ERR(result, "Unable to init resource manager");
	engine_config.pResourceManager = &resource_manager;

	result = ma_engine_init(&engine_config, &engine);
	MA_ERR(result, "Unable to init engine");

	ma_sound_config sound_config = ma_sound_config_init();
	sound_config.pFilePath = argv[1];
	sound_config.flags = MA_SOUND_FLAG_NO_SPATIALIZATION;

	ma_sound ogg_file;
	result = ma_sound_init_from_file(&engine,
									 argv[1],
									 MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE,
									 nullptr,
									 nullptr,
									 &ogg_file);
	MA_ERR(result, "Failed to init sound");

	// ma_resource_manager_data_source
	// ma_sound ogg_file2;
	// result = ma_sound_init_from_file(&engine,
	// 								 argv[2],
	// 								 MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE,
	// 								 nullptr,
	// 								 nullptr,
	// 								 &ogg_file2);
	// MA_ERR(result, "Failed to init sound");


	std::cout << "Finished " << argv[1] << "\n";
	ma_sound_start(&ogg_file);
	// ma_sound_start(&ogg_file2);

	while(1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

    return 0;
}
