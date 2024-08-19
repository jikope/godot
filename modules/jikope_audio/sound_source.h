#ifndef SOUND_SOURCE_H
#define SOUND_SOURCE_H


#include "core/object/object.h"
#include "core/object/ref_counted.h"
#include "libs/miniaudio_split/miniaudio.h"
#include <cstdint>

class SoundSource : public RefCounted
{
	GDCLASS(SoundSource, RefCounted)
public:
	Error load_file(const String &path, const String &name);
	Error play();
	Error pause();
	Error stop();
	Error seek(uint64_t milliseconds);

	short get_duration();
	uint64_t get_current_time(); // return ms

	bool is_playing();
	bool is_loaded();

	void player_callback();

    SoundSource();
    ~SoundSource();
private:
	ma_sound _sound;
	String _name;
	bool _is_playing = false;
	bool _is_loaded = false;

protected:
	static void _bind_methods();
};


#endif /* SOUND_SOURCE_H */
