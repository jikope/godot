#include "register_types.h"

#include "core/class_db.h"
#include "gendhing_editor.h"
#include "gendhing_roll_editor.h"


void register_gendhing_editor_types() {
    ClassDB::register_class<Player>();
    ClassDB::register_class<GendhingEditor>();
    ClassDB::register_class<GendhingRollEditor>();
}

void unregister_gendhing_editor_types() {
}
