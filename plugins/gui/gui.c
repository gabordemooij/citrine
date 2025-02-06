#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL.h>


#include "lvgl/lvgl.h"
#include "../../citrine.h"
#include <media.h>
#include <gui.h>

uint16_t CtrGUIWidth = 800;
uint16_t CtrGUIHeight = 400;
ctr_object* guiObject;
ctr_object* packageObject;
ctr_object* CtrGUIAssetPackage;


SDL_RWops* ctr_internal_gui_load_asset(char* asset_name, char asset_type) {
	SDL_RWops* res = NULL;
	// If we have no asset package, load from file instead
	if (CtrGUIAssetPackage == NULL) {
		res = SDL_RWFromFile(asset_name, "rb");
		return res;
	}
	char* path = ctr_heap_allocate_cstring(ctr_internal_object_property(CtrGUIAssetPackage, "path", NULL));
	SDL_RWops* asset_file = SDL_RWFromFile(path, "rb");
	ctr_heap_free(path);
	if (!asset_file) {
		unsigned int blob_len;
		void* blob;
		//No asset file? then maybe embedded data blob?
		blob = ctr_data_blob(&blob_len);
		if (blob == NULL) {
			return NULL;
		}
		asset_file = SDL_RWFromMem(blob, blob_len);
	}
	SDL_RWseek(asset_file, 0, RW_SEEK_SET);
	char* buffer = ctr_heap_allocate(500);
	while(1) {
		uint64_t read_start = SDL_RWtell(asset_file);
		int bytes_read = SDL_RWread(asset_file, buffer, 1, 500);
		if (strncmp(asset_name, buffer, bytes_read) == 0) {
			SDL_RWseek(asset_file, read_start + strlen(asset_name) + 1, RW_SEEK_SET);
			uint64_t next_entry = 0;
			SDL_RWread(asset_file, &next_entry, 8, 1);
			uint64_t curpos = SDL_RWtell(asset_file);
			uint64_t read_size = next_entry - curpos;
			char* read_buffer = malloc(read_size);
			SDL_RWread(asset_file, read_buffer, 1, read_size);
			res = SDL_RWFromMem(read_buffer, read_size);
			break;
		} else {
			char* boundary = strchr(buffer, 0);
			uint64_t jmp_address = *((uint64_t*) (boundary+1));
			if (jmp_address == 0) {
				break;
			}
			SDL_RWseek(asset_file, jmp_address, SEEK_SET);
		}
	}
	SDL_RWclose(asset_file);
	ctr_heap_free(buffer);
	return res;
}

void ctr_gui_internal_event_handler(lv_event_t* e) {
	ctr_argument* arguments;
	char* message;
	
	int event_code = lv_event_get_code(e);
	char* event_name = lv_event_code_get_name(lv_event_get_code(e));
	lv_obj_t* target = lv_event_get_target(e);
	uint32_t id = (uint32_t) lv_obj_get_id(target);
	if (id != NULL) {
		arguments = ctr_heap_allocate(sizeof(ctr_argument));
		arguments->object = ctr_build_number_from_float( (double) id );
		arguments->next = NULL;
		message = "???";
		if (event_code == LV_EVENT_CLICKED) {
			message = CTR_DICT_ON_CLICK;
		}
		ctr_send_message(guiObject, message, strlen(message), arguments);
		ctr_heap_free(arguments);
	}
}

void ctr_gui_destructor(ctr_resource* rs) {
	return;
}

ctr_object* ctr_gui_new(ctr_object* myself, ctr_argument* argumentList) {
	if (guiObject != NULL) {
		return guiObject;
	}
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	ctr_resource* rs = ctr_heap_allocate( sizeof(ctr_resource) );
	rs->ptr = NULL;
	rs->destructor = &ctr_gui_destructor;
	instance->link = myself;
	instance->value.rvalue = rs;
	instance->info.sticky = 1; // just in case, dont let gc clean it up
	return instance;
}


ctr_object* ctr_gui_width_height_set(ctr_object* myself, ctr_argument* argumentList) {
	CtrGUIWidth = ctr_tonum(argumentList->object);
	CtrGUIHeight = ctr_tonum(argumentList->next->object);
	return myself;
}


lv_event_dsc_t* CtrEventHandler = NULL;


ctr_object* ctr_gui_xml_at_set(ctr_object* myself, ctr_argument* argumentList) {
	lv_obj_t* root = lv_screen_active();
	char* xml = ctr_heap_allocate_cstring(argumentList->object);
	char* name = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->next->object));
	uint32_t id = ctr_tonum(argumentList->next->next->object);
	lv_obj_t* child = lv_obj_get_child_by_id(root, &id);
	if (CtrEventHandler == NULL) {
		CtrEventHandler = lv_obj_add_event_cb(root, &ctr_gui_internal_event_handler, LV_EVENT_ALL, NULL);
	}
	if (!child) child = root;
	uint32_t n = lv_obj_get_child_count(child);
	for(int i = 0; i < n; i++) {
		lv_obj_t* old = lv_obj_get_child(child, i);
		lv_obj_delete(old);
	}
	lv_xml_component_register_from_data(name, xml);
	lv_xml_create(child, name, NULL);
	ctr_heap_free(xml);
	ctr_heap_free(name);
	return myself;
}


ctr_object* ctr_gui_screen(ctr_object* myself, ctr_argument* argumentList) {
	lv_init();
    lv_sdl_window_create(CtrGUIWidth, CtrGUIHeight);
    lv_group_t * g = lv_group_create();
    lv_group_set_default(g);
    lv_indev_t* mouse = lv_sdl_mouse_create();
    lv_indev_set_group(mouse, lv_group_get_default());
	lv_indev_t * mousewheel = lv_sdl_mousewheel_create();
    lv_indev_set_group(mousewheel, lv_group_get_default());
    lv_indev_t * keyboard = lv_sdl_keyboard_create();
    lv_indev_set_group(keyboard, lv_group_get_default()); 
    ctr_argument xml;
    ctr_argument rootname;
    ctr_argument rootnode;
    xml.object = argumentList->object;
	rootname.object = ctr_build_string_from_cstring("root");
	rootnode.object = CtrStdNil;
	xml.next = &rootname;
	rootname.next = &rootnode;
	ctr_gui_xml_at_set(myself, &xml);
	//ctr_heap_free(rootname.object);
    uint32_t idle_time;
	ctr_argument* arguments = ctr_heap_allocate(sizeof(ctr_argument));
	arguments->object = CtrStdNil;
	ctr_send_message(guiObject, CTR_DICT_RUN, strlen(CTR_DICT_RUN), arguments);
	ctr_heap_free(arguments);
    /*Handle LVGL tasks*/
    while(1) {
		idle_time = lv_timer_handler(); /*Returns the time to the next timer execution*/
        usleep(idle_time * 1000);
    }
	return myself;
}

/**
 * @internal
 *
 * 'DataStart'
 * Exports can use this message to bootstrap from a data package.
 * The package is often part of the executable or the distribution.
 * This will connect to a data package called 'data' and include
 * program '__1__' for execution.
 */
ctr_object* ctr_gui_datastart(ctr_object* myself, ctr_argument* none) {
	ctr_argument* argumentList;
	ctr_object* data_package;
	argumentList = (ctr_argument*) ctr_heap_allocate(sizeof(ctr_argument));
	argumentList->object = ctr_build_string_from_cstring("data");
	argumentList->next = NULL;
	// Create an asset package for 'data'
	data_package = ctr_send_message( packageObject, CTR_DICT_NEW_SET, strlen(CTR_DICT_NEW_SET), argumentList );
	argumentList->object = data_package;
	// Connect the assets to the program
	ctr_send_message( guiObject, CTR_DICT_LINK_SET, strlen(CTR_DICT_LINK_SET), argumentList );
	argumentList->object = ctr_build_string_from_cstring("__1__");
	// Load the __1__ program from the data package
	ctr_send_message( guiObject, "use:", strlen("use:"), argumentList );
	ctr_heap_free(argumentList);
	return myself;
}



ctr_object* ctr_gui_include(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* pathStrObj = ctr_internal_cast2string(argumentList->object);
	char* pathString = ctr_heap_allocate_tracked(pathStrObj->value.svalue->vlen + 1);
	strncpy(pathString, pathStrObj->value.svalue->value, pathStrObj->value.svalue->vlen);
	pathString[pathStrObj->value.svalue->vlen] = '\0';
	SDL_RWops* asset_reader = ctr_internal_gui_load_asset(pathString, 1);
	if (!asset_reader) {
		ctr_error("Unable to open code asset.", 0);
		return CtrStdNil;
	}
	char* prg;
	int prg_id;
	int chunk = 512;
	size_t bytes_read;
	size_t offset = 0;
	ctr_tnode* parsedCode;
	prg = ctr_heap_allocate_tracked(chunk);
	prg_id = ctr_heap_get_latest_tracking_id();
	bytes_read = SDL_RWread(asset_reader, prg, 1, chunk);
	offset += bytes_read;
	while (bytes_read > 0) {
		prg = ctr_heap_reallocate_tracked(prg_id, offset + chunk + 1);
		bytes_read = SDL_RWread(asset_reader, prg + offset, 1, chunk);
        offset += bytes_read;
    }
    SDL_RWclose(asset_reader);
    prg[offset + 1] = '\0';
    ctr_program_length = offset;
	parsedCode = ctr_cparse_parse(prg, pathString);
	ctr_cwlk_subprogram++;
	ctr_cwlk_run(parsedCode);
	ctr_cwlk_subprogram--;
	return myself;
}

ctr_object* ctr_package_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	instance->link = myself;
	return instance;
}

ctr_object* ctr_package_new_set(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_package_new(myself, argumentList);
	ctr_internal_object_property(instance, "path", ctr_internal_copy2string(argumentList->object));
	return instance;
}

ctr_object* ctr_gui_link_package(ctr_object* myself, ctr_argument* argumentList) {
	if (argumentList->object->link != packageObject) {
		ctr_error("Not an asset package.\n", 0);
	}
	CtrGUIAssetPackage = argumentList->object;
	return myself;
}

void begin() {
	CtrGUIAssetPackage = NULL;
	packageObject = ctr_package_new(CtrStdObject, NULL);
	packageObject->link = CtrStdObject;
	ctr_internal_create_func(packageObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_package_new );
	ctr_internal_create_func(packageObject, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_package_new_set );
	guiObject = NULL;
	guiObject = ctr_gui_new(CtrStdObject, NULL);
	guiObject->link = CtrStdObject;
	ctr_internal_create_func(guiObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_gui_new );
	ctr_internal_create_func(guiObject, ctr_build_string_from_cstring( CTR_DICT_XML_NAME_AT_SET ), &ctr_gui_xml_at_set );
	ctr_internal_create_func(guiObject, ctr_build_string_from_cstring( CTR_DICT_WIDTH_HEIGHT_SET ), &ctr_gui_width_height_set );
	ctr_internal_create_func(guiObject, ctr_build_string_from_cstring( CTR_DICT_LINK_SET ), &ctr_gui_link_package );
	ctr_internal_create_func(guiObject, ctr_build_string_from_cstring( CTR_DICT_SCREEN ), &ctr_gui_screen );
	ctr_internal_create_func(guiObject, ctr_build_string_from_cstring( "use:" ), &ctr_gui_include );
	if (strcmp(CTR_DICT_USE_SET,"use:")!=0) {
		ctr_internal_create_func(guiObject, ctr_build_string_from_cstring( CTR_DICT_USE_SET ), &ctr_gui_include );
	}
	ctr_internal_create_func(guiObject, ctr_build_string_from_cstring( "_datastart" ), &ctr_gui_datastart );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_GUI_PLUGIN_ID ), guiObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Gui" ), guiObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}

void init_embedded_gui_plugin() {
	begin();
}