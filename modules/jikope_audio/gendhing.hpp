#ifndef GENDHING_H
#define GENDHING_H

enum NoteType {
	TEMPO_CHANGE,
	NOTE_ON
};

class NoteEvent {
public:
	NoteType GetType() { return type; };
private:
	NoteType type;
};

class NoteOnEvent : public NoteEvent {

};

class Gendhing {

};

#endif /* GENDHING_H */
