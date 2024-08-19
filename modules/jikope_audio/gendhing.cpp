#include "gendhing.h"
#include "core/error/error_list.h"
#include "core/io/resource_saver.h"
#include "core/object/object.h"


void Gendhing::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_title", "title"), &Gendhing::set_title);
	ClassDB::bind_method(D_METHOD("get_title"), &Gendhing::get_title);

	ClassDB::bind_method(D_METHOD("set_laras", "laras"), &Gendhing::set_laras);
	ClassDB::bind_method(D_METHOD("get_laras"), &Gendhing::get_laras);

	ClassDB::bind_method(D_METHOD("set_left_notes", "left_notes"), &Gendhing::set_left_notes);
	ClassDB::bind_method(D_METHOD("get_left_notes"), &Gendhing::get_left_notes);
	ClassDB::bind_method(D_METHOD("set_right_notes", "right_notes"), &Gendhing::set_right_notes);
	ClassDB::bind_method(D_METHOD("get_right_notes"), &Gendhing::get_right_notes);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "_title"), "set_title", "get_title");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "_laras", PROPERTY_HINT_ENUM, "Slendro,Pelog"), "set_laras", "get_laras");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "_left_notes"), "set_left_notes", "get_left_notes");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "_right_notes"), "set_right_notes", "get_right_notes");

	ClassDB::bind_method(D_METHOD("save"), &Gendhing::save);
}

Error Gendhing::save(const String& p_path) {
	print_line("Saving Gendhing to " + p_path + ".tres");
	return ResourceSaver::save(this, p_path + ".tres");
}

Gendhing::Gendhing() {
}
