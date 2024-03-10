#ifdef WINDOWS_MEDIA_SOCK
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501 //Windows XP
#include <winsock2.h>
#endif


#include "../../citrine.h"
#include "media.h"

#define PL_MPEG_IMPLEMENTATION

#ifndef CTRTEST
#include "pl_mpeg.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#else
#include "mock.h"
#endif

#include <stdio.h>
#include <math.h>

#include <sys/types.h>

#ifndef REPLACE_MEDIA_SOCK
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#ifdef MAC_MEDIA_SOCK
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

#ifdef WINDOWS_MEDIA_SOCK
#include <ws2tcpip.h>
#endif

#include <unistd.h>



/* Old Windows versions lack these functions
 * 
 * credit: Petar Korponaić
 * https://stackoverflow.com/questions/13731243/what-is-the-windows-xp-equivalent-of-inet-pton-or-inetpton
 */
#ifdef WINDOWS_MEDIA_SOCK
int inet_pton(int af, const char *src, void *dst)
{
  struct sockaddr_storage ss;
  int size = sizeof(ss);
  char src_copy[INET6_ADDRSTRLEN+1];

  ZeroMemory(&ss, sizeof(ss));
  strncpy (src_copy, src, INET6_ADDRSTRLEN+1);
  src_copy[INET6_ADDRSTRLEN] = 0;
  if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *)&ss, &size) == 0) {
    switch(af) {
      case AF_INET:
    *(struct in_addr *)dst = ((struct sockaddr_in *)&ss)->sin_addr;
    return 1;
      case AF_INET6:
    *(struct in6_addr *)dst = ((struct sockaddr_in6 *)&ss)->sin6_addr;
    return 1;
    }
  }
  return 0;
}

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
  struct sockaddr_storage ss;
  unsigned long s = size;

  ZeroMemory(&ss, sizeof(ss));
  ss.ss_family = af;

  switch(af) {
    case AF_INET:
      ((struct sockaddr_in *)&ss)->sin_addr = *(struct in_addr *)src;
      break;
    case AF_INET6:
      ((struct sockaddr_in6 *)&ss)->sin6_addr = *(struct in6_addr *)src;
      break;
    default:
      return NULL;
  }
  /* cannot direclty use &size because of strict aliasing rules */
  return (WSAAddressToString((struct sockaddr *)&ss, sizeof(ss), NULL, dst, &s) == 0)?
          dst : NULL;
}
#endif

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
ctr_size CtrMediaAutoReplaceRuleLen = 0;
int CtrMediaLastFrameOffsetX = 0;
int CtrMediaJumpHeightFactor = 100;
int CtrMediaControlMode = 0;
int CtrMediaRotation = 0;
int CtrMediaStdDelayTime = 16;
char CtrMediaBreakLoopFlag = 0;
uint16_t CtrMediaNetworkChunkSize = 350;
time_t CtrMediaFrameTimer = 0;
uint16_t CtrMediaSteps;
int CtrMediaVideoId = -1;
double CtrMediaVideoFPSRendering;
SDL_Texture* CtrMediaBGVideoTexture;

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

struct CtrMediaTextRenderCacheItem {
	char* text;
	SDL_Surface* surface;
	int state;
};
typedef struct CtrMediaTextRenderCacheItem CtrMediaTextRenderCacheItem;
CtrMediaTextRenderCacheItem CtrMediaEdCache[100];

struct CtrMediaAutoReplaceRule {
	char* word;		char* replacement;
};

typedef struct CtrMediaAutoReplaceRule CtrMediaAutoReplaceRule; 

CtrMediaAutoReplaceRule CtrMediaAutoReplaceRules[100];

struct MediaIMG {
	double			x;			int				h;
	double			y;			int				w;
	double			ox;			double			oy;
	double			tx;			double			ty;
	double			gravity;	double			gspeed;
	double			fric;		double			accel;
	double			speed;		double				dir;
	double			mov;		int				anims;
	int				solid;		int				collidable;
	char*			text;		TTF_Font*		font;
	char			editable;	ctr_object*		ref;
	ctr_size		paddingx;	ctr_size		paddingy;
	SDL_Color		color;		SDL_Color		backgroundColor;
	SDL_Texture*	texture;	SDL_Surface*	surface;
	ctr_size		textlength;	ctr_size		textbuffer;
	char 			bounce;
};
typedef struct MediaIMG MediaIMG;

struct MediaAUD {
	ctr_object* ref;
	void* blob;
};
typedef struct MediaAUD MediaAUD;

MediaIMG mediaIMGs[100];
int MaxIMG = 100;
int IMGCount = 0;

MediaIMG* CtrMediaContactSurface;

MediaAUD mediaAUDs[50];
int maxAUD = 50;
int AUDCount = 0;

int windowWidth = 0;
int windowHeight = 0;
int CtrMediaJumpHeight = 0;
char CtrMediaJump = 0;

ctr_object* colorObject;
ctr_object* mediaObject;
ctr_object* imageObject;
ctr_object* lineObject;
ctr_object* pointObject;
ctr_object* controllableObject;
ctr_object* focusObject;
ctr_object* soundObject;
ctr_object* musicObject;
ctr_object* audioObject;
ctr_object* networkObject;
ctr_object* packageObject;
SDL_GameController* gameController;

uint8_t CtrMediaEventListenFlagKeyUp;
uint8_t CtrMediaEventListenFlagKeyDown;
uint8_t CtrMediaEventListenFlagMouseClick;
uint8_t CtrMediaEventListenFlagGamePadBtnUp;
uint8_t CtrMediaEventListenFlagGamePadBtnDown;
uint8_t CtrMediaEventListenFlagTimer;
uint8_t CtrMediaEventListenFlagStep;

void ctr_internal_img_render_text(ctr_object* myself);
void ctr_internal_img_render_cursor(ctr_object* myself);
char* ctr_internal_media_normalize_line_endings(char* text);

int CtrMediaTimers[100];
int CtrMaxMediaTimers = 100;

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

void ctr_internal_media_reset() {
	controllableObject = NULL;
	focusObject = NULL;
	CtrMediaAutoReplaceRule* rule;
	int i;
	for(i = 0; i < IMGCount; i++) {
		MediaIMG* mediaImage = &mediaIMGs[i];
		if (mediaImage->text) {
			ctr_heap_free(mediaImage->text);
			mediaImage->text = NULL;
		}
	}
	for(i = 0; i < CtrMediaAutoReplaceRuleLen; i++) {
		rule = &CtrMediaAutoReplaceRules[i];
		ctr_heap_free(rule->word);
		ctr_heap_free(rule->replacement);
	}
	for(i = 0; i < 100; i++) {
		CtrMediaEdCache[i].surface = NULL;
		CtrMediaEdCache[i].text = NULL;
		CtrMediaEdCache[i].state = 0;
	}
	CtrMediaAutoReplaceRuleLen = 0;
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
	CtrMediaAutoReplaceRuleLen = 0;
	CtrMediaLastFrameOffsetX = 0;
	CtrMediaJumpHeightFactor = 100;
	CtrMediaControlMode = 0;
	CtrMediaRotation = 0;
	CtrMediaStdDelayTime = 16;
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
	return r;
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

void ctr_internal_unnatural_y( int* y ) {
	*y = windowHeight - *y;
}

void ctr_internal_natural_y( int* y ) {
	*y = windowHeight - *y;
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
		mediaImage->text[mediaImage->textlength]='\0';
		mediaImage->textlength -= (CtrMediaInputIndex - oldPos);
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
 * en: Cuts the selected text in the image (if editable).
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
 * [ Image ] insert: [ Text ]
 * 
 * @example
 * image insert: ‘abc’.
 * 
 * @result
 * en: Inserts text in image at cursor position (if editable).
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
 * Media with: 320 height: 200.
 *
 * @result
 * en: Sets camera size
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

void ctr_internal_media_autoreplace(MediaIMG* image) {
	int i, j;
	char len;
	CtrMediaAutoReplaceRule* rule;
	for(i = 0; i < CtrMediaAutoReplaceRuleLen; i++) {
		rule = &CtrMediaAutoReplaceRules[i];
		len = strlen(rule->word);
		if (strncmp(image->text+CtrMediaInputIndex-len, rule->word, len)==0) {
			for (j = 0; j < len; j++) {
				ctr_internal_media_textinsert(image, "\b");
			}
			ctr_internal_media_textinsert(image, rule->replacement);
		}
	}
}

void ctr_internal_cursormove(int x, int y) {
	int offset = 0;
	MediaIMG* text = ctr_internal_media_getfocusimage();
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
	int relx = x - image->x;
	int rely = y - image->y;
	int lineHeight;
	TTF_SizeUTF8(image->font, "X", NULL, &lineHeight);
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
	char* measurementBuffer = malloc(line_length + 1);
	memcpy(measurementBuffer, image->text+line_start, line_length);
	measurementBuffer[line_length] = '\0';
	int total_line_width = 0;
	TTF_SizeUTF8(image->font, measurementBuffer, &total_line_width, NULL);
	//Line is shorter than mouse pos, go to end of line
	if (total_line_width < relx) {
		ctr_internal_media_move_cursor_to_end_of_cur_line(image);
		return;
	}
	int line_segment_width = total_line_width;
	int last_line_segment_width = line_segment_width;
	while(CtrMediaInputIndex>line_start) {
		last_line_segment_width = line_segment_width;
		TTF_SizeUTF8(image->font, measurementBuffer, &line_segment_width, NULL);
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
	return;
}

void ctr_internal_media_anim_frames(MediaIMG* m, SDL_Rect* r, SDL_Rect* s) {
	int frame_width = m->w / (m->anims ? m->anims : 1);
	int step_offset = (m->mov) ? (int)floor(m->x / 5) % m->anims : 0;
	r->w = (m->anims > 0 && m->w > m->anims) ? m->w / m->anims : m->w;
	r->h = m->h;
	s->x = (frame_width * step_offset); 
	s->w = frame_width;
	s->h = m->h;
	s->y = 0;
}

void ctr_internal_media_img_resolvecollision(MediaIMG* m, MediaIMG* m2) {
	SDL_Rect r, r2;
	r = ctr_internal_media_image_maprect(m);
	r2 = ctr_internal_media_image_maprect(m2);
	int cx1 = m->x + (r.w/2);
	int cx2 = m2->x + (r2.w/2);
	int cy1 = m->y + r.h;
	int cy2 = m2->y + r2.h;
	r.x = m->ox;
	r.y = m->oy;
	r.x = m->x;
	if (cx1 < cx2) {
		if (SDL_HasIntersection(&r,&r2)) {
			m->x = m2->x - r.w;
		}
	} else {
		if (SDL_HasIntersection(&r,&r2)) {
			m->x = m2->x + r2.w;
		}
	}
	r.x = m->x;
	r.y = m->y;
	if (cy1 < cy2) {
		if (SDL_HasIntersection(&r,&r2)) {
			m->y = m2->y - r.h;
		}
	} else {
		if (SDL_HasIntersection(&r,&r2)) {
			m->y = m2->y + r2.h;
		}
	}
	r.y = m->y;
	m->x = r.x;
	m->y = r.y;
	if (SDL_HasIntersection(&r,&r2)) {
		//Avoid crushing
		int attempts = 0;
		while(SDL_HasIntersection(&r,&r2) && attempts < 100) {
			m->x += m2->mov * cos(m2->dir * M_PI / 180);
			m->y -= m2->mov * sin(m2->dir * M_PI / 180);
			r.x = m->x;
			r.y = m->y;
			attempts++;
		}
		return;
	}
}

int ctr_internal_media_mouse_down(SDL_Event event) {
	MediaIMG* focusImage;
	for(int i = 0; i < IMGCount; i++) {
		if (
		mediaIMGs[i].x < event.button.x && 
		mediaIMGs[i].y < event.button.y &&
		mediaIMGs[i].x + mediaIMGs[i].w > event.button.x &&
		mediaIMGs[i].y + mediaIMGs[i].h > event.button.y &&
		mediaIMGs[i].ref != NULL
		) {
			if (mediaIMGs[i].editable) {
				focusObject = mediaIMGs[i].ref;
				focusImage = (MediaIMG*) focusObject->value.rvalue->ptr;
				ctr_internal_media_infercursorpos(focusImage, event.button.x, event.button.y);
				CtrMediaSelectStart = 1;
				CtrMediaSelectBegin = CtrMediaInputIndex;
				CtrMediaSelectEnd = CtrMediaInputIndex;
				CtrMediaDoubleClick = 0;
				if(!SDL_TICKS_PASSED(SDL_GetTicks(), CtrMediaPrevClickTime + 1000) && CtrMediaPrevClickX == event.button.x && CtrMediaPrevClickY == event.button.y ) {
					CtrMediaDoubleClick = 1;
					ctr_internal_media_select_word(focusImage);
					CtrMediaSelectStart = 0;
				}
				CtrMediaPrevClickX = event.button.x;
				CtrMediaPrevClickY = event.button.y;
				CtrMediaPrevClickTime = SDL_GetTicks();
				ctr_internal_img_render_text(focusObject);
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
		if (controllableObject) {
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
		if (controllableObject) {
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
		if (controllableObject) {
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
		if (controllableObject) {
			player = (MediaIMG*) controllableObject->value.rvalue->ptr;
			if (player->gravity >= 1) {
				if (CtrMediaJump == 0) {
					CtrMediaJump = 1;
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
		if (m2 == m) continue;
		int h,w,w2;
		h = (int) m->h;
		w = (int) m->w / (m->anims ? m->anims : 1);
		w2 = (int) m2->w / (m2->anims ? m2->anims : 1);
		r2 = ctr_internal_media_image_maprect(m2);
		if (SDL_HasIntersection(&r,&r2)) {
			if (m2->solid) {
				if (m->gravity && m->x+w >= m2->x && m->x <= m2->x+w2 && m->y+h <= m2->y+(m->h/2)) {
						m->y = m2->y - h + 1;
						m->gspeed = 0;
						if (m == player) CtrMediaContactSurface = m2;
				} else {
						ctr_internal_media_img_resolvecollision(m, m2);
						m->mov = 0;
				}
				if (player && m == player && CtrMediaJump == 2) {
					CtrMediaJump = 3;
				} else if (player && m == player && CtrMediaJump == 3) {
					CtrMediaJump = 0;
				}
				if (m->bounce) {
					m->dir = (double) (((int)m->dir + 270) % 360);
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
	static int init_camera = 0;
	int border = 100;
	camera.w = CtrMediaCamera.w;
	camera.h = CtrMediaCamera.h;
	int left = camera.x + border;
	int right = camera.x + camera.w - border;
	int top = camera.y + border;
	int bottom = camera.y + camera.h - border;
	int cpx = player->x + ((player->w / player->anims)/2);
	int cpy = player->y + (player->h/2);
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
	if (!init_camera) {
		camera.x = 0;
		camera.y = 0;
		if (CtrMediaZoom) {
			SDL_RenderSetLogicalSize(CtrMediaRenderer, camera.w, camera.h);
		}
		init_camera = 1;
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
		if ((r->w + r->x) > camera.w) {
			r->w = r->w - ((r->w + r->x) - camera.w);
			s->w = r->w;
		}
		if (r->x < 0) {
			s->x = s->x - r->x;
			r->w = r->w + r->x;
			s->w = r->w;
			r->x = 0;
		}
		if (r->y < 0) {
			s->y = s->y - r->y;
			r->h = r->h + r->y;
			r->y = 0;
			s->h = r->h;
		}
		if ((r->h + r->y) > camera.h) {
			r->h = r->h - ((r->h + r->y) - camera.h);
			s->h = r->h;
		}
	}
	r->x += CtrMediaViewport.x;
	r->y += CtrMediaViewport.y;
}

void ctr_internal_media_render_image(MediaIMG* m, SDL_Rect r, SDL_Rect s, MediaIMG* player) {
	ctr_internal_media_anim_frames(m, &r, &s);
	if (CtrMediaCamera.w > 0 && CtrMediaCamera.h > 0) {
		ctr_internal_media_camera(m, &s, &r, player);
	}
	if (m->dir > -1 && !m->solid && CtrMediaControlMode == 1) {
		if (m->gravity) {
			int xdir = m->dir;
			if (m->gravity < 1) {
				if (m->dir == 180 || m->dir == 0) {
					CtrMediaLastFrameOffsetX = m->dir;
				}
				if (CtrMediaLastFrameOffsetX && m->dir != 180 && m->dir != 0) {
					xdir = CtrMediaLastFrameOffsetX;
				}
			}
			SDL_RenderCopyEx(CtrMediaRenderer, m->texture, &s, &r, 0, NULL, (xdir == 180) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
		} else {
			SDL_RenderCopyEx(CtrMediaRenderer, m->texture, &s, &r, (m->dir == -1 ? 0 : m->dir), NULL, (m->dir == 90 || m->dir == 270) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
		}
	}
	else if (CtrMediaControlMode == 4) {
		SDL_RenderCopyEx(CtrMediaRenderer, m->texture, &s, &r, 360-(m->dir == -1 ? 0 : m->dir), NULL, SDL_FLIP_NONE);
	} 
	else {
		SDL_RenderCopy(CtrMediaRenderer, m->texture, &s, &r);
	}
}

void ctr_internal_media_image_calculate_motion(MediaIMG* m) {
	MediaIMG* player;
	ctr_argument* a;
	if (m->mov < m->speed && ((controllableObject && m != controllableObject->value.rvalue->ptr)  || (controllableObject == NULL))) {
			m->mov += m->speed * m->accel;
	}
	if (m->fric > 0) {
		if (m->mov > 0) {
			m->mov = (m->fric > m->mov) ? 0 : m->mov - m->fric;
		}
	}
	if (m->gravity > 0 && m->y < windowHeight - m->h) {
		if (m->gravity >= 1) {
			m->gspeed += m->gravity * 0.1;
			m->y += m->gspeed;
			//@todo improve, reset contact surface to avoid player moving "on..." :-)
			if (m->gspeed > 0.1) CtrMediaContactSurface = NULL;
		} else if (m->gravity >= 0.1){
			m->y += m->gravity;
		}
	} else {
		if (controllableObject != NULL) {
			player = (MediaIMG*) controllableObject->value.rvalue->ptr;
			if (m == player) CtrMediaJump = 0;
		}
		m->gspeed = 0;
	}
	if (m->mov > 0 && m->dir > -1) {
		m->x += m->mov * cos(m->dir * M_PI / 180);
		m->y -= m->mov * sin(m->dir * M_PI / 180);
		if (round(m->x) == round(m->tx) && round(m->y) == round(m->ty)) {
			m->dir = -1;
			a = (ctr_argument*) ctr_heap_allocate( sizeof( ctr_argument ) );
			a->object = m->ref;
			ctr_send_message(m->ref, CTR_DICT_STOP_AT_SET, strlen(CTR_DICT_STOP_AT_SET), a );
			ctr_heap_free(a);
		}
	}
	if (controllableObject) {
		player = (MediaIMG*) controllableObject->value.rvalue->ptr;
		if (CtrMediaControlMode == 1 && m == player && CtrMediaContactSurface && CtrMediaContactSurface->mov && CtrMediaContactSurface->dir > -1 && !m->mov) {
			m->x += CtrMediaContactSurface->mov * cos(CtrMediaContactSurface->dir * M_PI / 180);
			m->y -= CtrMediaContactSurface->mov * sin(CtrMediaContactSurface->dir * M_PI / 180);
		}
	}
}

SDL_RWops* ctr_internal_media_load_asset(char* asset_name, char asset_type);

plm_t* plm;
uint8_t* rgb_buffer;
int wrgb;

void ctr_internal_media_rendervideoframe(SDL_Rect* rect) {
	plm_frame_t *frame = NULL;
	frame = plm_decode_video(plm);
	if (plm_has_ended(plm)) {
		plm_rewind(plm);
		plm->has_ended = FALSE;
		frame = plm_decode_video(plm);
	}
	plm_frame_to_rgb(frame, rgb_buffer, wrgb);
	SDL_UpdateTexture(CtrMediaBGVideoTexture, NULL, rgb_buffer, rect->w * 3);
	SDL_RenderCopy(CtrMediaRenderer, CtrMediaBGVideoTexture, NULL, rect);
}

uint8_t* ctr_media_video_buffer;
size_t ctr_media_video_memory_id;
void ctr_internal_media_loadvideobg(char* path, SDL_Rect* dimensions) {
	SDL_RWops* asset_reader = ctr_internal_media_load_asset(path, 1);
	if (!asset_reader) {
		ctr_error("Unable to open video asset.", 0);
		return;
	}
	int chunk = 512;
	size_t bytes_read;
	size_t offset = 0;
	ctr_media_video_buffer = ctr_heap_allocate_tracked(chunk);
	ctr_media_video_memory_id = ctr_heap_get_latest_tracking_id();
	bytes_read = SDL_RWread(asset_reader, ctr_media_video_buffer, 1, chunk);
	offset += bytes_read;
	while (bytes_read > 0) {
		ctr_media_video_buffer = ctr_heap_reallocate_tracked(ctr_media_video_memory_id, offset + chunk);
		bytes_read = SDL_RWread(asset_reader, ctr_media_video_buffer + offset, 1, chunk);
        offset += bytes_read;
    }
    SDL_RWclose(asset_reader);
	plm = plm_create_with_memory(ctr_media_video_buffer, offset, FALSE);
	if (!plm) {
		printf("Couldn't open file %s\n", path);
		exit(1);
	}
	plm_set_audio_enabled(plm, FALSE);
	plm_set_loop(plm, FALSE);
	plm_seek(plm, 39, FALSE);
	int w = plm_get_width(plm);
	int h = plm_get_height(plm);
	rgb_buffer = (uint8_t *)malloc(w * h * 3);
	wrgb = w * 3;
	CtrMediaBGVideoTexture = SDL_CreateTexture(CtrMediaRenderer, SDL_PIXELFORMAT_RGB24,
	SDL_TEXTUREACCESS_STREAMING | SDL_TEXTUREACCESS_TARGET,
	w, h);
	if (!CtrMediaBGVideoTexture) ctr_internal_media_fatalerror("texture", "FFMPEG");  
	SDL_SetWindowSize(CtrMediaWindow,w, h);
	SDL_Delay(100);
	dimensions->x = 0;
	dimensions->y = 0;
	dimensions->h = h;
	dimensions->w = w;
}

char ctr_internal_media_determine_filetype(char* path) {
	char magic[20];
	memset(magic, 0, 20);
	SDL_RWops* asset_reader = ctr_internal_media_load_asset(path, 1);
	SDL_RWread(asset_reader, magic, 1, 20);
	if (strcmp(magic, "\x00\x00\x01\xBA")==0) return 10;
	if (strcmp(magic, "\xFF\xD8")==0) return 20;
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
 * ☞ media ≔ Media new.
 * ☞ i ≔ 0.
 * media on: ‘start’ do: {
 * ✎ write: ‘start’, stop.
 * media timer: 1 after: 12.
 * }.
 * media on: ‘step’ do: {
 * ✎ write: i, stop.
 * i add: 1.
 * }.
 * media on: ‘timer:’ do: { :t
 * ✎ write: ‘timer:’ + t, stop.
 * media end.
 * }.
 * media screen: ‘canvas.png’.
 */
ctr_object* ctr_media_timer(ctr_object* myself, ctr_argument* argumentList) {
	int timer_no = (int) ctr_tonum(ctr_internal_cast2number(argumentList->object));
	int ms = (int) ctr_tonum(ctr_internal_cast2number(argumentList->next->object));
	if (timer_no < 1 || timer_no > CtrMaxMediaTimers) {
		ctr_error("Invalid timer", 0);
	} else {
		CtrMediaTimers[timer_no] = ms;
	}
	return myself;
}

/**
 * @def
 * [ Media ] screen: [ Text ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * 
 * media on: ‘start’ do: { ... }.
 * media on: ‘step’ do: { ... }.
 * media on: ‘key:’ do: { ... }.
 * media on: ‘key down:’ do: { ... }.
 * media on: ‘gamepad:’ do: { ... }.
 * media on: ‘gamepad down:’ do: { ... }.
 * media on: ‘click x:y:’ do: { ... }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * en: Opens a screen with a background image or video. Afterwards your media object will start receiving events (see above).
 *
 */
ctr_object* ctr_media_screen(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* player;
	MediaIMG* focusImage;
	int x = 0, y = 0, dir, c4speed;
	CtrMediaInputIndex = 0;
	CtrMediaSelectStart =0;
	CtrMediaSelectBegin = 0;
	CtrMediaSelectEnd=0;
	controllableObject = NULL;
	focusObject = NULL;
	CtrMediaSteps = 0;
	SDL_Rect dimensions;
	SDL_Texture* texture;
	char* imageFileStr = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	char ftype = ctr_internal_media_determine_filetype(imageFileStr);
	char background_is_video;
	if (ftype == 10) {
		ctr_internal_media_loadvideobg(imageFileStr, &dimensions);
		background_is_video = 1;
	} else {
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
		background_is_video = 0;
	}
	ctr_heap_free(imageFileStr);
	windowWidth = dimensions.w;
	windowHeight = dimensions.h;
	SDL_GL_GetDrawableSize(CtrMediaWindow, &CtrMediaDrawSizeX, &CtrMediaDrawSizeY);
	ctr_send_message(myself, CTR_DICT_RUN, strlen(CTR_DICT_RUN), NULL );
	if (CtrStdFlow) return myself;
	SDL_Event event;
	dir = -1;
	c4speed = 0;
	while (1) {
		ctr_gc_cycle(); 
		SDL_RenderClear(CtrMediaRenderer);
		player = NULL;
		if (controllableObject) {
			player = (MediaIMG*) controllableObject->value.rvalue->ptr;
		}
		if (background_is_video) {
			ctr_internal_media_rendervideoframe(&dimensions);
		} else {
		    SDL_Rect s = dimensions;
		    if (CtrMediaCamera.w > 0 && CtrMediaCamera.h > 0) {
				ctr_internal_media_camera(NULL, &s, &dimensions, player);
			}
			SDL_RenderCopy(CtrMediaRenderer, texture, &s, &dimensions);
		}
		myself->info.sticky = 1;
		if (CtrMediaEventListenFlagTimer) {
			for(int i = 1; i <= CtrMaxMediaTimers; i++) {
				if (CtrMediaTimers[i] < 0) continue;
				if (CtrMediaTimers[i] == 0) {
					ctr_media_event_timer(myself, CTR_DICT_ON_TIMER, i);
				}
				CtrMediaTimers[i]--;
			}
		}
		if (CtrMediaEventListenFlagStep) {
			ctr_send_message(myself, CTR_DICT_ON_STEP, strlen(CTR_DICT_ON_STEP), NULL );
		}
		myself->info.sticky = 0;
		if (CtrMediaBreakLoopFlag) {
			ctr_internal_media_reset();
			return myself;
		}
		while (SDL_PollEvent(&event)) {
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
					if (event.tfinger.x * windowWidth > player->x) {
						ctr_internal_media_keydown_right(&dir);
					}
					else if (event.tfinger.x * windowWidth < player->x - (player->w/player->anims)) {
						ctr_internal_media_keydown_left(&dir);
					}
					if (event.tfinger.y * windowHeight > player->y - player->h) {
						ctr_internal_media_keydown_down(&dir, &c4speed);
					}
					else if (event.tfinger.y * windowHeight < player->y - player->h) {
						ctr_internal_media_keydown_up(&dir, &c4speed);
					}
					break;
				case SDL_FINGERUP:
						ctr_internal_media_keyup_right(&dir, &c4speed);
						ctr_internal_media_keyup_down(&dir, &c4speed);
					break;
				case SDL_CONTROLLERBUTTONDOWN:
					if (CtrMediaEventListenFlagGamePadBtnDown) {
						ctr_media_event(myself, CTR_DICT_ON_GAMEPAD_DOWN, SDL_GameControllerGetStringForButton(event.cbutton.button));
					}
					switch(event.cbutton.button) {
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
					if (CtrMediaEventListenFlagGamePadBtnUp) {
						ctr_media_event(myself, CTR_DICT_ON_GAMEPAD_UP, SDL_GameControllerGetStringForButton(event.cbutton.button));
					}
					switch(event.cbutton.button) {
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
							ctr_internal_media_autoreplace(focusImage);
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
			if (CtrMediaJump == 1) {
				CtrMediaJumpHeight = player->h * CtrMediaJumpHeightFactor;
				CtrMediaJump = 2;
			}
			if (CtrMediaJump == 2) {
				player->y -= log(CtrMediaJumpHeight);
				CtrMediaJumpHeight /= 1.2;
				if (CtrMediaJumpHeight < 1) {
					CtrMediaJump = 3;
					CtrMediaJumpHeight = 0;
				}
			}
		}
		for(int i = 0; i < IMGCount; i ++) {
			MediaIMG* m = &mediaIMGs[i];
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
		if (focusObject) {
			ctr_internal_img_render_cursor(focusObject);
		}
		SDL_RenderPresent(CtrMediaRenderer);
		SDL_Delay(CtrMediaStdDelayTime);
		CtrMediaSteps++;
	}
	return myself;
}

/**
 * @def
 * Point
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ p ≔ Point new x: 10 y: 20.
 * ✎ write: p x?, stop.
 * ✎ write: p y?, stop.
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
 * ☞ media ≔ Media new.
 * ☞ p ≔ Point new x: 10 y: 20.
 * ✎ write: p x?, stop.
 * ✎ write: p y?, stop.
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
 * ☞ media ≔ Media new.
 * ☞ p ≔ Point new x: 10 y: 20.
 * ✎ write: p x?, stop.
 * ✎ write: p y?, stop.
 */
ctr_object* ctr_point_y(ctr_object* myself, ctr_argument* argumentList) {
	int y = (int) ctr_tonum(ctr_internal_object_property(myself, "y", NULL));
	ctr_internal_natural_y(&y);
	return ctr_build_number_from_float(y);
}

/**
 * @def
 * [ Point ] x: [ Number ] y: [ Number ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ p ≔ Point new x: 10 y: 20.
 * ✎ write: p x?, stop.
 * ✎ write: p y?, stop.
 */
ctr_object* ctr_point_xyset(ctr_object* myself, ctr_argument* argumentList) {
	int x = (int) ctr_tonum(ctr_internal_cast2number(argumentList->object));
	int y = (int) ctr_tonum(ctr_internal_cast2number(argumentList->next->object));
	ctr_internal_natural_y(&y);
	ctr_internal_object_property(myself, "x", ctr_build_number_from_float(x));
	ctr_internal_object_property(myself, "y", ctr_build_number_from_float(y));
	return myself;
}

/**
 * @def
 * Line
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Point new x: 10 y: 10.
 * ☞ b ≔ Point new x: 20 y: 20.
 * ☞ c ≔ Line from: a to: b.
 * ✎ write: c from, stop.
 * ✎ write: c to, stop.
 */
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
 * ☞ media ≔ Media new.
 * ☞ a ≔ Point new x: 10 y: 10.
 * ☞ b ≔ Point new x: 20 y: 20.
 * ☞ c ≔ Line from: a to: b.
 * ✎ write: c from, stop.
 * ✎ write: c to, stop.
 */
ctr_object* ctr_line_from_to(ctr_object* myself, ctr_argument* argumentList) {
	ctr_internal_object_property(myself, CTR_DICT_FROM, argumentList->object);
	ctr_internal_object_property(myself, CTR_DICT_TO, argumentList->next->object);
	return myself;
}

/**
 * @def
 * [ Line ] from
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Point new x: 10 y: 10.
 * ☞ b ≔ Point new x: 20 y: 20.
 * ☞ c ≔ Line from: a to: b.
 * ✎ write: c from, stop.
 * ✎ write: c to, stop.
 */
ctr_object* ctr_line_start(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(myself, CTR_DICT_FROM, NULL);
}

/**
 * @def
 * [ Line ] to
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Point new x: 10 y: 10.
 * ☞ b ≔ Point new x: 20 y: 20.
 * ☞ c ≔ Line from: a to: b.
 * ✎ write: c from, stop.
 * ✎ write: c to, stop.
 */
ctr_object* ctr_line_end(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(myself, CTR_DICT_TO, NULL);
}

/**
 * @def
 * [ Colour ] new
 *
 * @example
 * ☞ media ≔ Media new.
 * ☞ x ≔ Colour new.
 * ✎ write: x red, stop.
 * ✎ write: x green, stop.
 * ✎ write: x blue, stop.
 */
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
 * [ Colour ] red: [Number] green: [Number] blue [Number]
 *
 * @example
 * ☞ media ≔ Media new.
 * ☞ x ≔ Colour new red: 100 green: 150 blue: 200.
 * ✎ write: x red, stop.
 * ✎ write: x green, stop.
 * ✎ write: x blue, stop.
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

ctr_object* ctr_media_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	instance->link = myself;
	return instance;
}

ctr_object* ctr_audio_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	return instance;
}

void ctr_audio_destructor(ctr_resource* rs) {
	MediaAUD* mediaAUD = (MediaAUD*) rs->ptr;
	mediaAUD->ref = NULL;
}

/**
 * @def
 * Sound
 *
 * @example
 * ☞ fx ≔ Sound new: ‘boom.mp3’.
 * fx play.
 *
 * @result
 * (plays sound)
 */
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
	rs->destructor = &ctr_audio_destructor;
	AUDCount++;
	ctr_heap_free(audioFileStr);
	return audioInst;
}

/**
 * @def
 * [ Sound ] play
 *
 * @example
 * ☞ fx ≔ Sound new: ‘boom.mp3’.
 * fx play.
 *
 * @result
 * (plays sound)
 */
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
		return NULL;
	}
	SDL_RWseek(asset_file, 0, RW_SEEK_SET);
	char* buffer = malloc(500);
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
	return res;
}

/**
 * @def
 * Music
 *
 * @example
 * ☞ j ≔ Music new: ‘jazz.mp3’.
 * j play.
 * Moment wait: 1.
 * j silence.
 * j rewind.
 *
 * @result
 * 𝄞 (plays music)
 *
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
	rs->destructor = &ctr_audio_destructor;
	AUDCount++;
	ctr_heap_free(audioFileStr);
	return audioInst;
}

/**
 * @def
 * [ Music ] play
 *
 * @example
 * ☞ j ≔ Music new: ‘jazz.mp3’.
 * j play.
 * Moment wait: 1.
 * j silence.
 * j rewind.
 *
 * @result
 * 𝄞 (plays music)
 *
 */
ctr_object* ctr_music_play(ctr_object* myself, ctr_argument* argumentList) {
	MediaAUD* mediaAUD = ctr_internal_get_audio_from_object(myself);
	if (mediaAUD == NULL) return myself;
	if (mediaAUD->blob != NULL) {
			Mix_FadeInMusic((Mix_Music*)mediaAUD->blob,-1,0);
	}
	return myself;
}

/**
 * @def
 * [ Music ] silence
 *
 * @example
 * ☞ j ≔ Music new: ‘jazz.mp3’.
 * j play.
 * Moment wait: 1.
 * j silence.
 * j rewind.
 *
 * @result
 * 𝄞 (plays music)
 *
 */
ctr_object* ctr_music_silence(ctr_object* myself, ctr_argument* argumentList) {
	MediaAUD* mediaAUD = ctr_internal_get_audio_from_object(myself);
	if (mediaAUD == NULL) return myself;
	if (mediaAUD->blob != NULL) {
			Mix_PauseMusic();
	}
	return myself;
}

/**
 * @def
 * [ Music ] rewind
 *
 * @example
 * ☞ j ≔ Music new: ‘jazz.mp3’.
 * j play.
 * Moment wait: 1.
 * j silence.
 * j rewind.
 *
 * @result
 * 𝄞 (plays music)
 *
 */
ctr_object* ctr_music_rewind(ctr_object* myself, ctr_argument* argumentList) {
	MediaAUD* mediaAUD = ctr_internal_get_audio_from_object(myself);
	if (mediaAUD == NULL) return myself;
	if (mediaAUD->blob != NULL) {
			Mix_RewindMusic();
	}
	return myself;
}

int receiver_socket_descriptor;
int socket_descriptor;
int CtrNetworkConnectedFlag = 0;
struct sockaddr_in host;
struct sockaddr_in host2;

#ifndef REPLACE_MEDIA_SOCK
void ctr_internal_media_sock() {
	receiver_socket_descriptor = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	printf("receiver_socket_descriptor = %d %s \n", receiver_socket_descriptor,strerror(errno));
	if (receiver_socket_descriptor == -1) {
		ctr_error("Unable to open receiver socket: %s.\n", CTR_ERR);
	}
	socket_descriptor = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
	if (socket_descriptor == -1) {
		ctr_error("Unable to open receiver socket: %s.", CTR_ERR);
	}
}
#endif

#ifdef WINDOWS_MEDIA_SOCK
void ctr_internal_media_sock() {
	u_long socket_mode = 1;
	u_long socket_mode2 = 1;
	receiver_socket_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (receiver_socket_descriptor == INVALID_SOCKET) {
		ctr_error("Unable to open receiver socket: %s.", CTR_ERR);
	}
	socket_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socket_descriptor == INVALID_SOCKET) {
		ctr_error("Unable to open data socket: %s.", CTR_ERR);
	}
	if (ioctlsocket(receiver_socket_descriptor, FIONBIO, &socket_mode)) {
		ctr_error("Unable to configure receiver socket: %s.", CTR_ERR);
	}
	if (ioctlsocket(socket_descriptor, FIONBIO, &socket_mode2)) {
		ctr_error("Unable to configure data socket: %s.", CTR_ERR);
	}
}
#endif


#ifdef MAC_MEDIA_SOCK
void ctr_internal_media_sock() {
	receiver_socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
	if (receiver_socket_descriptor == -1) {
		ctr_error("Unable to open receiver socket: %s.", CTR_ERR);
	}
	socket_descriptor = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_descriptor == -1) {
		ctr_error("Unable to open data socket: %s.", CTR_ERR);
	}
	if (fcntl(receiver_socket_descriptor, F_SETFL, O_NONBLOCK)) {
		ctr_error("Unable to configure receiver socket: %s.", CTR_ERR);
	}
	if (fcntl(socket_descriptor, F_SETFL, O_NONBLOCK)) {
		ctr_error("Unable to configure data socket: %s.", CTR_ERR);
	}
}
#endif


/**
 * @def
 * Network
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ network ≔ Network new.
 * ✎ write: network, stop.
 */
ctr_object* ctr_network_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	return instance;
}

int ctr_internal_network_activate() {
	char* network_port_str;
	int network_port_num;
	if (CtrNetworkConnectedFlag == 0) {
		#ifdef WINDOWS_MEDIA_SOCK
		WSADATA version_info;
		int success_winsock = WSAStartup(MAKEWORD(2,2), &version_info);
		if (success_winsock != 0) {
			ctr_error("WSAStartup failed: %s.", CTR_ERR);
			CtrNetworkConnectedFlag = -1;
		}
		#endif
		ctr_internal_media_sock();
	}
	if (CtrNetworkConnectedFlag == 0) {
		host.sin_family = AF_INET;
		host2.sin_family = AF_INET;
		network_port_str = getenv("MediaNetPort1");
		if (network_port_str == NULL) {
			network_port_num = 9000;
		} else {
			network_port_num = atoi(network_port_str);
		}
		host.sin_port = htons(network_port_num);
		host.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(receiver_socket_descriptor, (struct sockaddr *) &host, sizeof(host))==-1) {
			ctr_error("Unable to bind reader socket to port %s.", CTR_ERR);
			CtrNetworkConnectedFlag = -1;
		}
	}
	if (CtrNetworkConnectedFlag == 0) {
		host2.sin_addr.s_addr = htonl(INADDR_ANY);
		host2.sin_port = htons(network_port_num + 1);
		if (bind(socket_descriptor, (struct sockaddr *) &host2, sizeof(host2))==-1) {
			ctr_error("Unable to bind writer socket to port %s.", CTR_ERR);
			CtrNetworkConnectedFlag = -1;
		}
		CtrNetworkConnectedFlag = 1;
	}
	return CtrNetworkConnectedFlag;
}

int ctr_internal_send_network_message(void* message, int messagelen, char* ip_str, uint16_t port) {
	if (ctr_internal_network_activate() != 1) return 0;
	struct sockaddr_in remote_host;
	if (inet_pton(AF_INET, ip_str, &remote_host.sin_addr)==0) {
		ctr_error("Invalid IP\n", 0);
		return 0;
	}
	remote_host.sin_family = AF_INET;
	remote_host.sin_port = htons(port);
	int bytes_sent = sendto(socket_descriptor, message, messagelen, 0, (struct sockaddr*) &remote_host, sizeof(remote_host));
	return bytes_sent;
}


int ctr_internal_receive_network_message(void* buffer, int messagelen, char* ip_str, uint16_t* port) {
	if (ctr_internal_network_activate() != 1) return 0;
	struct sockaddr_in source_host;
	int bytes_received = 0;
	socklen_t source_host_len = sizeof(struct sockaddr_in);
	bytes_received = recvfrom(receiver_socket_descriptor, buffer, messagelen, 0, (struct sockaddr*) &source_host, &source_host_len);
	if (bytes_received > 0) {
		inet_ntop(AF_INET, (struct sockaddr*) &source_host.sin_addr, ip_str, source_host_len);
		*port = ntohs(source_host.sin_port);
	}
	return bytes_received;
}

uint16_t CtrMediaNetworkCunkId = 1;


/**
 * @def
 * [ Network ] text: [ Text ] to: [ Text ]
 * 
 * @example
 * Program setting: network port value: ‘9002’.
 * Network text: ‘hello’ to: ‘127.0.0.1:9000’.
 * 
 * @result
 * en: Sends a text message to the specified computer.
*/
ctr_object* ctr_network_basic_text_send(ctr_object* myself, ctr_argument* argumentList) {
	char* data = argumentList->object->value.svalue->value;
	uint64_t total_size = argumentList->object->value.svalue->vlen;
	int bytes_sent;
	int bytes_received = -1;
	uint16_t recipient_port = 0;
	char ip_str_recipient[40];
	char ip_str[40];
	double timeout = 10;
	double interval = 0.01;
	uint16_t chunk_size = CtrMediaNetworkChunkSize;
	uint16_t port = 9000;
	uint64_t chunks = ceil((double)total_size / chunk_size);
	char* ip_str_and_port = ctr_heap_allocate_cstring(argumentList->next->object);
	char* colon_pos = strstr(ip_str_and_port,":");
	if (colon_pos == NULL) {
		strcpy(ip_str, ip_str_and_port);
		ctr_heap_free(ip_str_and_port);
	} else {
		memcpy(ip_str, ip_str_and_port, colon_pos-ip_str_and_port);
		ip_str[colon_pos-ip_str_and_port]='\0';
		ctr_heap_free(ip_str_and_port);
		port = atoi(colon_pos+1);
		if (port < 1024) {
			ctr_error("Invalid port\n",0);
			return CtrStdBoolFalse;
		}
	}
	ctr_size i;
	uint64_t remote_id = 0;
	char buffer[500];
	char buffer2[500];
	memset(buffer, 0, 500);
	memset(buffer2, 0, 500);
	int retry = 3;
	uint16_t chunk_id;
	uint16_t remote_chunk_id;
	for(i = 0; i < chunks; i++) {
		chunk_id = CtrMediaNetworkCunkId++;
		*(buffer + 0) = 1;
		memcpy(buffer + 1, &total_size, 8);
		memcpy(buffer + 9, (char*)&remote_id, 8);
		uint64_t offset = i * chunk_size;
		memcpy(buffer + 17, (char*)&offset, 8);
		memcpy(buffer + 25, &chunk_size, 2);
		memcpy(buffer + 27, data + offset, (int) fmin(chunk_size, total_size - offset));
		memcpy(buffer + 27 + chunk_size, &chunk_id, 2);
		bytes_sent = ctr_internal_send_network_message((void*)buffer, 500, ip_str, port);
		if (!bytes_sent) {
			return CtrStdBoolFalse;
		}
		while(1) {
			bytes_received = ctr_internal_receive_network_message(buffer2, 500, ip_str_recipient, &recipient_port);
			SDL_Delay(1);
			timeout -= interval;
			if (timeout < 1) break;
			if (bytes_received > 0) {
				memcpy(&remote_chunk_id, buffer2 + 9, 2);
				if (remote_chunk_id == chunk_id) {
					break;
				}
			}
		}
		if (timeout < 1 && retry>0) {
			retry--;
			timeout = 10;
			i--;
			CtrMediaNetworkCunkId--;
			continue;
		}
		if (timeout < 1 && retry < 1) {
			ctr_error("failed to send message\n", 0);
			return CtrStdBoolFalse;
		}
		retry = 0;
		if (buffer2[0] == 2) {
			remote_id = (uint64_t) *((uint64_t*)(buffer2 + 1));
		}
	}
	*(buffer + 0) = 1;
	*(buffer + 1) = total_size;
	memcpy(buffer + 9, (char*)&remote_id, 8);
	uint64_t offset = i * chunk_size;
	memcpy(buffer + 17, (char*)&offset, 8);
	memset(buffer + 25, 0, 2);
	chunk_id = CtrMediaNetworkCunkId++;
	memcpy(buffer + 27 + 0, &chunk_id, 2);
	retry = 3;
	timeout = 10;
	while(retry>0) {
		bytes_sent = ctr_internal_send_network_message((void*)buffer, 500, ip_str, port);
		while(1) {
			bytes_received = ctr_internal_receive_network_message(buffer2, 500, ip_str_recipient, &recipient_port);
			SDL_Delay(1);
			timeout -= interval;
			if (timeout < 1) break;
			if (bytes_received > 0 && buffer2[0] == 2) {
				memcpy(&remote_chunk_id, buffer2 + 9, 2);
				if (remote_chunk_id == chunk_id) {
					break;
				} else {
					ctr_error("received invalid reply\n", 0);
					return CtrStdBoolFalse;
				}
			}
		}
		if (remote_chunk_id != chunk_id) {
			if (timeout < 1 && retry>0) {
				retry--;
				timeout = 10;
				continue;
			}
			if (timeout < 1 && retry < 1) {
				ctr_error("failed to send final message\n", 0);
				return CtrStdBoolFalse;
			}
		} else {
			break;
		}
	}
	retry = 0;
	return CtrStdBoolTrue;
}


char* documents[100];
int max_documents = 100;

struct document_lock {
	time_t since;
	char* ip_str;
};
typedef struct document_lock document_lock;

document_lock* document_locks[100];

uint16_t chunk_size;

/**
 * @def
 * [ Network ] text messages
 * 
 * @example
 * ☞ receive ≔ Network text messages.
 * 
 * @result
 * en: Receives text messages send to this computer.
*/
ctr_object* ctr_network_basic_text_receive(ctr_object* myself, ctr_argument* argumentList) {
	char buffer[500];
	char buffer2[500];
	char ip_str[40];
	uint16_t src_port = 0;
	int bytes_received = 0;
	uint64_t offset = 0;
	uint64_t document_id = 0;
	int j;
	uint64_t total_size = 0;
	ctr_object* received_text_message = CtrStdNil;
	double timeout = 10;
	double interval = 0.01;
	int expire_time = 10;
	time_t current_time = time(NULL);
	for(int i = 0; i<max_documents; i++) {
		if (document_locks[i] != NULL && document_locks[i]->since < current_time - expire_time) {
			free(documents[i]);
			free(document_locks[i]->ip_str);
			free((void*)document_locks[i]);
			documents[i] = NULL;
			document_locks[i] = NULL;
		}
	}
	while(timeout>0) {
		bytes_received = ctr_internal_receive_network_message(buffer, 500, ip_str, &src_port);
		if (bytes_received < 1 || buffer[0]!=1) {
			SDL_Delay(1);
			timeout-=interval;
			continue;
		}
		if (bytes_received > 0 && buffer[0] == 1) {
			uint16_t chunk_id;
			document_id = (uint64_t) *((uint64_t*)(buffer + 9));
			if (document_id == 0) {
				for(j=0; j<max_documents; j++) {
					if (documents[j] == NULL) {
						break;
					}
				}
				if (j == max_documents) {
					ctr_error("Message buffers full\n", 0);
					return CtrStdNil;
				}
				total_size = (ctr_size) *((ctr_size*)(buffer + 1));
				documents[j] = malloc(total_size);
				document_locks[j] = (document_lock*) malloc(sizeof(document_lock));
				document_locks[j]->since = time(NULL);
				document_locks[j]->ip_str = malloc(40);
				memcpy(document_locks[j]->ip_str, ip_str, 40);
				memcpy(&chunk_size, buffer + 25, 2);
				offset = *(buffer + 17);
				memcpy(documents[j] + offset, buffer + 27, (int) fmin(chunk_size, total_size - offset));
			} else {
				for(j=0; j<max_documents; j++) {
					#ifdef WIN32_BIT
					if ((uint32_t) documents[j] == (uint32_t) *((uint64_t*)(buffer + 9))) {
						break;
					}
					#else
					if ((uint64_t) documents[j] == (uint64_t) *((uint64_t*)(buffer + 9))) {
						break;
					}
					#endif
				}
				if (j == max_documents && chunk_size>0) {
					ctr_error("Message buffer not found\n", 0);
					return CtrStdNil;
				}
				total_size = (ctr_size) *((ctr_size*)(buffer + 1));
				offset = (uint64_t) *((uint64_t*)(buffer + 17));
				memcpy(&chunk_size, buffer + 25, 2);
				if (chunk_size == 0 && j != max_documents) {
					received_text_message = ctr_build_string((char*) documents[j], (ctr_size) total_size);
					received_text_message->info.sticky = 1;
					free(documents[j]);
					free(document_locks[j]->ip_str);
					free((void*)document_locks[j]);
					documents[j] = NULL;
					document_locks[j] = NULL;
				} else {
					memcpy(documents[j] + offset, buffer + 27, (int) fmin(chunk_size, total_size - offset));
				}
			}
			memcpy(&chunk_id, buffer + 27 + chunk_size, 2);
			memset(buffer2, 0, 500);
			*(buffer2 + 0) = 2;
			if (j == max_documents) {
				memset(buffer2 + 1,  0, 8);
			} else {
				memcpy(buffer2 + 1,  (uint64_t*) &documents[j], 8);
			}
			memcpy(buffer2 + 9,  (char*) &chunk_id, 2);
			int bytes_sent = ctr_internal_send_network_message((void*)buffer2, 500, ip_str, src_port-1);
			if (!bytes_sent) {
				ctr_error("Unable to sent message receipt\n", 0);
				return CtrStdNil;
			}
		}
		if (received_text_message != CtrStdNil) break;
	}
	return received_text_message;
}

/**
 * @def
 * Image
 * 
 * @example
 * ☞ image ≔ Image new: ‘a.png’.
 * image on: ‘click’ do: { ... }.
 * image on: ‘hover’ do: { ... }.
 * image on: ‘collision:’ do: { :other ... }.
 * image on: ‘destination:’ do: { :other ... }.
 *
 * @result
 * en: Loads an image. Afterwards your image will start receiving events (see above).
 */
ctr_object* ctr_img_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	return instance;
}

void ctr_img_destructor(ctr_resource* rs) {
	MediaIMG* image = (MediaIMG*) rs->ptr;
	image->ref = NULL;
}

/**
 * @def
 * [ Image ] image: [ Text ]
 * 
 * @example
 * ☞ image ≔ Image new: ‘a.png’.
 * image image: ‘b.png’.
 * 
 * @result
 * en: Updates the graphical contents of an image.
 */
ctr_object* ctr_img_img(ctr_object* myself, ctr_argument* argumentList) {
	SDL_Rect dimensions;
	dimensions.x = 0;
	dimensions.y = 0;
	MediaIMG* mediaImage = ctr_internal_get_image_from_object(myself);
	if (mediaImage == NULL) {
		mediaImage = &mediaIMGs[IMGCount++];
		mediaImage->x = 0;
		mediaImage->y = 0;
		mediaImage->ox = 0;
		mediaImage->oy = 0;
		mediaImage->tx = -1;
		mediaImage->ty = -1;
		mediaImage->bounce = 0;
		mediaImage->solid = 0;
		mediaImage->collidable = 0;
		mediaImage->gravity = 0;
		mediaImage->speed = 1;
		mediaImage->fric = 0;
		mediaImage->accel = 1;
		mediaImage->gspeed = 0;
		mediaImage->dir = -1;
		mediaImage->mov = 0;
		mediaImage->anims = 1;
		mediaImage->editable = 0;
		mediaImage->text = NULL;
		mediaImage->paddingx = 0;
		mediaImage->paddingy = 0;
		mediaImage->textlength = 0;
		mediaImage->textbuffer = 0;
		mediaImage->font = NULL;
		mediaImage->color = (SDL_Color) {0,0,0,0};
		mediaImage->backgroundColor = (SDL_Color) {0,0,0,0};
		mediaImage->ref = myself;
		ctr_resource* rs = ctr_heap_allocate( sizeof(ctr_resource) );
		rs->ptr = mediaImage;
		rs->destructor = &ctr_img_destructor;
		myself->value.rvalue = rs;
	}
	char* imageFileStr = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	SDL_RWops* res;
	res = ctr_internal_media_load_asset(imageFileStr, 1);
	if (res) {
		mediaImage->texture = (void*) IMG_LoadTexture_RW(CtrMediaRenderer, res, 0);
		SDL_RWseek(res, 0, RW_SEEK_SET);
		mediaImage->surface = (void*) IMG_Load_RW(res, 0);
	}
	if (mediaImage->texture == NULL) ctr_internal_media_fatalerror("Unable to load texture", imageFileStr);
	if (mediaImage->surface == NULL) ctr_internal_media_fatalerror("Unable to load surface", imageFileStr);
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
 * [ Image ] controllable: [ Number ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a 
 * controllable: 4.
 * 
 * }.
 * 
 * media screen: ‘canvas.png’.
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
 * [ Image ] x: [ Number ] y: [ Number ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 * a x: 10 y: 5.
 * 
 * @result
 * [[img_xy]]
 * 
 */
ctr_object* ctr_img_xy(ctr_object* myself, ctr_argument* argumentList) {
	int x = (int) ctr_tonum(ctr_internal_cast2number(argumentList->object));
	int y = (int) ctr_tonum(ctr_internal_cast2number(argumentList->next->object));
	ctr_internal_natural_y(&y);
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->ox = x;
	image->oy = y;
	image->x = x;
	image->y = y - image->h;
	return myself;
}

/**
 * @def
 * [ Image ] x?
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 * a x: 10 y: 5.
 * ✎ write: a x?, stop.
 */
ctr_object* ctr_img_x(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	return ctr_build_number_from_float(image->x);
}

/**
 * @def
 * [ Image ] y?
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 * a x: 10 y: 5.
 * ✎ write: a y?, stop.
 */
ctr_object* ctr_img_y(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	int y = (int) ctr_tonum(ctr_build_number_from_float(image->y));
	y += image->h;
	ctr_internal_natural_y(&y);
	return ctr_build_number_from_float(y);
}


/**
 * @def
 * [ Image ] move to x: [ Number ] y: [ Number ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 * media on: ‘start’ do: {
 * a 
 * x: 10 y: 10, 
 * speed: 1,
 * move to x: 20 y: 20.
 * }.
 *
 * media screen: ‘canvas.png’.
 * 
 * @result
 * [[img_movset]]
 */
ctr_object* ctr_img_mov_set(ctr_object* myself, ctr_argument* argumentList) {
	double x = (int) ctr_internal_cast2number(argumentList->object)->value.nvalue;
	int y_ = (int) ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
	ctr_internal_natural_y(&y_);
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	double y = (double) (y_ - image->h);
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
 * [ Image ] bounce: [ Decision ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 * media on: ‘start’ do: {
 * a 
 * x: 20 y: 500, 
 * speed: 10,
 * move to x: 800 y: 0,
 * bounce: Yes.
 * }.
 *
 * media screen: ‘canvas.png’.
 * 
 * @result
 * en: Toggles whether the image will bounce upon collision.
 */
ctr_object* ctr_img_bounce(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->bounce = (int) ctr_internal_cast2bool(argumentList->object)->value.bvalue;
	return myself;
}

/**
 * @def
 * [ Image ] solid: [ Decision ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 * ☞ b ≔ Image new: ‘b.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a controllable: Yes.
 * b x: 100, y: 100. 
 * 
 * }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * [[img_solid]]
 */
ctr_object* ctr_img_solid(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->solid = (int) ctr_internal_cast2bool(argumentList->object)->value.bvalue;
	return myself;
}


/**
 * @def
 * [ Image ] active: [ Decision ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 * a active: Yes.
 * 
 * @result
 * en: Allows the image to receive messages upon events.
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
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a 
 * x: 1 y: 100,
 * gravity: 2.
 *
 * }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * [[img_accel]]
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
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a 
 * x: 1 y: 1,
 * accelerate: 0.01,
 * speed: 5,
 * friction: 1
 * move to x: 200 y: 1.
 *
 * }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * [[img_accel]]
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
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a 
 * x: 1 y: 1,
 * accelerate: 0.01,
 * speed: 5,
 * friction: 1
 * move to x: 200 y: 1.
 *
 * }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * [[img_accel]]
 */
ctr_object* ctr_img_friction(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->fric = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}

/**
 * @def
 * [ Image ] accelerate: [ Number ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a 
 * x: 1 y: 1,
 * accelerate: 0.01,
 * speed: 5,
 * friction: 1
 * move to x: 200 y: 1.
 *
 * }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * [[img_accel]]
 */
ctr_object* ctr_img_accel(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->accel = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}


/**
 * @def
 * [ Image ] jump height: [ Number ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a 
 * controllable: Yes,
 * gravity: 1,
 * jump height: 6.
 * 
 * }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * [[img_jump_height]]
 */
ctr_object* ctr_img_jump_height(ctr_object* myself, ctr_argument* argumentList) {
	CtrMediaJumpHeightFactor = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}


/**
 * @def
 * [ Image ] editable: [ Decision ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a
 * x: 0 y: 200,
 * font: ‘font.ttf’ size: 16,
 * align x: 40 y: 20,
 * colour: (Colour new red: 110 green: 110 blue: 110),
 * write: ‘...’,
 * editable: Yes.
 *
 * }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * en: Makes an image editable, you can now enter text inside it.
 */
ctr_object* ctr_img_editable(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->editable = ctr_internal_cast2bool(argumentList->object)->value.bvalue;
	return myself;
}


/**
 * @def
 * [ Image ] animations: [ Number ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a 
 * controllable: Yes,
 * animations: 2.
 * 
 * }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * [[img_anims]]
 */
ctr_object* ctr_img_anims(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->anims = (int) ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}


/**
 * @def
 * [ Image ] font: [ Text ] size: [ Number ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a
 * x: 0 y: 200,
 * font: ‘font.ttf’ size: 16,
 * align x: 40 y: 20,
 * colour: (Colour new red: 110 green: 110 blue: 110),
 * write: ‘ABC’.
 *
 * }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * en: Sets the font and size of the text in an image.
 */
ctr_object* ctr_img_font(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	char* path = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	image->font = TTF_OpenFont(path, (int)ctr_internal_cast2number(argumentList->next->object)->value.nvalue);
	ctr_heap_free(path);
	if (image->font == NULL) ctr_internal_media_fatalerror("Unable to load font", "TTF Font");
	/* Allow to set compile-time FONTSCRIPT for Harfbuzz shaper */
	#ifdef FONTSCRIPT
	int script_ok = TTF_SetFontScriptName(image->font, FONTSCRIPT);
	if (script_ok == -1) {
		ctr_print_error("Error setting font script.", -1);
	}
	#endif
	return myself;
}


/**
 * @def
 * [ Image ] colour: [ Colour ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a
 * x: 0 y: 200,
 * font: ‘font.ttf’ size: 16,
 * align x: 40 y: 20,
 * colour: (Colour new red: 110 green: 110 blue: 110),
 * write: ‘ABC’.
 * }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * en: Sets the colour for text in an image.
 */
ctr_object* ctr_img_color(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	uint8_t r = (uint8_t)ctr_color_r(argumentList->object, NULL)->value.nvalue;
	uint8_t g = (uint8_t)ctr_color_g(argumentList->object, NULL)->value.nvalue;
	uint8_t b = (uint8_t)ctr_color_b(argumentList->object, NULL)->value.nvalue;
	uint8_t a = (uint8_t)ctr_color_a(argumentList->object, NULL)->value.nvalue;
	image->color = (SDL_Color) { r, g, b, a };
	return myself;
}

/**
 * @def
 * [ Image ] background colour: [ Colour ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a
 * x: 0 y: 200,
 * font: ‘font.ttf’ size: 16,
 * align x: 40 y: 20,
 * background colour: (Colour new red: 110 green: 110 blue: 110),
 * write: ‘ABC’.
 * }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * en: Sets the background colour for text in an image.
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
 * [ Image ] align x: [ Number ] y: [ Number ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a
 * x: 0 y: 200,
 * font: ‘font.ttf’ size: 16,
 * align x: 40 y: 20,
 * background colour: (Colour new red: 110 green: 110 blue: 110),
 * write: ‘ABC’.
 *
 * }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * en: Sets the position of the text within the image.
 */
ctr_object* ctr_img_text_align(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = ctr_internal_get_image_from_object(myself);
	if (image == NULL) return myself;
	image->paddingx = (int)ctr_internal_cast2number(argumentList->object)->value.nvalue;
	int y = (int)ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
	int lineHeight;
	TTF_SizeUTF8(image->font, "X", NULL, &lineHeight);
	y = image->h - y;
	y -= lineHeight;
	image->paddingy = y;
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
	int x1 = 0;
	int y1 = 0;
	int beginline = 0;
	while(i<CtrMediaInputIndex) {
		if (image->text[i]=='\r') {
			y1 += 1;
			x1 = 0;
		} else if (image->text[i]=='\n') {
			beginline = i;
		} else if ((image->text[i] & 0x80) == 0x00 || (image->text[i] & 0xC0) == 0xC0) {
			x1++;
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
		char* measurementBuffer = malloc(measurementBufferSize);
		memcpy(measurementBuffer, measurementBufferStart, measurementBufferLength);
		measurementBuffer[measurementBufferLength] = '\0';
		if (TTF_SizeUTF8(image->font, measurementBuffer, &offsetx, NULL)) ctr_internal_media_fatalerror("Unable to measure font", "TTF_SizeUTF8");
		free(measurementBuffer);
	}
	if (TTF_SizeUTF8(image->font, "X", NULL, &height)) ctr_internal_media_fatalerror("Unable to measure font", "TTF_SizeUTF8");
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
	char* buff = malloc(buffsize + 1);
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
			int cacheoffset = CtrMediaCursorOffsetY % 100;
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
			buff = malloc(buffsize + 1);
			memcpy(buff, image->text+line_segment_start, i-line_segment_start);
			memcpy(buff+(i-line_segment_start), "\0", 1);
			if (state) {
				text =TTF_RenderUTF8_Shaded(font, buff, image->backgroundColor, image->color);
			} else {
				text = TTF_RenderUTF8_Blended(font, buff, image->color);
			}

			if (CtrMediaEdCache[q].surface) {
				SDL_FreeSurface(CtrMediaEdCache[q].surface);
				free(CtrMediaEdCache[q].text);
			}
			CtrMediaEdCache[q].surface = text;
			CtrMediaEdCache[q].text = buff;
			CtrMediaEdCache[q].state = state;
			}

			SDL_BlitSurface(text,NULL,dst,&t);
			TTF_SizeUTF8(font, buff, &text_width, &text_height);
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
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 *
 * media on: ‘start’ do: {
 * 
 * a
 * x: 0 y: 200,
 * font: ‘font.ttf’ size: 16,
 * write: ‘ABC’.
 *
 * }.
 * 
 * media screen: ‘canvas.png’.
 * 
 * @result
 * en: Writes text on an image.
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
 * [ Image ] draw: [ Sequence ] colour: [ Colour ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ canvas ≔ Image new: ‘canvas.png’.
 * media on: ‘start’ do: {
 *
 * ☞ a ≔ Point new x: 10 y: 10.
 * ☞ b ≔ Point new x: 20 y: 20.
 * ☞ c ≔ Line new from: a to: b.
 * ☞ d ≔ Point new x: 15 y: 5.
 * ☞ all ≔ Sequence ← c ; d. 
 * ☞ x ≔ Colour new red: 255 green: 0 blue:  0.
 * canvas 
 * x: 0 y: 0,
 * draw: all colour: x.
 * }.
 * media screen: ‘canvas.png’.
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
		texTarget = SDL_CreateTexture(CtrMediaRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, image->w, image->h);
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
 * ☞ media ≔ Media new.
 * ☞ a ≔ Image new: ‘a.png’.
 * a font: ‘font.ttf’ size: 10.
 * a write: ‘abc’.
 * ✎ write: a text, stop.
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
 * ☞ media ≔ Media new.
 * Media clipboard: ‘abc’.
 * ✎ write: Media clipboard.
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
 * ☞ media ≔ Media new.
 * Media clipboard: ‘abc’.
 * ✎ write: Media clipboard.
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
 * [ Media ] select.
 * 
 * @example
 * Media select.
 * 
 * @result
 * en: Returns selected text in an editable image.
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

/**
 * @def
 * [ Media ] digraph: [String] ligature: [String].
 * 
 * @example
 * Media digraph: ‘:=’ ligature: ‘≔’.
 * 
 * @result
 * en: Converts digraphs to ligatures real-time / while typing.
 */
ctr_object* ctr_media_autoreplace(ctr_object* myself, ctr_argument* argumentList) {
	CtrMediaAutoReplaceRule* rule;
	if (CtrMediaAutoReplaceRuleLen<100) {
		rule = &CtrMediaAutoReplaceRules[CtrMediaAutoReplaceRuleLen++];
		rule->word = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
		rule->replacement = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->next->object));
	}
	return myself;
}

void ctr_internal_media_init() {
	CtrMediaContactSurface = NULL;
	CtrMediaAssetPackage = NULL;
	CtrMediaAudioRate = MIX_DEFAULT_FREQUENCY;
	CtrMediaAudioFormat = MIX_DEFAULT_FORMAT;
	CtrMediaAudioChannels = MIX_DEFAULT_CHANNELS;
	CtrMediaAudioBuffers = 4096;
	CtrMediaAudioVolume = MIX_MAX_VOLUME;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) ctr_internal_media_fatalerror("SDL failed to init", SDL_GetError());
	IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
	#ifdef FULLSCREEN
	CtrMediaWindow = SDL_CreateWindow("Citrine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 100, 100, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
	#else
	CtrMediaWindow = SDL_CreateWindow("Citrine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 100, 100, SDL_WINDOW_OPENGL);
	#endif
	if (CtrMediaWindow == NULL) ctr_internal_media_fatalerror("Unable to create window", SDL_GetError());
	CtrMediaRenderer = SDL_CreateRenderer(CtrMediaWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
	if (!CtrMediaRenderer) {
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

/**
 * @def
 * Package
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Package new: ‘assets.dat’.
 * ✎ write: a, stop.
 */
ctr_object* ctr_package_new_set(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_package_new(myself, argumentList);
	ctr_internal_object_property(instance, "path", ctr_internal_copy2string(argumentList->object));
	return instance;
}

/**
 * @def
 * [ Media ] link: [ Package ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Package new: ‘assets.dat’.
 * media link: a.
 * ☞ b ≔ Image new: ‘a.png’.
 * 
 * @result
 * en: After linking an asset package, all resources will be retrieved from the package instead of disk.
 */
ctr_object* ctr_media_link_package(ctr_object* myself, ctr_argument* argumentList) {
	if (argumentList->object->link != packageObject) {
		ctr_error("Not an asset package.\n", 0);
	}
	CtrMediaAssetPackage = argumentList->object;
	return myself;
}

/**
 * @def
 * [ Package ] add: [ Object ]
 * 
 * @example
 * ☞ media ≔ Media new.
 * ☞ a ≔ Package new: ‘assets.dat’.
 * ☞ b ≔ Image new: ‘a.png’.
 * a add: b.
 * 
 * @result
 * en: Adds a resource to a package.
 */
ctr_object* ctr_package_add(ctr_object* myself, ctr_argument* argumentList) {
	char* path = ctr_heap_allocate_cstring(ctr_internal_object_property(myself, "path", NULL));
	FILE* outfile;
	if (access(path, F_OK) == 0) {
		outfile = fopen(path, "rb+");
	} else {
		outfile = fopen(path, "wb+");
	}
	fseek(outfile, 0, SEEK_END);
	int CHUNK_SIZE = 10000;
	uint64_t next_entry_at;
	int bytes_read;
	char* buffer;
	ctr_object* fileObject = argumentList->object;
	if (fileObject->link == CtrStdFile) {
		char* path = ctr_heap_allocate_cstring(
			ctr_internal_cast2string(
				ctr_internal_object_property(fileObject, "path", NULL)
			)
		);
		fwrite(path, 1, strlen(path), outfile);
		fwrite("\0", 1 , 1, outfile);
		next_entry_at = ftell(outfile);
		fwrite("\0\0\0\0\0\0\0\0", 8, 1, outfile);
		FILE* f = fopen(path, "rb");
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
	} else {
		ctr_error("Invalid argument\n", 0);
	}
	ctr_heap_free(path);
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

/**
 * @def
 * [ Media ] website: [ Text ]
 * 
 * @example
 * Media website: ‘https://citrine-lang.org’.
 * 
 * @result
 * en: Opens website in browser specified in BROWSER environment variable.
 */
ctr_object* ctr_media_website(ctr_object* myself, ctr_argument* argumentList) {
	char* default_url_opener;
	char* tpl;
	#ifdef WIN
	default_url_opener = "explorer"; //Win
	tpl = "%s %s";
	#else
	default_url_opener = "xdg-open"; //Linuxy
	tpl = NULL;
	#endif
	return ctr_internal_media_external_command(
		getenv("BROWSER"),
		default_url_opener,
		ctr_tostr(argumentList->object),
		tpl
	);
}

/**
 * @def
 * [ Media ] say: [ Text ]
 * 
 * @example
 * Media say: ‘Hello’.
 * 
 * @result
 * en: Speaks text using speech synthesizer system specified in SPEAK environment variable.
 */
ctr_object* ctr_media_speak(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_media_external_command(
		getenv("SPEAK"),
		"say",
		ctr_tostr(argumentList->object),
		NULL
	);
}

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
		ctr_tostr(ctr_internal_copy2string(argumentList->object)),
		NULL
	);
	//If it fails, start without terminal
	if (result == CtrStdBoolFalse) {
		result = ctr_internal_media_external_command(
		NULL,
		"",
		ctr_tostr(ctr_internal_copy2string(argumentList->object)),
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
    if( !CreateProcess(NULL,ctr_tostr(ctr_internal_copy2string(argumentList->object)),NULL,NULL,FALSE,0,NULL,NULL,&si,&pi)) return CtrStdBoolFalse;
	WaitForSingleObject( pi.hProcess, INFINITE );
	CloseHandle( pi.hProcess );
	CloseHandle( pi.hThread );
	result = CtrStdBoolTrue;
	#endif
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

void begin(){
	#ifdef WIN
	FreeConsole();
	#endif
	ctr_internal_media_reset();
	ctr_internal_media_init();
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
	ctr_internal_create_func(pointObject, ctr_build_string_from_cstring( CTR_DICT_XY_SET), &ctr_point_xyset );
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
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( "width:height:" ), &ctr_media_width_height );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( "left:top:" ), &ctr_media_left_top );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_CLIPBOARD ), &ctr_media_clipboard );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_CLIPBOARD_SET ), &ctr_media_clipboard_set );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_RUN ), &ctr_media_override );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_ON_STEP ), &ctr_media_override );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_SELECTION ), &ctr_media_select );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_DIGRAPH_LIGATURE_SET ), &ctr_media_autoreplace );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_SAY_SET ), &ctr_media_speak );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_LINK_SET ), &ctr_media_link_package );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_TIMER_SET ), &ctr_media_timer );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_WEBSITE_SET ), &ctr_media_website );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_ONDO ), &ctr_media_on_do );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_END ), &ctr_media_end );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( "sys:" ), &ctr_media_system );
	#ifdef WIN
	ctr_internal_create_func(CtrStdConsole, ctr_build_string_from_cstring(CTR_DICT_WRITE), &ctr_media_console_write );
	ctr_internal_create_func(CtrStdConsole, ctr_build_string_from_cstring( CTR_DICT_STOP ), &ctr_media_console_brk );
	#endif
	imageObject = ctr_img_new(CtrStdObject, NULL);
	imageObject->link = CtrStdObject;
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_img_new );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_img_new_set );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_IMAGE_SET ), &ctr_img_img );
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
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_ANIMATIONS_SET ), &ctr_img_anims );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_WRITE ), &ctr_img_text );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_TOSTRING ), &ctr_img_text_get );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_CUT ), &ctr_img_text_del );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_APPEND ), &ctr_img_text_ins );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_EDITABLE_SET ), &ctr_img_editable );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_FONT_TYPE_SIZE_SET ), &ctr_img_font );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_COLOR_SET ), &ctr_img_color );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_BACKGROUND_COLOR_SET ), &ctr_img_background_color );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_ALIGN_XY_SET ), &ctr_img_text_align );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_DRAW_COLOR_SET ), &ctr_img_draw );
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
	ctr_internal_create_func(networkObject, ctr_build_string_from_cstring(CTR_DICT_SEND_TEXT_MESSAGE), &ctr_network_basic_text_send );
	ctr_internal_create_func(networkObject, ctr_build_string_from_cstring(CTR_DICT_FETCH_TEXT_MESSAGES), &ctr_network_basic_text_receive );
	packageObject = ctr_package_new(CtrStdObject, NULL);
	packageObject->link = CtrStdObject;
	ctr_internal_create_func(packageObject, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_package_new_set );
	ctr_internal_create_func(packageObject, ctr_build_string_from_cstring(CTR_DICT_APPEND), &ctr_package_add );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_PLUGIN_ID ), mediaObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_IMAGE_OBJECT ), imageObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_COLOR_OBJECT ), colorObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_POINT_OBJECT ), pointObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_LINE_OBJECT ), lineObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_AUDIO_OBJECT ), audioObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_SOUND_OBJECT ), soundObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MUSIC_OBJECT ), musicObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_NETWORK_PORT ), ctr_build_string_from_cstring("MediaNetPort1"), CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_NETWORK_OBJECT), networkObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_PACKAGE_OBJECT ), packageObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_QUOTES ), ctr_build_string_from_cstring(CTR_DICT_QUOT_OPEN CTR_DICT_QUOT_CLOSE), CTR_CATEGORY_PUBLIC_PROPERTY);
	/* Untranslated reference for systems that do not support UTF-8 characters in file names (like Windows) */
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Media" ), mediaObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}

void init_embedded_media_plugin() {
	begin();
}