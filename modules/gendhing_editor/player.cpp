#include "player.h"
#include "core/class_db.h"
#include "core/engine.h"
#include "core/object.h"
#include "core/print_string.h"
#include "modules/jikope_audio/jikope_audio.h"
#include "scene/main/timer.h"
#include <chrono>
#include <string>
#include <thread>

typedef std::map<char, int> InstrumentMap;
InstrumentMap BALUNGAN_NOTE_MAP = {
	{'1', 71},
	{'2', 72},
	{'3', 73},
	{'5', 75},
	{'6', 76},
	{'!', 77}
};

InstrumentMap BONANG_BARUNG_NOTE_MAP = {
	{'2', 82},
	{'3', 83},
	{'5', 85},
	{'6', 86},
	{'!', 91},
	{'@', 92},
	{'#', 93},
	{'%', 95},
	{'^', 96},
	{'x', 101},
};

std::map<String, InstrumentMap> INSTRUMENTS_MAP = {
	{"BALUNGAN", BALUNGAN_NOTE_MAP},
	{"BONANG_BARUNG", BONANG_BARUNG_NOTE_MAP},
};


void Player::seek(int ms) {
	_start_time = ms;
}

void Player::get_track() {}
void Player::set_track(double max_time, Instruments track) {
	_max_time = max_time;
	_track = track;
}

void Player::play_track() {
	if (!check_audio_engine() || _is_playing) {
		return;
	}

	std::thread play(&Player::instrument_callback, this);
	play.detach();
	_is_playing = true;
	_audio->start_clock();
}

void Player::instrument_callback() {
	// std::map<int, char>
	int max = _max_time;
	Instruments track_copy = _track;

	if (_start_time > 0) {
		for (auto instrument : track_copy) {
			for (auto note : instrument.second) {
				if (note.first < _start_time) {
					// instrument.second.erase(note.first);
					track_copy.at(instrument.first).erase(note.first);
				} else {
					break;
				}
			}
		}
	}

	while (true) {
		double now = get_clock();

		if (!_is_playing) {
			break;
		}
		if (now >= max) {
			_is_playing = false;
			break;
		}

		for (auto instrument : track_copy) {
			std::map<int, char>* notes = &instrument.second;
			if (notes->size() == 0) {
				continue;
			}
			auto first_note = notes->begin();
			if (first_note == notes->end()) {
				continue;
			}

			if (first_note->first <= now) {
				String instrument_name = instrument.first;
				int timestamp = first_note->first;
				char note = first_note->second;

				std::map<char, int>::iterator midi_key = INSTRUMENTS_MAP.at(instrument_name).find(note);

				if (midi_key == INSTRUMENTS_MAP.at(instrument_name).end()) {
					String err = "ERROR: There is no note " + String::chr(note) + " in " + instrument_name + " instrument";
					print_error(err);
					_is_playing = false;
					goto end;
				}

				_audio->midi_note_on(instrument_name, midi_key->second, 110);
				std::thread note_off_thread (&Player::instrument_note_off, this, 800, instrument_name, midi_key->second);
				note_off_thread.detach();

				// printf("time %f - offset %d - playing %c - instrument %ls\n", now, first_note->first, first_note->second, instrument.first.c_str());
				track_copy.at(instrument_name).erase(timestamp);
			}
		}
		emit_signal("time_update");
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

 end:
	reset_track();
}

void Player::pause_track() {}
void Player::stop_track() {
	_is_playing = false;
}
void Player::reset_track() {
	for (auto instrument : _track) {
		_audio->midi_note_off_all(instrument.first);
	}
	emit_signal("track_end");
}

void Player::instrument_note_off(int delay_ms, String instrument, int key) {
	std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
	_audio->midi_note_off(instrument, key);
}

double Player::get_clock() {
	if (_is_playing) {
		return _audio->get_current_clock() + _start_time;
	}
	return 0;
}
void Player::get_duration() {}

bool Player::check_audio_engine() {
	if (_audio == nullptr) {
		return false;
	}
	return true;
}

void Player::_bind_methods() {
	ClassDB::bind_method(D_METHOD("instrument_note_off"), &Player::instrument_note_off);
	ClassDB::bind_method(D_METHOD("instrument_callback"), &Player::instrument_callback);
	ADD_SIGNAL(MethodInfo("time_update"));
	ADD_SIGNAL(MethodInfo("track_end"));
}

Player::Player() {
	if (Engine::get_singleton()->has_singleton("JikopeAudio")) {
		_audio = Object::cast_to<JikopeAudio>(Engine::get_singleton()->get_singleton_object("JikopeAudio"));
	}
}
Player::~Player() {}
