#include "register_types.h"

#include "core/class_db.h"
#include "core/engine.h"
#include "jikope_audio.h"
#include "sound_font.h"

static JikopeAudio *jikope_ptr;

void register_jikope_audio_types() {
    ClassDB::register_class<JikopeAudio>();
	jikope_ptr = memnew(JikopeAudio);
	Engine::get_singleton()->add_singleton(Engine::Singleton("JikopeAudio", JikopeAudio::get_singleton()));

    ClassDB::register_class<SoundFont>();

	if (Engine::get_singleton()->is_editor_hint()) {
		// jikope_ptr->initialize();
		Ref<SoundFontImporter> sf_importer;
		sf_importer.instance();
		ResourceFormatImporter::get_singleton()->add_importer(sf_importer);
	}
}

void unregister_jikope_audio_types() {
	memdelete(jikope_ptr);
}
