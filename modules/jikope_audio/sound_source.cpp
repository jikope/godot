#include "sound_source.h"
#include "core/error/error_list.h"
#include "core/error/error_macros.h"
#include "core/io/file_access.h"
#include "core/object/object.h"
#include "core/string/print_string.h"
#include "core/string/ustring.h"
#include "jikope_audio.h"
#include "modules/jikope_audio/libs/miniaudio_split/miniaudio.h"
#include <chrono>
#include <cstdint>
#include <thread>

Error SoundSource::load_file(const String &path, const String &name) {
	ma_engine* engine = JikopeAudio::get_singleton()->get_engine();
	ERR_FAIL_COND_V_MSG(engine == nullptr, FAILED, "Failed to get audio engine instance.");

	Ref<FileAccess> file_access = FileAccess::open(path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(!file_access.is_valid(), FAILED, "Failed to load file " + path);

	CharString abs_path = file_access->get_path_absolute().utf8();
	ma_sound_config config = ma_sound_config_init_2(engine);
	config.pFilePath = abs_path;
	// Load audio into memory at once. Cannot get duration if flag is set to MA_SOUND_FLAG_STREAM
	config.flags = MA_SOUND_FLAG_DECODE;
	ma_result result = ma_sound_init_ex(engine, &config, &_sound);
	print_line_rich(path + " loaded into SoundSource.");

	ERR_FAIL_COND_V_MSG(result != MA_SUCCESS, FAILED, "Failed to instantiate SoundSource." + String::num_int64(result));
	_is_loaded = true;
	ma_sound_set_volume(&_sound, 1.0);

	return OK;
}

bool SoundSource::is_playing() {
	return ma_sound_is_playing(&_sound);
}

void SoundSource::player_callback() {

	while (true) {
		if (!_is_playing) {
			break;
		}

		emit_signal("time_update");
		std::this_thread::sleep_for(std::chrono::milliseconds(16));
	}
}

bool SoundSource::is_loaded() {
	return _is_loaded;
}

uint64_t SoundSource::get_current_time() {
	ma_engine* engine = JikopeAudio::get_singleton()->get_engine();
	// return ma_engine_get_time_in_milliseconds(engine);

	ma_uint64 ms;
	ma_sound_get_cursor_in_pcm_frames(&_sound, &ms);
	return ((double)ms / (double)ma_engine_get_sample_rate(engine)) * 1000;

	// return ma_sound_get_time_in_milliseconds(&_sound);
}

Error SoundSource::play() {
	ma_result result;
	result = ma_sound_start(&_sound);

	ERR_FAIL_COND_V_MSG(result != MA_SUCCESS, FAILED, "Failed to play " + _name);
	std::thread play(&SoundSource::player_callback, this);
	play.detach();
	_is_playing = true;

	return OK;
}

Error SoundSource::stop() {
	ma_result result;
	result = ma_sound_stop(&_sound);
	ERR_FAIL_COND_V_MSG(result != MA_SUCCESS, FAILED, "Failed to stop " + _name);

	result = ma_sound_seek_to_pcm_frame(&_sound, 0);
	ERR_FAIL_COND_V_MSG(result != MA_SUCCESS, FAILED, "Failed to reset " + _name);
	_is_playing = false;

	return OK;
}

Error SoundSource::pause() {
	ma_result result;
	result = ma_sound_stop(&_sound);

	ERR_FAIL_COND_V_MSG(result != MA_SUCCESS, FAILED, "Failed to pause " + _name);
	_is_playing = false;

	return OK;
}

Error SoundSource::seek(uint64_t milliseconds) {
	ma_result result;

	int sr = ma_engine_get_sample_rate(ma_sound_get_engine(&_sound));
	result = ma_sound_seek_to_pcm_frame(&_sound, sr * ((float)milliseconds / (float)1000));
	ERR_FAIL_COND_V_MSG(result != MA_SUCCESS, FAILED, "Failed to seek ");

	// int frame = ma_sound_get_time_in_pcm_frames(&_sound);
	// ma_sound_set_stop_time_in_milliseconds(&_sound, sr * seconds);
	// ERR_FAIL_COND_V_MSG(result != MA_SUCCESS, FAILED, "Failed to set Time ");

	// print_line("Res " + String::num(result));
	// print_line("SR " + String::num(sr) + " Frame " + String::num(sr * seconds) + " Index: " + String::num(frame));
	return OK;
}

short SoundSource::get_duration() {
	float duration;
	ma_result result;

	if (_is_loaded) {
		result = ma_sound_get_length_in_seconds(&_sound, &duration);
		ERR_FAIL_COND_V_MSG(result != MA_SUCCESS, FAILED, "Unable to get song duration. " + String::num(result));

		return (short)duration;
	}

	return 0;
}

void SoundSource::_bind_methods() {
	// ClassDB::bind_method(D_METHOD("SoundSource", "path", "name"), &SoundSource::SoundSource);

	ClassDB::bind_method(D_METHOD("load_file", "path", "name"), &SoundSource::load_file);
	ClassDB::bind_method(D_METHOD("play"), &SoundSource::play);
	ClassDB::bind_method(D_METHOD("pause"), &SoundSource::pause);
	ClassDB::bind_method(D_METHOD("stop"), &SoundSource::stop);
	ClassDB::bind_method(D_METHOD("seek", "ms"), &SoundSource::seek);
	ClassDB::bind_method(D_METHOD("is_playing"), &SoundSource::is_playing);
	ClassDB::bind_method(D_METHOD("get_duration"), &SoundSource::get_duration);
	ClassDB::bind_method(D_METHOD("get_current_time"), &SoundSource::get_current_time);

	ADD_SIGNAL(MethodInfo("time_update"));
}

SoundSource::SoundSource() {
}

SoundSource::~SoundSource() {
	ma_sound_uninit(&_sound);
}
