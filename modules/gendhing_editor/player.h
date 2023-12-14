#ifndef PLAYER_H
#define PLAYER_H

#include "core/object.h"
#include "gendhing.h"
#include "modules/jikope_audio/jikope_audio.h"
#include <atomic>

class Player : public Object {
	GDCLASS(Player, Object)
public:
	void get_track();
	void set_track(double max_time, Instruments track);

	void play_track();
	void pause_track();
	void stop_track();
	void reset_track();

	void seek(int ms);
	double get_clock();
	void get_duration();
	bool check_audio_engine();

	void instrument_callback();
	void instrument_note_off(int delay_ms, String instrument, int key);

    Player();
    virtual ~Player();

	bool _is_playing = false;
private:
	double _start_time = 0.0;
	int _max_time;
	Instruments _track;
	JikopeAudio* _audio;

protected:
	static void _bind_methods();
};

#endif /* PLAYER_H */
