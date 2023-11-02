#ifndef SOUND_FONT_H
#define SOUND_FONT_H

#include "core/class_db.h"
#include "core/io/resource_importer.h"
#include "core/resource.h"
#include <cstdint>

// Register sf2 file for godot to understand
// References
// https://github.com/goblinengine/goblin/blob/main/modules/goblin/src/midi_player.h

class SoundFont : public Resource {
	GDCLASS(SoundFont, Resource);
	OBJ_CATEGORY("Resources");
	RES_BASE_EXTENSION("msf2");

	void* data = nullptr;
	uint32_t data_len = 0;
public:
	Error load(const String fileName);

	void set_data(const PoolVector<uint8_t> &p_data);
	PoolVector<uint8_t> get_data() const;

	SoundFont() {}
	~SoundFont() {}
protected:
	static void _bind_methods();

};

class SoundFontImporter : public ResourceImporter {
	GDCLASS(SoundFontImporter, ResourceImporter);

public:
	virtual String	get_save_extension() const { return "msf2"; }
	virtual String	get_visible_name() const { return "SoundFont"; }
	virtual String	get_resource_type() const { return "SoundFont"; }
	virtual String	get_importer_name() const { return "soundfont"; }
	virtual void	get_recognized_extensions(List<String> *p_extensions) const;

	virtual int		get_preset_count() const { return 0; }
	virtual String	get_preset_name(int p_idx) const { return String(); }

	virtual void	get_import_options(List<ImportOption> *r_options, int p_preset = 0) const { }
	virtual bool	get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const { return true; }

	virtual Error import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files = nullptr, Variant *r_metadata = nullptr);

	SoundFontImporter() {};
};

#endif /* SOUND_FONT_H */
