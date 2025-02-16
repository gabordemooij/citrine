#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include "lvgl/lvgl.h"
#include "../../citrine.h"
#include <media.h>
#include <gui.h>

uint16_t CtrGUIWidth = 800;
uint16_t CtrGUIHeight = 400;
ctr_object* guiObject;
ctr_object* packageObject;
ctr_object* CtrGUIAssetPackage;
ctr_object* colorObject;
ctr_object* fontObject;
lv_event_dsc_t* CtrEventHandler = NULL;

struct CtrGUIGlyphCacheEntry {
	uint32_t index;
	lv_font_glyph_dsc_t dsc;
	uint8_t* data;
};
typedef struct CtrGUIGlyphCacheEntry CtrGUIGlyphCacheEntry;


#define CTR_GUI_MAX_FONTS 10
#define CTR_GUI_GLYPHCACHE_MAX 250
CtrGUIGlyphCacheEntry ctr_gui_glyph_cache[CTR_GUI_GLYPHCACHE_MAX * CTR_GUI_MAX_FONTS];

struct GUIFNT {
	ctr_object* ref;
	char* name;
	char* path;
	int num;
	TTF_Font* font;
	lv_font_t* lvfont;
};
typedef struct GUIFNT GUIFNT;
GUIFNT GuiFNT[CTR_GUI_MAX_FONTS];
int maxFNT = CTR_GUI_MAX_FONTS;
int FNTCount = 0;


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

ctr_object* ctr_color_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	instance->link = myself;
	ctr_internal_object_property(instance, "r", ctr_build_number_from_float(0));
	ctr_internal_object_property(instance, "g", ctr_build_number_from_float(0));
	ctr_internal_object_property(instance, "b", ctr_build_number_from_float(0));
	ctr_internal_object_property(instance, "a", ctr_build_number_from_float(0));
	return instance;
}

/**
 * @def
 * [ Color ] rgbhex
 *
 * @example
 * >> gui := Gui new.
 * >> x := Color new red: 255 green: 255 blue: 255.
 * Out write: x rgbhex, stop.
 */
ctr_object* ctr_color_rgbhex(ctr_object* myself, ctr_argument* argumentList) {
	double r = ctr_tonum(ctr_internal_object_property(myself, "r", NULL));
	double g = ctr_tonum(ctr_internal_object_property(myself, "g", NULL));
	double b = ctr_tonum(ctr_internal_object_property(myself, "b", NULL));
	char* rgbhex = ctr_heap_allocate(9);
	sprintf(rgbhex, "0x%02x%02x%02x", (int)r, (int)g, (int)b);
	return ctr_build_string(rgbhex, strlen(rgbhex));
}

/**
 * @def
 * [ Color ] red: [Number] green: [Number] blue: [Number]
 *
 * @example
 * >> gui := Gui new.
 * >> x := Color new red: 100 green: 150 blue: 200.
 * Out write: x red, stop.
 * Out write: x green, stop.
 * Out write: x blue, stop.
 */
ctr_object* ctr_color_rgb_set(ctr_object* myself, ctr_argument* argumentList) {
	ctr_internal_object_property(myself, "r", ctr_internal_cast2number(argumentList->object));
	ctr_internal_object_property(myself, "g", ctr_internal_cast2number(argumentList->next->object));
	ctr_internal_object_property(myself, "b", ctr_internal_cast2number(argumentList->next->next->object));
	return myself;
}

ctr_object* ctr_color_a_set(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(myself, "a", ctr_internal_cast2number(argumentList->object));
}

ctr_object* ctr_color_r(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(myself, "r", NULL);
}

ctr_object* ctr_color_g(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(myself, "g", NULL);
}

ctr_object* ctr_color_b(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(myself, "b", NULL);
}

ctr_object* ctr_color_a(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(myself, "a", NULL);
}

void ctr_font_destructor(ctr_resource* rs) {
	/* todo */
}

ctr_object* ctr_font_new(ctr_object* myself, ctr_argument* argumentList) {
	if (FNTCount >= maxFNT) return CtrStdNil;
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	GUIFNT* fnt = &GuiFNT[FNTCount++];
	fnt->font = NULL;
	fnt->ref = instance;
	fnt->name = ctr_heap_allocate(10);
	fnt->path = NULL;
	fnt->num = FNTCount - 1;
	sprintf(fnt->name, "font%d", FNTCount-1);
	ctr_resource* rs = ctr_heap_allocate( sizeof(ctr_resource) );
	rs->ptr = fnt;
	rs->destructor = &ctr_font_destructor;
	instance->value.rvalue = rs;
	return instance;
}

GUIFNT* ctr_internal_get_font_from_object(ctr_object* object)	{
	if (object->value.rvalue == NULL) return NULL;
	return (GUIFNT*) object->value.rvalue->ptr;
}


bool ctr_internal_gui_describe_glyph(const lv_font_t * f, lv_font_glyph_dsc_t * glyph_dsc, uint32_t unicode, uint32_t unicode_letter_next) {
    CtrGUIGlyphCacheEntry* cache;
    int cache_index = (((GUIFNT*)f->dsc)->num * CTR_GUI_GLYPHCACHE_MAX) + (unicode % CTR_GUI_GLYPHCACHE_MAX);
    cache = &ctr_gui_glyph_cache[cache_index];
    if (cache->index == unicode) {
		*(glyph_dsc) = cache->dsc;
		return true;
	}
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* glyph_surface = TTF_RenderGlyph_Blended(((GUIFNT*)f->dsc)->font, (uint16_t)unicode, white);
	int w = glyph_surface->w;
    int h = glyph_surface->h;
    glyph_dsc->box_w = glyph_surface->w;
    glyph_dsc->box_h = glyph_surface->h;
    glyph_dsc->ofs_x = 0;
    glyph_dsc->ofs_y = -TTF_FontAscent(((GUIFNT*)f->dsc)->font);
    glyph_dsc->adv_w = glyph_surface->w;
    glyph_dsc->format= LV_FONT_GLYPH_FORMAT_A8;
    glyph_dsc->gid.index = unicode;
    cache->dsc = *glyph_dsc;
    cache->index = (uint32_t) unicode;
    cache->data = ctr_heap_allocate(h * w * sizeof(uint8_t));
    uint8_t* pixels = (uint8_t *)glyph_surface->pixels;
    int pitch = glyph_surface->pitch; // Bytes per row
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            uint32_t pixel = *(Uint32 *)(pixels + y * pitch + x * 4); // Read 32-bit pixel
            uint8_t alpha = (pixel >> 24) & 0xFF; // Extract alpha channel
            cache->data[y * w + x] = alpha;
        }
    }
    SDL_FreeSurface(glyph_surface);
    return true;
}

const void* ctr_internal_gui_render_glyph(lv_font_glyph_dsc_t* d, lv_draw_buf_t * buf) {
	GUIFNT* g = (GUIFNT*) d->resolved_font->dsc;
	TTF_Font* f = g->font;
	CtrGUIGlyphCacheEntry* cache;
    int cache_index = (g->num * CTR_GUI_GLYPHCACHE_MAX) + (d->gid.index % CTR_GUI_GLYPHCACHE_MAX);
    cache = &ctr_gui_glyph_cache[cache_index];
    if (cache->index == d->gid.index) {
		memcpy(buf->data, cache->data, (d->box_w * d->box_h * sizeof(uint8_t)));
		return buf;
	}
	ctr_error("Unexpected code path.",0);
}

/**
 * @def
 * [ Font ] new
 *
 * @example
 * gui font: (Font new source: ['font.ttf'] size: 20).
 *
 * @result
 * @info-font-source-size
 */
ctr_object* ctr_font_font(ctr_object* myself, ctr_argument* argumentList) {
	GUIFNT* fnt = ctr_internal_get_font_from_object(myself);
	if (fnt == NULL) return myself;
	char* path = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	SDL_RWops* res = ctr_internal_gui_load_asset(path, 1);
	ctr_heap_free(path);
	if (res == NULL) {
		ctr_error("Unable to load font.", 0);
		return myself;
	}
	fnt->font = TTF_OpenFontRW(res, 1, (int)ctr_internal_cast2number(argumentList->next->object)->value.nvalue);
	fnt->lvfont = ctr_heap_allocate(sizeof(lv_font_t));
	fnt->lvfont->get_glyph_dsc = ctr_internal_gui_describe_glyph;
	fnt->lvfont->get_glyph_bitmap = ctr_internal_gui_render_glyph;
	fnt->lvfont->line_height = TTF_FontHeight(fnt->font);
	fnt->lvfont->base_line = TTF_FontAscent(fnt->font);
	fnt->lvfont->dsc = fnt;
	fnt->lvfont->user_data = NULL;
	lv_xml_register_font(fnt->name, fnt->lvfont);
	return myself;
}

ctr_object* ctr_font_name(ctr_object* myself, ctr_argument* argumentList) {
	GUIFNT* fnt = ctr_internal_get_font_from_object(myself);
	if (fnt == NULL) return myself;
	return ctr_build_string( fnt->name, strlen(fnt->name) );
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

void ctr_internal_gui_init(void) {
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
	//g_font_manager = lv_font_manager_create(8);
	if (TTF_Init() < 0) {
		printf("TTF INIT FAIL\n");
	}
	for(int i = 0; i < CTR_GUI_GLYPHCACHE_MAX * CTR_GUI_MAX_FONTS; i++) {
		CtrGUIGlyphCacheEntry* cache = &ctr_gui_glyph_cache[i];
		cache->data = NULL;
		cache->index = 0;
	}
}

ctr_object* ctr_gui_screen(ctr_object* myself, ctr_argument* argumentList) {
    ctr_argument xml;
    ctr_argument rootname;
    ctr_argument rootnode;
    xml.object = argumentList->object;
	rootname.object = ctr_build_string_from_cstring("root");
	rootnode.object = CtrStdNil;
	xml.next = &rootname;
	rootname.next = &rootnode;
	ctr_gui_xml_at_set(myself, &xml);
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
	ctr_internal_gui_init();
	colorObject = ctr_color_new(CtrStdObject, NULL);
	colorObject->link = CtrStdObject;
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_color_new );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_RED_GREEN_BLUE_SET ), &ctr_color_rgb_set );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_TRANSPARENCY_SET ), &ctr_color_a_set );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_RED ), &ctr_color_r );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_GREEN ), &ctr_color_g );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_BLUE ), &ctr_color_b );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_TRANSPARENCY ), &ctr_color_a );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( "rgbhex" ), &ctr_color_rgbhex );
	fontObject = ctr_font_new(CtrStdObject, NULL);
	fontObject->link = CtrStdObject;
	ctr_internal_create_func(fontObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_font_new );
	ctr_internal_create_func(fontObject, ctr_build_string_from_cstring( CTR_DICT_SOURCE_SIZE_SET ), &ctr_font_font );
	ctr_internal_create_func(fontObject, ctr_build_string_from_cstring( CTR_DICT_NAME ), &ctr_font_name );
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
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_FONT_OBJECT ), fontObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_COLOR_OBJECT ), colorObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Gui" ), guiObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}

void init_embedded_gui_plugin() {
	begin();
}