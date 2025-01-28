#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "lvgl/lvgl.h"
#include "../../citrine.h"
#include <media.h>
#include <gui.h>

uint16_t CtrGUIWidth = 800;
uint16_t CtrGUIHeight = 400;
ctr_object* guiObject;




void ctr_gui_internal_event_handler(lv_event_t* e) {
	ctr_argument* arguments;
	char* message;
	
	int event_code = lv_event_get_code(e);
	char* event_name = lv_event_code_get_name(lv_event_get_code(e));
	lv_obj_t* target = lv_event_get_target(e);
	printf("---- event: %s \n", event_name);
	uint32_t id = (uint32_t) lv_obj_get_id(target);
	if (id != NULL) {
		printf("id %p \n", id);
		arguments = ctr_heap_allocate(sizeof(ctr_argument));
		arguments->object = ctr_build_number_from_float( (double) id );
		arguments->next = NULL;
		message = "???";
		if (event_code == LV_EVENT_CLICKED) {
			message = CTR_DICT_ON_CLICK;
		}
		ctr_send_message(guiObject, message, strlen(message), arguments);
		ctr_heap_free(arguments);
		//ctr_heap_free(message);
	}
}

void ctr_gui_destructor(ctr_resource* rs) {
	return;
}

ctr_object* ctr_gui_new(ctr_object* myself, ctr_argument* argumentList) {
	printf("Nieuwe GUI!\n");
	if (guiObject != NULL) {
		printf("singleton!\n");
		return guiObject;
	}
	printf("guiObject @ %p \n",  &guiObject);
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
	printf("XML change: %d at: %p name: %s\n", (int) strlen(xml), child, name);
	lv_xml_component_register_from_data(name, xml);
	lv_xml_create(child, name, NULL);
	ctr_heap_free(xml);
	ctr_heap_free(name);
	return myself;
}


ctr_object* ctr_gui_screen(ctr_object* myself, ctr_argument* argumentList) {
	printf("Start GUI!\n");
	lv_init();
    printf("sdl\n");
    lv_sdl_window_create(CtrGUIWidth, CtrGUIHeight);
    printf("created window\n");
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
	printf("startbericht zenden\n");
	ctr_argument* arguments = ctr_heap_allocate(sizeof(ctr_argument));
	arguments->object = CtrStdNil;
	ctr_send_message(guiObject, CTR_DICT_RUN, strlen(CTR_DICT_RUN), arguments);
	ctr_heap_free(arguments);
	printf("startbericht verzonden\n");
    /*Handle LVGL tasks*/
    while(1) {
		idle_time = lv_timer_handler(); /*Returns the time to the next timer execution*/
        usleep(idle_time * 1000);
    }
	return myself;
}

void begin() {
	guiObject = NULL;
	guiObject = ctr_gui_new(CtrStdObject, NULL);
	guiObject->link = CtrStdObject;
	ctr_internal_create_func(guiObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_gui_new );
	ctr_internal_create_func(guiObject, ctr_build_string_from_cstring( CTR_DICT_XML_NAME_AT_SET ), &ctr_gui_xml_at_set );
	ctr_internal_create_func(guiObject, ctr_build_string_from_cstring( CTR_DICT_WIDTH_HEIGHT_SET ), &ctr_gui_width_height_set );
	ctr_internal_create_func(guiObject, ctr_build_string_from_cstring( CTR_DICT_SCREEN ), &ctr_gui_screen );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_GUI_PLUGIN_ID ), guiObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Gui" ), guiObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}
