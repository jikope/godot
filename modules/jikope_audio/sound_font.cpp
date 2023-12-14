#include "sound_font.h"
#include "core/error_list.h"
#include "core/error_macros.h"
#include "core/io/resource_saver.h"
#include "core/os/file_access.h"
#include "core/pool_vector.h"


void SoundFont::_bind_methods() {
	ClassDB::bind_method(D_METHOD("load"), &SoundFont::load);
	ClassDB::bind_method(D_METHOD("get_data"), &SoundFont::get_data);
	ClassDB::bind_method(D_METHOD("set_data", "data"), &SoundFont::set_data);
	ADD_PROPERTY(PropertyInfo(Variant::POOL_BYTE_ARRAY, "data", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NOEDITOR), "set_data", "get_data");
}

Error SoundFont::load(const String fileName) {
	String ext = fileName.get_extension().to_lower();
	if (ext != "sf2" && ext != "sf3") {
		ERR_FAIL_COND_V("Expected SoundFont file (sf2).", FAILED);
	}

	FileAccess *file = FileAccess::open(fileName, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(!file, FAILED, "Couldn't open soundfont file " + fileName + ".");

	int size = file->get_len();
	// print_line("Size ", size);
	// print_line(vformat("Size of file %d bytes", size));

	PoolVector<uint8_t> data;
	data.resize(size);

	PoolVector<uint8_t>::Write writer = data.write();
	int theReadSize = file->get_buffer(&writer[0], size);

	if (theReadSize < size) {
		data.resize(size);
	}

	ERR_FAIL_COND_V_MSG(theReadSize == 0, FAILED, "Could't read soundfont file.");

	set_data(data);
	memdelete(file);

	return OK;
}


PoolVector<uint8_t> SoundFont::get_data() const {
	PoolVector<uint8_t> data_ref;
	if (data_len && data) {
		data_ref.resize(data_len);
		{
			PoolVector<uint8_t>::Write writer = data_ref.write();
			memcpy(writer.ptr(), data, data_len);
		}
	}
	return data_ref;
};

void SoundFont::set_data(const PoolVector<uint8_t> &p_data) {
	data_len = p_data.size();
	data = memalloc(data_len);
	memcpy(data, p_data.read().ptr(), data_len);
}


// Importer
void SoundFontImporter::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("sf2");
	p_extensions->push_back("sf3");
}

Error SoundFontImporter::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	Ref<SoundFont> sf;
	sf.instance();
	sf->load(p_source_file);

	return ResourceSaver::save(p_save_path + ".msf2", sf);

	return OK;
}
