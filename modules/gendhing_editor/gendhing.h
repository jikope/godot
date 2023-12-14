#ifndef GENDHING_H
#define GENDHING_H

#include "core/ustring.h"
#include <map>

typedef std::map<int, char> InstrumentNotes;
typedef std::map<String, InstrumentNotes> Instruments;

class GendhingFormat {
public:
	GendhingFormat();
	virtual ~GendhingFormat();

private:
	Instruments _instruments;
};



#endif /* GENDHING_H */
