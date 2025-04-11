#ifdef LIBSECRET
#include <libsecret/secret.h>
#endif

#ifdef WINPASS
#include <windows.h>
#include <wincrypt.h>
#include <shlobj.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gui.h>
#include "../../citrine.h"

/* Linux libsecret implementation */
#ifdef LIBSECRET
SecretSchema ctr_gui_vault_schema;
ctr_gui_internal_vault_init() {
	ctr_gui_vault_schema.name = "com.xoscript.secretschema";
	ctr_gui_vault_schema.flags = SECRET_SCHEMA_NONE;
	ctr_gui_vault_schema.attributes[0].name = "lookup_name";
	ctr_gui_vault_schema.attributes[0].type = SECRET_SCHEMA_ATTRIBUTE_STRING;
	ctr_gui_vault_schema.attributes[1].name = NULL;
	ctr_gui_vault_schema.attributes[1].type = 0;
}

int ctr_gui_vault_platform_store(char* vault_name, char* lookup_name, char* password) {
	GError *error = NULL;
	gboolean result = secret_password_store_sync(
        &ctr_gui_vault_schema,
        SECRET_COLLECTION_DEFAULT,
        vault_name,
        password,
        NULL, // GCancellable
        &error,
        "lookup_name",
        lookup_name,
        NULL
    );
     if (error) {
		g_error_free(error);
		return -1;
	}
    return 0;
}

int ctr_gui_vault_platform_retrieve(char* vault_name, char* lookup_name, char** password) {
	GError *error = NULL;
	char *p = secret_password_lookup_sync(
        &ctr_gui_vault_schema,
        NULL, // GCancellable
        &error,
        "lookup_name", lookup_name,
        NULL
    );
    if (error) {
		g_error_free(error);
		return -1;
	}
	*(password) = p;
	return 0;
}

void ctr_gui_vault_platform_destroy(char** password) {
	secret_password_free(*password);
}
#endif


/* Win64 implementation */
#ifdef WINPASS

void ctr_gui_internal_vault_init() {
	/* not needed for win64 */
}

int ctr_gui_vault_platform_store(char* vault_name, char* lookup_name, char* password) {
	char path[MAX_PATH];
	HRESULT success;
	DATA_BLOB DataIn;
    DATA_BLOB DataOut;
    DWORD dwError;
	DataIn.pbData = (BYTE*) password;
    DataIn.cbData = strlen(password) + 1;
    int error = 1;
    if (CryptProtectData(&DataIn, password, NULL, NULL, NULL, 0, &DataOut)) { //DPAPI
        success = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path); // AppData\Roaming
		if (success == S_OK) {
			strcat(path, "\\");
			strcat(path, vault_name);
			strcat(path, "\\");
			CreateDirectory(path, NULL);  // Create the directory if it doesn't exist
			strcat(path, lookup_name);
			FILE* file = fopen(path, "wb");
			if (file) {
				fwrite(DataOut.pbData, DataOut.cbData, 1, file);
				fclose(file);
				error = 0;
			}
		}
		LocalFree(DataOut.pbData);
	}
	return error;
}

int ctr_gui_vault_platform_retrieve(char* vault_name, char* lookup_name, char** password) {
	DATA_BLOB DataIn;
    DATA_BLOB DataOut;
    DWORD dwError;
    char path[MAX_PATH];
	HRESULT success;
	int error = 1;
	int bytes_read = 0;
    DataIn.pbData = (BYTE*)ctr_heap_allocate(1000);
    DataIn.cbData = 0;
    *password = NULL;
	success = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path); // AppData\Roaming
	if (success == S_OK) {
		strcat(path, "\\");
		strcat(path, vault_name);
		strcat(path, "\\");
		strcat(path, lookup_name);
		FILE* file = fopen(path, "rb");
		if (file) {
			DataIn.cbData = fread(DataIn.pbData, 1, 1000, file);
			fclose(file);
			if (DataIn.cbData > 0) {
				if (CryptUnprotectData(&DataIn, NULL, NULL, NULL, NULL, 0, &DataOut)) {
					*password = ctr_heap_allocate(DataOut.cbData);
					memcpy(*password, DataOut.pbData, DataOut.cbData);
					LocalFree(DataOut.pbData);
					ctr_heap_free(DataIn.pbData);
					error = 0;
				}
			}
		}
	}
	return error;
}

void ctr_gui_vault_platform_destroy(char** password) {
	if (*password) ctr_heap_free(*password);
}

#endif

char* ctr_internal_gui_vault_name(ctr_object* myself) {
	return ctr_heap_allocate_cstring(
		ctr_internal_cast2string(
			ctr_internal_object_property(
				myself,
				"name",
				NULL
			)
		)
	);
}

ctr_object* ctr_gui_vault_new_set(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* inst = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_object_property(
		inst,
		"name",
		ctr_internal_copy2string(argumentList->object)
	);
	inst->link = myself;
	return inst;
}

ctr_object* ctr_gui_vault_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument a;
	a.object = ctr_build_string_from_cstring("default");
	a.next = NULL;
	return ctr_gui_vault_new_set(myself, &a);
}

ctr_object* ctr_gui_vault_name(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(
		myself,
		"name",
		NULL
	);
}

ctr_object* ctr_gui_vault_set(ctr_object* myself, ctr_argument* argumentList) {
	char* label = ctr_heap_allocate_cstring( ctr_internal_copy2string( argumentList->object ) );
	char* password = ctr_heap_allocate_cstring( ctr_internal_copy2string( argumentList->next->object ) );
	char* vault_name;
	vault_name = ctr_internal_gui_vault_name(myself);
	int error = ctr_gui_vault_platform_store(vault_name, label, password);
	ctr_heap_free(vault_name);
	ctr_heap_free(label);
	ctr_heap_free(password);
	if (error) {
        ctr_error("Unable to store password to secret store.", 0);
		return CtrStdNil;
    }
    return myself;
}

ctr_object* ctr_gui_vault_get(ctr_object* myself, ctr_argument* argumentList) {
	char* password;
	char* vault_name;
	vault_name = ctr_internal_gui_vault_name(myself);
	char* lookup_name = ctr_heap_allocate_cstring( ctr_internal_copy2string( argumentList->object ) );
	int error = ctr_gui_vault_platform_retrieve(vault_name, lookup_name, &password);
	ctr_heap_free(vault_name);
	ctr_heap_free(lookup_name);
	if (error) {
        ctr_error("Unable to retrieve password from secret store.", 0);
        return CtrStdNil;
    }
    ctr_object* password_str = ctr_build_string_from_cstring(password);
	ctr_gui_vault_platform_destroy(&password);
	return password_str;
}

ctr_object* vaultObject;
void begin_vault() {
	ctr_gui_internal_vault_init();
	vaultObject = ctr_gui_vault_new(CtrStdObject, NULL);
	vaultObject->link = CtrStdObject;
	ctr_internal_create_func(vaultObject, ctr_build_string_from_cstring( "new" ), &ctr_gui_vault_new );
	ctr_internal_create_func(vaultObject, ctr_build_string_from_cstring( "new:" ), &ctr_gui_vault_new_set );
	ctr_internal_create_func(vaultObject, ctr_build_string_from_cstring( "name" ), &ctr_gui_vault_name );
	ctr_internal_create_func(vaultObject, ctr_build_string_from_cstring( "set:password:" ), &ctr_gui_vault_set );
	ctr_internal_create_func(vaultObject, ctr_build_string_from_cstring( "get:" ), &ctr_gui_vault_get );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Vault" ), vaultObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}
