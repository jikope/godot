#ifndef GENDHING_H
#define GENDHING_H

#include "core/io/resource.h"
#include "core/io/resource_loader.h"
#include "core/io/resource_saver.h"
#include "core/object/object.h"
#include "core/string/ustring.h"
#include "core/variant/dictionary.h"
#include "editor/directory_create_dialog.h"
#include "modules/jikope_audio/gendhing_editor.h"

class Gendhing : public Resource
{
	GDCLASS(Gendhing, Resource);
	// RES_BASE_EXTENSION("bb");

	String _title;
	int _laras;
	Dictionary _left_notes;
	Dictionary _right_notes;
public:
	Error save(const String& p_path);

	void set_title(const String& title) { _title = title; }
	String get_title() { return _title; }

	void set_laras(int laras) { _laras = laras; }
	int get_laras() { return _laras; }

	void set_left_notes(const Dictionary& left_notes) { _left_notes = left_notes; }
	void set_right_notes(const Dictionary& right_notes) { _right_notes = right_notes; }
	Dictionary get_left_notes() { return _left_notes; }
	Dictionary get_right_notes() { return _right_notes; }

	Gendhing();
	~Gendhing() { };

protected:
	static void _bind_methods();
};

#endif /* GENDHING_H */
