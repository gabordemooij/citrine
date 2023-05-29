#include "../../citrine.h"
#include "media.h"
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <math.h>

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
time_t CtrMediaFrameTimer = 0;
uint16_t CtrMediaSteps;
AVFormatContext* CtrMediaVideoFormatCtx;
AVCodec* CtrMediaBGVideoCodec;
AVCodecContext* CtrMediaBGVideoCdcCtx;
int CtrMediaVideoId = -1;
double CtrMediaVideoFPSRendering;
AVCodecParameters* CtrMediaVideoParams;
AVFrame* CtrMediaVideoFrame;
AVPacket* CtrMediaVideoPacket;
SDL_Texture* CtrMediaBGVideoTexture;

int CtrMediaAudioRate;
uint16_t CtrMediaAudioFormat;
int CtrMediaAudioChannels;
int CtrMediaAudioBuffers;
int CtrMediaAudioVolume;

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
SDL_GameController* gameController;

void ctr_internal_img_render_text(ctr_object* myself);
void ctr_internal_img_render_cursor(ctr_object* myself);

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
	if (CtrMediaVideoId != -1) {
		av_packet_free(&CtrMediaVideoPacket);
		av_frame_free(&CtrMediaVideoFrame);
		avcodec_free_context(&CtrMediaBGVideoCdcCtx);
		avformat_close_input(&CtrMediaVideoFormatCtx);
		avformat_free_context(CtrMediaVideoFormatCtx);
		CtrMediaVideoFormatCtx = NULL;
		CtrMediaVideoId = -1;
		CtrMediaVideoFPSRendering = 0;
		CtrMediaFrameTimer = 0;
		CtrMediaVideoParams = NULL;
		CtrMediaVideoFrame = NULL;
		CtrMediaVideoPacket = NULL;
		CtrMediaBGVideoTexture = NULL;
		CtrMediaBGVideoCodec = NULL;
		CtrMediaBGVideoCdcCtx = NULL;
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
		if ((haystack->text[CtrMediaInputIndex]=='\n')) {
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
		if ((haystack->text[CtrMediaInputIndex]=='\n')) {
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

void ctr_internal_media_loadvideobg(char* path, SDL_Rect* dimensions) {
	CtrMediaVideoId = -1;
	CtrMediaVideoFPSRendering = 0.0;
	CtrMediaVideoFormatCtx = avformat_alloc_context();
	if (avformat_open_input(&CtrMediaVideoFormatCtx, path, NULL, NULL) < 0) ctr_internal_media_fatalerror("Unable to open video", "FFMPEG");
	if (avformat_find_stream_info(CtrMediaVideoFormatCtx, NULL) < 0) ctr_internal_media_fatalerror("Unable to find stream in video", "FFMPEG");
	char foundVideo = 0;
	for (int i = 0; i < CtrMediaVideoFormatCtx->nb_streams; i++) {
		AVCodecParameters* localparam = CtrMediaVideoFormatCtx->streams[i]->codecpar;
		AVCodec* localcodec = avcodec_find_decoder(localparam->codec_id);
		if (localparam->codec_type == AVMEDIA_TYPE_VIDEO && !foundVideo) {
			CtrMediaBGVideoCodec = localcodec;
			CtrMediaVideoParams = localparam;
			CtrMediaVideoId = i;
			AVRational rational = CtrMediaVideoFormatCtx->streams[i]->avg_frame_rate;
			CtrMediaVideoFPSRendering = 1.0 / ((double)rational.num / (double)(rational.den));
			foundVideo = 1;
		}
	}
    CtrMediaBGVideoCdcCtx = avcodec_alloc_context3(CtrMediaBGVideoCodec);
    if (avcodec_parameters_to_context(CtrMediaBGVideoCdcCtx, CtrMediaVideoParams) < 0) ctr_internal_media_fatalerror("CtrMediaBGVideoCdcCtx","FFMPEG");
    if (avcodec_open2(CtrMediaBGVideoCdcCtx, CtrMediaBGVideoCodec, NULL) < 0) ctr_internal_media_fatalerror("CtrMediaBGVideoCdcCtx2", "FFMPEG");
    CtrMediaVideoFrame = av_frame_alloc();
    CtrMediaVideoPacket = av_packet_alloc();
    CtrMediaBGVideoTexture = SDL_CreateTexture(CtrMediaRenderer, SDL_PIXELFORMAT_IYUV,
        SDL_TEXTUREACCESS_STREAMING | SDL_TEXTUREACCESS_TARGET,
        CtrMediaVideoParams->width, CtrMediaVideoParams->height);
    if (!CtrMediaBGVideoTexture) ctr_internal_media_fatalerror("texture", "FFMPEG");
    SDL_SetWindowSize(CtrMediaWindow, CtrMediaVideoParams->width, CtrMediaVideoParams->height);
	SDL_Delay(100);
	dimensions->x = 0;
	dimensions->y = 0;
	dimensions->h = CtrMediaVideoParams->height;
	dimensions->w = CtrMediaVideoParams->width;
}


void ctr_internal_media_rendervideoframe(SDL_Rect* rect) {
	if (av_read_frame(CtrMediaVideoFormatCtx, CtrMediaVideoPacket) < 0) {
		av_seek_frame(CtrMediaVideoFormatCtx, CtrMediaVideoId, 0, 0);
		if (av_read_frame(CtrMediaVideoFormatCtx, CtrMediaVideoPacket) < 0) return;
	}
	if (CtrMediaVideoPacket->stream_index != CtrMediaVideoId) return;
	if (avcodec_send_packet(CtrMediaBGVideoCdcCtx, CtrMediaVideoPacket) < 0) ctr_internal_media_fatalerror("sp", "FFMPEG");
	if (avcodec_receive_frame(CtrMediaBGVideoCdcCtx, CtrMediaVideoFrame) < 0) return;
	SDL_UpdateYUVTexture(CtrMediaBGVideoTexture, rect,
		CtrMediaVideoFrame->data[0], CtrMediaVideoFrame->linesize[0],
		CtrMediaVideoFrame->data[1], CtrMediaVideoFrame->linesize[1],
		CtrMediaVideoFrame->data[2], CtrMediaVideoFrame->linesize[2]);
	SDL_RenderCopy(CtrMediaRenderer, CtrMediaBGVideoTexture, NULL, rect);
	time_t end = time(NULL);
	double diffms = difftime(end, CtrMediaFrameTimer) / 1000.0;
	if (diffms < CtrMediaVideoFPSRendering - (CtrMediaStdDelayTime/1000)) {
		uint32_t diff = (uint32_t)((CtrMediaVideoFPSRendering - diffms) * 1000);
		SDL_Delay(diff - CtrMediaStdDelayTime);
	}
	av_packet_unref(CtrMediaVideoPacket);
	CtrMediaFrameTimer = time(NULL);
}

char ctr_internal_media_determine_filetype(char* path) {
	char magic[20];
	FILE* fh = fopen(path, "rb");
	fread(magic, 20, 1, fh);
	if (strcmp(magic+4, "ftypmp42")==0) return 10;
	if (strcmp(magic, "\xFF\xD8")==0) return 20;
	return 0;
	fclose(fh);
}

ctr_object* ctr_media_end(ctr_object* myself, ctr_argument* argumentList) {
	CtrMediaBreakLoopFlag = 1;
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
		ctr_send_message(myself, CTR_DICT_MEDIA_MEDIA_ON_STEP, strlen(CTR_DICT_MEDIA_MEDIA_ON_STEP), NULL );
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
					break;
				case SDL_MOUSEBUTTONDOWN:
					if (ctr_internal_media_mouse_down(event)) return CtrStdFlow;
					break;
				case SDL_CONTROLLERBUTTONDOWN:
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
	mediaAUD->blob = (void*)Mix_LoadMUS(audioFileStr);
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
	mediaImage->texture = (void*) IMG_LoadTexture(CtrMediaRenderer, imageFileStr);
	if (mediaImage->texture == NULL) ctr_internal_media_fatalerror("Unable to load texture", imageFileStr);
	mediaImage->surface = (void*) IMG_Load(imageFileStr);
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
	if (!CtrMediaRenderer) ctr_internal_media_fatalerror("Unable to create renderer", SDL_GetError());
	SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
	gameController = SDL_GameControllerOpen(0);
	if (TTF_Init() < 0) ctr_internal_media_fatalerror("Unable to init TTF", SDL_GetError());
	if (SDL_Init(SDL_INIT_AUDIO) < 0) ctr_internal_media_fatalerror("Couldn't initialize SDL: %s\n",SDL_GetError());
	if (Mix_OpenAudio(CtrMediaAudioRate, CtrMediaAudioFormat, CtrMediaAudioChannels, CtrMediaAudioBuffers) < 0) ctr_internal_media_fatalerror("Couldn't open audio: %s\n", SDL_GetError());
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
	ctr_internal_create_func(imageObject, ctr_build_string_from_cstring( CTR_DICT_NEW ), &ctr_audio_new );
	soundObject = ctr_audio_new(audioObject, NULL);
	soundObject->link = audioObject;
	ctr_internal_create_func(soundObject, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_sound_new_set );
	ctr_internal_create_func(soundObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_AUDIO_PLAY ), &ctr_sound_play );
	musicObject = ctr_audio_new(audioObject, NULL);
	musicObject->link = audioObject;
	ctr_internal_create_func(musicObject, ctr_build_string_from_cstring( CTR_DICT_NEW_SET ), &ctr_music_new_set );
	ctr_internal_create_func(musicObject, ctr_build_string_from_cstring( CTR_DICT_MEDIA_AUDIO_PLAY ), &ctr_music_play );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MEDIA_OBJECT ), mediaObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_IMAGE_OBJECT ), imageObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_COLOR_OBJECT ), colorObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_POINT_OBJECT ), pointObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_LINE_OBJECT ), lineObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_AUDIO_OBJECT ), audioObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_SOUND_OBJECT ), soundObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( CTR_DICT_MEDIA_MUSIC_OBJECT ), musicObject, CTR_CATEGORY_PUBLIC_PROPERTY);
	/* Untranslated reference for systems that do not support UTF-8 characters in file names (like Windows) */
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Media" ), mediaObject, CTR_CATEGORY_PUBLIC_PROPERTY);
}
