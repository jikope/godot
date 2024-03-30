#include "register_types.h"

#include "core/object/class_db.h"
#include "core/config/engine.h"
// #include "sound_font.h"
#include "jikope_audio.h"
#include "modules/jikope_audio/gendhing_editor.h"
#include "sound_source.h"

static JikopeAudio *jikope_ptr;

void initialize_jikope_audio_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	GDREGISTER_CLASS(JikopeAudio);
	jikope_ptr = memnew(JikopeAudio);
	jikope_ptr->initialize();
	Engine::get_singleton()->add_singleton(Engine::Singleton("Jikope", jikope_ptr));

	ClassDB::register_class<SoundSource>();
	ClassDB::register_class<GendhingEditor>();

    // ClassDB::register_class<SoundFont>();

	// if (Engine::get_singleton()->is_editor_hint()) {
		// jikope_ptr->initialize();
		// Ref<SoundFontImporter> sf_importer;
		// sf_importer.instance();
		// ResourceFormatImporter::get_singleton()->add_importer(sf_importer);
	// }
}

void uninitialize_jikope_audio_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	memdelete(jikope_ptr);
}
