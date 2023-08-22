#ifdef WINDOWS_MEDIA_SOCK
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0501 //Windows XP
#include <winsock2.h>
#endif


#include "../../citrine.h"
#include "media.h"

#define PL_MPEG_IMPLEMENTATION
#include "pl_mpeg.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
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

#ifndef NO_MEDIA_ESPEAK
#include <espeak/speak_lib.h>
#endif



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
/*
AVFormatContext* CtrMediaVideoFormatCtx;
const AVCodec* CtrMediaBGVideoCodec;
AVCodecContext* CtrMediaBGVideoCdcCtx;
*/
int CtrMediaVideoId = -1;
double CtrMediaVideoFPSRendering;
/*
AVCodecParameters* CtrMediaVideoParams;
AVFrame* CtrMediaVideoFrame;
AVPacket* CtrMediaVideoPacket;
*/
SDL_Texture* CtrMediaBGVideoTexture;

int CtrMediaAudioRate;
uint16_t CtrMediaAudioFormat;
int CtrMediaAudioChannels;
int CtrMediaAudioBuffers;
int CtrMediaAudioVolume;
ctr_object* CtrMediaAssetPackage;

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
	double			speed;		int				dir;
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

int CtrMediaTimers[100];
int CtrMaxMediaTimers = 100;

void ctr_internal_media_fatalerror(char* msg, const char* info)	{ fprintf(stderr,"Media Plugin FATAL ERROR: %s (%s) \n", msg, info); SDL_Quit(); exit(1); }
MediaIMG* ctr_internal_media_getfocusimage()						{ return (MediaIMG*) focusObject->value.rvalue->ptr; }
MediaIMG* ctr_internal_media_getplayer()							{ return (controllableObject == NULL) ? NULL : (MediaIMG*) controllableObject->value.rvalue->ptr; }
MediaIMG* ctr_internal_get_image_from_object(ctr_object* object)	{ return (MediaIMG*) object->value.rvalue->ptr; }
MediaAUD* ctr_internal_get_audio_from_object(ctr_object* object)   { return (MediaAUD*) object->value.rvalue->ptr; }
char ctr_internal_media_has_selection()								{ return (CtrMediaSelectBegin != CtrMediaSelectEnd); }

void ctr_internal_media_reset() {
	controllableObject = NULL;
	focusObject = NULL;
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
	while(i > 0 && 
		haystack->text[i]!='\n' &&
		haystack->text[i]!='\r' &&
		haystack->text[i]!=' '
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
		memcpy(mediaImage->text+oldPos, mediaImage->text+CtrMediaInputIndex,mediaImage->textlength);
		mediaImage->text[mediaImage->textlength]='\0';
		mediaImage->textlength -= (CtrMediaInputIndex - oldPos);
		CtrMediaInputIndex = oldPos;
		return;
	}
	mediaImage->textlength = mediaImage->textlength + insertTextLength;
	if (mediaImage->textlength+1 > mediaImage->textbuffer) {
		mediaImage->textbuffer = (mediaImage->textbuffer) ? mediaImage->textbuffer * 2 : 1;
		mediaImage->text = ctr_heap_reallocate(mediaImage->text, mediaImage->textbuffer + 1);
	}
	memcpy(mediaImage->text+CtrMediaInputIndex+insertTextLength, mediaImage->text+CtrMediaInputIndex,mediaImage->textlength-CtrMediaInputIndex-insertTextLength);
	memcpy(mediaImage->text+CtrMediaInputIndex,text,insertTextLength);
	mediaImage->text[mediaImage->textlength + 1] = '\0';
	CtrMediaInputIndex += insertTextLength;
}

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
			ctr_send_message(mediaIMGs[i].ref, CTR_DICT_MEDIA_IMAGE_ON_CLICK_SET, strlen(CTR_DICT_MEDIA_IMAGE_ON_CLICK_SET), args);
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

MediaIMG* CtrMediaContactSurface = NULL;
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
					m->dir = (m->dir + 270) % 360;
				}
			}
			if (m->collidable) {
				collider->object = m2->ref;
				ctr_send_message(m->ref, CTR_DICT_MEDIA_IMAGE_COLLISION_SET, strlen(CTR_DICT_MEDIA_IMAGE_COLLISION_SET), collider );
			}
			if (m2->collidable) {
				collider->object = m->ref;
				ctr_send_message(m2->ref, CTR_DICT_MEDIA_IMAGE_COLLISION_SET, strlen(CTR_DICT_MEDIA_IMAGE_COLLISION_SET), collider );
			}
		}
	}
	ctr_heap_free(collider);
}

void ctr_internal_media_render_image(MediaIMG* m, SDL_Rect r, SDL_Rect s) {
	ctr_internal_media_anim_frames(m, &r, &s);
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
	if (m->mov < m->speed && controllableObject && m != controllableObject->value.rvalue->ptr) {
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
			ctr_send_message(m->ref, CTR_DICT_MEDIA_IMAGE_DESTINATION_SET, strlen(CTR_DICT_MEDIA_IMAGE_DESTINATION_SET), a );
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
FILE* vf;

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


void ctr_internal_media_loadvideobg(char* path, SDL_Rect* dimensions) {
	vf = fopen(path, "rb");
	plm = plm_create_with_file(vf, FALSE);
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
	return;
}

char ctr_internal_media_determine_filetype(char* path) {
	char magic[20];
	if (CtrMediaAssetPackage) {
		SDL_RWops* asset_reader = ctr_internal_media_load_asset(path, 1);
		asset_reader->read(asset_reader, magic, 20, 1);
	} else {
		FILE* fh = fopen(path, "rb");
		fread(magic, 20, 1, fh);
		fclose(fh);
	}
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
	SDL_StartTextInput();
	SDL_Rect dimensions;
	SDL_Texture* texture;
	char* imageFileStr = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	char ftype = ctr_internal_media_determine_filetype(imageFileStr);
	char background_is_video;
	if (ftype == 10) {
		ctr_internal_media_loadvideobg(imageFileStr, &dimensions);
		background_is_video = 1;
	} else {
		texture = IMG_LoadTexture(CtrMediaRenderer, imageFileStr);
		SDL_QueryTexture(texture, NULL, NULL, &dimensions.w, &dimensions.h);
		dimensions.x = x;
		dimensions.y = y;
		SDL_SetWindowSize(CtrMediaWindow, dimensions.w, dimensions.h);
		SDL_Delay(100);
		SDL_RenderCopy(CtrMediaRenderer, texture, NULL, &dimensions);
		background_is_video = 0;
	}
	windowWidth = dimensions.w;
	windowHeight = dimensions.h;
	ctr_send_message(myself, CTR_DICT_MEDIA_MEDIA_ON_START, strlen(CTR_DICT_MEDIA_MEDIA_ON_START), NULL );
	if (CtrStdFlow) return myself;
	SDL_Event event;
	dir = -1;
	c4speed = 0;
	while (1) {
		ctr_gc_cycle(); 
		SDL_RenderClear(CtrMediaRenderer);
		if (background_is_video) {
			ctr_internal_media_rendervideoframe(&dimensions);
		} else {
			SDL_RenderCopy(CtrMediaRenderer, texture, NULL, &dimensions);
		}
		myself->info.sticky = 1;
		if (CtrMediaEventListenFlagTimer) {
			for(int i = 1; i <= CtrMaxMediaTimers; i++) {
				if (CtrMediaTimers[i] < 0) continue;
				if (CtrMediaTimers[i] == 0) {
					ctr_media_event_timer(myself, CTR_DICT_MEDIA_MEDIA_TIMER, i);
				}
				CtrMediaTimers[i]--;
			}
		}
		if (CtrMediaEventListenFlagStep) {
			ctr_send_message(myself, CTR_DICT_MEDIA_MEDIA_ON_STEP, strlen(CTR_DICT_MEDIA_MEDIA_ON_STEP), NULL );
		}
		myself->info.sticky = 0;
		while (SDL_PollEvent(&event)) {
			if (CtrMediaBreakLoopFlag) {
				ctr_internal_media_reset();
				return myself;
			}
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
						ctr_media_event_coords(myself, CTR_DICT_MEDIA_MEDIA_MOUSE_CLICK, event.button.x, event.button.y);
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (ctr_internal_media_mouse_down(event)) return CtrStdFlow;
					break;
				case SDL_CONTROLLERBUTTONDOWN:
					if (CtrMediaEventListenFlagGamePadBtnDown) {
						ctr_media_event(myself, CTR_DICT_MEDIA_MEDIA_GAMEPAD_DOWN, SDL_GameControllerGetStringForButton(event.cbutton.button));
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
						ctr_media_event(myself, CTR_DICT_MEDIA_MEDIA_GAMEPAD_UP, SDL_GameControllerGetStringForButton(event.cbutton.button));
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
						ctr_media_event(myself, CTR_DICT_MEDIA_MEDIA_KEY, SDL_GetKeyName(event.key.keysym.sym));
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
						ctr_media_event(myself, CTR_DICT_MEDIA_MEDIA_KEY_DOWN, SDL_GetKeyName(event.key.keysym.sym));
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
			ctr_internal_media_render_image(m,r,s);
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

ctr_object* ctr_point_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	instance->link = myself;
	ctr_internal_object_property(instance, "x", ctr_build_number_from_float(0));
	ctr_internal_object_property(instance, "y", ctr_build_number_from_float(0));
	return instance;
}

ctr_object* ctr_point_x(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(myself, "x", NULL);
}

ctr_object* ctr_point_y(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(myself, "y", NULL);
}

ctr_object* ctr_point_xyset(ctr_object* myself, ctr_argument* argumentList) {
	ctr_internal_object_property(myself, "x", argumentList->object);
	ctr_internal_object_property(myself, "y", argumentList->next->object);
	return myself;
}

ctr_object* ctr_line_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	instance->link = myself;
	ctr_internal_object_property(instance, CTR_DICT_MEDIA_LINE_POINT_START, CtrStdNil);
	ctr_internal_object_property(instance, CTR_DICT_MEDIA_LINE_POINT_END, CtrStdNil);
	return instance;
}

ctr_object* ctr_line_from_to(ctr_object* myself, ctr_argument* argumentList) {
	ctr_internal_object_property(myself, CTR_DICT_MEDIA_LINE_POINT_START, argumentList->object);
	ctr_internal_object_property(myself, CTR_DICT_MEDIA_LINE_POINT_END, argumentList->next->object);
	return myself;
}

ctr_object* ctr_line_start(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(myself, CTR_DICT_MEDIA_LINE_POINT_START, NULL);
}

ctr_object* ctr_line_end(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_internal_object_property(myself, CTR_DICT_MEDIA_LINE_POINT_END, NULL);
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

ctr_object* ctr_sound_new_set(ctr_object* myself, ctr_argument* argumentList) {
	char* audioFileStr = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	ctr_object* audioInst = ctr_audio_new(myself, argumentList);
	MediaAUD* mediaAUD = &mediaAUDs[AUDCount];
	ctr_resource* rs = ctr_heap_allocate( sizeof(ctr_resource) );
	audioInst->value.rvalue = rs;
	rs->ptr = mediaAUD;
	if (mediaAUD->blob != NULL) {
		Mix_FreeChunk((Mix_Chunk*)mediaAUD->blob);
	}
	mediaAUD->blob = (void*) Mix_LoadWAV(audioFileStr);
	if (mediaAUD->blob == NULL) {
		CtrStdFlow = ctr_build_string_from_cstring((char*)SDL_GetError());
	}
	rs->destructor = &ctr_audio_destructor;
	AUDCount++;
	return audioInst;
}

ctr_object* ctr_sound_play(ctr_object* myself, ctr_argument* argumentList) {
	MediaAUD* mediaAUD = ctr_internal_get_audio_from_object(myself);
	if (mediaAUD->blob != NULL) {
		Mix_PlayChannel(0, (Mix_Chunk*) mediaAUD->blob, 0);
	}
	return myself;
}


SDL_RWops* ctr_internal_media_load_asset(char* asset_name, char asset_type) {
	if (CtrMediaAssetPackage == NULL) {
		return NULL;
	}
	SDL_RWops* res = NULL;
	char* path = ctr_heap_allocate_cstring(ctr_internal_object_property(CtrMediaAssetPackage, "path", NULL));
	FILE* asset_file = fopen(path, "rb");
	if (!asset_file) return NULL;
	fseek(asset_file, 0, SEEK_SET);
	char* buffer = malloc(500);
	while(1) {
		uint64_t read_start = ftell(asset_file);
		int bytes_read = fread(buffer, 1, 500, asset_file);
		if (strncmp(asset_name, buffer, bytes_read) == 0) {
			fseek(asset_file, read_start + strlen(asset_name) + 1, SEEK_SET);
			uint64_t next_entry = 0;
			fread(&next_entry, 8, 1, asset_file);
			uint64_t curpos = ftell(asset_file);
			uint64_t read_size = next_entry - curpos;
			char* read_buffer = malloc(read_size);
			fread(read_buffer, 1, read_size, asset_file);
			res = SDL_RWFromMem(read_buffer, read_size);
			break;
		} else {
			char* boundary = strchr(buffer, 0);
			uint64_t jmp_address = *((uint64_t*) (boundary+1));
			if (jmp_address == 0) {
				break;
			}
			fseek(asset_file, jmp_address, SEEK_SET);
		}
	}
	fclose(asset_file);
	return res;
}
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
	} else {
		mediaAUD->blob = (void*)Mix_LoadMUS(audioFileStr);
	}
	if (mediaAUD->blob == NULL) {
		CtrStdFlow = ctr_build_string_from_cstring((char*)SDL_GetError());
	}
	rs->destructor = &ctr_audio_destructor;
	AUDCount++;
	return audioInst;
}


ctr_object* ctr_music_play(ctr_object* myself, ctr_argument* argumentList) {
	MediaAUD* mediaAUD = ctr_internal_get_audio_from_object(myself);
	if (mediaAUD->blob != NULL) {
			Mix_FadeInMusic((Mix_Music*)mediaAUD->blob,-1,0);
	}
	return myself;
}

ctr_object* ctr_music_silence(ctr_object* myself, ctr_argument* argumentList) {
	MediaAUD* mediaAUD = ctr_internal_get_audio_from_object(myself);
	if (mediaAUD->blob != NULL) {
			Mix_PauseMusic();
	}
	return myself;
}

ctr_object* ctr_music_rewind(ctr_object* myself, ctr_argument* argumentList) {
	MediaAUD* mediaAUD = ctr_internal_get_audio_from_object(myself);
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

ctr_object* ctr_network_basic_text_send(ctr_object* myself, ctr_argument* argumentList) {
	char* data = argumentList->object->value.svalue->value;
	uint64_t total_size = argumentList->object->value.svalue->vlen;
	int bytes_sent;
	int bytes_received = -1;
	uint16_t recipient_port = 0;
	char ip_str_recipient[40];
	char ip_str[40];
	double timeout = 10;
	double interval = 0.0001;
	uint16_t chunk_size = CtrMediaNetworkChunkSize;
	uint16_t port = 9000;
	uint64_t chunks = ceil((double)total_size / chunk_size);
	char* ip_str_and_port = ctr_heap_allocate_cstring(argumentList->next->object);
	char* colon_pos = strstr(ip_str_and_port,":");
	if (colon_pos == NULL) {
		strcpy(ip_str, ip_str_and_port);
	} else {
		memcpy(ip_str, ip_str_and_port, colon_pos-ip_str_and_port);
		ip_str[colon_pos-ip_str_and_port]='\0';
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

ctr_object* ctr_img_new(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* instance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTEX);
	instance->link = myself;
	return instance;
}

void ctr_img_destructor(ctr_resource* rs) {
	MediaIMG* image = (MediaIMG*) rs->ptr;
	image->ref = NULL;
}




ctr_object* ctr_img_img(ctr_object* myself, ctr_argument* argumentList) {
	SDL_Rect dimensions;
	dimensions.x = 0;
	dimensions.y = 0;
	char* imageFileStr = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	ctr_object* imageInst = myself;
	MediaIMG* mediaImage = &mediaIMGs[IMGCount]; 
	ctr_resource* rs = ctr_heap_allocate( sizeof(ctr_resource) );
	rs->ptr = mediaImage;
	rs->destructor = &ctr_img_destructor;
	imageInst->value.rvalue = rs;
	SDL_RWops* res;
	res = ctr_internal_media_load_asset(imageFileStr, 1);
	if (res) {
		mediaImage->texture = (void*) IMG_LoadTexture_RW(CtrMediaRenderer, res,0);
		res = ctr_internal_media_load_asset(imageFileStr, 1);
		mediaImage->surface = (void*) IMG_Load_RW(res, 0);
	} else {
		mediaImage->texture = (void*) IMG_LoadTexture(CtrMediaRenderer, imageFileStr);
		mediaImage->surface = (void*) IMG_Load(imageFileStr);
	}
	if (mediaImage->texture == NULL) ctr_internal_media_fatalerror("Unable to load texture", imageFileStr);
	if (mediaImage->surface == NULL) ctr_internal_media_fatalerror("Unable to load surface", imageFileStr);
	ctr_heap_free(imageFileStr);
	SDL_QueryTexture(mediaImage->texture, NULL, NULL, &dimensions.w, &dimensions.h);
	mediaImage->h = dimensions.h;
	mediaImage->w = dimensions.w;
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
	mediaImage->ref = imageInst;
	SDL_RenderCopy(CtrMediaRenderer, mediaImage->texture, NULL, &dimensions);
	SDL_RenderPresent(CtrMediaRenderer);
	IMGCount ++;
	return imageInst;
}

ctr_object* ctr_img_new_set(ctr_object* myself, ctr_argument* argumentList) {
	return ctr_img_img(ctr_img_new(myself, argumentList), argumentList);
}

ctr_object* ctr_img_controllable(ctr_object* myself, ctr_argument* argumentList) {
	controllableObject = myself;
	CtrMediaControlMode = (int) ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}

ctr_object* ctr_img_xy(ctr_object* myself, ctr_argument* argumentList) {
	int x = (int) ctr_internal_cast2number(argumentList->object)->value.nvalue;
	int y = (int) ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	image->ox = x;
	image->oy = y;
	image->x = x;
	image->y = y;
	return myself;
}

ctr_object* ctr_img_x(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	return ctr_build_number_from_float(image->x);
}

ctr_object* ctr_img_y(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	return ctr_build_number_from_float(image->y);
}

ctr_object* ctr_img_mov_set(ctr_object* myself, ctr_argument* argumentList) {
	double x = (int) ctr_internal_cast2number(argumentList->object)->value.nvalue;
	double y = (int) ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	double delta_x = x - image->x;
	double delta_y = y - image->y;
	double rad = atan2(-1 * delta_y, delta_x);
	int deg = rad * (180 / M_PI);
	image->dir = (deg < 0) ? 360 + deg : deg;
	image->tx = x;
	image->ty = y;
	return myself;
}

ctr_object* ctr_img_bounce(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	image->bounce = (int) ctr_internal_cast2bool(argumentList->object)->value.bvalue;
	return myself;
}

ctr_object* ctr_img_solid(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	image->solid = (int) ctr_internal_cast2bool(argumentList->object)->value.bvalue;
	return myself;
}

ctr_object* ctr_img_active(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	image->collidable = (int) ctr_internal_cast2bool(argumentList->object)->value.bvalue;
	return myself;
}

ctr_object* ctr_img_gravity(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	image->gravity = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}

ctr_object* ctr_img_speed(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	image->speed = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}
ctr_object* ctr_img_friction(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	image->fric = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}

ctr_object* ctr_img_accel(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	image->accel = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}

ctr_object* ctr_img_jump_height(ctr_object* myself, ctr_argument* argumentList) {
	CtrMediaJumpHeightFactor = ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}

ctr_object* ctr_img_editable(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	image->editable = ctr_internal_cast2bool(argumentList->object)->value.bvalue;
	return myself;
}

ctr_object* ctr_img_anims(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	image->anims = (int) ctr_internal_cast2number(argumentList->object)->value.nvalue;
	return myself;
}

ctr_object* ctr_img_font(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	image->font = TTF_OpenFont(ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object)), (int)ctr_internal_cast2number(argumentList->next->object)->value.nvalue);
	if (image->font == NULL) ctr_internal_media_fatalerror("Unable to load font", "TTF Font");
	return myself;
}

ctr_object* ctr_img_color(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	uint8_t r = (uint8_t)ctr_color_r(argumentList->object, NULL)->value.nvalue;
	uint8_t g = (uint8_t)ctr_color_g(argumentList->object, NULL)->value.nvalue;
	uint8_t b = (uint8_t)ctr_color_b(argumentList->object, NULL)->value.nvalue;
	uint8_t a = (uint8_t)ctr_color_a(argumentList->object, NULL)->value.nvalue;
	image->color = (SDL_Color) { r, g, b, a };
	return myself;
}

ctr_object* ctr_img_background_color(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	uint8_t r = (uint8_t)ctr_color_r(argumentList->object, NULL)->value.nvalue;
	uint8_t g = (uint8_t)ctr_color_g(argumentList->object, NULL)->value.nvalue;
	uint8_t b = (uint8_t)ctr_color_b(argumentList->object, NULL)->value.nvalue;
	uint8_t a = (uint8_t)ctr_color_a(argumentList->object, NULL)->value.nvalue;
	image->backgroundColor = (SDL_Color) { r, g, b, a };
	return myself;
}

ctr_object* ctr_img_text_align(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	image->paddingx = (int)ctr_internal_cast2number(argumentList->object)->value.nvalue;
	image->paddingy = (int)ctr_internal_cast2number(argumentList->next->object)->value.nvalue;
	return myself;
}

void ctr_internal_img_render_cursor(ctr_object* focusObject) {
	if (!focusObject) return;
	MediaIMG* image = (MediaIMG*) focusObject->value.rvalue->ptr;
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
		SDL_SetRenderDrawColor(CtrMediaRenderer, 255, 255, 255, 255);
		SDL_RenderDrawLine(CtrMediaRenderer, image->x + (offsetx), image->y + (y1*height), image->x + (offsetx), image->y + ((y1*height)+height));
	}
	CtrMediaCursorLine = y1;
}

void ctr_internal_img_render_text(ctr_object* myself) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
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
			if (i-line_segment_start > buffsize) {
				buffsize = i-line_segment_start;
				buff = realloc(buff, buffsize + 1);
			}
			memcpy(buff, image->text+line_segment_start, i-line_segment_start);
			memcpy(buff+(i-line_segment_start), "\0", 1);
			if (state) {
				text =TTF_RenderUTF8_Shaded(font, buff, image->backgroundColor, image->color);
			} else {
				text = TTF_RenderUTF8_Blended(font, buff, image->color);
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
}

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

ctr_object* ctr_img_text(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	ctr_object* textObject = ctr_internal_cast2string(argumentList->object);
	if (image->text != NULL) {
		ctr_heap_free(image->text);
	} 
	image->text = ctr_internal_media_normalize_line_endings(ctr_heap_allocate_cstring(textObject));
	image->textlength = strlen(image->text);
	image->textbuffer = image->textlength;
	ctr_internal_img_render_text(myself);
	return myself;
}

ctr_object* ctr_img_draw(ctr_object* myself, ctr_argument* argumentList) {
	ctr_argument* arg;
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
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
			subshape1 = ctr_internal_object_property(shape, CTR_DICT_MEDIA_LINE_POINT_START, NULL);
			subshape2 = ctr_internal_object_property(shape, CTR_DICT_MEDIA_LINE_POINT_END, NULL);
			SDL_RenderDrawLine(CtrMediaRenderer,
				(int) ctr_tonum(ctr_internal_object_property(subshape1, "x", NULL)),
				(int) ctr_tonum(ctr_internal_object_property(subshape1, "y", NULL)),
				(int) ctr_tonum(ctr_internal_object_property(subshape2, "x", NULL)),
				(int) ctr_tonum(ctr_internal_object_property(subshape2, "y", NULL))
			);
		}
		i++;
	}
	SDL_SetRenderTarget(CtrMediaRenderer, NULL);
	return myself;
}


ctr_object* ctr_media_override(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}

ctr_object* ctr_img_text_get(ctr_object* myself, ctr_argument* argumentList) {
	MediaIMG* image = (MediaIMG*) myself->value.rvalue->ptr;
	if (image->text == NULL) {
		return ctr_build_empty_string();
	}
	return ctr_build_string_from_cstring(image->text);
}

ctr_object* ctr_media_clipboard(ctr_object* myself, ctr_argument* argumentList) {
	ctr_object* text;
	char* buffer;
	buffer = SDL_GetClipboardText();
	if (buffer != NULL) {
		text = ctr_build_string_from_cstring(buffer);
		SDL_free(buffer);
	} else {
		text = ctr_build_empty_string();
	}
	return text;
}

ctr_object* ctr_media_clipboard_set(ctr_object* myself, ctr_argument* argumentList) {
	char* buffer;
	buffer = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	SDL_SetClipboardText(buffer);
	return myself;
}

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
	CtrMediaAssetPackage = NULL;
	CtrMediaAudioRate = MIX_DEFAULT_FREQUENCY;
	CtrMediaAudioFormat = MIX_DEFAULT_FORMAT;
	CtrMediaAudioChannels = MIX_DEFAULT_CHANNELS;
	CtrMediaAudioBuffers = 4096;
	CtrMediaAudioVolume = MIX_MAX_VOLUME;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) ctr_internal_media_fatalerror("SDL failed to init", SDL_GetError());
	IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
	CtrMediaWindow = SDL_CreateWindow("Citrine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 100, 100, SDL_WINDOW_OPENGL);
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


char CtrMediaAudioVoiceInit = 0;


#ifndef NO_MEDIA_ESPEAK
ctr_object* ctr_media_speak(ctr_object* myself, ctr_argument* argumentList) {
	char* text = ctr_heap_allocate_cstring(ctr_internal_cast2string(argumentList->object));
	ctr_object* spoken = CtrStdBoolFalse;
	espeak_AUDIO_OUTPUT output;
	output = AUDIO_OUTPUT_PLAYBACK;
	if (!CtrMediaAudioVoiceInit) {
		int result = espeak_Initialize(output, 500, NULL, 0);
		if (result == -1) {
			ctr_error("Unable to init speech synth: %s.\n", errno);
		} else {
			CtrMediaAudioVoiceInit = 1;
		}
	}
	if (CtrMediaAudioVoiceInit) {
		espeak_VOICE voice;
		voice.languages = (const char*) CTR_DICT_MEDIA_AUDIO_VOICE_LANG_CODE;
		voice.name = "media";
		voice.variant = 2;
		voice.gender = 1;
		espeak_SetVoiceByProperties(&voice);
		unsigned int* uid = 0;
		void* callback = NULL;
		espeak_Synth( text, argumentList->object->value.svalue->vlen+1, 0, 0, 0, espeakCHARS_UTF8, uid, callback );
		espeak_Synchronize();
		spoken = CtrStdBoolTrue;
	}
	return spoken;
}
#else
ctr_object* ctr_media_speak(ctr_object* myself, ctr_argument* argumentList) {
	return myself;
}
#endif



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

ctr_object* ctr_media_link_package(ctr_object* myself, ctr_argument* argumentList) {
	if (argumentList->object->link != packageObject) {
		ctr_error("Not an asset package.\n", 0);
	}
	CtrMediaAssetPackage = argumentList->object;
	return myself;
}

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
	if (strcmp(CTR_DICT_MEDIA_MEDIA_KEY, event_name)==0) CtrMediaEventListenFlagKeyUp = (listener != CtrStdNil);
	else if (strcmp(CTR_DICT_MEDIA_MEDIA_KEY_DOWN, event_name)==0) CtrMediaEventListenFlagKeyDown = (listener != CtrStdNil);
	else if (strcmp(CTR_DICT_MEDIA_MEDIA_MOUSE_CLICK, event_name)==0) CtrMediaEventListenFlagMouseClick = (listener != CtrStdNil);
	else if (strcmp(CTR_DICT_MEDIA_MEDIA_GAMEPAD_DOWN, event_name)==0) CtrMediaEventListenFlagGamePadBtnDown = (listener != CtrStdNil);
	else if (strcmp(CTR_DICT_MEDIA_MEDIA_GAMEPAD_UP, event_name)==0) CtrMediaEventListenFlagGamePadBtnUp = (listener != CtrStdNil);
	else if (strcmp(CTR_DICT_MEDIA_MEDIA_TIMER, event_name)==0) CtrMediaEventListenFlagTimer = (listener != CtrStdNil);
	else if (strcmp(CTR_DICT_MEDIA_MEDIA_ON_STEP, event_name)==0) CtrMediaEventListenFlagStep = (listener != CtrStdNil);
	ctr_heap_free(event_name);
	return ctr_object_on_do(myself, argumentList);
}

ctr_object* ctr_media_website(ctr_object* myself, ctr_argument* argumentList) {
	char* url = ctr_heap_allocate_cstring(argumentList->object);
	char  command[200];
	char* browsers[10];
	browsers[0] = "firefox";
	browsers[1] = "chromium";
	browsers[2] = "chrome";
	browsers[3] = "edge";
	browsers[4] = "safari";
	//check url for safety
	for (int j = 0; j<strlen(url); j++) {
		if (url[j]=='\'') return myself;
	}
	if (strlen(url)<150) {
		for (int i = 0; i < 5; i++) {
			memset(command, '\0', 100);
			sprintf(command, "%s '%s'", browsers[i], url);
			if (system(command)==0) {
				break;
			}
		}
	}
	ctr_heap_free(url);
	return myself;
}

void begin(){
	ctr_internal_media_reset();
	ctr_internal_media_init();
	colorObject = ctr_media_new(CtrStdObject, NULL);
	colorObject->link = CtrStdObject;
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_color_new );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_RED_GREEN_BLUE_SET ), &ctr_color_rgb_set );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_ALPHA_SET ), &ctr_color_a_set );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_RED ), &ctr_color_r );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_GREEN ), &ctr_color_g );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_BLUE ), &ctr_color_b );
	ctr_internal_create_func(colorObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_ALPHA ), &ctr_color_a );
	pointObject = ctr_media_new(CtrStdObject, NULL);
	pointObject->link = CtrStdObject;
	ctr_internal_create_func(pointObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_point_new );
	ctr_internal_create_func(pointObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_POINT_XY), &ctr_point_xyset );
	ctr_internal_create_func(pointObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_POINT_X ), &ctr_point_x );
	ctr_internal_create_func(pointObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_POINT_Y ), &ctr_point_y );
	lineObject = ctr_media_new(CtrStdObject, NULL);
	lineObject->link = CtrStdObject;
	ctr_internal_create_func(lineObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_line_new );
	ctr_internal_create_func(lineObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_LINE_FROM_TO ), &ctr_line_from_to );
	ctr_internal_create_func(lineObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_LINE_POINT_START ), &ctr_line_start );
	ctr_internal_create_func(lineObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_LINE_POINT_END ), &ctr_line_end );
	mediaObject = ctr_media_new(CtrStdObject, NULL);
	mediaObject->link = CtrStdObject;
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_media_new );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_SCREEN ), &ctr_media_screen );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_CLIPBOARD ), &ctr_media_clipboard );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_CLIPBOARD_SET ), &ctr_media_clipboard_set );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_ON_START ), &ctr_media_override );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_ON_STEP ), &ctr_media_override );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_SELECTED ), &ctr_media_select );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_DIGRAPH_LIGATURE ), &ctr_media_autoreplace );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_SAY ), &ctr_media_speak );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_LINK_SET ), &ctr_media_link_package );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_TIMER_SET ), &ctr_media_timer );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_WEBSITE ), &ctr_media_website );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_ONDO ), &ctr_media_on_do );
	ctr_internal_create_func(mediaObject, ctr_build_string_from_cstring( CTR_DICT_END ), &ctr_media_end );
	imageObject = ctr_img_new(CtrStdObject, NULL);
	imageObject->link = CtrStdObject;
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_img_new );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_img_new_set );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_SET ), &ctr_img_img );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_CONTROLLABLE ), &ctr_img_controllable );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_XY_SET ), &ctr_img_xy );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_X ), &ctr_img_x );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_Y ), &ctr_img_y );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_MOVE_TO_SET ), &ctr_img_mov_set );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_BOUNCE ), &ctr_img_bounce );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_SOLID_SET ), &ctr_img_solid );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_ACTIVE_SET ), &ctr_img_active );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_GRAVITY_SET ), &ctr_img_gravity );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_SPEED_SET ), &ctr_img_speed );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_FRICTION_SET ), &ctr_img_friction );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_ACCEL_SET ), &ctr_img_accel );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_JUMPHEIGHT_SET ), &ctr_img_jump_height );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_COLLISION_SET ), &ctr_media_override );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_ANIMATIONS_SET ), &ctr_img_anims );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_TEXT_SET ), &ctr_img_text );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_TEXT ), &ctr_img_text_get );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_REMOVE_SELECTION ), &ctr_img_text_del );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_INSERT_TEXT ), &ctr_img_text_ins );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_EDITABLE_SET ), &ctr_img_editable );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_FONT_TYPE_SIZE_SET ), &ctr_img_font );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_TEXT_COLOR ), &ctr_img_color );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_BACKGROUND_COLOR ), &ctr_img_background_color );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_TEXT_ALIGN ), &ctr_img_text_align );
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_DRAW ), &ctr_img_draw );
	audioObject = ctr_audio_new(CtrStdObject, NULL);
	audioObject->link = CtrStdObject;
	ctr_internal_create_func(audioObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_audio_new );
	soundObject = ctr_audio_new(audioObject, NULL);
	soundObject->link = audioObject;
	ctr_internal_create_func(soundObject, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_sound_new_set );
	ctr_internal_create_func(soundObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_AUDIO_PLAY ), &ctr_sound_play );
	musicObject = ctr_audio_new(audioObject, NULL);
	musicObject->link = audioObject;
	ctr_internal_create_func(musicObject, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_music_new_set );
	ctr_internal_create_func(musicObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_AUDIO_PLAY ), &ctr_music_play );
	ctr_internal_create_func(audioObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_AUDIO_SILENCE ), &ctr_music_silence );
	ctr_internal_create_func(audioObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_AUDIO_REWIND ), &ctr_music_rewind );
	networkObject = ctr_network_new(CtrStdObject, NULL);
	networkObject->link = CtrStdObject;
	ctr_internal_create_func(networkObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_network_new );
	ctr_internal_create_func(networkObject, ctr_build_string_from_cstring(CTR_DICT_MEDIA_MEDIA_NET_SEND_TO), &ctr_network_basic_text_send );
	ctr_internal_create_func(networkObject, ctr_build_string_from_cstring(CTR_DICT_MEDIA_MEDIA_NET_FETCH_FROM), &ctr_network_basic_text_receive );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_OBJECT ), mediaObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_OBJECT ), imageObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_COLOR_OBJECT ), colorObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_POINT_OBJECT ), pointObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_LINE_OBJECT ), lineObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_AUDIO_OBJECT ), audioObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_SOUND_OBJECT ), soundObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MUSIC_OBJECT ), musicObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_NET_SETTING_PORT ), ctr_build_string_from_cstring("MediaNetPort1"), CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_NET), networkObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	packageObject = ctr_package_new(CtrStdObject, NULL);
	packageObject->link = CtrStdObject;
	ctr_internal_create_func(packageObject, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_package_new_set );
	ctr_internal_create_func(packageObject, ctr_build_string_from_cstring(CTR_DICT_APPEND), &ctr_package_add );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_PACKAGE ), packageObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	/* Untranslated reference for systems that do not support UTF-8 characters in file names (like Windows) */
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Media" ), mediaObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}
