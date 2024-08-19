#include "sound_font.h"
#include "core/string/print_string.h"


SoundFont::SoundFont() {
	ClassDB::bind_method(D_METHOD("load"), &SoundFont::load);
	ClassDB::bind_method(D_METHOD("get_data"), &SoundFont::get_data);
	ClassDB::bind_method(D_METHOD("set_data", "data"), &SoundFont::set_data);
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_BYTE_ARRAY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NO_EDITOR), "set_data", "get_data");
}

Error SoundFont::load(const String fileName) {
	print_line("Loading Sounf Font");

	String ext = fileName.get_extension().to_lower();
	if (ext != "sf2" && ext != "sf3") {
		ERR_FAIL_COND_V("Expected SoundFont file (sf2).", FAILED);
	}

	Ref<FileAccess> file = FileAccess::open(fileName, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(file.is_null(), FAILED, "Couldn't open soundfont file " + fileName + ".");

	int size = file->get_length();

	PackedByteArray data;
	data.resize(size);

	uint64_t theReadSize = file->get_buffer(data.ptrw(), size);

	if (theReadSize < size) {
		data.resize(size);
	}

	ERR_FAIL_COND_V_MSG(theReadSize == 0, FAILED, "Could't read soundfont file.");

	set_data(data);

	return OK;
}

PackedByteArray SoundFont::get_data() const {
	PackedByteArray data_ref;

	if (len && data) {
		data_ref.resize(len);

		{
			memcpy(data_ref.ptrw(), data, len);
		}
	}

	return data_ref;
};

void SoundFont::set_data(const PackedByteArray &p_data) {
	len = p_data.size();
	data = memalloc(len);
	memcpy(data, p_data.ptr(), len);
}


Error SoundFontImporter::import(const String &p_source_file, const String &p_save_path, const HashMap<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	Ref<SoundFont> sf;
	sf.instantiate();
	sf->load(p_source_file);

	return ResourceSaver::save(sf, p_save_path + ".msf2");

	return OK;
}
