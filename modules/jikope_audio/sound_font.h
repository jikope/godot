#ifndef SOUND_FONT_H
#define SOUND_FONT_H

#include "core/error/error_list.h"
#include "core/io/resource.h"
#include "core/io/resource_importer.h"
#include "core/object/object.h"
#include "core/variant/variant.h"
#include <cstdint>

class SoundFont : public Resource {
	GDCLASS(SoundFont, Resource);
	RES_BASE_EXTENSION("msf2");

private:
	void* data;
	uint32_t len;

public:
	Error load(const String filename);

	void set_data(const PackedByteArray &data);
	PackedByteArray get_data() const;

	SoundFont();
	~SoundFont() { }
};


class SoundFontImporter : public ResourceImporter {
	GDCLASS(SoundFontImporter, ResourceImporter);

public:
	String get_importer_name() const override { return "soundfont"; };
	String get_visible_name() const override { return "SoundFont"; };
	String get_save_extension() const override { return "msf2"; };
	String get_resource_type() const override { return "SoundFont"; };
	void get_recognized_extensions(List<String> *p_extensions) const override {
		p_extensions->push_back("sf2");
		p_extensions->push_back("sf3");
	};

	float get_priority() const override { return 1.0; }
	int get_import_order() const override { return IMPORT_ORDER_DEFAULT; }
	int get_format_version() const override { return 0; }

	int		get_preset_count() const override { return 0; }
	String	get_preset_name(int p_idx) const override { return String(); }

	void get_import_options(const String &p_path, List<ImportOption> *r_options, int p_preset = 0) const override { };
	bool get_option_visibility(const String &p_path, const String &p_option, const HashMap<StringName, Variant> &p_options) const override { return true; };

	Error import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files = nullptr, Variant *r_metadata = nullptr) override;

};

#endif /* SOUND_FONT_H */
