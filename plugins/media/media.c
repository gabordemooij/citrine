#include "../../citrine.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "passw.c"
#include "media.h"
#include "jsmn.h"

#ifdef SDL
#include <SDL3/SDL.h>
#include <SDL3/SDL_image.h>
#include <SDL3/SDL_mixer.h>
#include <SDL3/SDL_ttf.h>
#endif


#ifdef MACSDL
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#endif



#include <stdio.h>
#include <math.h>

#include <sys/types.h>
#include <unistd.h>


#ifdef LIBCURL
#include <curl/curl.h>
#endif

#ifdef FFI
#include <ffi.h>
#endif

#ifdef MOCK
#include "mock.h"
#endif

#define CTR_MEDIA_FX_FLAG_MEDIA_TEST 0
#define CTR_MEDIA_FX_FLAG_MEDIA_REMAP_ALL 1
#define CTR_MEDIA_FX_FLAG_SMOOTH_GAME_CONTROL 2
#define CTR_MEDIA_FX_FLAG_AUDIO_JUMP 2000


uint64_t CtrMediaTicks1 = 0;
uint64_t CtrMediaTicks2 = 0;
uint64_t CtrMediaPerfCountStart = 0;
uint64_t CtrMediaPerfCountEnd = 0;
char CtrMediaFlagSoftwareVSync = 0;

SDL_Window* CtrMediaWindow = NULL;
SDL_Renderer* CtrMediaRenderer = NULL;

int CtrMediaInputIndex, CtrMediaSelectStart, CtrMediaSelectBegin, CtrMediaSelectEnd;
int CtrMediaMaxLines = 30;
int CtrMediaCursorLine = 0;
int CtrMediaCursorOffsetY = 0;
int CtrMediaPrevClickX = 0;
int CtrMediaPrevClickY = 0;
int CtrMediaPrevClickTime = 0;
char CtrMediaDoubleClick = 0;
int CtrMediaJumpHeightFactor = 100;
int CtrMediaControlMode = 0;
int CtrMediaRotation = 0;
int CtrMediaStdDelayTime = 0;
int CtrMediaTime = 0;
int CtrMediaLastScanCode = 0;
char CtrMediaBreakLoopFlag = 0;
uint16_t CtrMediaNetworkChunkSize = 350;
time_t CtrMediaFrameTimer = 0;
uint16_t CtrMediaSteps;
double CtrMediaVideoFPSRendering;
int CtrMediaCameraInit = 0;
ctr_object* CtrMediaInputFreeze = NULL;
int CtrStopBubbling = 0;

int CtrMediaAudioRate;
uint16_t CtrMediaAudioFormat;
int CtrMediaAudioChannels;
int CtrMediaAudioBuffers;
int CtrMediaAudioVolume;
ctr_object* CtrMediaAssetPackage;

SDL_Rect CtrMediaCamera;
SDL_Rect CtrMediaViewport;
int CtrMediaZoom;
int CtrMediaDrawSizeX;
int CtrMediaDrawSizeY;
int CtrMediaScreenActive;

double CtrMediaVersionTime = 0;

struct CtrMediaTextRenderCacheItem {
	char* text;
	SDL_Surface* surface;
	int state;
};
typedef struct CtrMediaTextRenderCacheItem CtrMediaTextRenderCacheItem;
CtrMediaTextRenderCacheItem CtrMediaEdCache[400];

struct MediaIMG {
	double			x;			int				h;
	double			y;			int				w;
	double			ox;			double			oy;
	double			tx;			double			ty;
	double			gravity;	double			gspeed;
	double			fric;		double			accel;
	double			speed;		double			dir;
	double          lastdir;
	double			mov;		int				anims;
	int				animspeed;
	int				solid;		int				collidable;
	char*			text;		TTF_Font*       font;
	char			editable;	ctr_object*		ref;
	ctr_size		paddingx;	ctr_size		paddingy;
	SDL_Color		color;		SDL_Color		backgroundColor;
	SDL_Texture*	texture;	SDL_Surface*	surface;
	ctr_size		textlength;	ctr_size		textbuffer;
	char 			bounce;
	char            fixed;
	char            ghost;
	char            nodirani;
	int             lineheight;
	char            visible;
	char            clickable;
};
typedef struct MediaIMG MediaIMG;

struct MediaAUD {
	ctr_object* ref;
	void* blob;
};
typedef struct MediaAUD MediaAUD;

#define MAX_AUDIO_SLOTS 51
#define MAX_IMAGE_SLOTS 101
#define MAX_FONT_SLOTS  11

MediaIMG mediaIMGs[MAX_IMAGE_SLOTS];
int MaxIMG = MAX_IMAGE_SLOTS;
int IMGCount = 0;

MediaIMG* CtrMediaContactSurface;

MediaAUD mediaAUDs[MAX_AUDIO_SLOTS];
int maxAUD = MAX_AUDIO_SLOTS;
int AUDCount = 0;

struct MediaFNT {
	ctr_object* ref;
	TTF_Font* font;
	char* fscript;
	int textdir;
};
typedef struct MediaFNT MediaFNT;

MediaFNT mediaFNT[MAX_FONT_SLOTS];
int maxFNT = MAX_FONT_SLOTS;
int FNTCount = 0;

int windowWidth = 0;
int windowHeight = 0;
int CtrMediaJumpHeight = 0;
char CtrMediaJump = 0;

ctr_object* colorObject;
ctr_object* mediaObject;
ctr_object* imageObject;
ctr_object* fontObject;
ctr_object* lineObject;
ctr_object* pointObject;
ctr_object* controllableObject;
ctr_object* focusObject;
ctr_object* soundObject;
ctr_object* musicObject;
ctr_object* audioObject;
ctr_object* networkObject;
ctr_object* packageObject;
ctr_object* CtrMediaFFIObjectBase;
ctr_object* CtrMediaDataBlob;
SDL_Gamepad* gameController;

uint8_t CtrMediaEventListenFlagKeyUp;
uint8_t CtrMediaEventListenFlagKeyDown;
uint8_t CtrMediaEventListenFlagMouseClick;
uint8_t CtrMediaEventListenFlagGamePadBtnUp;
uint8_t CtrMediaEventListenFlagGamePadBtnDown;
uint8_t CtrMediaEventListenFlagTimer;
uint8_t CtrMediaEventListenFlagStep;

// FX flags
int CtrMediaFXFlagMapABXY2Up;
int CtrMediaFXFlagJumpSFX;
int CtrMediaFXFlagSmoothGameControl;

// FX data
ctr_object* CtrMediaFXFlagJumpSound;


void ctr_internal_img_render_text(ctr_object* myself);
void ctr_internal_img_render_cursor(ctr_object* myself);
char* ctr_internal_media_normalize_line_endings(char* text);

int CtrMediaTimers[100];
int CtrMaxMediaTimers = 100;

void ctr_internal_media_measure_text(TTF_Font* font, char* text_buffer, int* w, int* h) {
	TTF_Text ttf_text;
	TTF_SetTextFont(&ttf_text, font);
	TTF_SetTextString(&ttf_text, text_buffer, strlen(text_buffer));
	TTF_GetTextSize(&ttf_text, w, h);
}


/**
 * [String] escape: '\n'.
 *
 * Escapes the specified ASCII character in a string.
 * If the character is a control character, the well known
 * C-based character substitute will be used.
 */
ctr_object* ctr_string_escape(ctr_object* myself, ctr_argument* argumentList)  {
	ctr_object* escape = ctr_internal_cast2string( argumentList->object );
	ctr_object* newString = NULL;
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	char* tstr;
	long i=0;
	long k=0;
	ctr_size q = 0;
	ctr_size numOfCharacters = 0;
	long tlen = 0;
	char* characters;
	char character;
	char characterDescription;
	char isControlChar = 0;
	char escaped;
	long tag_len = 0;
	characters = escape->value.svalue->value;
	numOfCharacters = escape->value.svalue->vlen;
	if (numOfCharacters < 1) {
		return myself;
	}
	for (q = 0; q < numOfCharacters; q ++) {
		character = characters[q];
		isControlChar = 0;
		characterDescription = character;
		for(i =0; i < len; i++) {
			char c = str[i];
			if (c == character) {
				tag_len += 1;
			}
		}
	}
	
	tlen = len + tag_len;
	tstr = ctr_heap_allocate( tlen * sizeof( char ) );
	for(i = 0; i < len; i++) {
		char c = str[i];
		escaped = 0;
		for (q = 0; q < numOfCharacters; q ++) {
			character = characters[q];
			isControlChar = 0;
			if (character == '\t') {
				characterDescription = 't';
				isControlChar = 1;
			}
			if (character == '\r') {
				characterDescription = 'r';
				isControlChar = 1;
			}
			if (character == '\n') {
				characterDescription = 'n';
				isControlChar = 1;
			}
			if (character == '\b') {
				characterDescription = 'b';
				isControlChar = 1;
			}
			if (c == character) {
				tstr[k++] = '\\';
				if (isControlChar) {
					tstr[k++] = characterDescription;
				} else {
					tstr[k++] = str[i];
				}
				escaped = 1;
				break;
			}
		}
		if (!escaped) {
			tstr[k++] = str[i];
		}
	}
	newString = ctr_build_string(tstr, tlen);
	ctr_heap_free( tstr );
	return newString;
}


/**
 * [String] unescape: '\n'.
 *
 * 'UnEscapes' the specified ASCII character in a string.
 */
ctr_object* ctr_string_unescape(ctr_object* myself, ctr_argument* argumentList)  {
	ctr_object* escape = ctr_internal_cast2string( argumentList->object );
	ctr_object* newString = NULL;
	char character;
	char characterDescription;
	char* str = myself->value.svalue->value;
	long  len = myself->value.svalue->vlen;
	char* tstr;
	char isControlChar = 0;
	char* characters;
	ctr_size numOfCharacters;
	char unescaped;
	ctr_size q;
	long i=0;
	long k=0;
	long tlen = 0;
	long tag_len = 0;
	characters = escape->value.svalue->value;
	numOfCharacters = escape->value.svalue->vlen;
	if (numOfCharacters < 1) {
		return myself;
	}
	for (q = 0; q < numOfCharacters; q ++) {
		character = characters[q];
		isControlChar = 0;
		characterDescription = character;
		if (character == '\t') {
			characterDescription = 't';
			isControlChar = 1;
		}
		if (character == '\r') {
			characterDescription = 'r';
			isControlChar = 1;
		}
		if (character == '\n') {
			characterDescription = 'n';
			isControlChar = 1;
		}
		if (character == '\b') {
			characterDescription = 'b';
			isControlChar = 1;
		}
		for(i = 0; i < len; i++) {
			if (i<len-1 && str[i] == '\\' && str[i+1] == characterDescription) {
				tag_len -= 1;
			}
		}
	}
	tlen = len + tag_len;
	tstr = ctr_heap_allocate( tlen * sizeof( char ) );
	for(i = 0; i < len; i++) {
		unescaped = 0;
		for (q = 0; q < numOfCharacters; q ++) {
			character = characters[q];
			characterDescription = character;
			isControlChar = 0;
			if (character == '\t') {
				characterDescription = 't';
				isControlChar = 1;
			}
			if (character == '\r') {
				characterDescription = 'r';
				isControlChar = 1;
			}
			if (character == '\n') {
				characterDescription = 'n';
				isControlChar = 1;
			}
			if (character == '\b') {
				characterDescription = 'b';
				isControlChar = 1;
			}
			if (i<len-1 && str[i] == '\\' && str[i+1] == characterDescription) {
				if (isControlChar) {
					if ( characterDescription == 'n' ) {
						tstr[k++] = '\n';
					}
					if ( characterDescription == 'r' ) {
						tstr[k++] = '\r';
					}
					if ( characterDescription == 't' ) {
						tstr[k++] = '\t';
					}
					if ( characterDescription == 'b' ) {
						tstr[k++] = '\b';
					}
				} else {
					tstr[k++] = str[i+1];
				}
				i++;
				unescaped = 1;
			}
		}
		if (unescaped == 0) {
			tstr[k++] = str[i];
		}
	}
	newString = ctr_build_string(tstr, tlen);
	ctr_heap_free( tstr );
	return newString;
}


/**
 * [Json] new
 *
 * Creates a new instance of the Json object.
 * The Json object allows you to use the Json protocol to communicate with
 * other applications.
 */
ctr_object* ctr_json_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* jsonInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	jsonInstance->link = myself;
	return jsonInstance;
}

/**
 * @internal
 */
ctr_object* ctr_jsmn_dump( char* data, jsmntok_t** tt ) {
	ctr_object* answer;
	ctr_argument* a;
	ctr_argument* b;
	char c;
	a = NULL;
	b = NULL;
	int i;
	jsmntok_t* t = *(tt);
	ctr_size len = strlen(data);
	if (CtrStdFlow != NULL) {
		return CtrStdNil;
	}
	if (t == NULL) {
		CtrStdFlow = ctr_error("Invalid JSON.", 0);
		return CtrStdNil;
	}
	if (t->start < 0 || t->end < 0 || t->start >= len || ((t->end - t->start) < 0) || (( t->end - t->start ) > len) ) {
		CtrStdFlow = ctr_error("Invalid JSON.", 0);
		return CtrStdNil;
	}
	if (t->type == JSMN_STRING) {
		answer = ctr_build_string( (data + t->start), (t->end - t->start) );
		a = ctr_heap_allocate( sizeof(ctr_argument) );
		a->object = ctr_build_string_from_cstring("\"\t\b\n\r\f\\");
		answer = ctr_string_unescape( answer, a );
		ctr_heap_free(a);
		*(tt)+=1;
	}
	else if (t->type == JSMN_PRIMITIVE ) {
		c = *(data + t->start);
		if ( c == 't' ) {
			answer = ctr_build_bool( 1 );
		} else if ( c == 'f' ) {
			answer = ctr_build_bool( 0 );
		} else if ( c == 'n' ) {
			answer = ctr_build_nil();
		} else if ( c == '-' || ( c >= '0' && c <= '9' ) ) {
			answer = ctr_string_to_number( ctr_build_string( (char*) (data + t->start), (t->end - t->start) ), a );
		} else {
			answer = CtrStdNil;
		}
		*(tt)+=1;
	}
	else if (t->type == JSMN_ARRAY) {
		a = ctr_heap_allocate( sizeof(ctr_argument) );
		answer = ctr_array_new( CtrStdArray, NULL );
		*(tt)+=1;
		for(i = 0; i<t->size; i++ ) {
			ctr_object* element = ctr_jsmn_dump( data, tt );
			a->object = element;
			ctr_array_push( answer, a );
		}
		ctr_heap_free(a);
	}
	else if (t->type == JSMN_OBJECT ) {
		a = ctr_heap_allocate( sizeof(ctr_argument) );
		b = ctr_heap_allocate( sizeof(ctr_argument) );
		answer = ctr_map_new( CtrStdMap, NULL );
		*(tt)+=1;
		for (i = 0; i<(t->size); i+=1) {
			ctr_object* property = ctr_jsmn_dump( data, tt );
			ctr_object* value = ctr_jsmn_dump( data, tt );
			if (CtrStdFlow != NULL) break;
			a->object = value;
			b->object = property;
			a->next = b;
			ctr_map_put( answer, a );
		}
		ctr_heap_free(a);
		ctr_heap_free(b);
	}
	else {
		CtrStdFlow = ctr_error("Invalid JSON.", 0);
		answer = CtrStdNil;
	}
	return answer;
}

ctr_object* ctr_json_parse(ctr_object* myself, ctr_argument* argumentList) {
	char* jsonString = ctr_heap_allocate_cstring(
		ctr_internal_cast2string( argumentList->object )
	);
	ctr_size size;
	jsmn_parser jsmn;
	jsmn_parser jsmn2;
	ctr_object* answer;
	jsmntok_t* t;
	jsmntok_t* ot;
	int r;
	int s;
	jsmn_init(&jsmn);
	s = jsmn_parse(&jsmn, jsonString, strlen(jsonString), NULL, 0);
	if ( s <= 0 ) {
		ctr_heap_free( jsonString );
		CtrStdFlow = ctr_error("Invalid JSON.", 0);
		return CtrStdNil;
	}
	size = (ctr_size) s;
	t = (jsmntok_t*) ctr_heap_allocate( sizeof(jsmntok_t) * size );
	ot = t;
	jsmn_init(&jsmn2);
	r = jsmn_parse(&jsmn2, jsonString, strlen(jsonString), t, size);
	if (r < size || t[0].type != JSMN_OBJECT) {
		ctr_heap_free( t );
		ctr_heap_free( jsonString );
		CtrStdFlow = ctr_error("Invalid JSON.", 0);
		return CtrStdNil;
	}
	answer = ctr_jsmn_dump(jsonString, &t);
	ctr_heap_free( ot );
	ctr_heap_free( jsonString );
	return answer;
}


ctr_object* ctr_json_jsonify(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object*  string;
	ctr_mapitem* mapItem;
	ctr_argument* newArgumentList;
	string  = ctr_build_string_from_cstring( "{" );
	mapItem = argumentList->object->properties->head;
	newArgumentList = ctr_heap_allocate( sizeof( ctr_argument ) );
	while( mapItem ) {
		if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTBOOL && mapItem->key->value.bvalue == 1) {
			newArgumentList->object = ctr_build_string_from_cstring( "true" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTBOOL && mapItem->key->value.bvalue == 0) {
			newArgumentList->object = ctr_build_string_from_cstring( "false" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTNIL ) {
			newArgumentList->object = ctr_build_string_from_cstring( "null" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTNUMBER ) {
			newArgumentList->object = mapItem->value;
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->key->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
			newArgumentList->object = ctr_build_string_from_cstring( "\"" );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring("\"\n\b\r\t\f\\");
			newArgumentList->object = ctr_string_escape( mapItem->key, newArgumentList );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring( "\"" );
			ctr_string_append( string, newArgumentList );
		}
		newArgumentList->object = ctr_build_string_from_cstring( ":" );
		ctr_string_append( string, newArgumentList );
		if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTBOOL && mapItem->value->value.bvalue == 1) {
			newArgumentList->object = ctr_build_string_from_cstring( "true" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTBOOL && mapItem->value->value.bvalue == 0) {
			newArgumentList->object = ctr_build_string_from_cstring( "false" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTNIL ) {
			newArgumentList->object = ctr_build_string_from_cstring( "null" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTNUMBER ) {
			newArgumentList->object = mapItem->value;
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
			newArgumentList->object = ctr_build_string_from_cstring( "\"" );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring("\"\n\b\r\t\f\\");
			newArgumentList->object = ctr_string_escape( mapItem->value, newArgumentList );
			ctr_string_append( string, newArgumentList );
			newArgumentList->object = ctr_build_string_from_cstring( "\"" );
			ctr_string_append( string, newArgumentList );
		}
		else if ( mapItem->value->info.type == CTR_OBJECT_TYPE_OTARRAY ) {
			int i;
			ctr_object* array = mapItem->value;
			ctr_object* arrayElement;
			newArgumentList->object = ctr_build_string_from_cstring( "[" );
			ctr_string_append( string, newArgumentList );
			for(i=array->value.avalue->tail; i<array->value.avalue->head; i++) {
				if ( i > array->value.avalue->tail ) {
					newArgumentList->object = ctr_build_string_from_cstring( "," );
					ctr_string_append( string, newArgumentList );
				}
				arrayElement = *( array->value.avalue->elements + i );
				if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTBOOL && arrayElement->value.bvalue == 1) {
					newArgumentList->object = ctr_build_string_from_cstring( "true" );
					ctr_string_append( string, newArgumentList );
				}
				else if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTBOOL && arrayElement->value.bvalue == 0) {
					newArgumentList->object = ctr_build_string_from_cstring( "false" );
					ctr_string_append( string, newArgumentList );
				}
				else if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTNIL ) {
					newArgumentList->object = ctr_build_string_from_cstring( "null" );
					ctr_string_append( string, newArgumentList );
				}
				else if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTNUMBER ) {
					newArgumentList->object = arrayElement;
					ctr_string_append( string, newArgumentList );
				}
				else if ( arrayElement->info.type == CTR_OBJECT_TYPE_OTSTRING ) {
					newArgumentList->object = ctr_build_string_from_cstring( "\"" );
					ctr_string_append( string, newArgumentList );
					newArgumentList->object = ctr_string_escape( arrayElement, newArgumentList );
					ctr_string_append( string, newArgumentList );
					newArgumentList->object = ctr_build_string_from_cstring( "\"" );
					ctr_string_append( string, newArgumentList );
				}
				else {
					newArgumentList->object = arrayElement;
					newArgumentList->object = ctr_json_jsonify( myself, newArgumentList );
					ctr_string_append( string, newArgumentList );
				}
			}
			newArgumentList->object = ctr_build_string_from_cstring( "]" );
			ctr_string_append( string, newArgumentList );
		}
		else {
			newArgumentList->object = mapItem->value;
			newArgumentList->object = ctr_json_jsonify( myself, newArgumentList );
			ctr_string_append( string, newArgumentList );
		}
		mapItem = mapItem->next;
		if ( mapItem ) {
			newArgumentList->object = ctr_build_string_from_cstring( ", " );
			ctr_string_append( string, newArgumentList );
		}
	}
	newArgumentList->object = ctr_build_string_from_cstring( "}" );
	ctr_string_append( string, newArgumentList );
	ctr_heap_free( newArgumentList );
	return string;
}



void ctr_internal_media_fatalerror(char* msg, const char* info)	{ fprintf(stderr,"Media Plugin FATAL ERROR: %s (%s) \n", msg, info); SDL_Quit(); exit(1); }
MediaIMG* ctr_internal_media_getfocusimage()						{ return (MediaIMG*) focusObject->value.rvalue->ptr; }
MediaIMG* ctr_internal_media_getplayer()							{ return (controllableObject == NULL) ? NULL : (MediaIMG*) controllableObject->value.rvalue->ptr; }
char ctr_internal_media_has_selection()								{ return (CtrMediaSelectBegin != CtrMediaSelectEnd); }

MediaAUD* ctr_internal_get_audio_from_object(ctr_object* object)	{
	if (object->value.rvalue == NULL) return NULL;
	return (MediaAUD*) object->value.rvalue->ptr;
}

MediaIMG* ctr_internal_get_image_from_object(ctr_object* object)	{
	if (object->value.rvalue == NULL) return NULL;
	return (MediaIMG*) object->value.rvalue->ptr;
}

MediaFNT* ctr_internal_get_font_from_object(ctr_object* object)	{
	if (object->value.rvalue == NULL) return NULL;
	return (MediaFNT*) object->value.rvalue->ptr;
}

void ctr_internal_media_clear_edcache() {
	for(int i = 0; i < 400; i++) {
		CtrMediaEdCache[i].surface = NULL;
		if (CtrMediaEdCache[i].text) {
			ctr_heap_free(CtrMediaEdCache[i].text);
		}
		CtrMediaEdCache[i].text = NULL;
		CtrMediaEdCache[i].state = 0;
	}
}

void ctr_internal_media_reset() {
	controllableObject = NULL;
	focusObject = NULL;
	int i;
	for(i = 0; i < IMGCount; i++) {
		MediaIMG* mediaImage = &mediaIMGs[i];
		if (mediaImage->text) {
			ctr_heap_free(mediaImage->text);
			mediaImage->text = NULL;
		}
	}
	ctr_internal_media_clear_edcache();
	IMGCount = 0;
	AUDCount = 0;
	CtrMediaJumpHeight = 0;
	CtrMediaJump = 0;
	CtrMediaMaxLines = 30;
	CtrMediaCursorLine = 0;
	CtrMediaCursorOffsetY = 0;
	CtrMediaPrevClickX = 0;
	CtrMediaPrevClickY = 0;
	CtrMediaPrevClickTime = 0;
	CtrMediaDoubleClick = 0;
	CtrMediaJumpHeightFactor = 100;
	CtrMediaControlMode = 0;
	CtrMediaRotation = 0;
	CtrMediaStdDelayTime =  0; // Only for @testing
	CtrMediaBreakLoopFlag = 0;
	CtrMediaInputIndex = 0;
	CtrMediaSelectStart = 0;
	CtrMediaSelectBegin = 0;
	CtrMediaSelectEnd = 0;
	CtrMediaSteps = 0;
	CtrMediaEventListenFlagKeyUp = 0;
	CtrMediaEventListenFlagKeyDown = 0;
	CtrMediaEventListenFlagMouseClick = 0;
	CtrMediaEventListenFlagGamePadBtnUp = 0;
	CtrMediaEventListenFlagGamePadBtnDown = 0;
	CtrMediaEventListenFlagTimer = 0;
	CtrMediaEventListenFlagStep = 0;
	CtrMediaContactSurface = NULL;
	CtrMediaCamera.w = 0;
	CtrMediaCamera.h = 0;
	CtrMediaCamera.x = 0;
	CtrMediaCamera.y = 0;
	CtrMediaViewport.x = 0;
	CtrMediaViewport.y = 0;
	CtrMediaViewport.w = 0;
	CtrMediaViewport.h = 0;
	CtrMediaZoom = 0;
	CtrMediaFXFlagMapABXY2Up = 0;
	for(int i = 1; i<=CtrMaxMediaTimers; i++) {
		CtrMediaTimers[i] = -1;
	}
}


SDL_Rect ctr_internal_media_image_maprect(MediaIMG* m) {
	SDL_Rect r;
	r.x = (int)(m->x);
	r.y = (int)(m->y);
	r.h = (int) m->h;
	r.w = (int) m->w/(m->anims ? m->anims : 1);
	MediaIMG* player = ctr_internal_media_getplayer();
	// For platformers and simple control modes this is all we need...
	if (CtrMediaControlMode == 1 && (player && player->gravity) ) return r;
	if (CtrMediaControlMode != 4 && CtrMediaControlMode != 1) return r;
	// For top-down, radius (and 3D) we need to calculate the bounding box of the
	// rotated rectangle...
	int cx = r.x + (r.w / 2);
	int cy = r.y + (r.h / 2);
	double rad = (m->dir) * (M_PI / 180.0);
	int ltx = cx+(r.x-cx)*cos(rad)+(r.y-cy)*sin(rad);
	int lty = cy-(r.x-cx)*sin(rad)+(r.y-cy)*cos(rad);
	int rtx = cx+((r.x+r.w)-cx)*cos(rad)+(r.y-cy)*sin(rad);
	int rty = cy-((r.x+r.w)-cx)*sin(rad)+(r.y-cy)*cos(rad);
	int lbx = cx+(r.x-cx)*cos(rad)+((r.y+r.h)-cy)*sin(rad);
	int lby = cy-(r.x-cx)*sin(rad)+((r.y+r.h)-cy)*cos(rad);
	int rbx = cx+((r.x+r.w)-cx)*cos(rad)+((r.y+r.h)-cy)*sin(rad);
	int rby = cy-((r.x+r.w)-cx)*sin(rad)+((r.y+r.h)-cy)*cos(rad);
	SDL_Rect r2;
	r2.x = ltx;
	if (rtx < r2.x) r2.x = rtx;
	if (lbx < r2.x) r2.x = lbx;
	if (rbx < r2.x) r2.x = rbx;
	r2.y = lty;
	if (rty < r2.y) r2.y = rty;
	if (lby < r2.y) r2.y = lby;
	if (rby < r2.y) r2.y = rby;
	int x2;
	int y2;
	x2 = ltx;
	if (rtx > x2) x2 = rtx;
	if (lbx > x2) x2 = lbx;
	if (rbx > x2) x2 = rbx;
	y2 = lty;
	if (rty > y2) y2 = rty;
	if (lby > y2) y2 = lby;
	if (rby > y2) y2 = rby;
	r2.h = y2 - r2.y;
	r2.w = x2 - r2.x;
	return r2;
}

char ctr_internal_media_image_intersect(MediaIMG* m1, MediaIMG* m2) {
	SDL_Rect r1 = ctr_internal_media_image_maprect(m1);
	SDL_Rect r2 = ctr_internal_media_image_maprect(m2);
	return SDL_HasIntersection(&r1, &r2);
}

void ctr_internal_media_get_selection(int* begin, int* end) {
	if (CtrMediaSelectBegin > CtrMediaSelectEnd) {
		*begin = CtrMediaSelectEnd;
		*end = CtrMediaSelectBegin;
	} else {
		*begin = CtrMediaSelectBegin;
		*end = CtrMediaSelectEnd;
	}
}

ctr_size ctr_internal_media_bytepos2utf8pos( MediaIMG* image, int bytepos ) {
	int i = 0;
	ctr_size chars = 0;
	char byte;
	while(i < bytepos) {
		byte = image->text[i];
		if ((byte & 0x80)==0x00 || (byte & 0xC0)==0xC0) chars += 1;
		i++;
	}
	return chars;
}

void ctr_internal_media_reset_selection() {
	CtrMediaSelectBegin = CtrMediaSelectEnd;
}

void ctr_internal_media_move_cursor_right(MediaIMG* haystack, ctr_size steps, char jump) {
	ctr_size i;
	for(i=0; i<steps; i++) {
		if (CtrMediaInputIndex == haystack->textlength) return;
		CtrMediaInputIndex +=1;
		while ( //Skip over bytes that are part of unicode chars
			CtrMediaInputIndex < haystack->textlength &&
			(haystack->text[CtrMediaInputIndex] & 0x80)!=0x00 &&
			(haystack->text[CtrMediaInputIndex] & 0xC0)!=0xC0
		) CtrMediaInputIndex++;
		if (haystack->text[CtrMediaInputIndex]=='\n') {
			if (jump) {
				CtrMediaInputIndex += 1;
			} else {
				CtrMediaInputIndex -= 1;
			}
			
			return;
		}
	}
}

void ctr_internal_media_move_cursor_left(MediaIMG* haystack, ctr_size steps, char jump) {
	if (CtrMediaInputIndex == 0) return;
	CtrMediaInputIndex -=1;
	ctr_size i;
	for(i=0; i<steps; i++) {
		while (
			CtrMediaInputIndex > 0 &&
			(haystack->text[CtrMediaInputIndex] & 0x80)!=0x00 &&
			(haystack->text[CtrMediaInputIndex] & 0xC0)!=0xC0
		) CtrMediaInputIndex--;
		if (haystack->text[CtrMediaInputIndex]=='\n') {
			if (jump) {
				CtrMediaInputIndex -= 1;
			} else {
				CtrMediaInputIndex += 1;
			}
			return;
		}
	}
}

void ctr_internal_media_move_cursor_to_end_of_cur_line(MediaIMG* haystack) {
	while (haystack->text[CtrMediaInputIndex] !='\r') {
		if (CtrMediaInputIndex == haystack->textlength) return;
		CtrMediaInputIndex++;
	}
}

void ctr_internal_media_move_cursor_to_last_char_of_cur_line(MediaIMG* haystack) {
	while (haystack->text[CtrMediaInputIndex] !='\r') {
		if (CtrMediaInputIndex == haystack->textlength) return;
		CtrMediaInputIndex++;
	}
	CtrMediaInputIndex--;
}

void ctr_internal_media_move_cursor_to_first_char_of_cur_line(MediaIMG* haystack) {
	while (haystack->text[CtrMediaInputIndex] !='\n') {
		if (CtrMediaInputIndex == 0) return;
		CtrMediaInputIndex--;
	}
	CtrMediaInputIndex++;
}

void ctr_internal_media_move_cursor_to_last_char_of_prev_line(MediaIMG* haystack) {
	ctr_internal_media_move_cursor_to_first_char_of_cur_line(haystack);
	if (CtrMediaInputIndex == 0) return;
	CtrMediaInputIndex-=3;
}

void ctr_internal_media_move_cursor_to_first_char_of_next_line(MediaIMG* haystack) {
	ctr_internal_media_move_cursor_to_last_char_of_cur_line(haystack);
	if (CtrMediaInputIndex == haystack->textlength) return;
	CtrMediaInputIndex+=3;
}

void ctr_internal_media_move_cursor_to_first_char_of_prev_line(MediaIMG* haystack) {
	ctr_internal_media_move_cursor_to_last_char_of_prev_line(haystack);
	ctr_internal_media_move_cursor_to_first_char_of_cur_line(haystack);
}

void ctr_internal_media_arrowkey_selection() {
	if (focusObject) {
		if (CtrMediaSelectStart) {
			CtrMediaSelectEnd = CtrMediaInputIndex;
			ctr_internal_img_render_text(focusObject);
		} else {
			CtrMediaSelectBegin = CtrMediaInputIndex;
			CtrMediaSelectEnd = CtrMediaInputIndex;
		}
	}
}

int ctr_internal_media_get_current_char_line(MediaIMG* haystack) {
	int chars = 0;
	int i = CtrMediaInputIndex;
	while (haystack->text[i] !='\n' && i>0) {
		if	((haystack->text[i] & 0x80)==0x00 ||
			(haystack->text[i] & 0xC0)==0xC0
		) chars++;
		i--;
	}
	if (i>0) chars-=1; //dont count the CR char if we did hit one
	return chars;
}

void ctr_internal_media_select_word(MediaIMG* haystack) {
	ctr_internal_media_reset_selection();
	int i = CtrMediaInputIndex;
	while(i-1 >= 0 && 
		haystack->text[i-1]!='\n' &&
		haystack->text[i-1]!='\r' &&
		haystack->text[i-1]!=' '
	) i--;
	CtrMediaSelectBegin = i;
	i = CtrMediaInputIndex;
	while(i < haystack->textlength && 
		haystack->text[i]!='\n' &&
		haystack->text[i]!='\r' &&
		haystack->text[i]!=' '
	) i++;
	CtrMediaSelectEnd = i;
	return;
}


void ctr_internal_media_textinsert(MediaIMG* mediaImage, char* text) {
	if (mediaImage->text == NULL) return;
	ctr_size insertTextLength = strlen(text);
	int len;
	ctr_internal_media_reset_selection();
	if (insertTextLength==1 && text[0]=='\b') {
		if (CtrMediaInputIndex==0) return;
		int oldPos = CtrMediaInputIndex;
		ctr_internal_media_move_cursor_left(mediaImage, 1,1);
		len = oldPos - CtrMediaInputIndex;
		memcpy(mediaImage->text+CtrMediaInputIndex, mediaImage->text+oldPos,mediaImage->textlength-oldPos);
		mediaImage->textlength -= len;
		memset(mediaImage->text+mediaImage->textlength,'\0',len);
		return;
	}
	if (insertTextLength==1 && text[0]=='\x07F') {
		if (CtrMediaInputIndex==mediaImage->textlength) return;
		int oldPos = CtrMediaInputIndex;
		ctr_internal_media_move_cursor_right(mediaImage, 1,1);
		memcpy(mediaImage->text+oldPos, mediaImage->text+CtrMediaInputIndex,mediaImage->textlength-CtrMediaInputIndex);
		mediaImage->textlength -= (CtrMediaInputIndex - oldPos);
		mediaImage->text[mediaImage->textlength]='\0';
		CtrMediaInputIndex = oldPos;
		return;
	}
	mediaImage->textlength = mediaImage->textlength + insertTextLength;
	if (mediaImage->textlength+1 > mediaImage->textbuffer) {
		mediaImage->textbuffer = mediaImage->textlength + 500;
		mediaImage->text = ctr_heap_reallocate(mediaImage->text, mediaImage->textbuffer + 1);
	}
	memcpy(mediaImage->text+CtrMediaInputIndex+insertTextLength, mediaImage->text+CtrMediaInputIndex,mediaImage->textlength-CtrMediaInputIndex-insertTextLength);
	memcpy(mediaImage->text+CtrMediaInputIndex,text,insertTextLength);
	mediaImage->text[mediaImage->textlength + 1] = '\0';
	CtrMediaInputIndex += insertTextLength;
}

/**
 * @def
 * [ Image ] cut
 * 
 * @example
 * image cut
 * 
 * @result
 * @info-image-cut
 */
ctr_object* ctr_img_text_del(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* mediaImage;
	int begin, end, len;
	ctr_internal_media_get_selection(&begin, &end);
	mediaImage = ctr_internal_get_image_from_object(myself);
	len = end - begin;
	if (len) {
		memcpy(mediaImage->text+begin, mediaImage->text+end, mediaImage->textlength-end);
		mediaImage->textlength -= len;
		memset(mediaImage->text+mediaImage->textlength, '\0', len);
	}
	CtrMediaInputIndex = begin;
	ctr_internal_media_reset_selection();
	ctr_internal_img_render_text(myself);
	return myself;
}

/**
 * @def
 * [ Image ] append: [ Text ]
 * 
 * @example
 * image append: ['abc'].
 * 
 * @result
 * @info-image-insert
 */
ctr_object* ctr_img_text_ins(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* mediaImage;
	char* buffer;
	int begin, end;
	ctr_internal_media_get_selection(&begin, &end);
	buffer = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	mediaImage = ctr_internal_get_image_from_object(myself);
	if (begin < end) {
		ctr_img_text_del(myself, argumentList);
		CtrMediaInputIndex = begin;
	}
	ctr_internal_media_textinsert(mediaImage, buffer);
	ctr_heap_free(buffer);
	ctr_internal_img_render_text(myself);
	return myself;
}

/**
 * @def
 * [ Media ] width: [ Number ] height: [ Number ]
 *
 * @example
 * >> m := Media new.
 * m width: 320 height: 200.
 *
 * @result
 * @info-media-width-height
 */
ctr_object* ctr_media_width_height( ctr_object* myself, ctr_argument* argumentList ) {
	CtrMediaCamera.w = (int) ctr_tonum(argumentList->object);
	CtrMediaCamera.h = (int) ctr_tonum(argumentList->next->object);
	if (CtrMediaViewport.x == 0 && CtrMediaViewport.y == 0) {
		CtrMediaZoom = 1;
	}
	return myself;
}

ctr_object* ctr_media_left_top( ctr_object* myself, ctr_argument* argumentList ) {
	CtrMediaViewport.x = (int) ctr_tonum(argumentList->object);
	CtrMediaViewport.y = (int) ctr_tonum(argumentList->next->object);
	CtrMediaZoom = 0;
	return myself;
}

void ctr_internal_cursormove(int x, int y) {
	int offset = 0;
	MediaIMG* text = ctr_internal_media_getfocusimage();
	if (text == NULL) return;
	if (text->text == NULL) return;
	if (y == -1) {
		offset = ctr_internal_media_get_current_char_line(text);
		ctr_internal_media_move_cursor_to_first_char_of_prev_line(text);
		ctr_internal_media_move_cursor_right(text,offset,0);
	}
	if (y == 1) {
		offset = ctr_internal_media_get_current_char_line(text);
		ctr_internal_media_move_cursor_to_first_char_of_next_line(text);
		ctr_internal_media_move_cursor_right(text,offset,0);
	}
	if (x == -1) ctr_internal_media_move_cursor_left(text,1,1);
	if (x ==  1) ctr_internal_media_move_cursor_right(text,1,1);
	if (x == -2) ctr_internal_media_move_cursor_to_first_char_of_cur_line(text);
	if (x ==  2) ctr_internal_media_move_cursor_to_end_of_cur_line(text);
	if (y ==  2) for (int i = 0; i<10; i++) ctr_internal_cursormove(0, 1);
	if (y == -2) for (int i = 0; i<10; i++) ctr_internal_cursormove(0, -1);
}

void ctr_internal_media_infercursorpos(MediaIMG* image, int x, int y) {
	// no text? then index = 0
	if (!image->textlength) {
		CtrMediaInputIndex = 0;
		return;
	}
	int relx = x - image->x - image->paddingx;
	int rely = y - image->y - image->paddingy;
	int lineHeight;
	lineHeight = image->lineheight;
	CtrMediaMaxLines = floor(image->h/lineHeight);
	int line = CtrMediaCursorOffsetY + (rely / lineHeight);
	CtrMediaInputIndex = 0;
	for(int i = 0; i<line; i++) {
		ctr_internal_media_move_cursor_to_first_char_of_next_line(image);
	}
	int line_start = CtrMediaInputIndex;
	ctr_internal_media_move_cursor_to_end_of_cur_line(image);
	int end_of_line = CtrMediaInputIndex;
	if (line_start == end_of_line) {
		return;
	}
	int line_length = end_of_line - line_start;
	char* measurementBuffer = ctr_heap_allocate(line_length + 1);
	memcpy(measurementBuffer, image->text+line_start, line_length);
	measurementBuffer[line_length] = '\0';
	int total_line_width = 0;
	ctr_internal_media_measure_text(image->font, measurementBuffer, &total_line_width, NULL);
	//Line is shorter than mouse pos, go to end of line
	if (total_line_width < relx) {
		ctr_heap_free(measurementBuffer);
		ctr_internal_media_move_cursor_to_end_of_cur_line(image);
		return;
	}
	int line_segment_width = total_line_width;
	int last_line_segment_width = line_segment_width;
	while(CtrMediaInputIndex>line_start) {
		last_line_segment_width = line_segment_width;
		ctr_internal_media_measure_text(image->font, measurementBuffer, &line_segment_width, NULL);
		if (line_segment_width < relx) {
			int d1 = relx - line_segment_width;
			int d2 = last_line_segment_width - relx;
			if (d2 < d1) {
				ctr_internal_media_move_cursor_right(image, 1, 0);
			}
			break;
		}
		ctr_internal_media_move_cursor_left(image, 1, 0);
		measurementBuffer[CtrMediaInputIndex-line_start] = '\0';
	}
	ctr_heap_free(measurementBuffer);
	return;
}

void ctr_internal_media_anim_frames(MediaIMG* m, SDL_Rect* r, SDL_Rect* s) {
	int frame_width = m->w / (m->anims ? m->anims : 1);
	int step_offset;
	int animspeed;
	if (m->nodirani == 2) {
		animspeed = m->animspeed;
		if (animspeed >= 100) animspeed = 99;
		if (animspeed < 0) animspeed = 0;
		CtrMediaTime = CtrMediaTicks2;
		step_offset = (int)floor(CtrMediaTime / (1000 - animspeed * 10)) % m->anims;
	} else {
		step_offset = (m->mov) ? (int)floor(m->x / m->animspeed) % m->anims : 0;
	}
	r->w = (m->anims > 0 && m->w > m->anims) ? m->w / m->anims : m->w;
	r->h = m->h;
	s->x = (frame_width * step_offset); 
	s->w = frame_width;
	s->h = m->h;
	s->y = 0;
}

void ctr_internal_media_image_resolvecollision1(
	MediaIMG* m, MediaIMG* m2,
	SDL_Rect r, SDL_Rect r2,
	int right, int down, double nx, double ny) {
	SDL_Rect r3;
	int xdepth;
	int ydepth;
	if (SDL_IntersectRect(&r, &r2, &r3)) {
		if (right) {
			xdepth = ((r.x > r2.x) ? (r.x - r2.x) : 0) + r3.w;
		} else {
			xdepth = (((r.x+r.w) < (r2.x+r2.w)) ? ((r2.x+r2.w)-(r.x+r.w)) : 0) + r3.w;
		}
		if (down) {
			ydepth = ((r.y > r2.y) ? (r.y - r2.y) : 0) + r3.h;
		} else {
			ydepth = (((r.y+r.h) < (r2.y+r2.h)) ? ((r2.y+r2.h)-(r.y+r.h))  : 0) + r3.h;
		}
		if (xdepth <= ydepth) {
			if (right) {
				m->x = floor(m->x - xdepth);
			} else {
				m->x = ceil(m->x + xdepth);
			}
		} else {
			if (down) {
				m->gspeed = 0; //reset speed from gravity to avoid creeping towards the middle
				m->y = floor(m->y - ydepth);
			} else {
				m->y = ceil(m->y + ydepth);
			}
		}
	}
}

void ctr_internal_media_image_resolvecollision(MediaIMG* m, MediaIMG* m2) {
	SDL_Rect r, rold, r2;
	r = ctr_internal_media_image_maprect(m); // obtain shape target position
	double nx, ny; int newy;
	nx=m->x; ny=m->y; // store the target position
	m->x = m->ox; m->y = m->oy; // reset old pos for per-axis check
	rold = ctr_internal_media_image_maprect(m); // obtain prev shape
	m->x = nx; m->y = ny; // restore target position, we now have rold
	r2 = ctr_internal_media_image_maprect(m2); // shape of other
	newy = r.y; r.y = rold.y; // set y back
	MediaIMG* player = ctr_internal_media_getplayer();
	// calc center for impact vector
	int cx1 = rold.x + (rold.w/2);
	int cx2 = r2.x + (r2.w/2);
	int cy1 = rold.y + (rold.h/2);
	int cy2 = r2.y + (r2.h/2);
	int right;
	int down;
	// Normally with the default collision resolver,
	// a moving down platform can crush the player into the ground,
	// in platform-style games (where player has gravity >=1 and
	// controlmode == 1) it's nicer to shift the player out of the way
	// than to run him into the floor... this is also more efficient
	// because we can simply bail out with a simple movement in this case.
	// note that the player might be wanting to jump on the platform or
	// against it, in that case normal resolving takes place.
	if (m == player &&
		CtrMediaControlMode == 1
			&& m->gravity >= 1
			&& m2->mov
			&& !m2->gravity
			&& m2->dir > 180
			&& m2->y < m->y
			&& CtrMediaJump == 0
	) {
		if (m->x < cx2) {
			m->x = m2->x - r.w;
		} else {
			m->x = m2->x + r2.w;
		}
		return;
	}
	// same but for top-crush
	if (m == player &&
		CtrMediaControlMode == 1
			&& m->gravity >= 1
			&& m2->mov
			&& !m2->gravity
			&& m2->dir < 180
			&& m2->y < m->y
			&& CtrMediaJump == 0
	) {
		if (m->x < cx2) {
			m->x = m2->x - r.w;
		} else {
			m->x = m2->x + r2.w;
		}
		return;
	}
	// for radius control we infer direction from centers,
	// because upon rotating ox/oy might be invalid
	// old/newpos does not always work (jump through platform from bottom)
	right = cx1 < cx2;
	down = cy1 < cy2;
	// first resolve collision on x-axis
	ctr_internal_media_image_resolvecollision1(m,m2,r,r2,right,down,nx,ny);
	r.y = newy; // then on y-axis
	ctr_internal_media_image_resolvecollision1(m,m2,r,r2,right,down,nx,ny);
}

int ctr_internal_media_mouse_down(SDL_Event event) {
	MediaIMG* focusImage;
	CtrStopBubbling = 0;
	for(int i = 0; i < IMGCount; i++) {
		if (
		mediaIMGs[i].x < event.button.x && 
		mediaIMGs[i].y < event.button.y &&
		mediaIMGs[i].x + (mediaIMGs[i].w / (mediaIMGs[i].anims ? mediaIMGs[i].anims : 1)) > event.button.x &&
		mediaIMGs[i].y + mediaIMGs[i].h > event.button.y &&
		mediaIMGs[i].ref != NULL &&
		mediaIMGs[i].clickable &&
		!CtrStopBubbling
		) {
			if (mediaIMGs[i].editable) {
				focusObject = mediaIMGs[i].ref;
				ctr_internal_media_clear_edcache();
				focusImage = (MediaIMG*) focusObject->value.rvalue->ptr;
				ctr_internal_media_infercursorpos(focusImage, event.button.x, event.button.y);
				CtrMediaSelectStart = 1;
				CtrMediaSelectBegin = CtrMediaInputIndex;
				CtrMediaSelectEnd = CtrMediaInputIndex;
				CtrMediaDoubleClick = 0;
				if(!SDL_TICKS_PASSED(SDL_GetTicks64(), CtrMediaPrevClickTime + 1000) && CtrMediaPrevClickX == event.button.x && CtrMediaPrevClickY == event.button.y ) {
					CtrMediaDoubleClick = 1;
					ctr_internal_media_select_word(focusImage);
					CtrMediaSelectStart = 0;
				}
				CtrMediaPrevClickX = event.button.x;
				CtrMediaPrevClickY = event.button.y;
				CtrMediaPrevClickTime = SDL_GetTicks64();
				ctr_internal_img_render_text(focusObject);
				SDL_StartTextInput();
			}
			else {
				focusObject = NULL;
				SDL_StopTextInput();
			}
			ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
			ctr_send_message(mediaIMGs[i].ref, CTR_DICT_ON_CLICK, strlen(CTR_DICT_ON_CLICK), args);
			ctr_heap_free(args);
			if (CtrStdFlow) {
				return 1;
			}
		}
	}
	return 0;
}

void ctr_internal_media_keydown_left(int* dir) {
	if (focusObject) {
		ctr_internal_media_arrowkey_selection();
		ctr_internal_cursormove(-1, 0);
		if (CtrMediaSelectStart) {
			CtrMediaSelectEnd = CtrMediaInputIndex;
			ctr_internal_img_render_text(focusObject);
		}
	} else {
		if (controllableObject && CtrMediaInputFreeze == CtrStdBoolFalse) {
			if (CtrMediaControlMode == 1 || CtrMediaControlMode == 3) {
				*dir = 180;
			} else if (CtrMediaControlMode == 4) {
				CtrMediaRotation = 5;
			}
		}
	}
}

void ctr_internal_media_keydown_right(int* dir) {
	if (focusObject) {
		ctr_internal_media_arrowkey_selection();
		ctr_internal_cursormove(1, 0);
		if (CtrMediaSelectStart) {
			CtrMediaSelectEnd = CtrMediaInputIndex;
			ctr_internal_img_render_text(focusObject);
		} else {
			CtrMediaSelectBegin=0;
			CtrMediaSelectEnd=0;
			ctr_internal_img_render_text(focusObject);
		}
	} else {
		if (controllableObject && CtrMediaInputFreeze == CtrStdBoolFalse) {
			if (CtrMediaControlMode == 1 || CtrMediaControlMode == 3) {
				*dir = 0;
			} else if (CtrMediaControlMode == 4) {
				CtrMediaRotation = -5;
			}
		}
	}
}

void ctr_internal_media_keydown_down(int* dir, int* c4speed) {
	MediaIMG* player;
	if (focusObject) {
		ctr_internal_media_arrowkey_selection();
		ctr_internal_cursormove(0,1);
		if (CtrMediaSelectStart) {
			CtrMediaSelectEnd = CtrMediaInputIndex;
			ctr_internal_img_render_text(focusObject);
		}
	} else {
		if (controllableObject && CtrMediaInputFreeze == CtrStdBoolFalse) {
			player = (MediaIMG*) controllableObject->value.rvalue->ptr;
			if ( player->gravity < 1 && (CtrMediaControlMode == 1 || CtrMediaControlMode == 2)) {
				*dir = 270;
			} else if (CtrMediaControlMode == 4) {
				*c4speed = -1;
			}
		}
	}
}

void ctr_internal_media_keyup_down(int* dir, int* c4speed) {
	MediaIMG* player;
	player = ctr_internal_media_getplayer();
	if (CtrMediaControlMode != 4) {
		if (player && player->gravity < 1) {
			*dir = -1;
		}
	} else {
		*c4speed = 0;
	}
}

void ctr_internal_media_keyup_right(int* dir, int* c4speed) {
	if (CtrMediaControlMode != 4) {
		*dir = -1;
	} else {
		CtrMediaRotation = 0;
	}
}

void ctr_internal_media_keydown_up(int* dir, int* c4speed) {
	MediaIMG* player;
	if (focusObject) {
		ctr_internal_media_arrowkey_selection();
		ctr_internal_cursormove(0, -1);
		if (CtrMediaSelectStart) {
			CtrMediaSelectEnd = CtrMediaInputIndex;
			ctr_internal_img_render_text(focusObject);
		}
	} else {
		if (controllableObject && CtrMediaInputFreeze == CtrStdBoolFalse) {
			player = (MediaIMG*) controllableObject->value.rvalue->ptr;
			if (player->gravity >= 1 && CtrMediaControlMode == 1) {
				if (CtrMediaJump == 0) {
					CtrMediaJump = 1;
					if (CtrMediaFXFlagJumpSFX && CtrMediaFXFlagJumpSound) {
						MediaAUD* jumpsound = ctr_internal_get_audio_from_object(CtrMediaFXFlagJumpSound);
						if (jumpsound) {
							Mix_PlayChannel(1, (Mix_Chunk*) jumpsound->blob, 0);
						}
					}
				}
			}
			else if ((CtrMediaControlMode == 1 || CtrMediaControlMode == 2)) {
				*dir = 90;
			}
			else if (CtrMediaControlMode == 4) {
				*c4speed = 1;
			}
		}
	}
}

void ctr_internal_media_keydown_del() {
	if (focusObject) {
		MediaIMG* focusImage = (MediaIMG*) focusObject->value.rvalue->ptr;
		if (ctr_internal_media_has_selection()) {
			ctr_img_text_del(focusObject, NULL);
		} else {
			ctr_internal_media_textinsert(focusImage, "\x7F");
			ctr_internal_img_render_text(focusObject);
		}
	}
}

void ctr_internal_media_detect_collisions(MediaIMG* m, SDL_Rect r) {
	MediaIMG* player;
	SDL_Rect r2;
	player = NULL;
	player = ctr_internal_media_getplayer();
	ctr_argument* collider = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	collider->object = CtrStdNil;
	collider->next = NULL;
	for (int j = 0; j < IMGCount; j++) {
		MediaIMG* m2 = &mediaIMGs[j];
		//skip if image is same, fixed or gc'ed (ref)
		if (m2 == m || m->fixed || m2->fixed || m->ref == NULL || m2->ref == NULL) continue;
		int h,w,w2;
		h = (int) m->h;
		w = (int) m->w / (m->anims ? m->anims : 1);
		w2 = (int) m2->w / (m2->anims ? m2->anims : 1);
		r2 = ctr_internal_media_image_maprect(m2);
		if (SDL_HasIntersection(&r,&r2)) {
			if (m2->solid && !m->solid && !m->ghost) {
				if (m->gravity && m->x+w >= m2->x && m->x <= m2->x+w2 && m->y+h <= m2->y+(m->h/2)) {
						m->y = m2->y - h + 1;
						m->gspeed = 0;
						if (m == player) CtrMediaContactSurface = m2;
				} else {
						ctr_internal_media_image_resolvecollision(m, m2);
						m->mov = 0;
				}
				if (player && m == player && CtrMediaJump == 2) {
					CtrMediaJump = 3;
				} else if (player && m == player && CtrMediaJump == 3) {
					CtrMediaJump = 0;
				}
				if (m->bounce) {
					m->dir = (double) (((int)m->dir + 145) % 360);
					m->gspeed = 0;
				}
			}
			if (m->collidable) {
				collider->object = m2->ref;
				ctr_send_message(m->ref, CTR_DICT_COLLISION_SET, strlen(CTR_DICT_COLLISION_SET), collider );
			}
			if (m2->collidable) {
				collider->object = m->ref;
				ctr_send_message(m2->ref, CTR_DICT_COLLISION_SET, strlen(CTR_DICT_COLLISION_SET), collider );
			}
		}
	}
	ctr_heap_free(collider);
}

void ctr_internal_media_camera(MediaIMG* m, SDL_Rect* s, SDL_Rect* r, MediaIMG* player) {
	static SDL_Rect camera;
	int border = CtrMediaCamera.w * 0.3;
	camera.w = CtrMediaCamera.w;
	camera.h = CtrMediaCamera.h;
	int left = camera.x + border;
	int right = camera.x + camera.w - border;
	int top = camera.y + border;
	int bottom = camera.y + camera.h - border;
	int cpx = 0;
	int cpy = 0;
	if (player) {
		cpx = player->x + ((player->w / player->anims)/2);
		cpy = player->y + (player->h/2);
	}
	if (cpx > right && camera.x < windowWidth - camera.w) {
		camera.x ++;
	}
	if (cpx < left && camera.x > 0) {
		camera.x --;
	}
	if (cpy > bottom && camera.y < windowHeight - camera.h) {
		camera.y ++;
	}
	if (cpy < top && camera.y > 0) {
		camera.y --;
	}
	if (!CtrMediaCameraInit) {
		camera.x = 0;
		camera.y = 0;
		if (CtrMediaZoom) {
			SDL_RenderSetLogicalSize(CtrMediaRenderer, camera.w, camera.h);
		}
		CtrMediaCameraInit = 1;
	}
	if (m == NULL) {
		s->x = camera.x;
		s->y = camera.y;
		s->w = camera.w;
		s->h = camera.h;
		r->x = 0;
		r->y = 0;
		r->w = camera.w;
		r->h = camera.h;
	} else {
		r->x = r->x - camera.x;
        r->y = r->y - camera.y;
		//If image is outside camera views, just set dimensions to 0.
		//Don't try to render half images by reducing size, this will get nasty
		//because we tend to flip images (keep it simple!)
		if (r->x > camera.w || (r->x + r->w)<0 || r->y > camera.h || (r->y+r->h)<0 ) {
			r->w = 0;
			r->h = 0;
		}
	}
	r->x += CtrMediaViewport.x;
	r->y += CtrMediaViewport.y;
}

void ctr_internal_media_render_image(MediaIMG* m, SDL_Rect r, SDL_Rect s, MediaIMG* player) {
	ctr_internal_media_anim_frames(m, &r, &s);
	if (CtrMediaCamera.w > 0 && CtrMediaCamera.h > 0 && !m->fixed) {
		ctr_internal_media_camera(m, &s, &r, player);
	}
	if (!r.w || !r.h) {
		return;
	}
	if (m->dir > -1 && !m->solid && !m->nodirani && CtrMediaControlMode == 1) {
		if (m->gravity) {
			int xdir = m->dir;
			if (m->gravity < 1) {
				/* drifting/hovering (space ship, fish) - in this case going up/down you don't want to change direction */
				if (m->dir == 180 || m->dir == 0) {
					m->lastdir = m->dir;
				}
				if (m->lastdir != -1 && m->dir != 180 && m->dir != 0) {
					xdir = m->lastdir;
				}
				if (m == player) {
					SDL_RenderCopyEx(CtrMediaRenderer, m->texture, &s, &r, 0, NULL, ( xdir == 180 ) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
				} else {
					// for other objects no clear left/right (because user fills in custom x/y)
					SDL_RenderCopyEx(CtrMediaRenderer, m->texture, &s, &r, 0, NULL, ( xdir >= 90 && xdir <= 270  ) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
				}
			} else {
				/* platform game style (Mario/Sonic/Giana-style) - in this case adjust direction of image based on 360 degrees (for ease of use) */
				SDL_RenderCopyEx(CtrMediaRenderer, m->texture, &s, &r, 0, NULL, ( xdir >= 180 && xdir <= 270  ) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
			
			}
		} else {
			/* no gravity, top-down game style */
			SDL_RenderCopyEx(CtrMediaRenderer, m->texture, &s, &r, (m->dir == -1 ? 0 : m->dir), NULL, (m->dir == 90 || m->dir == 270) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
		}
	}
	else if (CtrMediaControlMode == 4 && !m->solid && !m->nodirani) {
		/* radius style (2D top-down racing), in this case adjust direction to exact degrees */
		SDL_RenderCopyEx(CtrMediaRenderer, m->texture, &s, &r, 360-(m->dir == -1 ? 0 : m->dir), NULL, SDL_FLIP_NONE);
	} 
	else {
		/* for other control modes (breakout/pong) or custom mode */
		SDL_RenderCopy(CtrMediaRenderer, m->texture, &s, &r);
	}
}

void ctr_internal_media_image_calculate_motion(MediaIMG* m) {
	MediaIMG* player = NULL;
	// keep a constant physics speed by calculating the timediff
	#ifdef TEST
	double dt = 1;
	#else
	double delta_in_seconds = ((CtrMediaTicks2 - CtrMediaTicks1) / 1000.0f );
	double dt  = 60 * delta_in_seconds;
	#endif
	if (controllableObject != NULL) {
		player = (MediaIMG*) controllableObject->value.rvalue->ptr;
		if (CtrMediaJump == 2 && player == m) {
			player->y = player->y - (log(CtrMediaJumpHeight) * dt);
			CtrMediaJumpHeight /= pow(1.2, dt);
			if (CtrMediaJumpHeight < 1) {
				CtrMediaJump = 3;
				CtrMediaJumpHeight = 0;
			}
		}
	}
	// If you want to test, insert a random delay and observe that
	// game speed is still the same
	// CtrMediaStdDelayTime = rand() % 100;
	if (m->mov < m->speed && ((controllableObject && m != controllableObject->value.rvalue->ptr)  || (controllableObject == NULL))) {
			m->mov += m->speed * m->accel;
	}
	if (m->fric > 0) {
		if (m->mov > 0) {
			m->mov = (m->fric > m->mov) ? 0 : m->mov - m->fric;
		}
	}
	// Bugfix: can't set speed to lower value
	if (CtrMediaVersionTime >= CTR_VERSON_TIME_ID_1_0_3) {
		if (m->mov > m->speed) m->mov = m->speed;
	}
	if (!m->ghost && m->gravity > 0 && m->y < windowHeight - m->h) {
		if (m->gravity >= 1) {
			m->gspeed += (m->gravity * 0.1) * dt;
			m->y += dt * m->gspeed;
			if (CtrMediaContactSurface && m == player) {
				if (!(
				m->gravity &&
				m->x+(m->w/m->anims) >= CtrMediaContactSurface->x &&
				m->x <= CtrMediaContactSurface->x+CtrMediaContactSurface->w/CtrMediaContactSurface->anims &&
				m->y+m->h <= CtrMediaContactSurface->y+(m->h/2))) {
					CtrMediaContactSurface = NULL;
				}
			}
		} else if (m->gravity >= 0.1){
			m->y += dt * m->gravity;
		}
	} else {
		if (controllableObject != NULL) {
			player = (MediaIMG*) controllableObject->value.rvalue->ptr;
			if (m == player) CtrMediaJump = 0;
		}
		m->gspeed = 0;
	}
	if (m->mov > 0 && m->dir > -1) {
		if (m->x != m->tx)
		m->x += dt * m->mov * cos(m->dir * M_PI / 180);
		if (m->y != m->ty)
		m->y -= dt * m->mov * sin(m->dir * M_PI / 180);
		// an image reaches its destination if the destination falls between
		// this step and the previous one (i.e. tx is between ox and x and ty
		// between oy and ty). Note that gravity etc. might interfere with this,
		// because it adjust ox/oy. @todo maybe tweak this? (but keep backward compat.)
		if (
			// is X reached?
			((round(m->tx) >= round(m->ox) && round(m->tx) <= round(m->x))
			|| (round(m->tx) <= round(m->ox) && round(m->tx) >= round(m->x)))
			&&
			// is Y reached?
			((round(m->ty) >= round(m->oy) && round(m->ty) <= round(m->y))
			|| (round(m->ty) <= round(m->oy) && round(m->ty) >= round(m->y)))
		) {
			m->dir = -1;
			ctr_argument* a;
			a = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
			ctr_send_message(m->ref, CTR_DICT_STOP_AT_SET, strlen(CTR_DICT_STOP_AT_SET), a );
			ctr_heap_free(a);
		}
	}
	if (controllableObject) {
		player = (MediaIMG*) controllableObject->value.rvalue->ptr;
		if (CtrMediaControlMode == 1 && m == player && CtrMediaContactSurface && CtrMediaContactSurface->mov && CtrMediaContactSurface->dir > -1){
			m->x += dt * CtrMediaContactSurface->mov * cos(CtrMediaContactSurface->dir * M_PI / 180);
			m->y -= dt * CtrMediaContactSurface->mov * sin(CtrMediaContactSurface->dir * M_PI / 180);
		}
		if (m == player && CtrMediaControlMode < 5) {
			int xmin = 0;
			int ymin = 0;
			int xmax = windowWidth - (m->w/m->anims);
			int ymax = windowHeight - (m->h);
			if (m->x > xmax) m->x = xmax;
			if (m->y > ymax) m->y = ymax;
			if (m->x < xmin) m->x = xmin;
			if (CtrMediaControlMode != 1 || !CtrMediaJump) {
				// in platform games, players are allowed to jump 'out of vision' at the top
				if (m->y < ymin) m->y = ymin;
			}
		}
	}
}

SDL_RWops* ctr_internal_media_load_asset(char* asset_name, char asset_type);


char ctr_internal_media_determine_filetype(char* path) {
	char magic[20];
	memset(magic, 0, 20);
	SDL_RWops* asset_reader = ctr_internal_media_load_asset(path, 1);
	if (asset_reader == NULL) {
		ctr_error(CTR_ERR_FOPEN, 0);
		return 0;
	}
	SDL_RWread(asset_reader, magic, 1, 20);
	if (strncmp(magic, "\xFF\xD8", 2)==0) return 20; //JPG
	if (strncmp(magic, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A",8)==0) return 30; //PNG
	return 0;
}

ctr_object* ctr_media_end(ctr_object* myself, ctr_argument* argumentList) {
	CtrMediaBreakLoopFlag = 1;
	return myself;
}

void ctr_media_event(ctr_object* myself, char* event, const char* detail) {
	ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
	args->object = ctr_build_string_from_cstring((char*)detail);
	myself->info.sticky = 1;
	ctr_send_message(myself, event, strlen(event), args );
	myself->info.sticky = 0;
	ctr_heap_free(args);
}

void ctr_media_event_timer(ctr_object* myself, char* event, int timer_no) {
	ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
	args->object = ctr_build_number_from_float((double)timer_no);
	myself->info.sticky = 1;
	ctr_send_message(myself, event, strlen(event), args );
	myself->info.sticky = 0;
	ctr_heap_free(args);
}

void ctr_media_event_coords(ctr_object* myself, char* event, int x, int y) {
	ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
	args->object = ctr_build_number_from_float((double)x);
	ctr_argument* next = ctr_heap_allocate(sizeof(ctr_argument));
	args->next = next;
	next->object = ctr_build_number_from_float((double)y);
	myself->info.sticky = 1;
	ctr_send_message(myself, event, strlen(event), args );
	myself->info.sticky = 0;
	ctr_heap_free(args->next);
	ctr_heap_free(args);
}

/**
 * @def
 * [ Media ] timer: [ Number ] after: [ Number ]
 * 
 * @example
 * >> media := Media new.
 * >> i := 0.
 * media on: ['start'] do: {
 * Out write: ['start'], stop.
 * media timer: 1 after: 12.
 * }.
 * media on: ['step'] do: {
 * Out write: i, stop.
 * i add: 1.
 * }.
 * media on: ['timer:'] do: { :t
 * Out write: ['timer:'] + t, stop.
 * media end.
 * }.
 * media screen: ['canvas.png'].
 */
ctr_object* ctr_media_timer(ctr_object* myself, ctr_argument* argumentList) {
	int timer_no = (int) ctr_tonum(ctr_internal_cast2number(argumentList->object));
	int ms = (int) ctr_tonum(ctr_internal_cast2number(argumentList->next->object));
	if (timer_no < 1 || timer_no > CtrMaxMediaTimers) {
		ctr_error("Invalid timer", 0);
	} else if ( ms > -1 ) {
		CtrMediaTimers[timer_no] = CtrMediaTicks2 + ms;
	} else {
		CtrMediaTimers[timer_no] = -1;
	}
	return myself;
}

void ctr_internal_media_update_timers(ctr_object* media) {
	for(int i = 1; i < CtrMaxMediaTimers; i++) {
		if (CtrMediaTimers[i] < 0) continue;
		if (CtrMediaTimers[i] < CtrMediaTicks2) {
			CtrMediaTimers[i] = -1;
			ctr_media_event_timer(media, CTR_DICT_ON_TIMER, i);
		}
	}
}

/**
 * @def
 * [ Media ] screen: [ Text ]
 * 
 * @example
 * >> media := Media new.
 * 
 * media on: ['start'] do: { ... }.
 * media on: ['step'] do: { ... }.
 * media on: ['key:'] do: { ... }.
 * media on: ['key-down:'] do: { ... }.
 * media on: ['gamepad:'] do: { ... }.
 * media on: ['gamepad-down:'] do: { ... }.
 * media on: ['click-x:y:'] do: { ... }.
 * 
 * media screen: ['canvas.png'].
 * 
 * @result
 * @info-media-screen
 */
ctr_object* ctr_media_screen(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* player;
	MediaIMG* focusImage;
	// prevent running multiple loops.
	if (CtrMediaScreenActive) return myself;
	int cbutton;
	int x = 0, y = 0, dir, c4speed;
	CtrMediaInputIndex = 0;
	CtrMediaSelectStart =0;
	CtrMediaSelectBegin = 0;
	CtrMediaSelectEnd=0;
	CtrMediaCameraInit = 0;
	focusObject = NULL;
	CtrMediaSteps = 0;
	SDL_Rect dimensions;
	SDL_Texture* texture;
	CtrMediaVersionTime = ctr_internal_versiontime(); //can change per screen
	char* imageFileStr = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	char ftype = ctr_internal_media_determine_filetype(imageFileStr);
	if (ftype == 0) {
		ctr_heap_free(imageFileStr);
		ctr_error(CTR_ERR_FOPEN, 0);
		return myself;
	}
	for(int i = 0; i < IMGCount; i++) {
		mediaIMGs[i].mov = 0;
		mediaIMGs[i].gspeed = 0;
	}
	SDL_RWops* res = NULL;
	res = ctr_internal_media_load_asset(imageFileStr, 1);
	texture = IMG_LoadTexture_RW(CtrMediaRenderer, res, 0);
	SDL_QueryTexture(texture, NULL, NULL, &dimensions.w, &dimensions.h);
	dimensions.x = x;
	dimensions.y = y;
	SDL_SetWindowSize(CtrMediaWindow, dimensions.w, dimensions.h);
	SDL_SetWindowPosition(CtrMediaWindow,SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED);
	SDL_Delay(100);
	SDL_RenderCopy(CtrMediaRenderer, texture, NULL, &dimensions);
	ctr_heap_free(imageFileStr);
	windowWidth = dimensions.w;
	windowHeight = dimensions.h;
	SDL_GL_GetDrawableSize(CtrMediaWindow, &CtrMediaDrawSizeX, &CtrMediaDrawSizeY);
	ctr_send_message(myself, CTR_DICT_RUN, strlen(CTR_DICT_RUN), NULL );
	if (CtrStdFlow) return myself;
	SDL_Event event;
	dir = -1;
	c4speed = 0;
	CtrMediaScreenActive = 1;
	CtrMediaTicks1 = SDL_GetTicks64(); //reset ticks, otherwise a dialog at the begin will cause motions to be far 'further'
	while (!CtrStdFlow) {
		if (CtrMediaFlagSoftwareVSync) {
			CtrMediaPerfCountStart = SDL_GetPerformanceCounter();
		}
		ctr_gc_cycle();
		SDL_RenderClear(CtrMediaRenderer);
		player = NULL;
		if (controllableObject) {
			player = (MediaIMG*) controllableObject->value.rvalue->ptr;
		}
		SDL_Rect s = dimensions;
		if (CtrMediaCamera.w > 0 && CtrMediaCamera.h > 0) {
			ctr_internal_media_camera(NULL, &s, &dimensions, player);
		}
		SDL_RenderCopy(CtrMediaRenderer, texture, &s, &dimensions);
		myself->info.sticky = 1;
		if (CtrMediaEventListenFlagStep) {
			ctr_send_message(myself, CTR_DICT_ON_STEP, strlen(CTR_DICT_ON_STEP), NULL );
		}
		myself->info.sticky = 0;
		if (CtrMediaBreakLoopFlag) {
			CtrMediaBreakLoopFlag = 0;
			CtrMediaScreenActive = 0;
			return myself;
		}
		//Update timers, both outside and inside event loop (otherwise you could stall it)
		if (CtrMediaEventListenFlagTimer) ctr_internal_media_update_timers(myself);
		while (SDL_PollEvent(&event)) {
			if (CtrMediaEventListenFlagTimer) ctr_internal_media_update_timers(myself);
			player = NULL;
			focusImage = NULL;
			if (controllableObject) {
				player = (MediaIMG*) controllableObject->value.rvalue->ptr;
			}
			if (focusObject) {
				focusImage = ctr_internal_media_getfocusimage();
			}
			switch (event.type) {
				case SDL_QUIT:
					if (mediaObject) {
					ctr_argument* args = ctr_heap_allocate(sizeof(ctr_argument));
					ctr_send_message(mediaObject,CTR_DICT_END, strlen(CTR_DICT_END), args);
					ctr_heap_free(args);
					ctr_internal_media_reset();
					CtrStdFlow = CtrStdExit;
					return myself;
					}
					break;
				case SDL_MOUSEWHEEL:
					if (focusObject) { //only for text areas
						if (event.wheel.y>0) {
							ctr_internal_media_keydown_up(NULL, NULL);
						} else {
							ctr_internal_media_keydown_down(NULL, NULL);
						}
					}
					break;
				case SDL_MOUSEMOTION:
					if (CtrMediaSelectStart && focusImage) {
							focusImage = ctr_internal_media_getfocusimage();
							ctr_internal_media_infercursorpos(focusImage, event.button.x, event.button.y);
							CtrMediaSelectEnd = CtrMediaInputIndex;
							ctr_internal_img_render_text(focusImage->ref);
					}
					break;
				case SDL_MOUSEBUTTONUP:
					if (CtrMediaSelectStart) {
							MediaIMG* focusImage = ctr_internal_media_getfocusimage();
							ctr_internal_media_infercursorpos(focusImage, event.button.x, event.button.y);
							CtrMediaSelectEnd = CtrMediaInputIndex;
							CtrMediaSelectStart = 0;
							ctr_internal_img_render_text(focusImage->ref);
					}
					if (
					event.button.button == SDL_BUTTON_LEFT &&
					event.button.clicks == 1 &&
					CtrMediaEventListenFlagMouseClick
					) {
						ctr_media_event_coords(myself, CTR_DICT_ON_CLICK_XY, event.button.x, event.button.y);
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					//If we have lost the focus, regain upon click
					if (!SDL_GetKeyboardFocus()) {
						SDL_RaiseWindow(CtrMediaWindow);
						SDL_SetWindowInputFocus(CtrMediaWindow); //might return an error, but what can we do?
					}
					if (ctr_internal_media_mouse_down(event)) return CtrStdFlow;
					break;
				case SDL_FINGERDOWN:
					if (event.tfinger.y > 0.75) {
						if (event.tfinger.x < 0.1) {
							ctr_internal_media_keydown_left(&dir);
						}
						if (event.tfinger.x > 0.1 && event.tfinger.x < 0.2) {
							ctr_internal_media_keydown_right(&dir);
						}
					}
					if (event.tfinger.x > 0.9) {
							if (event.tfinger.y > 0.8) {
								ctr_internal_media_keydown_down(&dir, &c4speed);
							}
							if (event.tfinger.y < 0.8 && event.tfinger.y > 0.5) {
								ctr_internal_media_keydown_up(&dir, &c4speed);
							}
						}
					break;
				case SDL_FINGERUP:
						ctr_internal_media_keyup_right(&dir, &c4speed);
						ctr_internal_media_keyup_down(&dir, &c4speed);
					break;
				case SDL_CONTROLLERBUTTONDOWN:
					cbutton = event.cbutton.button;
					if (CtrMediaEventListenFlagGamePadBtnDown) {
						ctr_media_event(myself, CTR_DICT_ON_GAMEPAD_DOWN, SDL_GameControllerGetStringForButton(cbutton));
					}
					/* FX: remap ABXY -> up */
					if (CtrMediaFXFlagMapABXY2Up == CTR_MEDIA_FX_FLAG_MEDIA_REMAP_ALL && (
							cbutton == SDL_CONTROLLER_BUTTON_A ||
							cbutton == SDL_CONTROLLER_BUTTON_B ||
							cbutton == SDL_CONTROLLER_BUTTON_X ||
							cbutton == SDL_CONTROLLER_BUTTON_Y
						)
					) {
						cbutton = SDL_CONTROLLER_BUTTON_DPAD_UP;
						// resend as remapped
						ctr_media_event(myself, CTR_DICT_ON_GAMEPAD_DOWN, SDL_GameControllerGetStringForButton(cbutton));
					}
					switch(cbutton) {
						case SDL_CONTROLLER_BUTTON_DPAD_UP:
							ctr_internal_media_keydown_up(&dir, &c4speed);
							break;
						case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
							ctr_internal_media_keydown_down(&dir, &c4speed);
							break;
						case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
							ctr_internal_media_keydown_left(&dir);
							break;
						case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
							ctr_internal_media_keydown_right(&dir);
							break;
						default:
							break;
					}
					break;
				case SDL_CONTROLLERBUTTONUP:
					cbutton = event.cbutton.button;
					if (CtrMediaEventListenFlagGamePadBtnUp) {
						ctr_media_event(myself, CTR_DICT_ON_GAMEPAD_UP, SDL_GameControllerGetStringForButton(cbutton));
					}
					/* FX: remap ABXY -> up */
					if (CtrMediaFXFlagMapABXY2Up == CTR_MEDIA_FX_FLAG_MEDIA_REMAP_ALL && (
							cbutton == SDL_CONTROLLER_BUTTON_A ||
							cbutton == SDL_CONTROLLER_BUTTON_B ||
							cbutton == SDL_CONTROLLER_BUTTON_X ||
							cbutton == SDL_CONTROLLER_BUTTON_Y
						)
					) {
						cbutton = SDL_CONTROLLER_BUTTON_DPAD_UP;
						//resend as remapped
						ctr_media_event(myself, CTR_DICT_ON_GAMEPAD_UP, SDL_GameControllerGetStringForButton(cbutton));
					}
					switch(cbutton) {
						case SDL_CONTROLLER_BUTTON_DPAD_UP:
						case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
							ctr_internal_media_keyup_down(&dir, &c4speed);
							break;
						case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
						case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
							ctr_internal_media_keyup_right(&dir, &c4speed);
							break;
						default:
							break;
					}
					break;
				case SDL_KEYUP:
					if (CtrMediaEventListenFlagKeyUp) {
						ctr_media_event(myself, CTR_DICT_ON_KEY_UP, SDL_GetKeyName(event.key.keysym.sym));
					}
					if (CtrMediaFXFlagSmoothGameControl && CtrMediaLastScanCode != event.key.keysym.scancode) break;
					switch(event.key.keysym.scancode) {
						case SDL_SCANCODE_LEFT:
						case SDL_SCANCODE_RIGHT:
							ctr_internal_media_keyup_right(&dir, &c4speed);
							break;
						case SDL_SCANCODE_UP:
						case SDL_SCANCODE_DOWN:
							ctr_internal_media_keyup_down(&dir, &c4speed);
							break;
						case SDL_SCANCODE_LSHIFT:
						case SDL_SCANCODE_RSHIFT:
							CtrMediaSelectStart = 0;
							break;
						default:
							break;
					}
					break;
				case SDL_KEYDOWN:
					if (CtrMediaEventListenFlagKeyDown) {
						ctr_media_event(myself, CTR_DICT_ON_KEY_DOWN, SDL_GetKeyName(event.key.keysym.sym));
					}
					CtrMediaLastScanCode = event.key.keysym.scancode;
					switch(event.key.keysym.scancode) {
						case SDL_SCANCODE_LSHIFT:
						case SDL_SCANCODE_RSHIFT:
							CtrMediaSelectBegin=CtrMediaInputIndex;
							CtrMediaSelectStart = 1;
							break;
						case SDL_SCANCODE_RETURN:
							if (focusObject) {
								MediaIMG* focusImage = (MediaIMG*) focusObject->value.rvalue->ptr;
								ctr_internal_media_textinsert(focusImage, "\r\n");
								ctr_internal_img_render_text(focusObject);
							}
							break;
						case SDL_SCANCODE_TAB:
							if (focusObject) {
								MediaIMG* focusImage = (MediaIMG*) focusObject->value.rvalue->ptr;
								ctr_internal_media_textinsert(focusImage, "    ");
								ctr_internal_img_render_text(focusObject);
							}
							break;
						case SDL_SCANCODE_BACKSPACE:
							if (focusObject) {
								MediaIMG* focusImage = (MediaIMG*) focusObject->value.rvalue->ptr;
								if (ctr_internal_media_has_selection()) {
									ctr_img_text_del(focusObject, NULL);
								} else {
									ctr_internal_media_textinsert(focusImage, "\b");
									ctr_internal_img_render_text(focusObject);
								}
							}
							break;
						case SDL_SCANCODE_DELETE:
							ctr_internal_media_keydown_del();
							break;
						case SDL_SCANCODE_UP:
							ctr_internal_media_keydown_up(&dir, &c4speed);
							break;
						case SDL_SCANCODE_DOWN:
							ctr_internal_media_keydown_down(&dir, &c4speed);
							break;
						case SDL_SCANCODE_LEFT:
							ctr_internal_media_keydown_left(&dir);
							break;
						case SDL_SCANCODE_HOME:
							if (focusObject) {
								ctr_internal_cursormove(-2, 0);
								if (CtrMediaSelectStart) {
									CtrMediaSelectEnd = CtrMediaInputIndex;
									ctr_internal_img_render_text(focusObject);
								}
							}
							break;
						case SDL_SCANCODE_PAGEUP:
							if (focusObject) {
								ctr_internal_cursormove(0, -2);
								if (CtrMediaSelectStart) {
									CtrMediaSelectEnd = CtrMediaInputIndex;
									ctr_internal_img_render_text(focusObject);
								}
							}
							break;
						case SDL_SCANCODE_RIGHT:
							ctr_internal_media_keydown_right(&dir);
							break;
						case SDL_SCANCODE_END:
							if (focusObject) {
								ctr_internal_cursormove(2, 0);
								if (CtrMediaSelectStart) {
									CtrMediaSelectEnd = CtrMediaInputIndex;
									ctr_internal_img_render_text(focusObject);
								}
							}
							break;
						case SDL_SCANCODE_PAGEDOWN:
							if (focusObject) {
								ctr_internal_cursormove(0, 2);
								if (CtrMediaSelectStart) {
									CtrMediaSelectEnd = CtrMediaInputIndex;
									ctr_internal_img_render_text(focusObject);
								}
							}
							break;
						default:
							break;
					}
					break;
				case SDL_TEXTINPUT:
					ctr_internal_media_reset_selection();
					if (SDL_strlen(event.text.text) == 0){
						break;
					}
					if (focusObject) {
						MediaIMG* focusImage = (MediaIMG*) focusObject->value.rvalue->ptr;
						if (focusImage->editable) {
							ctr_internal_media_textinsert(focusImage,event.text.text);
							ctr_internal_img_render_text(focusObject);
						}
					}
					break;
				case SDL_TEXTEDITING:
					break;
				default:
					break;
			}
		}
		if (CtrMediaControlMode == 4) {
			dir += CtrMediaRotation;
			if (dir>359) dir = 0;
			if (dir<0) dir = 359;
		}
		if (dir > -1 && controllableObject) {
			player = (MediaIMG*) controllableObject->value.rvalue->ptr;
			player->dir = dir;
			player->mov = fmax(0, fmin(player->speed, (player->mov + player->speed * player->accel * (CtrMediaControlMode==4 ? c4speed : 1))));
		}
		if (controllableObject != NULL) {
			player = (MediaIMG*) controllableObject->value.rvalue->ptr;
			if (CtrMediaJump == 1 && CtrMediaJumpHeightFactor) {
				CtrMediaJumpHeight = player->h * CtrMediaJumpHeightFactor;
				CtrMediaJump = 2;
			}
		}
		CtrMediaTicks2 = SDL_GetTicks64();
		for(int i = 0; i < IMGCount; i ++) {
			MediaIMG* m = &mediaIMGs[i];
			if (!m->visible) continue;
			ctr_internal_media_image_calculate_motion(m);
			SDL_Rect r,s;
			r = ctr_internal_media_image_maprect(m);
			ctr_internal_media_detect_collisions(m,r);
			m->ox = m->x;
			m->oy = m->y;
			r.x = (int)(m->x);
			r.y = (int)(m->y);
			s.x = 0;
			s.y = 0;
			s.h = (int) m->h;
			s.w = (int) m->w/(m->anims ? m->anims : 1);
			ctr_internal_media_render_image(m,r,s, player);
		}
		CtrMediaTicks1 = CtrMediaTicks2;
		if (focusObject) {
			ctr_internal_img_render_cursor(focusObject);
		}
		SDL_RenderPresent(CtrMediaRenderer);
		if (CtrMediaFlagSoftwareVSync) {
			CtrMediaPerfCountEnd = SDL_GetPerformanceCounter();
			double elapsedMS = (CtrMediaPerfCountEnd - CtrMediaPerfCountStart) / (double)SDL_GetPerformanceFrequency() * 1000.0f;
			double delay = floor(16.666f - elapsedMS);
			if (delay > 0) SDL_Delay(delay); //Cap to 60 FPS
		}
		// The standard delay is almost never used, unless we are testing stuff...
		if (CtrMediaStdDelayTime > 0) {
			SDL_Delay(CtrMediaStdDelayTime);
		}
		CtrMediaSteps++;
		#ifdef __EMSCRIPTEN__
		emscripten_sleep(1);
		#endif
	}
	return myself;
}

/**
 * @def
 * Point
 * 
 * @example
 * >> media := Media new.
 * >> p := Point new x: 10 y: 20.
 * Out write: p x?, stop.
 * Out write: p y?, stop.
 */
ctr_object* ctr_point_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	instance->link = myself;
	ctr_internal_object_property(instance, "x", ctr_build_number_from_float(0));
	ctr_internal_object_property(instance, "y", ctr_build_number_from_float(0));
	return instance;
}

/**
 * @def
 * [ Point ] x?
 * 
 * @example
 * >> media := Media new.
 * >> p := Point new x: 10 y: 20.
 * Out write: p x?, stop.
 * Out write: p y?, stop.
 */
ctr_object* ctr_point_x(ctr_object* myself, ctr_argument* argumentList) {
	int x = (int) ctr_tonum(ctr_internal_object_property(myself, "x", NULL));
	return ctr_build_number_from_float(x);
}

/**
 * @def
 * [ Point ] y?
 * 
 * @example
 * >> media := Media new.
 * >> p := Point new x: 10 y: 20.
 * Out write: p x?, stop.
 * Out write: p y?, stop.
 */
ctr_object* ctr_point_y(ctr_object* myself, ctr_argument* argumentList) {
	int y = (int) ctr_tonum(ctr_internal_object_property(myself, "y", NULL));
	return ctr_build_number_from_float(y);
}

/**
 * @def
 * [ Point ] x: [ Number ] y: [ Number ]
 * 
 * @example
 * >> media := Media new.
 * >> p := Point new x: 10 y: 20.
 * Out write: p x?, stop.
 * Out write: p y?, stop.
 */
ctr_object* ctr_point_xyset(ctr_object* myself, ctr_argument* argumentList) {
	int x = (int) ctr_tonum(ctr_internal_cast2number(argumentList->object));
	int y = (int) ctr_tonum(ctr_internal_cast2number(argumentList->next->object));
	ctr_internal_object_property(myself, "x", ctr_build_number_from_float(x));
	ctr_internal_object_property(myself, "y", ctr_build_number_from_float(y));
	return myself;
}

ctr_object* ctr_line_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	instance->link = myself;
	ctr_internal_object_property(instance, CTR_DICT_FROM, CtrStdNil);
	ctr_internal_object_property(instance, CTR_DICT_TO, CtrStdNil);
	return instance;
}

/**
 * @def
 * [ Line ] from: [ Point ] to: [ Point ]
 * 
 * @example
 * >> media := Media new.
 * >> a := Point new x: 10 y: 10.
 * >> b := Point new x: 20 y: 20.
 * >> c := Line from: a to: b.
 * Out write: c from x?, stop.
 * Out write: c to y?, stop.
 */
ctr_object* ctr_line_from_to(ctr_object* myself, ctr_argument* argumentList) {
	ctr_internal_object_property(myself, CTR_DICT_FROM, argumentList->object);
	ctr_internal_object_property(myself, CTR_DICT_TO, argumentList->next->object);
	return myself;
}

ctr_object* ctr_line_start(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(myself, CTR_DICT_FROM, NULL);
}

ctr_object* ctr_line_end(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(myself, CTR_DICT_TO, NULL);
}

void ctr_font_destructor(ctr_resource* rs) {
	MediaFNT* fnt = (MediaFNT*) rs->ptr;
	// Check if resource still used by image
	// If so, emit a warning, but set the font to NULL anyway
	// to avoid segfault. Also, we cannot leave it open as we
	// will leak memory then.
	if (fnt->font) {
		for(int i=0; i<MaxIMG; i++) {
			if (mediaIMGs[i].font == fnt->font) {
				printf("Warning: GC'ed font is still in use by image resource #%d!\n", i);
				// This will probably make the program error out, but that's okay.
				// Same happens with images that dissapear because GC.
				mediaIMGs[i].font = NULL;
			}
		}
		TTF_CloseFont(fnt->font);
	}
	if (fnt->fscript) {
		ctr_heap_free(fnt->fscript);
	}
	fnt->ref = NULL;
}

ctr_object* ctr_font_new(ctr_object* myself, ctr_argument* argumentList) {
	
	// If there are no more fonts slots available, generate an exception
	if (FNTCount >= maxFNT - 1) {
		char error_message[500]; // keep error message < 500 chars
		if (!sprintf(error_message, CTR_ERR_MEDIA_FNT_DEPLETED, maxFNT - 1)) {
			// if we fail to generate a proper error message, generate a generic error
			ctr_error("Unable to generate error code for font depletion.", 0);
		}
		CtrStdFlow = ctr_error( error_message, 0 );
		return CtrStdNil;
	}
	
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	MediaFNT* fnt = &mediaFNT[FNTCount++];
	fnt->font = NULL;
	fnt->fscript = NULL;
	fnt->textdir = 0;
	fnt->ref = instance;
	ctr_resource* rs = ctr_heap_allocate( sizeof(ctr_resource) );
	rs->ptr = fnt;
	rs->destructor = &ctr_font_destructor;
	instance->value.rvalue = rs;
	return instance;
}



int ctr_internal_media_setfontdir(TTF_Font* fnt, int dircode) {
	if (dircode == 0) {
		return TTF_SetFontDirection(fnt, TTF_DIRECTION_LTR);
	} else if (dircode == 1)  {
		return TTF_SetFontDirection(fnt,TTF_DIRECTION_RTL);
	} else if (dircode == 2)  {
		return TTF_SetFontDirection(fnt,TTF_DIRECTION_TTB);
	} else if (dircode == 3)  {
		return TTF_SetFontDirection(fnt,TTF_DIRECTION_BTT);
	}
	return -1;
}

/**
 * @def
 * [ Font ] new
 *
 * @example
 * >> media := Media new.
 * >> img := Image new.
 * img font: (Font new source: ['font.ttf'] size: 20).
 * 
 * @result
 * @info-font-source-size
 */
ctr_object* ctr_font_font(ctr_object* myself, ctr_argument* argumentList) {
	MediaFNT* fnt = ctr_internal_get_font_from_object(myself);
	if (fnt == NULL) return myself;
	char* path = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	SDL_RWops* res = ctr_internal_media_load_asset(path, 1);
	ctr_heap_free(path);
	if (res == NULL) {
		ctr_internal_media_fatalerror("Unable to load font", "TTF Font");
	}
	fnt->font = TTF_OpenFontRW(res, 1, (int)ctr_internal_cast2number(argumentList->next->object)->value.nvalue);
	/* Allow to set compile-time FONTSCRIPT for Harfbuzz shaper */
	#ifdef FONTSCRIPT
	int script_ok = TTF_SetFontScriptName(fnt->font, FONTSCRIPT);
	if (script_ok == -1) {
		ctr_print_error("Error setting font script.", -1);
	}
	#endif
	#ifdef FONTDIRECTION
	ctr_internal_media_setfontdir(fnt->font, FONTDIRECTION);
	#endif
	return myself;
}

/**
 * @def
 * [ Font ] style: [ Text ] direction: [ Number ]
 *
 * @example
 * font style: ['Arab'] direction: 1.
 * 
 * @result
 * @info-font-style-direction
 */
ctr_object* ctr_font_script_dir(ctr_object* myself, ctr_argument* argumentList) {
	MediaFNT* fnt = ctr_internal_get_font_from_object(myself);
	if (fnt == NULL) return myself;
	fnt->fscript = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	if (TTF_SetFontScriptName(fnt->font, fnt->fscript) == -1) {
		ctr_error("Error setting font script.", 0);
		return myself;
	}
	fnt->textdir = ctr_tonum(argumentList->next->object);
	if (ctr_internal_media_setfontdir(fnt->font, fnt->textdir) == -1) {
		ctr_error("Error setting text direction.", 0);
		return myself;
	}
	return myself;
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
 * [ Color ] red: [Number] green: [Number] blue: [Number]
 *
 * @example
 * >> media := Media new.
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

void ctr_media_destructor(ctr_resource* rs) {
	ctr_internal_media_clear_edcache();
}

ctr_object* ctr_media_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	// although media does not have a real resource we want to register
	// a destructor to clean-up stuff (like edcache)
	ctr_resource* rs = ctr_heap_allocate( sizeof(ctr_resource) );
	rs->ptr = NULL;
	rs->destructor = &ctr_media_destructor;
	instance->link = myself;
	instance->value.rvalue = rs;
	instance->info.sticky = 1; // just in case, dont let gc clean it up
	return instance;
}

ctr_object* ctr_audio_new(ctr_object* myself, ctr_argument* argumentList) {
	
	// If there are no more fonts slots available, generate an exception
	if (AUDCount >= maxAUD - 1) {
		char error_message[500]; // keep error message < 500 chars
		if (!sprintf(error_message, CTR_ERR_MEDIA_SFX_DEPLETED, maxAUD - 1)) {
			// if we fail to generate a proper error message, generate a generic error
			ctr_error("Unable to generate error code for sfx/music depletion.", 0);
		}
		CtrStdFlow = ctr_error( error_message, 0 );
		return CtrStdNil;
	}
	
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	return instance;
}

void ctr_music_destructor(ctr_resource* rs) {
	MediaAUD* mediaAUD = (MediaAUD*) rs->ptr;
	Mix_FreeMusic(mediaAUD->blob);
	mediaAUD->ref = NULL;
}

void ctr_sound_destructor(ctr_resource* rs) {
	MediaAUD* mediaAUD = (MediaAUD*) rs->ptr;
	Mix_FreeChunk(mediaAUD->blob);
	mediaAUD->ref = NULL;
}


ctr_object* ctr_sound_new_set(ctr_object* myself, ctr_argument* argumentList) {
	if (AUDCount >= maxAUD) return CtrStdNil;
	char* audioFileStr = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	ctr_object* audioInst = ctr_audio_new(myself, argumentList);
	MediaAUD* mediaAUD = &mediaAUDs[AUDCount];
	ctr_resource* rs = ctr_heap_allocate( sizeof(ctr_resource) );
	audioInst->value.rvalue = rs;
	rs->ptr = mediaAUD;
	if (mediaAUD->blob != NULL) {
		Mix_FreeChunk((Mix_Chunk*)mediaAUD->blob);
	}
	SDL_RWops* res = ctr_internal_media_load_asset(audioFileStr, 1);
	if (res) {
		mediaAUD->blob = (void*)Mix_LoadWAV_RW(res, 1);
	}
	if (mediaAUD->blob == NULL) {
		CtrStdFlow = ctr_build_string_from_cstring((char*)SDL_GetError());
	}
	rs->destructor = &ctr_sound_destructor;
	AUDCount++;
	ctr_heap_free(audioFileStr);
	return audioInst;
}


ctr_object* ctr_sound_play(ctr_object* myself, ctr_argument* argumentList) {
	MediaAUD* mediaAUD = ctr_internal_get_audio_from_object(myself);
	if (mediaAUD == NULL) return myself;
	if (mediaAUD->blob != NULL) {
		Mix_PlayChannel(0, (Mix_Chunk*) mediaAUD->blob, 0);
	}
	return myself;
}


SDL_RWops* ctr_internal_media_load_asset(char* asset_name, char asset_type) {
	SDL_RWops* res = NULL;
	// If we have no asset package, load from file instead
	if (CtrMediaAssetPackage == NULL) {
		res = SDL_RWFromFile(asset_name, "rb");
		return res;
	}
	char* path = ctr_heap_allocate_cstring(ctr_internal_object_property(CtrMediaAssetPackage, "path", NULL));
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


/**
 * @def
 * Audio
 *
 * @example
 * >> fx := Sound new: ['boom.mp3'].
 * fx play.
 * >> jazz := Music new: ['jazz.mp3'].
 * jazz play.
 * Moment wait: 1.
 * jazz silence.
 * jazz rewind.
 *
 * @result
 * @info-audio
 */
ctr_object* ctr_music_new_set(ctr_object* myself, ctr_argument* argumentList) {
	char* audioFileStr = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	ctr_object* audioInst = ctr_audio_new(myself, argumentList);
	MediaAUD* mediaAUD = &mediaAUDs[AUDCount];
	ctr_resource* rs = ctr_heap_allocate( sizeof(ctr_resource) );
	audioInst->value.rvalue = rs;
	rs->ptr = mediaAUD;
	if (mediaAUD->blob != NULL) {
		Mix_FreeMusic((Mix_Music*)mediaAUD->blob);
	}
	SDL_RWops* res = ctr_internal_media_load_asset(audioFileStr, 1);
	if (res) {
		mediaAUD->blob = (void*)Mix_LoadMUS_RW(res, 1);
	}
	if (mediaAUD->blob == NULL) {
		CtrStdFlow = ctr_build_string_from_cstring((char*)SDL_GetError());
	}
	rs->destructor = &ctr_music_destructor;
	AUDCount++;
	ctr_heap_free(audioFileStr);
	return audioInst;
}

ctr_object* ctr_music_play(ctr_object* myself, ctr_argument* argumentList) {
	MediaAUD* mediaAUD = ctr_internal_get_audio_from_object(myself);
	if (mediaAUD == NULL) return myself;
	if (mediaAUD->blob != NULL) {
			Mix_FadeInMusic((Mix_Music*)mediaAUD->blob,-1,0);
	}
	return myself;
}

ctr_object* ctr_music_silence(ctr_object* myself, ctr_argument* argumentList) {
	MediaAUD* mediaAUD = ctr_internal_get_audio_from_object(myself);
	if (mediaAUD == NULL) return myself;
	if (mediaAUD->blob != NULL) {
			Mix_PauseMusic();
	}
	return myself;
}

ctr_object* ctr_music_rewind(ctr_object* myself, ctr_argument* argumentList) {
	MediaAUD* mediaAUD = ctr_internal_get_audio_from_object(myself);
	if (mediaAUD == NULL) return myself;
	if (mediaAUD->blob != NULL) {
			Mix_RewindMusic();
	}
	return myself;
}


int CtrNetworkConnectedFlag = 0;
/**
 * @def
 * Network
 * 
 * @example
 * >> media := Media new.
 * >> network := Network new.
 * Out write: (
 * 	network 
 * 	send: None to: ['https://citrine-lang.org']
 * ), stop.
 * 
 * @result
 * <!DOCTYPE html>
 * <html lang="en">
 * <head>
 * <title>Localized Programming Language Citrine</title>
 * <meta charset="UTF-8">
 * ...etc...etc...
 * 
 */
ctr_object* ctr_network_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	return instance;
}

#ifdef LIBCURL
char* CtrMediaCurlBuffer;
size_t CtrMediaCurlBufferSize;
size_t CtrMediaCurlBytesRead;
size_t ctr_curl_write_callback(char* ptr, size_t size, size_t nmemb, void *userdata) {
	size_t len = (size * nmemb);
	size_t required_size = len + CtrMediaCurlBytesRead;
	if (required_size > CtrMediaCurlBufferSize) {
		CtrMediaCurlBuffer = ctr_heap_reallocate(CtrMediaCurlBuffer, required_size);
		CtrMediaCurlBufferSize = required_size;
	}
	memcpy(CtrMediaCurlBuffer+CtrMediaCurlBytesRead, ptr, len);
	CtrMediaCurlBytesRead += len;
	return len;
}

ctr_object* ctr_network_basic_text_send(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	char* message_str = NULL;
	char* destination = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->next->object));
	CURL* curl;
	CURLcode res;
	CtrMediaCurlBuffer = ctr_heap_allocate(10);
	CtrMediaCurlBufferSize = 10;
	CtrMediaCurlBytesRead = 0;
	if (!CtrNetworkConnectedFlag) {
		curl_global_init(CURL_GLOBAL_DEFAULT);
	}
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, destination);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	if (argumentList->object != CtrStdNil) {
		message_str = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, message_str);
	}
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &ctr_curl_write_callback);
	res = curl_easy_perform(curl);
	if(res != CURLE_OK) {
		result = ctr_error((char*)curl_easy_strerror(res), 0);
	} else {
		curl_easy_cleanup(curl);
		result = ctr_build_string_from_cstring(CtrMediaCurlBuffer);
	}
	CtrMediaCurlBytesRead = 0;
	if (CtrMediaCurlBufferSize) {
		ctr_heap_free(CtrMediaCurlBuffer);
		CtrMediaCurlBuffer = NULL;
		CtrMediaCurlBufferSize = 0;
	}
	ctr_heap_free(destination);
	if (message_str) {
		ctr_heap_free(message_str);
	}
    return result;
}
#endif

#ifdef ANDROID_EXPORT
// For Android we use JNI
extern ctr_object* ctr_network_basic_text_send(ctr_object* myself, ctr_argument* argumentList);
#endif


void ctr_img_destructor(ctr_resource* rs) {
	MediaIMG* image = (MediaIMG*) rs->ptr;
	if (image->text != NULL) {
		ctr_heap_free(image->text);
		image->text = NULL;
	}
	if (image->texture) {
		SDL_DestroyTexture(image->texture);
	}
	if (image->surface) {
		SDL_FreeSurface(image->surface);
	}
	image->ref = NULL;
	image->font = NULL;
}

/**
 * @def
 * Image
 * 
 * @example
 * >> image := Image new: ['a.png'].
 *
 * @result
 * @info-image
 */
ctr_object* ctr_img_new(ctr_object* myself, ctr_argument* argumentList) {
	
	// If there are no more image slots available, generate an exception
	// To keep up speed and avoid gc cycles we use fixed image slots
	if (IMGCount >= MaxIMG - 1) {
		char error_message[500]; // keep error message < 500 chars
		if (!sprintf(error_message, CTR_ERR_MEDIA_IMG_DEPLETED, MaxIMG - 1)) {
			// if we fail to generate a proper error message, generate a generic error
			ctr_error("Unable to generate error code for image depletion.", 0);
		}
		CtrStdFlow = ctr_error( error_message, 0 );
		return CtrStdNil;
	}

	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	MediaIMG* mediaImage = &mediaIMGs[IMGCount++];
	mediaImage->x = 0;
	mediaImage->y = 0;
	mediaImage->ox = 0;
	mediaImage->oy = 0;
	mediaImage->tx = -1;
	mediaImage->ty = -1;
	mediaImage->bounce = 0;
	mediaImage->fixed = 0;
	mediaImage->ghost = 0;
	mediaImage->solid = 0;
	mediaImage->collidable = 0;
	mediaImage->gravity = 0;
	mediaImage->speed = 1;
	mediaImage->fric = 0;
	mediaImage->accel = 1;
	mediaImage->gspeed = 0;
	mediaImage->dir = -1;
	mediaImage->lastdir = mediaImage->dir;
	mediaImage->mov = 0;
	mediaImage->visible = 1;
	mediaImage->anims = 1;
	mediaImage->animspeed = 5;
	mediaImage->nodirani = 0;
	mediaImage->editable = 0;
	mediaImage->text = NULL;
	mediaImage->paddingx = 0;
	mediaImage->paddingy = 0;
	mediaImage->textlength = 0;
	mediaImage->textbuffer = 0;
	mediaImage->font = NULL;
	mediaImage->texture = NULL;
	mediaImage->surface = NULL;
	mediaImage->lineheight = 10;
	mediaImage->color = (SDL_Color) {0,0,0,0};
	mediaImage->backgroundColor = (SDL_Color) {0,0,0,0};
	mediaImage->clickable = 1;
	mediaImage->ref = instance;
	ctr_resource* rs = ctr_heap_allocate( sizeof(ctr_resource) );
	rs->ptr = mediaImage;
	rs->destructor = &ctr_img_destructor;
	instance->value.rvalue = rs;
	return instance;
}

/**
 * @def
 * [ Image ] source: [ Text ]
 * 
 * @example
 * >> image := Image new: ['a.png'].
 * image source: ['b.png'].
 * 
 * @result
 * @info-image-source
 */
ctr_object* ctr_img_img(ctr_object* myself, ctr_argument* argumentList) {
	SDL_Rect dimensions;
	dimensions.x = 0;
	dimensions.y = 0;
	MediaIMG* mediaImage = ctr_internal_get_image_from_object(myself);
	char* imageFileStr = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	SDL_RWops* res;
	res = ctr_internal_media_load_asset(imageFileStr, 1);
	if (res == NULL) {
		ctr_heap_free(imageFileStr);
		ctr_error(CTR_ERR_FOPEN, 0);
		return myself;
	}
	mediaImage->surface = (void*) IMG_Load_RW(res, 1);
	if (mediaImage->surface == NULL) {
		ctr_internal_media_fatalerror("Unable to load surface", imageFileStr);
	}
	mediaImage->texture = SDL_CreateTextureFromSurface(CtrMediaRenderer, mediaImage->surface);
	if (mediaImage->texture == NULL) {
		ctr_internal_media_fatalerror("Unable to load texture", imageFileStr);
	}
	ctr_heap_free(imageFileStr);
	SDL_QueryTexture(mediaImage->texture, NULL, NULL, &dimensions.w, &dimensions.h);
	mediaImage->h = dimensions.h;
	mediaImage->w = dimensions.w;
	return myself;
}

ctr_object* ctr_img_new_set(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_img_img(ctr_img_new(myself, argumentList), argumentList);
}

/**
 * @def
 * [ Image ] static: [ Boolean ]
 * 
 * @example
 * image static: True.
 * 
 * @result
 * @info-image-static
 */
ctr_object* ctr_img_fixed_set(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* mediaImage = ctr_internal_get_image_from_object(myself);
	mediaImage->fixed = ctr_internal_cast2bool( argumentList->object )->value.bvalue;
	return myself;
}

ctr_object* ctr_img_clickable_set(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* mediaImage = ctr_internal_get_image_from_object(myself);
	mediaImage->clickable = ctr_internal_cast2bool( argumentList->object )->value.bvalue;
	return myself;
}


/**
 * @def
 * [ Image ] ghost: [ Boolean ]
 * 
 * @example
 * image ghost: True.
 * 
 * @result
 * @info-image-ghost
 */
ctr_object* ctr_img_ghost_set(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* mediaImage = ctr_internal_get_image_from_object(myself);
	mediaImage->ghost = ctr_internal_cast2bool( argumentList->object )->value.bvalue;
	return myself;
}

/**
 * @def
 * [ Image ] visible: [ Boolean ]
 * 
 * @example
 * image visible: True.
 * 
 * @result
 * @info-image-visible
 */
ctr_object* ctr_img_visible_set(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* mediaImage = ctr_internal_get_image_from_object(myself);
	mediaImage->visible = ctr_internal_cast2bool( argumentList->object )->value.bvalue;
	return myself;
}

/**
 * @def
 * [ Image ] fixate: [ Boolean ]
 * 
 * @example
 * image fixate: True.
 * 
 * @result
 * @info-image-fixate
 */
ctr_object* ctr_media_nodirani(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* mediaImage = ctr_internal_get_image_from_object(myself);
	mediaImage->nodirani = ctr_internal_cast2bool( argumentList->object )->value.bvalue;
	return myself;
}

/**
 * @def
 * [ Image ] controllable: [ Number ]
 * 
 * @example
 * >> media := Media new.
 * >> a := Image new: ['a.png'].
 *
 * media on: ['start'] do: {
 * 
 * a 
 * controllable: 4.
 * 
 * }.
 * 
 * media screen: ['canvas.png'].
 * 
 * @result
 * [[img_controllable]]
 */
ctr_object* ctr_img_controllable(ctr_object* myself, ctr_argument* argumentList) {
	if (!ctr_internal_get_image_from_object(myself)) return myself;
	controllableObject = myself;
	CtrMediaControlMode = (int) ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}

/**
 * @def
 * [ Image ] freeze: [ Boolean ]
 *
 * @example
 * image freeze: True.
 *
 * @result
 * @info-image-freeze
 */
ctr_object* ctr_img_freeze(ctr_object* myself, ctr_argument* argumentList) {
	if (!ctr_internal_get_image_from_object(myself)) return myself;
	CtrMediaInputFreeze = ctr_internal_cast2bool(argumentList->object);
	return myself;
}


/**
 * @def
 * [ Image ] mask: [ Number ]
 *
 * @example
 * image mask: 10.
 *
 * @result
 * @info-image-mask
 */
ctr_object* ctr_img_mask_set(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (!image) return myself;
	image->font = TTF_OpenFontRW(
		SDL_RWFromConstMem(
			CTR_MEDIA_PASSW_FONT,
			CTR_MEDIA_PASSW_FONT_LEN),
		0,
		ctr_tonum(argumentList->object)
	);
	return myself;
}

/**
 * @def
 * [ Image ] x: [ Number ] y: [ Number ]
 * 
 * @example
 * >> media := Media new.
 * >> a := Image new: ['a.png'].
 * a x: 10 y: 5.
 * Out write: a x?, stop.
 * Out write: a y?, stop.
 * 
 */
ctr_object* ctr_img_xy(ctr_object* myself, ctr_argument* argumentList) {
	int x = (int) ctr_tonum(ctr_internal_cast2number(argumentList->object));
	int y = (int) ctr_tonum(ctr_internal_cast2number(argumentList->next->object));
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->oy = y;
	image->ox = x;
	image->x = x;
	image->y = y;
	return myself;
}

ctr_object* ctr_img_x(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	return ctr_build_number_from_float(image->x);
}

ctr_object* ctr_img_y(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	int y = (int) ctr_tonum(ctr_build_number_from_float(image->y));
	return ctr_build_number_from_float(y);
}


/**
 * @def
 * [ Image ] to-x: [ Number ] y: [ Number ]
 * 
 * @example
 * image to-x: 100 y: 200.
 * 
 * @result
 * @info-image-move
 */
ctr_object* ctr_img_mov_set(ctr_object* myself, ctr_argument* argumentList) {
	double x = (double) ctr_internal_cast2number(argumentList->object)->value.nvalue;
	double y = (double) ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	double delta_x = x - image->x;
	double delta_y = y - image->y;
	double rad = atan2(-1 * delta_y, delta_x);
	double deg = rad * (180 / M_PI);
	image->dir = (deg < 0) ? 360 + deg : deg;
	image->tx = x;
	image->ty = y;
	return myself;
}

/**
 * @def
 * [ Image ] bounce: [ Boolean ]
 * 
 * @example
 * image bounce: True.
 * 
 * @result
 * @info-image-bounce
 */
ctr_object* ctr_img_bounce(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->bounce = (int) ctr_internal_cast2bool(argumentList->object)->value.bvalue;
	return myself;
}

/**
 * @def
 * [ Image ] wall: [ Boolean ]
 * 
 * @example
 * image wall: True.
 * 
 * @result
 * @info-image-wall
 */
ctr_object* ctr_img_solid(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->solid = (int) ctr_internal_cast2bool(argumentList->object)->value.bvalue;
	return myself;
}


/**
 * @def
 * [ Image ] active: [ Boolean ]
 * 
 * @example
 * >> media := Media new.
 * >> a := Image new: ['a.png'].
 * a active: True.
 * 
 * a on: ['destination:'] do: { ... }.
 * a on: ['collision:'] do: { ... }.
 * a on: ['click:'] do: { ... }.
 *
 * @result
 * @info-image-active
 */
ctr_object* ctr_img_active(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->collidable = (int) ctr_internal_cast2bool(argumentList->object)->value.bvalue;
	return myself;
}


/**
 * @def
 * [ Image ] gravity: [ Number ]
 * 
 * @example
 * image gravity: 1.
 * 
 * @result
 * @info-image-gravity
 */
ctr_object* ctr_img_gravity(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->gravity = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}

/**
 * @def
 * [ Image ] speed: [ Number ]
 * 
 * @example
 * image speed: 2.
 *
 * @result
 * @info-image-speed
 */
ctr_object* ctr_img_speed(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->speed = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}

/**
 * @def
 * [ Image ] friction: [ Number ]
 * 
 * @example
 * image friction: 1.
 * 
 * @result
 * @info-image-friction
 */
ctr_object* ctr_img_friction(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->fric = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}

/**
 * @def
 * [ Image ] font: [ Font ]
 * 
 * @example
 * >> f := Font new source: ['arial.ttf'] size: 16.
 * image font: f.
 * 
 * @result
 * @info-image-font
 */
ctr_object* ctr_img_font(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	MediaFNT* font = ctr_internal_get_font_from_object(argumentList->object);
	image->font = font->font;
	return myself;
}

/**
 * @def
 * [ Image ] accelerate: [ Number ]
 * 
 * @example
 * image accelerate: 0.01.
 * 
 * @result
 * @info-image-accelerate
 */
ctr_object* ctr_img_accel(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->accel = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}

/**
 * @def
 * [ Image ] jump-height: [ Number ]
 * 
 * @example
 * image jump-height: 6.
 *
 * @result
 * @info-image-jump
 */
ctr_object* ctr_img_jump_height(ctr_object* myself, ctr_argument* argumentList) {
	CtrMediaJumpHeightFactor = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}

/**
 * @def
 * [ Image ] editable: [ Boolean ]
 * 
 * @example
 * image editable: True.
 * 
 * @result
 * @info-image-editable
 */
ctr_object* ctr_img_editable(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->editable = ctr_internal_cast2bool(argumentList->object)->value.bvalue;
	return myself;
}

/**
 * @def
 * [ Image ] autoplay: [ Number ].
 * 
 * @example
 * image autoplay: True.
 *
 * @result
 * @info-image-autoplay
 */
ctr_object* ctr_img_autoplay(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->nodirani = (int) ctr_internal_cast2bool(argumentList->object)->value.bvalue * 2;
	return myself;
}

 /*
 *  @def
 * [ Image ] reel: [ Number ] speed: [Number].
 *
 * @example
 * image reel: 2 speed: 20.
 *
 * @result
 * @info-image-animations
 */
ctr_object* ctr_img_reel(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->anims = (int) ctr_internal_cast2number(argumentList->object)->value.nvalue;
	image->animspeed = (int) ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
	return myself;
}

/**
 * @def
 * [ Image ] ink: [ Color ]
 * 
 * @example
 * >> red := Color new red: 255 green: 0 blue: 0.
 * image ink: red.
 * 
 * @result
 * @info-image-color
 */
ctr_object* ctr_img_color(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	uint8_t r = (uint8_t)ctr_color_r(argumentList->object, NULL)->value.nvalue;
	uint8_t g = (uint8_t)ctr_color_g(argumentList->object, NULL)->value.nvalue;
	uint8_t b = (uint8_t)ctr_color_b(argumentList->object, NULL)->value.nvalue;
	uint8_t a = (uint8_t)ctr_color_a(argumentList->object, NULL)->value.nvalue;
	image->color = (SDL_Color) { r, g, b, a };
	ctr_internal_media_clear_edcache();
	return myself;
}

/**
 * @def
 * [ Image ] highlight: [ Color ]
 * 
 * @example
 * >> green := Color new red: 0 green: 255 blue: 0.
 * image highlight: green.
 * 
 * @result
 * @info-image-background-color
 */
ctr_object* ctr_img_background_color(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	uint8_t r = (uint8_t)ctr_color_r(argumentList->object, NULL)->value.nvalue;
	uint8_t g = (uint8_t)ctr_color_g(argumentList->object, NULL)->value.nvalue;
	uint8_t b = (uint8_t)ctr_color_b(argumentList->object, NULL)->value.nvalue;
	uint8_t a = (uint8_t)ctr_color_a(argumentList->object, NULL)->value.nvalue;
	image->backgroundColor = (SDL_Color) { r, g, b, a };
	return myself;
}

/**
 * @def
 * [ Image ] align-x: [ Number ] y: [ Number ]
 * 
 * @example
 * image align-x: 10 y: 10.
 *
 * @result
 * @info-image-align
 */
ctr_object* ctr_img_text_align(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->paddingx = (int)ctr_tonum(argumentList->object);
	image->paddingy = (int)ctr_tonum(argumentList->next->object);
	ctr_internal_img_render_text(myself);
	return myself;
}

/**
 * @def
 * [ Image ] line-height: [ Number ]
 * 
 * @example
 * image line-height: 16.
 *
 * @result
 * @info-image-line-height
 */
ctr_object* ctr_img_lineheight(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->lineheight = ctr_tonum(argumentList->object);
	if (image->lineheight < 1) {
		image->lineheight = 1;
	} else if (image->lineheight > 100) {
		image->lineheight = 100;
	}
	ctr_internal_img_render_text(myself);
	return myself;
}

void ctr_internal_img_render_cursor(ctr_object* focusObject) {
	if (!focusObject) return;
	MediaIMG* image = ctr_internal_get_image_from_object(focusObject);
	if (image == NULL) return;
	if (image->text == NULL) return;
	if (CtrMediaInputIndex > image->textlength) CtrMediaInputIndex = 0;
	if (CtrMediaSelectBegin > image->textlength) CtrMediaSelectBegin = 0;
	if (CtrMediaSelectEnd > image->textlength) CtrMediaSelectEnd = 0;
	int i = 0;
	int y1 = 0;
	int beginline = 0;
	while(i<CtrMediaInputIndex) {
		if (image->text[i]=='\r') {
			y1 += 1;
		} else if (image->text[i]=='\n') {
			beginline = i;
		} else if ((image->text[i] & 0x80) == 0x00 || (image->text[i] & 0xC0) == 0xC0) {
		    //x ++?
		}
		i++;
	}
	if (y1>0) beginline++;
	int measurementBufferLength = i - beginline;
	int offsetx = 0;
	int height;
	if (measurementBufferLength) {
		int measurementBufferSize = measurementBufferLength + 1;
		char* measurementBufferStart = image->text+beginline;
		char* measurementBuffer = ctr_heap_allocate(measurementBufferSize);
		memcpy(measurementBuffer, measurementBufferStart, measurementBufferLength);
		measurementBuffer[measurementBufferLength] = '\0';
		if (TTF_SizeUTF8(image->font, measurementBuffer, &offsetx, NULL)) ctr_internal_media_fatalerror("Unable to measure font", "TTF_SizeUTF8");
		ctr_heap_free(measurementBuffer);
	}
	height = image->lineheight;
	if (y1 >= (CtrMediaCursorOffsetY+CtrMediaMaxLines)) {
		CtrMediaCursorOffsetY++;
		ctr_internal_img_render_text(focusObject);
	}
	if (y1 < CtrMediaCursorOffsetY && CtrMediaCursorOffsetY>0) {
		CtrMediaCursorOffsetY--;
		ctr_internal_img_render_text(focusObject); 
	 }
	y1 -= (CtrMediaCursorOffsetY);
	if ((CtrMediaSteps / 50) % 2) {
		SDL_SetRenderDrawColor(CtrMediaRenderer, image->color.r, image->color.g, image->color.b, 255);
		SDL_RenderDrawLine(CtrMediaRenderer, image->x + (offsetx) + (image->paddingx), image->y + (y1*height) + (image->paddingy), image->x + (offsetx) + (image->paddingx), image->y + ((y1*height)+height) + (image->paddingy));
	}
	CtrMediaCursorLine = y1;
}

void ctr_internal_img_render_text(ctr_object* myself) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return;
	if (image->surface == NULL) return;
	int begin, end, len;
	ctr_internal_media_get_selection(&begin, &end);
	len = end - begin;
	TTF_Font* font;
	SDL_Surface* text = NULL;
	font = image->font;
	if (font == NULL) ctr_internal_media_fatalerror("Undefined font", "text without font");
	int text_width = 0;
	int text_height = 0;
	TTF_SizeUTF8(font, image->text, &text_width, &text_height);
	SDL_Surface* dst = SDL_CreateRGBSurface(0, image->w, image->h,
		image->surface->format->BitsPerPixel,
		image->surface->format->Rmask, image->surface->format->Gmask,
		image->surface->format->Bmask, image->surface->format->Amask);
	SDL_Rect t;
	t.x = image->paddingx;
	t.y = image->paddingy;
	SDL_BlitSurface(image->surface,NULL,dst,NULL);
	int i = 0;
	int line_segment_start = 0;
	int textLine = 0;
	ctr_size buffsize = 10;
	char* buff;
	int state = 0; //0 = normal, 1 = selected
	while(i < image->textlength) {
		line_segment_start = i;
		if (i>=begin) state = 1;
		if (i>=end) state = 0;
		while(image->text[i]!='\r' && i < image->textlength && (!len || ((i!=begin || state==1) && (i!=end || state==0)))) i++;
		int text_too_low = textLine > (CtrMediaCursorOffsetY + CtrMediaMaxLines);
		int text_too_high= textLine < CtrMediaCursorOffsetY;
		int text_in_view = (!text_too_low && !text_too_high);
		text_width = 0;
		text_height = 0;
		if (text_in_view) {
			int viewline = textLine - CtrMediaCursorOffsetY;
			int cacheoffset = CtrMediaCursorOffsetY % 200;
			int q = cacheoffset + viewline;
			if (
			CtrMediaEdCache[q].surface && CtrMediaEdCache[q].state == state
			&& strlen(CtrMediaEdCache[q].text)==i-line_segment_start
			&& strncmp(image->text+line_segment_start, CtrMediaEdCache[q].text, i-line_segment_start)==0
			) {
				text = CtrMediaEdCache[q].surface;
				buff = CtrMediaEdCache[q].text;
			} else {

			buffsize = i-line_segment_start;
			buff = ctr_heap_allocate(buffsize + 1);
			memcpy(buff, image->text+line_segment_start, i-line_segment_start);
			memcpy(buff+(i-line_segment_start), "\0", 1);
			if (state) {
				text =TTF_RenderUTF8_Shaded(font, buff, image->backgroundColor, image->color);
			} else {
				text = TTF_RenderUTF8_Blended(font, buff, image->color);
			}

			if (CtrMediaEdCache[q].surface) {
				SDL_FreeSurface(CtrMediaEdCache[q].surface);
				ctr_heap_free(CtrMediaEdCache[q].text);
			}
			CtrMediaEdCache[q].surface = text;
			CtrMediaEdCache[q].text = buff;
			CtrMediaEdCache[q].state = state;
			}
			SDL_BlitSurface(text,NULL,dst,&t);
			TTF_SizeUTF8(font, buff, &text_width, NULL);
			text_height = image->lineheight;
		}
		t.x += text_width;
		if (image->text[i]=='\r') {
			t.x = image->paddingx;
			t.y += text_height;
			i+=2;
			textLine++;
			continue;
		}
		if (len && !state && i==begin) continue;
		if (len && state && i==end) continue;
		i++;
	}
	image->texture = (void*) SDL_CreateTextureFromSurface(CtrMediaRenderer, dst);
	SDL_FreeSurface(dst);
}

/**
 * This function will change the pointer to original text buffer
 * and return the new buffer. If you control memory, then make a copy first.
 */
char* ctr_internal_media_normalize_line_endings(char* original_text) {
	ctr_size i, len, d;
	char* normalized_text;
	char current_char, previous_char;
	len = strlen(original_text);
	normalized_text = ctr_heap_allocate(len + 1);
	d = 0;
	current_char = '\0';
	for (i = 0; i < len; i++) {
		previous_char = current_char;
		current_char = original_text[i];
		if (current_char == '\t') {
			current_char = ' ';
		}
		if (current_char == '\n' && previous_char != '\r') {
			normalized_text[d + i] = '\r';
			d++;
			normalized_text = ctr_heap_reallocate(normalized_text, len + d + 1);
		}
		if (previous_char == '\r' && current_char != '\n') {
			normalized_text[d + i] = '\n';
			d++;
			normalized_text = ctr_heap_reallocate(normalized_text, len + d + 1);
		}
		normalized_text[d + i] = current_char;
	}
	ctr_heap_free(original_text);
	return normalized_text;
}

/**
 * @def
 * [ Image ] write: [ Text ]
 * 
 * @example
 * image write: ['hello world'].
 *
 * @result
 * @info-image-text
 */
ctr_object* ctr_img_text(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	ctr_object* textObject = ctr_internal_cast2string(argumentList->object);
	if (image->text != NULL) {
		ctr_heap_free(image->text);
		image->text = NULL;
	} 
	image->text = ctr_internal_media_normalize_line_endings(ctr_heap_allocate_cstring(textObject));
	image->textlength = strlen(image->text);
	image->textbuffer = image->textlength;
	ctr_internal_img_render_text(myself);
	return myself;
}

/**
 * @def
 * [ Image ] draw: [ Sequence ] color: [ Color ]
 * 
 * @example
 * image draw: ( Sequence new ; line ; point ) color: blue.
 * 
 * @result
 * [[img_draw]]
 */
ctr_object* ctr_img_draw(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* arg;
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	ctr_object* drawList = argumentList->object;
	ctr_object* color = argumentList->next->object;
	ctr_object* shape;
	ctr_object* subshape1;
	ctr_object* subshape2;
	SDL_Texture* texTarget;
	int errorCode;
	errorCode = SDL_SetRenderTarget(CtrMediaRenderer, image->texture);
	if (errorCode) {
		#ifdef __EMSCRIPTEN__
		texTarget = SDL_CreateTexture(CtrMediaRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_TARGET, image->w, image->h);
		#else
		texTarget = SDL_CreateTexture(CtrMediaRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, image->w, image->h);
		#endif
		errorCode = SDL_SetRenderTarget(CtrMediaRenderer, texTarget);
		SDL_RenderCopy(CtrMediaRenderer, image->texture, NULL, NULL);
		image->texture = texTarget;
	}
	arg = ctr_heap_allocate(sizeof(ctr_argument));
	SDL_SetRenderDrawColor(CtrMediaRenderer,
		(char) ctr_tonum(ctr_internal_object_property(color, "r", NULL)),
		(char) ctr_tonum(ctr_internal_object_property(color, "g", NULL)),
		(char) ctr_tonum(ctr_internal_object_property(color, "b", NULL)), 255);
	int i = 1;
	while(1) {
		arg->object = ctr_build_number_from_float(i);
		shape = ctr_array_get(drawList, arg);
		if (shape == CtrStdNil) break;
		if (shape->link == pointObject) {
			SDL_RenderDrawPoint(CtrMediaRenderer, 
			(int) ctr_tonum(ctr_internal_object_property(shape, "x", NULL)), 
			(int) ctr_tonum(ctr_internal_object_property(shape, "y", NULL)));
		}
		if (shape->link == lineObject) {
			subshape1 = ctr_internal_object_property(shape, CTR_DICT_FROM, NULL);
			subshape2 = ctr_internal_object_property(shape, CTR_DICT_TO, NULL);
			SDL_RenderDrawLine(CtrMediaRenderer,
				(int) ctr_tonum(ctr_internal_object_property(subshape1, "x", NULL)),
				(int) ctr_tonum(ctr_internal_object_property(subshape1, "y", NULL)),
				(int) ctr_tonum(ctr_internal_object_property(subshape2, "x", NULL)),
				(int) ctr_tonum(ctr_internal_object_property(subshape2, "y", NULL))
			);
		}
		i++;
	}
	ctr_heap_free(arg);
	SDL_SetRenderTarget(CtrMediaRenderer, NULL);
	return myself;
}

ctr_object* ctr_media_override(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

/**
 * @def
 * [ Image ] text
 * 
 * @example
 * image text.
 * 
 * @result
 * @info-image-get-text
 */
ctr_object* ctr_img_text_get(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL || image->text == NULL) {
		return ctr_build_empty_string();
	}
	return ctr_build_string_from_cstring(image->text);
}

/**
 * @def
 * [ Media ] clipboard
 * 
 * @example
 * >> media := Media new.
 * Media clipboard: ['abc'].
 * Out write: Media clipboard.
 */
ctr_object* ctr_media_clipboard(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* text;
	char* buffer;
	buffer = SDL_GetClipboardText();
	if (buffer != NULL) {
		int len = strlen(buffer);
		char* copy = ctr_heap_allocate(len);
		char* normalized;
		//We copy the SDL buffer first because we don't want to interfere with SDL memory.
		memcpy(copy, buffer, len);
		SDL_free(buffer);
		/* Normalize line endings */
		normalized = ctr_internal_media_normalize_line_endings(copy); // This will destroy the copy buffer, no free needed!
		text = ctr_build_string_from_cstring(normalized);
		/* Prevent leak, normalized buffer no longer needed. */
		ctr_heap_free(normalized);
	} else {
		text = ctr_build_empty_string();
	}
	return text;
}

/**
 * @def
 * [ Media ] clipboard: [ Text ]
 * 
 * @example
 * >> media := Media new.
 * Media clipboard: ['abc'].
 * Out write: Media clipboard.
 */
ctr_object* ctr_media_clipboard_set(ctr_object* myself, ctr_argument* argumentList) {
	char* buffer;
	buffer = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	SDL_SetClipboardText(buffer);
	ctr_heap_free(buffer); /* SetClipboardText strdups the string */
	return myself;
}

/**
 * @def
 * [ Media ] selected.
 * 
 * @example
 * Media selected.
 * 
 * @result
 * @info-media-selected
 */
ctr_object* ctr_media_select(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* list = ctr_array_new(CtrStdArray, NULL);
	int begin, end;
	MediaIMG* focusImage;
	if (!focusObject) return list;
	ctr_internal_media_get_selection(&begin, &end);
	focusImage = ctr_internal_get_image_from_object(focusObject);
	begin = ctr_internal_media_bytepos2utf8pos(focusImage, begin);
	end = ctr_internal_media_bytepos2utf8pos(focusImage, end);
	ctr_argument* arguments = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
	arguments->object = ctr_build_number_from_float((ctr_number) begin + 1);
	ctr_array_push(list, arguments);
	arguments->object = ctr_build_number_from_float((ctr_number) end + 1);
	ctr_array_push(list, arguments);
	ctr_heap_free(arguments);
	return list;
}

void ctr_internal_media_init() {
	CtrMediaScreenActive = 0;
	CtrMediaInputFreeze = CtrStdBoolFalse;
	CtrMediaContactSurface = NULL;
	CtrMediaAssetPackage = NULL;
	CtrMediaAudioRate = MIX_DEFAULT_FREQUENCY;
	CtrMediaAudioFormat = MIX_DEFAULT_FORMAT;
	CtrMediaAudioChannels = MIX_DEFAULT_CHANNELS;
	CtrMediaAudioBuffers = 4096;
	CtrMediaAudioVolume = MIX_MAX_VOLUME;

	#ifdef TEST
		CtrMediaWindow = SDL_CreateWindow("Citrine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 100, 100, SDL_WINDOW_OPENGL );
		CtrMediaRenderer = SDL_CreateRenderer(CtrMediaWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
		return;
	#endif

	if (SDL_Init(SDL_INIT_VIDEO) < 0) ctr_internal_media_fatalerror("SDL failed to init", SDL_GetError());
	IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
	
	#ifdef FULLSCREEN
	CtrMediaWindow = SDL_CreateWindow("Citrine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 100, 100, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
	#elif DESKTOP_FULLSCREEN
	CtrMediaWindow = SDL_CreateWindow("Citrine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 100, 100, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
	#else
	char* screen_mode = getenv("screenmode");
	if (screen_mode && strcmp(screen_mode,"full")==0) {
	   CtrMediaWindow = SDL_CreateWindow("Citrine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 100, 100, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
	} else {
	   CtrMediaWindow = SDL_CreateWindow("Citrine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 100, 100, SDL_WINDOW_OPENGL);
	}
	#endif
	if (CtrMediaWindow == NULL) ctr_internal_media_fatalerror("Unable to create window", SDL_GetError());
	CtrMediaFlagSoftwareVSync = 0;
	CtrMediaRenderer = SDL_CreateRenderer(CtrMediaWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
	if (!CtrMediaRenderer) {
		// If we are going to use software rendering we need to cap fps ourselves (vsync)
		CtrMediaFlagSoftwareVSync = 1;
		printf("Failed to create renderer, trying software renderer instead...\n");
		CtrMediaRenderer = SDL_CreateRenderer(CtrMediaWindow, -1, SDL_RENDERER_SOFTWARE);
	}
	if (!CtrMediaRenderer) ctr_internal_media_fatalerror("Unable to create renderer", SDL_GetError());
	SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
	gameController = SDL_GameControllerOpen(0);
	if (TTF_Init() < 0) ctr_internal_media_fatalerror("Unable to init TTF", SDL_GetError());
	if (SDL_Init(SDL_INIT_AUDIO) < 0) ctr_internal_media_fatalerror("Couldn't initialize SDL: %s\n",SDL_GetError());
	if (Mix_OpenAudio(CtrMediaAudioRate, CtrMediaAudioFormat, CtrMediaAudioChannels, CtrMediaAudioBuffers) < 0) {
		fprintf(stderr, "Couldn't open audio device.");
	}
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

#ifdef FFI
ctr_object* ctr_ffi_object_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	instance->link = myself;
	return instance;
}


void ctr_media_blob_destructor(ctr_resource* resource_value) {
	// the destructor cant remove the memory automatically
	// because upon returning from a c-function it is not known
	// whether the memory belongs to us or to the called function
	// you have to call free / freestruct.
}

ffi_type* ctr_internal_media_ffi_map_type(char* description);
void* ctr_internal_media_ffi_convert_value(ffi_type* type, ctr_object* obj);


/**
 * @def
 * [ Blob ] deref.
 * 
 * @example
 * blob deref.
 *
 * @result
 * @info-blob-deref
 */
ctr_object* ctr_blob_deref(ctr_object* myself, ctr_argument* argumentList) {
	myself->value.rvalue->ptr = (void*) *((void**)myself->value.rvalue->ptr);
	return myself;
}

/**
 * @def
 * [ Blob ] fill: [ Sequence ]
 * 
 * @example
 * blob fill: (Sequence new ; 1 ; 2 ; 3).
 *
 * @result
 * @info-blob-fill
 */
ctr_object* ctr_blob_fill(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* arr = argumentList->object;
	for(int i = 0; i < arr->value.avalue->head; i++) {
		*((char*)myself->value.rvalue->ptr + i) = (char) ctr_tonum(*(arr->value.avalue->elements + i));
	}
	return myself;
}

/**
 * @def
 * [ Blob ] free
 * 
 * @example
 * blob free
 *
 * @result
 * @info-blob-free
 */
ctr_object* ctr_blob_free(ctr_object* myself, ctr_argument* argumentList) {
	ctr_heap_free(myself->value.rvalue->ptr);
	myself->value.rvalue->ptr = NULL;
	return myself;
}


ctr_object* ctr_blob_tostring(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_build_string_from_cstring(myself->value.rvalue->ptr);
}


/**
 * @def
 * [ Blob ] new: [ Number ]
 * 
 * @example
 * >> x := Blob new: 100.
 * 
 * @result
 * @info-blob-new
 */
ctr_object* ctr_blob_new_set(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	size_t size = (size_t) ctr_tonum(argumentList->object);
	ctr_resource* buffer = ctr_heap_allocate(sizeof(ctr_resource));
	buffer->ptr = ctr_heap_allocate(size);
	buffer->destructor = &ctr_media_blob_destructor;
	instance->value.rvalue = buffer;
	instance->info.sticky = 1;
	return instance;
}

/**
 * @def
 * [ Blob ] utf8: [ Text ]
 * 
 * @example
 * blob utf8: ['abc'].
 * 
 * @result
 * @info-blob-utf8
 */
ctr_object* ctr_blob_utf8_set(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	ctr_resource* buffer = ctr_heap_allocate(sizeof(ctr_resource));
	buffer->ptr = ctr_heap_allocate_cstring(argumentList->object);
	buffer->destructor = &ctr_media_blob_destructor;
	instance->value.rvalue = buffer;
	instance->info.sticky = 1;
	return instance;
}

ffi_type* ctr_internal_media_ffi_map_type_obj(ctr_object* obj);
ctr_object* ctr_blob_new_set_type(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	ctr_resource* buffer;
	ffi_type* type = ctr_internal_media_ffi_map_type_obj(argumentList->next->object);
	if (type) {
		buffer = ctr_heap_allocate(sizeof(ctr_resource));
		buffer->destructor = &ctr_media_blob_destructor;
		buffer->ptr = ctr_internal_media_ffi_convert_value(type, argumentList->object);
	} else {
		buffer = NULL;
	}
	instance->value.rvalue = buffer;
	instance->info.sticky = 1;
	return instance;
}

/**
 * @def
 * [ Blob ] from: [ Number ] length: [ Number ]
 * 
 * @example
 * >> data := blob from: 0 length 10.
 * 
 * @result
 * @info-blob-read
 */
ctr_object* ctr_blob_read(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* pushArg;
	ctr_argument* elnumArg;
	ctr_object* elnum;
	ctr_object* startElement = ctr_internal_cast2number(argumentList->object);
	ctr_object* count = ctr_internal_cast2number(argumentList->next->object);
	int start = (int) startElement->value.nvalue;
	int len = (int) count->value.nvalue;
	int i = 0;
	ctr_object* newArray = ctr_array_new(CtrStdArray, NULL);
	for(i = start; i < start + len; i++) {
		pushArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		elnumArg = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
		elnum = ctr_build_number_from_float((ctr_number) i);
		elnumArg->object = elnum;
		pushArg->object = ctr_build_number_from_float( (ctr_number) *((char*) myself->value.rvalue->ptr + (i - 1)) );
		ctr_array_push(newArray, pushArg);
		ctr_heap_free( elnumArg );
		ctr_heap_free( pushArg );
	}
	return newArray;
}

/**
 * @def
 * FFI
 * 
 * @example
 * media link: (
 * 		Sequence new ; ['libc.so.6'] ;
 * 		['printf'] ;
 * 		( Sequence new ; ['pointer'] ; ['int'] ) ;
 * 		['void'] ;
 * 		['stdio'] ;
 * 		['printf:'] ;
 * 		1
 * ).
 * >> b := Blob utf8: ['I got %d appels'].
 * stdio printf: (Sequence new ; b ; 6).
 * 
 * @result
 * I got 6 apples
 */
struct CtrMediaFFI {
	void* handle;
	void* symbol;
	ffi_type* args[20];
	ffi_type* rtype;
	int nargs;
	ffi_cif* cif;
	ctr_object* owner;
};
typedef struct CtrMediaFFI CtrMediaFFI;

struct ctr_media_test_struct {
	int a;
	int b;
};
typedef struct ctr_media_test_struct ctr_media_test_struct;

int ctr_media_internal_structtest(ctr_media_test_struct sum) {
	printf("sum of %d and %d is: %d \n", sum.a, sum.b, sum.a + sum.b);
	return sum.a + sum.b;
}

void ctr_media_ffi_destructor(ctr_resource* resource_value) {
	CtrMediaFFI* ff = (CtrMediaFFI*) resource_value->ptr;
	ctr_heap_free(ff->cif);
	ff->cif = NULL;
	ctr_heap_free(ff);	
}


ffi_type* ctr_internal_media_ffi_map_type(char* description) {
	if (strcmp(description, "void")==0) {
		return &ffi_type_void;
	} else if (strcmp(description, "uint")==0) {
		return &ffi_type_uint;
	} else if (strcmp(description, "int")==0) {
		return &ffi_type_sint;
	} else if (strcmp(description,"uint8_t")==0) {
		return &ffi_type_uint8;
	} else if (strcmp(description,"int8_t")==0) {
		return &ffi_type_sint8;
	} else if (strcmp(description,"uint16_t")==0) {
		return &ffi_type_uint16;
	} else if (strcmp(description,"int16_t")==0) {
		return &ffi_type_sint16;
	} else if (strcmp(description,"uint32_t")==0) {
		return &ffi_type_uint32;
	} else if (strcmp(description,"int32_t")==0) {
		return &ffi_type_sint32;
	} else if (strcmp(description,"uint64_t")==0) {
		return &ffi_type_uint64;
	} else if (strcmp(description,"int64_t")==0) {
		return &ffi_type_sint64;
	} else if (strcmp(description,"float")==0) {
		return &ffi_type_float;
	} else if (strcmp(description,"double")==0) {
		return &ffi_type_double;
	} else if (strcmp(description,"ushort")==0) {
		return &ffi_type_ushort;
	} else if (strcmp(description,"short")==0) {
		return &ffi_type_sshort;
	} else if (strcmp(description,"uchar")==0) {
		return &ffi_type_uchar;
	} else if (strcmp(description,"char")==0) {
		return &ffi_type_schar;
	} else if (strcmp(description,"pointer")==0) {
		return &ffi_type_pointer;
	} else if (strcmp(description,"ulong")==0) {
		return &ffi_type_ulong;
	} else if (strcmp(description,"long")==0) {
		return &ffi_type_slong;
	}
	return NULL;
}

ffi_type* ctr_internal_media_ffi_map_type_obj(ctr_object* obj) {
	ffi_type* result;
	if (obj->info.type == CTR_OBJECT_TYPE_OTSTRING) {
		char* description = ctr_heap_allocate_cstring(obj);
		result = ctr_internal_media_ffi_map_type(description);
		ctr_heap_free(description);
	} else {
		result = obj->value.rvalue->ptr;
	}
	return result;
}

/**
 * @def
 * [ Blob ] struct: [ Sequence ]
 * 
 * @example
 * >> ints ≔ Blob struct: (Sequence new ; ['int'] ; ['int']).
 *
 * @result
 * @info-blob-struct
 */
ctr_object* ctr_blob_new_struct(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	ctr_resource* buffer = ctr_heap_allocate(sizeof(ctr_resource));
	buffer->destructor = &ctr_media_blob_destructor;
	ffi_type* cstruct = ctr_heap_allocate(sizeof(ffi_type));
	cstruct->type = FFI_TYPE_STRUCT;
	cstruct->alignment = 0;
	cstruct->size = 0;
	if (argumentList->object->info.type == CTR_OBJECT_TYPE_OTARRAY) {
		ctr_object* arr = argumentList->object;
		int nargs = arr->value.avalue->head;
		cstruct->elements = (ffi_type**) ctr_heap_allocate(sizeof(ffi_type*) * (nargs+1));
		int i;
		for (i = 0; i < nargs; i++) {
			cstruct->elements[i] = ctr_internal_media_ffi_map_type_obj(*(arr->value.avalue->elements + i));
		}
		cstruct->elements[i] = NULL;
	}
	buffer->ptr = cstruct;
	instance->value.rvalue = buffer;
	return instance;
}

/**
 * @def
 * [ Blob ] freestruct
 * 
 * @example
 * blob freestruct
 *
 * @result
 * @info-blob-freestruct
 */
ctr_object* ctr_blob_free_struct(ctr_object* myself, ctr_argument* argumentList) {
	if (myself->info.type == CTR_OBJECT_TYPE_OTEX) {
		ctr_resource* buffer = (ctr_resource*) myself->value.rvalue;
		ffi_type* cstruct = (ffi_type*) buffer->ptr;
		if (cstruct->elements) {
			ctr_heap_free(cstruct->elements);
		}
	}
	return CtrStdNil;
}

void* ctr_internal_media_ffi_convert_value(ffi_type* type, ctr_object* obj) {
	//allocate for return type, must be at least ffi_arg
	ctr_size size = type->size;
	if (obj == NULL) {
		if (type->size < sizeof(ffi_arg)) {
			size = sizeof(ffi_arg); 
		}
	}
	void* ptr = ctr_heap_allocate(size);
	if (obj != NULL) {
		if (type == &ffi_type_uint8) {
			*((uint8_t*)ptr) = (uint8_t) ctr_tonum(obj);
		} else if (type == &ffi_type_sint8) {
			*((int8_t*)ptr) = (int8_t) ctr_tonum(obj);
		} else if (type == &ffi_type_uint16) {
			*((uint16_t*)ptr) = (uint16_t) ctr_tonum(obj);
		} else if (type == &ffi_type_sint16) {
			*((int16_t*)ptr) = (int16_t) ctr_tonum(obj);
		} else if (type == &ffi_type_uint32) {
			*((uint32_t*)ptr) = (uint32_t) ctr_tonum(obj);
		} else if (type == &ffi_type_sint32) {
			*((int32_t*)ptr) = (int32_t) ctr_tonum(obj);
		} else if (type == &ffi_type_uint64) {
			*((uint64_t*)ptr) = (uint64_t) ctr_tonum(obj);
		} else if (type == &ffi_type_sint64) {
			*((int64_t*)ptr) = (int64_t) ctr_tonum(obj);
		} else if (type == &ffi_type_float) {
			*((float*)ptr) = (float) ctr_tonum(obj);
		} else if (type == &ffi_type_double) {
			*((double*)ptr) = (double) ctr_tonum(obj);
		} else if (type == &ffi_type_uchar) {
			*((unsigned char*)ptr) = (unsigned char) ctr_tonum(obj);
		} else if (type == &ffi_type_schar) {
			*((char*)ptr) = (char) ctr_tonum(obj);
		} else if (type == &ffi_type_ushort) {
			*((unsigned short*)ptr) = (unsigned short) ctr_tonum(obj);
		} else if (type == &ffi_type_sshort) {
			*((short*)ptr) = (short) ctr_tonum(obj);
		} else if (type == &ffi_type_uint) {
			*((unsigned int*)ptr) = (unsigned int) ctr_tonum(obj);
		} else if (type == &ffi_type_sint) {
			*((int*)ptr) = (int) ctr_tonum(obj);
		} else if (type == &ffi_type_ulong) {
			*((unsigned long*)ptr) = (unsigned long) ctr_tonum(obj);
		} else if (type == &ffi_type_slong) {
			*((long*)ptr) = (long) ctr_tonum(obj);
		} else if (type == &ffi_type_pointer) {
			if (obj == CtrStdNil) {
				*((void**)ptr) = NULL;
			}
			else if (obj->link == CtrMediaDataBlob) {
				*((void**)ptr) = (void*) obj->value.rvalue->ptr;
			}
			else {
				ctr_error("FFI Pointer requires Blob.\n", 0);
			}
		} else {
			memcpy(ptr, obj->value.rvalue->ptr, type->size);
		}
	}
	return ptr;
}

ctr_object* ctr_internal_media_ffi_convert_value_back(ffi_type* type, void* ptr) {
	ctr_object* result = CtrStdNil;
	if (type == &ffi_type_void) {
		result = CtrStdNil;
	} else if (type == &ffi_type_uint8) {
		result = ctr_build_number_from_float( (ctr_number) *((uint8_t*) ptr) );
	} else if (type == &ffi_type_sint8) {
		result = ctr_build_number_from_float( (ctr_number) *((int8_t*) ptr) );
	} else if (type == &ffi_type_uint16) {
		result = ctr_build_number_from_float( (ctr_number) *((uint16_t*) ptr) );
	} else if (type == &ffi_type_sint16) {
		result = ctr_build_number_from_float( (ctr_number) *((int16_t*) ptr) );
	} else if (type == &ffi_type_uint32) {
		result = ctr_build_number_from_float( (ctr_number) *((uint32_t*) ptr) );
	} else if (type == &ffi_type_sint32) {
		result = ctr_build_number_from_float( (ctr_number) *((int32_t*) ptr) );
	} else if (type == &ffi_type_uint64) {
		result = ctr_build_number_from_float( (ctr_number) *((uint64_t*) ptr) );
	} else if (type == &ffi_type_sint64) {
		result = ctr_build_number_from_float( (ctr_number) *((int64_t*) ptr) );
	} else if (type == &ffi_type_float) {
		result = ctr_build_number_from_float( (ctr_number) *((float*) ptr) );
	} else if (type == &ffi_type_double) {
		result = ctr_build_number_from_float( (ctr_number) *((double*) ptr) );
	} else if (type == &ffi_type_uchar) {
		result = ctr_build_number_from_float( (ctr_number) *((unsigned char*) ptr) );
	} else if (type == &ffi_type_schar) {
		result = ctr_build_number_from_float( (ctr_number) *((char*) ptr) );
	} else if (type == &ffi_type_ushort) {
		result = ctr_build_number_from_float( (ctr_number) *((unsigned short*) ptr) );
	} else if (type == &ffi_type_sshort) {
		result = ctr_build_number_from_float( (ctr_number) *((short*) ptr) );
	} else if (type == &ffi_type_uint) {
		result = ctr_build_number_from_float( (ctr_number) *((unsigned int*) ptr) );
	} else if (type == &ffi_type_sint) {
		result = ctr_build_number_from_float( (ctr_number) *((int*) ptr) );
	} else if (type == &ffi_type_ulong) {
		result = ctr_build_number_from_float( (ctr_number) *((unsigned long*) ptr) );
	} else if (type == &ffi_type_slong) {
		result = ctr_build_number_from_float( (ctr_number) *((long*) ptr) );
	} else if (type == &ffi_type_pointer) {
		ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
		instance->link = CtrMediaDataBlob;
		ctr_resource* buffer = ctr_heap_allocate(sizeof(ctr_resource));
		buffer->ptr = *((void**)ptr);
		buffer->destructor = &ctr_media_blob_destructor;
		instance->value.rvalue = buffer;
		instance->info.sticky = 1;
		result = instance;
	}
	return result;
}

CtrMediaFFI* ctr_internal_media_ffi_get(ctr_object* obj, ctr_object* property) {
	ctr_object* resource_holder = ctr_internal_object_find_property(
		obj,
		ctr_internal_cast2string( property ),
		0
	);
	if (!resource_holder) {
		return NULL;
	}
	ctr_resource* resource = resource_holder->value.rvalue;
	if (!resource) {
		return NULL;
	}
	CtrMediaFFI* ff = (CtrMediaFFI*) resource->ptr;
	return ff;
}

ctr_object* ctr_media_ffi_respond_to_and_and_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	void* return_value;
	CtrMediaFFI* ff = ctr_internal_media_ffi_get(myself, argumentList->object);
	if (!ff) {
		return ctr_error("Unable to find FFI property.", 0);
	}
	void* values[3];
	values[0] = ctr_internal_media_ffi_convert_value(ff->args[0], argumentList->next->object);
	values[1] = ctr_internal_media_ffi_convert_value(ff->args[1], argumentList->next->next->object);
	values[2] = ctr_internal_media_ffi_convert_value(ff->args[2], argumentList->next->next->next->object);
	return_value = ctr_internal_media_ffi_convert_value(ff->rtype, NULL);
	ffi_call(ff->cif, ff->symbol, return_value, values);
	result = ctr_internal_media_ffi_convert_value_back(ff->rtype, return_value);
	ctr_heap_free(values[0]);
	ctr_heap_free(values[1]);
	ctr_heap_free(values[2]);
	ctr_heap_free(return_value);
	return result;
}

ctr_object* ctr_media_ffi_respond_to_and_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	void* return_value;
	CtrMediaFFI* ff = ctr_internal_media_ffi_get(myself, argumentList->object);
	if (!ff) {
		return ctr_error("Unable to find FFI property.", 0);
	}
	void* values[20];
	values[0] = ctr_internal_media_ffi_convert_value(ff->args[0], argumentList->next->object);
	values[1] = ctr_internal_media_ffi_convert_value(ff->args[1], argumentList->next->next->object);
	return_value = ctr_internal_media_ffi_convert_value(ff->rtype, NULL);
	ffi_call(ff->cif, ff->symbol, return_value, values);
	result = ctr_internal_media_ffi_convert_value_back(ff->rtype, return_value);
	ctr_heap_free(values[0]);
	ctr_heap_free(values[1]);
	ctr_heap_free(return_value);
	return result;
}

ctr_object* ctr_media_ffi_respond_to_and(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	ctr_object* arr;
	void* return_value;
	CtrMediaFFI* ff = ctr_internal_media_ffi_get(myself, argumentList->object);
	if (!ff) {
		return ctr_error("Unable to find FFI property.", 0);
	}
	void* values[20];
	if (argumentList->next->object->info.type == CTR_OBJECT_TYPE_OTARRAY) {
		arr = argumentList->next->object;
		if (arr->value.avalue->head>20) {
			return ctr_error("Too many parameters for FFI.", 0);
		}
		for(int i = 0; i<arr->value.avalue->head; i++) {
			if (i > ff->nargs) break;
			values[i] = ctr_internal_media_ffi_convert_value(ff->args[i], *(arr->value.avalue->elements+i));
		}
	} else {
		values[0] = ctr_internal_media_ffi_convert_value(ff->args[0], argumentList->next->object);
	}
	return_value = ctr_internal_media_ffi_convert_value(ff->rtype, NULL);
	ffi_call(ff->cif, ff->symbol, return_value, values);
	result = ctr_internal_media_ffi_convert_value_back(ff->rtype, return_value);
	if (argumentList->next->object->info.type == CTR_OBJECT_TYPE_OTARRAY) {
		arr = argumentList->next->object;
		for(int i = 0; i<arr->value.avalue->head; i++) {
			if (i > ff->nargs) break;
			ctr_heap_free(values[i]);
		}
	} else {
		ctr_heap_free(values[0]);
	}
	ctr_heap_free(return_value);
	return result;
}

ctr_object* ctr_media_ffi_respond_to(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	void* return_value;
	CtrMediaFFI* ff = ctr_internal_media_ffi_get(myself, argumentList->object);
	if (!ff) {
		return ctr_error("Unable to find FFI property.", 0);
	}
	return_value = ctr_internal_media_ffi_convert_value(ff->rtype, NULL);
	ffi_call(ff->cif, ff->symbol, return_value, NULL);
	result = ctr_internal_media_ffi_convert_value_back(ff->rtype, return_value);
	ctr_heap_free(return_value);
	return result;
}

/**
 * FFI creates objects in the world whose properties
 * contain function names and call definitions.
 * The generic methods are then used to map those properties
 * to messages.
 */
CtrMediaFFI* CtrMediaPreviousFFIEntry = NULL;
void ctr_internal_media_ffi(ctr_object* ffispec) {
	char* library_path;
	char* symbol_name;
	char* ffi_property_name;
	char* rtype_desc;
	CtrMediaFFI* ff;
	ctr_object* arg1;
	ctr_object* arg2;
	ctr_object* arg3;
	ctr_object* arg4;
	ctr_object* arg5;
	ctr_object* arg6;
	ctr_object* arg7 = NULL;
	ctr_object* cif_resource_holder;
	ctr_resource* resource;
	// Check number of required arguments for FFI-binding
	if (ffispec->value.avalue->head < 6) {
		ctr_error("Too few arguments to create FFI binding",0);
		return;
	}
	// Load arguments	
	arg1 = *(ffispec->value.avalue->elements + 0);
	arg2 = *(ffispec->value.avalue->elements + 1);
	arg3 = *(ffispec->value.avalue->elements + 2);
	arg4 = *(ffispec->value.avalue->elements + 3);
	arg5 = *(ffispec->value.avalue->elements + 4);
	arg6 = *(ffispec->value.avalue->elements + 5);
	if (ffispec->value.avalue->head == 7) {
		arg7 = *(ffispec->value.avalue->elements + 6);	
	}
	// Create FFI entry
	ff = ctr_heap_allocate(sizeof(CtrMediaFFI));
	if (!ff) {
		ctr_error("Unable to allocate FFI handle.", 0);
		return;
	}
	ff->cif = ctr_heap_allocate(sizeof(ffi_cif));
	ff->handle = NULL;
	// Load dynamic library
	if (arg1 == CtrStdNil) {
		if (CtrMediaPreviousFFIEntry) {
			ff->handle = CtrMediaPreviousFFIEntry->handle;
		} else {
			ctr_error("No FFI handle", 0);
			return;
		}
	} else {
		library_path = ctr_heap_allocate_cstring(ctr_internal_cast2string(arg1));
		if (strcmp("@structtest", library_path)!=0) {
		#ifdef WIN
		ff->handle = LoadLibrary(library_path);
		#else
		ff->handle = dlopen(library_path, RTLD_NOW);
		#endif
		ctr_heap_free(library_path);
		if ( !ff->handle ) {
			#if defined WIN
			ctr_error("Unable to open library",0);
			#else
			ctr_error(dlerror(),0);
			#endif
			return;
		}
		} else {
			ctr_heap_free(library_path);
		}
	}
	// Obtain symbol reference
	if (arg2 == CtrStdNil) {
		if (CtrMediaPreviousFFIEntry) {
			ff->symbol = CtrMediaPreviousFFIEntry->symbol;
		} else {
			ctr_error("No FFI symbol", 0);
			return;
		}
	} else {
		symbol_name = ctr_heap_allocate_cstring(ctr_internal_cast2string(arg2));
		if (strcmp("@structtest", symbol_name)==0) {
			ff->symbol = &ctr_media_internal_structtest;
			ctr_heap_free(symbol_name);
		} else {
			#ifdef WIN
			ff->symbol = GetProcAddress( ff->handle, symbol_name ); 
			#else
			ff->symbol = dlsym( ff->handle, symbol_name );
			#endif
			ctr_heap_free(symbol_name);
			if (!ff->symbol) {
				#ifdef WIN
				ctr_error("No symbol",0);
				#else
				ctr_error(dlerror(),0);
				#endif
				return;
			}
		}
	}
	// Build the argument list
	if (arg3->link != CtrStdArray) {
		ctr_error("No FFI arguments", 0);
		return;
	}
	// Load the number of arguments
	ff->nargs = arg3->value.avalue->head;
	//No more than 20 arguments are supported for an FFI binding
	if (ff->nargs > 20) {
		ctr_error("FFI: up to 20 arguments supported per call.", 0);
		return;
	}
	for(int i = 0; i<ff->nargs; i++) {
		ff->args[i] = ctr_internal_media_ffi_map_type_obj(*(arg3->value.avalue->elements + i));
		if (!ff->args[i]) {
			ctr_error("Unable to map argument type.", 0);
			return;
		}
		
	}
	// Build the return type
	rtype_desc = ctr_heap_allocate_cstring(ctr_internal_cast2string(arg4));
	ff->rtype = NULL;
	ff->rtype = ctr_internal_media_ffi_map_type(rtype_desc);
	ctr_heap_free(rtype_desc);
	
	if (!ff->rtype) {
		ctr_error("Invalid FFI return type.",0);
		return;
	}
	ffi_status ok;
	if (arg7) {
		ok = ffi_prep_cif_var(ff->cif, FFI_DEFAULT_ABI, (int) arg7->value.nvalue, ff->nargs, ff->rtype, ff->args);
	} else {
		ok = ffi_prep_cif(ff->cif, FFI_DEFAULT_ABI, ff->nargs, ff->rtype, ff->args);
	}
	if (ok != FFI_OK) {
		ctr_error("Invalid FFI function signature",0);
		return;
	}
	// Create the owner object in the world
	if (arg5 == CtrStdNil) {
		if (CtrMediaPreviousFFIEntry) {
			ff->owner = CtrMediaPreviousFFIEntry->owner;
		} else {
			ctr_error("No FFI bridge object",0);
			return;
		}
	} else {
		ff->owner = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
		ff->owner->link = CtrMediaFFIObjectBase;
		ctr_internal_object_add_property(
			CtrStdWorld,
			ctr_internal_cast2string(arg5),
			ff->owner,
			CTR_CATEGORY_PUBLIC_PROPERTY
		);
	}
	// Create the message bridge
	if (arg6 == CtrStdNil) {
		ctr_error("FFI: no message mapping",0);
		return;
	}
	resource = ctr_heap_allocate(sizeof(ctr_resource));
	resource->ptr = ff;
	resource->type = 2;
	resource->destructor = &ctr_media_ffi_destructor;
	cif_resource_holder = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	cif_resource_holder->value.rvalue = resource;
	ffi_property_name = ctr_heap_allocate_cstring(
		ctr_internal_cast2string(arg6)
	);
	ctr_internal_object_property(
		ff->owner,
		ffi_property_name,
		cif_resource_holder
	);
	ctr_heap_free(ffi_property_name);
	// Succesfully created FFI bridge, store this bridge in cache
	CtrMediaPreviousFFIEntry = ff;
}

#endif

/**
 * @def
 * [ Media ] link: [ Package ]
 * 
 * @example
 * >> media := Media new.
 * media link: (Package new: ['assets.dat']).
 * 
 * @result
 * @info-media-package-link
 */
ctr_object* ctr_media_link_package(ctr_object* myself, ctr_argument* argumentList) {
	if (argumentList->object->link == CtrStdArray) {
		#ifdef FFI		
		ctr_internal_media_ffi(argumentList->object);
		#else
		ctr_error("FFI not available.", 0);
		#endif
	} else if (argumentList->object->link != packageObject) {
		ctr_error("Not an asset package.\n", 0);
	}
	CtrMediaAssetPackage = argumentList->object;
	return myself;
}

/**
 * @def
 * [ Package ] append: [ Object ]
 * 
 * @example
 * >> media := Media new.
 * >> a := Package new: ['assets.dat'].
 * >> b := Image new: ['a.png'].
 * a append: b.
 * 
 * @result
 * @info-package-add
 */
ctr_object* ctr_package_add(ctr_object* myself, ctr_argument* argumentList) {
	char* path;
	char* pkgpath = ctr_heap_allocate_cstring(ctr_internal_object_property(myself, "path", NULL));
	FILE* outfile;
	if (access(pkgpath, F_OK) == 0) {
		outfile = fopen(pkgpath, "rb+");
	} else {
		outfile = fopen(pkgpath, "wb+");
	}
	fseek(outfile, 0, SEEK_END);
	int CHUNK_SIZE = 10000;
	uint64_t next_entry_at;
	int bytes_read;
	char* buffer;
	ctr_object* fileObject = argumentList->object;
	if (fileObject->link == CtrStdFile) {
		path = ctr_heap_allocate_cstring(
			ctr_internal_cast2string(
				ctr_internal_object_property(fileObject, "path", NULL)
			)
		);
		FILE* f = fopen(path, "rb");
		if (f == NULL) {
			ctr_error( CTR_ERR_OPEN, errno );
			ctr_heap_free(path);
			ctr_heap_free(pkgpath);
			fclose(outfile);
			return CtrStdNil;
		}
		fwrite(path, 1, strlen(path), outfile);
		fwrite("\0", 1 , 1, outfile);
		next_entry_at = ftell(outfile);
		fwrite("\0\0\0\0\0\0\0\0", 8, 1, outfile);
		buffer = calloc(1,CHUNK_SIZE);
		clearerr(f);
		while(!feof(f)) {
			bytes_read = fread(buffer,1,CHUNK_SIZE,f);
			fwrite(buffer, 1, bytes_read, outfile);
		}
		free(buffer);
		uint64_t next_entry = (uint64_t) ftell(outfile);
		fseek(outfile, next_entry_at, SEEK_SET);
		fwrite(&next_entry, sizeof(uint64_t), 1, outfile);
		fclose(outfile);
		fclose(f);
		ctr_heap_free(path);
	} else {
		ctr_error("Invalid argument\n", 0);
	}
	ctr_heap_free(pkgpath);
	return myself;
}


ctr_object* ctr_media_on_do(ctr_object* myself, ctr_argument* argumentList) {
	char* event_name = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	ctr_object* listener = argumentList->next->object;
	if (strcmp(CTR_DICT_ON_KEY_UP, event_name)==0) CtrMediaEventListenFlagKeyUp = (listener != CtrStdNil);
	else if (strcmp(CTR_DICT_ON_KEY_DOWN, event_name)==0) CtrMediaEventListenFlagKeyDown = (listener != CtrStdNil);
	else if (strcmp(CTR_DICT_ON_CLICK_XY, event_name)==0) CtrMediaEventListenFlagMouseClick = (listener != CtrStdNil);
	else if (strcmp(CTR_DICT_ON_GAMEPAD_DOWN, event_name)==0) CtrMediaEventListenFlagGamePadBtnDown = (listener != CtrStdNil);
	else if (strcmp(CTR_DICT_ON_GAMEPAD_UP, event_name)==0) CtrMediaEventListenFlagGamePadBtnUp = (listener != CtrStdNil);
	else if (strcmp(CTR_DICT_ON_TIMER, event_name)==0) CtrMediaEventListenFlagTimer = (listener != CtrStdNil);
	else if (strcmp(CTR_DICT_ON_STEP, event_name)==0) CtrMediaEventListenFlagStep = (listener != CtrStdNil);
	ctr_heap_free(event_name);
	return ctr_object_on_do(myself, argumentList);
}

#ifndef REPLACE_MEDIA_SYSTEM_COMMAND
ctr_object* ctr_internal_media_external_command(char* command_str, char* fallback, char* parameter_str, char* template_str) {
	char* default_template_str = "%s '%s'";
	int maxlen = 500;
	char command[maxlen];
	if (command_str == NULL) command_str = fallback;
	if (template_str == NULL) template_str = default_template_str;
	memset(command, '\0', maxlen);
	if (strlen(template_str) + strlen(command_str) + strlen(parameter_str) >= maxlen) return CtrStdBoolFalse;
	sprintf(command, template_str, command_str, parameter_str);
	if  (system(command)==0) return CtrStdBoolTrue;
	return CtrStdBoolFalse;
}
#endif

#ifdef MEDIA_SYSTEM_COMMAND_STUB
ctr_object* ctr_internal_media_external_command(char* command_str, char* fallback, char* parameter_str, char* template_str) {
	return CtrStdBoolFalse;
}
#endif

/**
 * @internal
 *
 * Media sys: [Text].
 *
 * Starts a subprocess and opens a terminal if possible to allow you to see
 * results of write-operations.
 */
ctr_object* ctr_media_system(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* result;
	char* command_str = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	#ifndef WIN
	//In Linux/BSD we cannot attach a terminal to a process afterwards so
	//we always open a terminal. If users don't want a terminal they have to start
	//the application themselves. Also on Linux we don't know which terminal to
	//use, by default we use /usr/bin/x-terminal-emulator and the -e option,
	//this is a standard shared link installed on most systems, the -e option
	//is xterm-compatible, however GNOME terminal uses -x and other terminals
	//may also be incompatible, you can override the TERMINAL to use with env.
	result = ctr_internal_media_external_command(
		getenv("TERMINAL"),
		"/usr/bin/x-terminal-emulator -e",
		command_str,
		NULL
	);
	//If it fails, start without terminal
	if (result == CtrStdBoolFalse) {
		result = ctr_internal_media_external_command(
		NULL,
		"",
		command_str,
		"%s %s"
		);
	}
	#else
	// Use CreateProcess for Windows to avoid useless terminal screen
	// We will 'attach' a real terminal screen that can actually be used by
	// write: afterwards upon the first write-operation (hook).
	STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    // Start the child process. 
    if( !CreateProcess(NULL,command_str,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)) return CtrStdBoolFalse;
	WaitForSingleObject( pi.hProcess, INFINITE );
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	result = CtrStdBoolTrue;
	#endif
	ctr_heap_free(command_str);
	return result;
}

#ifdef WIN
int CtrConsoleAttached = 0;
HANDLE ctr_media_stdout;

ctr_object* ctr_media_console_write(ctr_object* myself, ctr_argument* argumentList) {
	if (!CtrConsoleAttached) {
		/* Allocate console on-the-fly and set console in/out to UTF-8 (=65001) */
		if (!AllocConsole() || !SetConsoleOutputCP(65001) || !SetConsoleCP(65001)) {
			printf("Failed to alloc console!\n");
			exit(0);
		}
		ctr_media_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	}
	CtrConsoleAttached = 1;
	ctr_object* argument1 = argumentList->object;
	ctr_object* strObject = ctr_internal_cast2string(argument1);
	DWORD dwBytesWritten;
	WriteFile(ctr_media_stdout, strObject->value.svalue->value, strObject->value.svalue->vlen, &dwBytesWritten, 0);
	return myself;
}

ctr_object* ctr_media_console_brk(ctr_object* myself, ctr_argument* argumentList) {
	DWORD dwBytesWritten;
	WriteFile(ctr_media_stdout, "\n", 1, &dwBytesWritten, 0);
	FlushFileBuffers(ctr_media_stdout);
	return myself;
}
#endif

ctr_object* ctr_media_include(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* pathStrObj = ctr_internal_cast2string(argumentList->object);
	char* pathString = ctr_heap_allocate_tracked(pathStrObj->value.svalue->vlen + 1);
	strncpy(pathString, pathStrObj->value.svalue->value, pathStrObj->value.svalue->vlen);
	pathString[pathStrObj->value.svalue->vlen] = '\0';
	SDL_RWops* asset_reader = ctr_internal_media_load_asset(pathString, 1);
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
	if (parsedCode) {
		ctr_cwlk_subprogram++;
		ctr_cwlk_run(parsedCode);
		ctr_cwlk_subprogram--;
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
ctr_object* ctr_media_datastart(ctr_object* myself, ctr_argument* none) {
	ctr_argument* argumentList;
	ctr_object* data_package;
	argumentList = (ctr_argument*) ctr_heap_allocate(sizeof(ctr_argument));
	argumentList->object = ctr_build_string_from_cstring("data");
	argumentList->next = NULL;
	// Create an asset package for 'data'
	data_package = ctr_send_message( packageObject, CTR_DICT_NEW_SET, strlen(CTR_DICT_NEW_SET), argumentList );
	argumentList->object = data_package;
	// Connect the assets to the program
	ctr_send_message( mediaObject, CTR_DICT_LINK_SET, strlen(CTR_DICT_LINK_SET), argumentList );
	argumentList->object = ctr_build_string_from_cstring("__1__");
	// Load the __1__ program from the data package
	ctr_send_message( mediaObject, "use:", strlen("use:"), argumentList );
	ctr_heap_free(argumentList);
	return myself;
}

/**
 * @def
 * [ Media ] show: [ Text ]
 * 
 * @example
 * Media show: ['TEST 123'].
 * 
 * @result
 * @info-media-notification
 */
ctr_object* ctr_media_dialog(ctr_object* myself, ctr_argument* argumentList) {
	char* message = ctr_heap_allocate_cstring(
		ctr_internal_cast2string(argumentList->object)
	);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Message", message, CtrMediaWindow);
	ctr_heap_free(message);
	return myself;
}

/**
 * @def
 * [ Media ] website: [ Text ]
 *
 * @example
 * Media website: ['https://www.citrine-lang.org'].
 *
 * @result
 * www.citrine-lang.org
 */
ctr_object* ctr_media_website(ctr_object* myself, ctr_argument* argumentList) {
	char* url = ctr_heap_allocate_cstring(
		ctr_internal_cast2string(argumentList->object)
	);
	SDL_OpenURL(url);
	ctr_heap_free(url);
	return myself;
}

ctr_object* ctr_media_stop_bubbling(ctr_object* myself, ctr_argument* argumentList) {
	CtrStopBubbling = ctr_tobool(argumentList->object);
	return myself;
}

/**
 * @def
 * [ Media ] effect: [ Number ] options: [ Object ]
 *
 * @example
 * Media effect: 0 options: 0.
 */
ctr_object* ctr_media_fx(ctr_object* myself, ctr_argument* argumentList) {

	/**
	 * We use Special Effects (FX) to tweak the engine and add
	 * misc. options or features. Instead of adding new messages for
	 * non-essential features consider adding an FX-flag.
	 *
	 * Flag ranges:
	 * Media...................0000 - 1000
	 * Image...................1000 - 2000
	 * Audio...................2000 - 3000
	 * Other...................3000 - 4000
	 * Export..................4000 - 5000
	 * Mixed...................>5000
	 */
	int fx_code = (int) ctr_tonum(argumentList->object);
	ctr_object* data = argumentList->next->object;

	// Effect #0: test effect
	if (fx_code == 0) {
		// This is just a test message
		printf("Special FX 0: Test Message to Stdout\n");
	}
	// Effect #1: remap gamepad buttons to up
	else if (fx_code == CTR_MEDIA_FX_FLAG_MEDIA_REMAP_ALL) {
		// for now we only set TRUE (0 or else...)
		// other options might be added later...
		CtrMediaFXFlagMapABXY2Up = ctr_tonum(data);
	}
	else if (fx_code == CTR_MEDIA_FX_FLAG_AUDIO_JUMP) {
		// associate a sound object with jumping in platform mode
		// regardless of device (key, gamepad, touch) on a separate
		// audio channel.
		CtrMediaFXFlagJumpSFX = ctr_tonum(data);
		CtrMediaFXFlagJumpSound = argumentList->next->object;
	}
	else if (fx_code == CTR_MEDIA_FX_FLAG_SMOOTH_GAME_CONTROL) {
		// enable smoother control for games
		// in this case, keyup will not fire if another key has been pressed
		// this is different from normal control where up is always processed
		// however for games this might cause hiccups
		// there will probably be more versions in the future, the argument
		// indicates the version for smooth game control to use ie 1 = version 1.0
		CtrMediaFXFlagSmoothGameControl = ctr_tonum(data);
	}
	return myself;
}

void begin(){
	ctr_gc_clean_free = 1; // only for debugging
	#ifdef WIN
	#ifndef NOFREECONSOLE
	FreeConsole();
	#endif
	#endif
	ctr_internal_media_reset();
	ctr_internal_media_init();
	#ifdef FFI
	CtrMediaDataBlob = ctr_media_new(CtrStdObject, NULL);
	CtrMediaDataBlob->link = CtrStdObject;
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_blob_new_set);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_blob_tostring);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_BYTES_SET ), &ctr_blob_fill);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_UTF8_SET ), &ctr_blob_utf8_set);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_NEW_TYPE_SET ), &ctr_blob_new_set_type);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_DEREF ), &ctr_blob_deref);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_FREE ), &ctr_blob_free);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_STRUCT_SET ), &ctr_blob_new_struct);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_FREE_STRUCT ), &ctr_blob_free_struct);
	ctr_internal_create_func(CtrMediaDataBlob, ctr_build_string_from_cstring( CTR_DICT_FROM_LENGTH ), &ctr_blob_read);
	#endif
	colorObject = ctr_media_new(CtrStdObject, NULL);
	colorObject->link = CtrStdObject;
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_color_new );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_RED_GREEN_BLUE_SET ), &ctr_color_rgb_set );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_TRANSPARENCY_SET ), &ctr_color_a_set );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_RED ), &ctr_color_r );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_GREEN ), &ctr_color_g );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_BLUE ), &ctr_color_b );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_TRANSPARENCY ), &ctr_color_a );
	pointObject = ctr_media_new(CtrStdObject, NULL);
	pointObject->link = CtrStdObject;
	ctr_internal_create_func(pointObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_point_new );
	ctr_internal_create_func(pointObject, ctr_build_string_from_cstring( CTR_DICT_XY_SET ), &ctr_point_xyset );
	ctr_internal_create_func(pointObject, ctr_build_string_from_cstring( CTR_DICT_X ), &ctr_point_x );
	ctr_internal_create_func(pointObject, ctr_build_string_from_cstring( CTR_DICT_Y ), &ctr_point_y );
	lineObject = ctr_media_new(CtrStdObject, NULL);
	lineObject->link = CtrStdObject;
	ctr_internal_create_func(lineObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_line_new );
	ctr_internal_create_func(lineObject, ctr_build_string_from_cstring( CTR_DICT_FROM_TO_SET ), &ctr_line_from_to );
	ctr_internal_create_func(lineObject, ctr_build_string_from_cstring( CTR_DICT_FROM ), &ctr_line_start );
	ctr_internal_create_func(lineObject, ctr_build_string_from_cstring( CTR_DICT_TO ), &ctr_line_end );
	mediaObject = ctr_media_new(CtrStdObject, NULL);
	mediaObject->link = CtrStdObject;
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_media_new );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_SCREEN ), &ctr_media_screen );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_WIDTH_HEIGHT_SET ), &ctr_media_width_height );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_XY_SET ), &ctr_media_left_top );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_CLIPBOARD ), &ctr_media_clipboard );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_CLIPBOARD_SET ), &ctr_media_clipboard_set );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_RUN ), &ctr_media_override );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_ON_STEP ), &ctr_media_override );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_SELECTION ), &ctr_media_select );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_LINK_SET ), &ctr_media_link_package );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_TIMER_SET ), &ctr_media_timer );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_ONDO ), &ctr_media_on_do );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_END ), &ctr_media_end );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( "sys:" ), &ctr_media_system );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( "use:" ), &ctr_media_include );
	if (strcmp(CTR_DICT_USE_SET,"use:")!=0) {
		ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_USE_SET ), &ctr_media_include );
	}
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( "_datastart" ), &ctr_media_datastart );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_DIALOG_SET ), &ctr_media_dialog );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( "website:" ), &ctr_media_website );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_FX ), &ctr_media_fx );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( "stopbubbling:" ), &ctr_media_stop_bubbling );

	#ifndef TEST
	#ifdef WIN
	ctr_internal_create_func(CtrStdConsole, ctr_build_string_from_cstring(CTR_DICT_WRITE), &ctr_media_console_write );
	ctr_internal_create_func(CtrStdConsole, ctr_build_string_from_cstring( CTR_DICT_STOP ), &ctr_media_console_brk );
	#endif
	#endif
	imageObject = ctr_img_new(CtrStdObject, NULL);
	imageObject->link = CtrStdObject;
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_img_new );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_img_new_set );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_SOURCE_SET ), &ctr_img_img );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_CONTROLLABLE ), &ctr_img_controllable );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_XY_SET ), &ctr_img_xy );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_X ), &ctr_img_x );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_Y ), &ctr_img_y );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MOVE_TO_XY_SET ), &ctr_img_mov_set );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_BOUNCE_SET ), &ctr_img_bounce );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_SOLID_SET ), &ctr_img_solid );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_ACTIVE_SET ), &ctr_img_active );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_GRAVITY_SET ), &ctr_img_gravity );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_SPEED_SET ), &ctr_img_speed );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_FRICTION_SET ), &ctr_img_friction );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_ACCELERATE_SET ), &ctr_img_accel );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_JUMPHEIGHT_SET ), &ctr_img_jump_height );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_COLLISION_SET ), &ctr_media_override );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_REEL_SET ), &ctr_img_reel );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_REEL_AUTOPLAY_SET ), &ctr_img_autoplay );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_WRITE ), &ctr_img_text );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_img_text_get );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_CUT ), &ctr_img_text_del );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_APPEND ), &ctr_img_text_ins );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_EDITABLE_SET ), &ctr_img_editable );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_COLOR_SET ), &ctr_img_color );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_BACKGROUND_COLOR_SET ), &ctr_img_background_color );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_ALIGN_XY_SET ), &ctr_img_text_align );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_DRAW_COLOR_SET ), &ctr_img_draw );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_STATIC ), &ctr_img_fixed_set );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_GHOST_SET ), &ctr_img_ghost_set );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_NODIRANI_SET ), &ctr_media_nodirani );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_FONT_SET ), &ctr_img_font );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_LINEHEIGHT_SET ), &ctr_img_lineheight );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_XY_SET ), &ctr_img_xy );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MOVE_TO_XY_SET ), &ctr_img_mov_set );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_VISIBLE_SET ), &ctr_img_visible_set );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_FREEZE_SET ), &ctr_img_freeze );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MASK_SET ), &ctr_img_mask_set );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_CLICKABLE_SET ), &ctr_img_clickable_set );
	
	fontObject = ctr_font_new(CtrStdObject, NULL);
	fontObject->link = CtrStdObject;
	ctr_internal_create_func(fontObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_font_new );
	ctr_internal_create_func(fontObject, ctr_build_string_from_cstring( CTR_DICT_SOURCE_SIZE_SET ), &ctr_font_font );
	ctr_internal_create_func(fontObject, ctr_build_string_from_cstring( CTR_DICT_FONTSCRIPT_TXTDIR_SET ), &ctr_font_script_dir );
	audioObject = ctr_audio_new(CtrStdObject, NULL);
	audioObject->link = CtrStdObject;
	ctr_internal_create_func(audioObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_audio_new );
	soundObject = ctr_audio_new(audioObject, NULL);
	soundObject->link = audioObject;
	ctr_internal_create_func(soundObject, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_sound_new_set );
	ctr_internal_create_func(soundObject, ctr_build_string_from_cstring( CTR_DICT_AUDIO_PLAY ), &ctr_sound_play );
	musicObject = ctr_audio_new(audioObject, NULL);
	musicObject->link = audioObject;
	ctr_internal_create_func(musicObject, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_music_new_set );
	ctr_internal_create_func(musicObject, ctr_build_string_from_cstring( CTR_DICT_AUDIO_PLAY ), &ctr_music_play );
	ctr_internal_create_func(audioObject, ctr_build_string_from_cstring( CTR_DICT_AUDIO_SILENCE ), &ctr_music_silence );
	ctr_internal_create_func(audioObject, ctr_build_string_from_cstring( CTR_DICT_AUDIO_REWIND ), &ctr_music_rewind );
	networkObject = ctr_network_new(CtrStdObject, NULL);
	networkObject->link = CtrStdObject;
	ctr_internal_create_func(networkObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_network_new );
	#ifdef LIBCURL
	ctr_internal_create_func(networkObject, ctr_build_string_from_cstring(CTR_DICT_SEND_TEXT_MESSAGE), &ctr_network_basic_text_send );
	#endif
	packageObject = ctr_package_new(CtrStdObject, NULL);
	packageObject->link = CtrStdObject;
	ctr_internal_create_func(packageObject, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_package_new_set );
	ctr_internal_create_func(packageObject, ctr_build_string_from_cstring(CTR_DICT_APPEND), &ctr_package_add );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_PLUGIN_ID ), mediaObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	#ifdef FFI
	CtrMediaFFIObjectBase = ctr_ffi_object_new(CtrStdObject, NULL);
	CtrMediaFFIObjectBase->link = CtrStdObject;
	ctr_internal_create_func(CtrMediaFFIObjectBase, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO ), &ctr_media_ffi_respond_to );
	ctr_internal_create_func(CtrMediaFFIObjectBase, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND ), &ctr_media_ffi_respond_to_and );
	ctr_internal_create_func(CtrMediaFFIObjectBase, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND_AND ), &ctr_media_ffi_respond_to_and_and );
	ctr_internal_create_func(CtrMediaFFIObjectBase, ctr_build_string_from_cstring( CTR_DICT_RESPOND_TO_AND_AND_AND ), &ctr_media_ffi_respond_to_and_and_and );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring(CTR_DICT_BLOB_OBJECT), CtrMediaDataBlob, CTR_CATEGORY_PUBLIC_PROPERTY);
	#endif
	

	ctr_object* jsonObject = ctr_json_new(CtrStdObject, NULL);
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( CTR_DICT_UNJSONIFY_SET ), &ctr_json_parse );
	ctr_internal_create_func(jsonObject, ctr_build_string_from_cstring( CTR_DICT_JSONIFY_SET ), &ctr_json_jsonify );
	
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_IMAGE_OBJECT ), imageObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_FONT_OBJECT ), fontObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_COLOR_OBJECT ), colorObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_POINT_OBJECT ), pointObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_LINE_OBJECT ), lineObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_AUDIO_OBJECT ), audioObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_SOUND_OBJECT ), soundObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MUSIC_OBJECT ), musicObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_NETWORK_OBJECT), networkObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_PACKAGE_OBJECT ), packageObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "JSON"), jsonObject, CTR_CATEGORY_PUBLIC_PROPERTY);

	
	/* Untranslated reference for systems that do not support UTF-8 characters in file names (like Windows) */
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Media" ), mediaObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}

void init_embedded_media_plugin() {
	begin();
}
