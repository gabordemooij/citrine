#define FALSE 0
#define TRUE 1

#define SDL_INIT_AUDIO 1
#define SDL_INIT_GAMECONTROLLER 2
#define SDL_MESSAGEBOX_INFORMATION 3
#define SDL_RENDERER_ACCELERATED 4
#define SDL_RENDERER_TARGETTEXTURE 5
#define SDL_RENDERER_SOFTWARE 6
#define SDL_WINDOW_OPENGL 7
#define SDL_WINDOWPOS_CENTERED 8
#define IMG_INIT_PNG 9
#define IMG_INIT_JPG 10
#define SDL_INIT_VIDEO 11
#define MIX_MAX_VOLUME 12
#define MIX_DEFAULT_CHANNELS 13
#define MIX_DEFAULT_FORMAT 14
#define MIX_DEFAULT_FREQUENCY 15
#define SDL_TEXTUREACCESS_TARGET 16
#define SDL_PIXELFORMAT_RGBA8888 17
#define RW_SEEK_SET 18
#define SDL_SCANCODE_LEFT 19
#define SDL_SCANCODE_RIGHT 20
#define SDL_SCANCODE_UP 21
#define SDL_SCANCODE_DOWN 22
#define SDL_SCANCODE_LSHIFT 23
#define SDL_SCANCODE_RSHIFT 24
#define SDL_KEYDOWN 25
#define SDL_SCANCODE_RETURN 26
#define SDL_SCANCODE_TAB 27
#define SDL_SCANCODE_BACKSPACE 28
#define SDL_SCANCODE_DELETE 29
#define SDL_SCANCODE_HOME 30
#define SDL_SCANCODE_PAGEUP 31
#define SDL_SCANCODE_END 32
#define SDL_SCANCODE_PAGEDOWN 33
#define SDL_TEXTEDITING 34
#define SDL_TEXTINPUT 35
#define SDL_KEYUP 36
#define SDL_CONTROLLERBUTTONUP 37
#define SDL_CONTROLLERBUTTONDOWN 42
#define SDL_FINGERUP 43
#define SDL_FINGERDOWN 44
#define SDL_MOUSEBUTTONDOWN 45
#define SDL_BUTTON_LEFT 46
#define SDL_MOUSEBUTTONUP 47
#define SDL_MOUSEMOTION 48
#define SDL_QUIT 0x100
#define SDL_TEXTUREACCESS_STREAMING 50
#define SDL_PIXELFORMAT_RGB24 51
#define SDL_FLIP_HORIZONTAL NULL
#define SDL_FLIP_NONE NULL
#define SDL_RENDERER_PRESENTVSYNC 54
#define SDL_WINDOW_FULLSCREEN_DESKTOP 55
#define SDL_MOUSEWHEEL 56
#define TTF_DIRECTION_LTR 57
#define TTF_DIRECTION_RTL 58
#define TTF_DIRECTION_BTT 59
#define TTF_DIRECTION_TTB 60




#define SDL_TICKS_PASSED(A, B)  ((uint32_t)((B) - (A)) <= 0)


typedef void* SDL_Gamepad;
typedef void* SDL_Renderer;
typedef void* SDL_Texture;
typedef void* TTF_Font;
typedef void* Mix_Music;
typedef void* Mix_Chunk;
typedef void* SDL_Keycode;
typedef void* SDL_Point;
typedef void* SDL_RendererFlip;
typedef void* SDL_Button;
typedef void* TTF_Text;

typedef void* plm_frame_t;

typedef enum SDL_GameControllerButton
{
    SDL_CONTROLLER_BUTTON_INVALID = -1,
    SDL_CONTROLLER_BUTTON_A,
    SDL_CONTROLLER_BUTTON_B,
    SDL_CONTROLLER_BUTTON_X,
    SDL_CONTROLLER_BUTTON_Y,
    SDL_CONTROLLER_BUTTON_BACK,
    SDL_CONTROLLER_BUTTON_GUIDE,
    SDL_CONTROLLER_BUTTON_START,
    SDL_CONTROLLER_BUTTON_LEFTSTICK,
    SDL_CONTROLLER_BUTTON_RIGHTSTICK,
    SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
    SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    SDL_CONTROLLER_BUTTON_DPAD_UP,
    SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT,
    SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
    SDL_CONTROLLER_BUTTON_MISC1,    /* Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button */
    SDL_CONTROLLER_BUTTON_PADDLE1,  /* Xbox Elite paddle P1 (upper left, facing the back) */
    SDL_CONTROLLER_BUTTON_PADDLE2,  /* Xbox Elite paddle P3 (upper right, facing the back) */
    SDL_CONTROLLER_BUTTON_PADDLE3,  /* Xbox Elite paddle P2 (lower left, facing the back) */
    SDL_CONTROLLER_BUTTON_PADDLE4,  /* Xbox Elite paddle P4 (lower right, facing the back) */
    SDL_CONTROLLER_BUTTON_TOUCHPAD, /* PS4/PS5 touchpad button */
    SDL_CONTROLLER_BUTTON_MAX
} SDL_GameControllerButton;

struct SDL_Color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

typedef struct SDL_Color SDL_Color;

struct SDL_Format {
	uint8_t Rmask;
	uint8_t Gmask;
	uint8_t Bmask;
	uint8_t Amask;
	uint8_t BitsPerPixel;
};
typedef struct SDL_Format SDL_Format;

struct SDL_Surface {
	SDL_Format* format;
};
typedef struct SDL_Surface SDL_Surface;

struct SDL_Rect {
	int x;
	int y;
	int w;
	int h;
};

typedef struct SDL_Rect SDL_Rect;

struct SDL_TFinger {
	int x;
	int y;
};

typedef struct SDL_TFinger SDL_TFinger;

struct SDL_Event_Button {
	int button;
	int clicks;
	int x;
	int y;
};

typedef struct SDL_Event_Button SDL_Event_Button;

struct SDL_Event_CButton {
	int button;
};

typedef struct SDL_Event_CButton SDL_Event_CButton;

struct SDL_Event_KeySym {
	SDL_Keycode sym;
	int scancode;
};

typedef struct SDL_Event_KeySym SDL_Event_KeySym;

struct SDL_Event_Key {
	SDL_Event_KeySym keysym;
};

typedef struct SDL_Event_Key SDL_Event_Key;

struct SDL_Event_Text {
	char* text;
};

typedef struct SDL_Event_Text SDL_Event_Text;

struct SDL_Event_Wheel {
	int x;
	int y;
};

typedef struct SDL_Event_Wheel SDL_Event_Wheel;


struct SDL_Event {
	int type;
	SDL_Event_Wheel wheel;
	SDL_Event_Text text;
	SDL_Event_Key key;
	SDL_Event_CButton cbutton;
	SDL_Event_Button button;
	SDL_TFinger tfinger;
};

typedef struct SDL_Event SDL_Event;


struct SDL_Window {
	int mock;
};

typedef struct SDL_Window SDL_Window;

struct SDL_RWops {
	char* mock_file;
};

typedef struct SDL_RWops SDL_RWops;

SDL_Surface MockSurface;
SDL_Rect MockRect;
SDL_Color MockColor;
SDL_Window MockWindow;
SDL_Renderer MockRenderer;
SDL_RWops MockRWops;
Mix_Chunk MockChunk;
Mix_Music MusicMock;
SDL_Texture MockTexture;

int MockTicks64 = 0;
int MockPerformanceCounter = 0;
int MockPerformanceFrequency = 0;

char* MockValTestJPG = "test.jpg";
char* MockValScreenPNG = "screen.png";

char* SDL_GetError() {
	printf("SDL_GetError()\n");
	return "";
}

int Mix_OpenAudio(int frequency, int format, int channels, int chunksize) {
	printf("Mix_OpenAudio(%d,%d,%d,%d)\n", frequency, format, channels, chunksize);
	return 0;
}

int TTF_Init() {
	printf("TTF_Init()\n");
	return 0;
}

SDL_GameController* SDL_GameControllerOpen(int joystick_index) {
	printf("SDL_GameControllerOpen(%d)\n", joystick_index);
	return NULL;
}

int SDL_InitSubSystem(int flags) {
	printf("SDL_InitSubSystem(%d)\n", flags);
	return 0;
}

void SDL_ShowSimpleMessageBox(int type, char* title, char* message, void* window) { 
	printf("SDL_ShowSimpleMessageBox(%d,%s,%s)\n", type, title, message);
}

SDL_Renderer* SDL_CreateRenderer(void* window, int param2, int mode) {
	return &MockRenderer;
}

SDL_Window* SDL_CreateWindow(const char *title, int x, int y, int w, int h, uint32_t flags) {
	return &MockWindow;
}

int IMG_Init(int flags) {
	printf("IMG_Init(%d)\n", flags);
	return 0;
}

int SDL_Init(int flags) {
	printf("SDL_Init(%d)\n", flags);
	return 0;
}

int SDL_SetClipboardText(const char *text) {
	printf("SDL_SetClipboardText(%s)\n", text);
	return 0;
}

void SDL_free(void* buffer) {
	printf("SDL_free(%p)\n", buffer);
}

char* SDL_GetClipboardText(void) {
	printf("SDL_GetClipboardText()\n");
	return NULL;
}

int SDL_RenderDrawPoint(SDL_Renderer* renderer, int x, int y) {
	printf("SDL_RenderDrawPoint(%p,%d,%d)\n",renderer,x,y);
	return 0;
}

SDL_Texture* SDL_CreateTexture(SDL_Renderer* renderer, uint32_t format, int access, int w, int h) {
	printf("SDL_CreateTexture(%p,%u,%d,%d,%d)\n",renderer,format,access,w,h);
	return NULL;
}

int SDL_SetRenderTarget(SDL_Renderer *renderer, SDL_Texture *texture) {
	printf("SDL_SetRenderTarget(%p,%p)\n",renderer,texture);
	return 0;
}

SDL_Texture* SDL_CreateTextureFromSurface(SDL_Renderer* renderer, SDL_Surface* surface) {
	printf("SDL_CreateTextureFromSurface()\n");
	return &MockTexture;
}

SDL_Surface* TTF_RenderUTF8_Shaded(TTF_Font* font, const char *text, SDL_Color fg, SDL_Color bg) {
	printf("TTF_RenderUTF8_Shaded(%p,%s,%p,%p)\n",font,text,&fg,&bg);
	return &MockSurface;
}

SDL_Surface* TTF_RenderUTF8_Blended(TTF_Font* font, const char *text, SDL_Color fg) {
	printf("TTF_RenderUTF8_Blended(%p,%s,%p)\n",font,text,&fg);
	return &MockSurface;
}

int SDL_BlitSurface(SDL_Surface* src, const SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect) {
	printf("SDL_BlitSurface(%p,%p,%p,%p)\n",src,srcrect,dst,dstrect);
	return 0;
}

void SDL_FreeSurface(SDL_Surface* surface) {
	printf("SDL_FreeSurface(surface)\n");
}

SDL_Surface* SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
	printf("SDL_CreateRGBSurface(%u,%d,%d,%u,%u,%u,%u,%u)\n",flags,width,height,depth,Rmask, Gmask,Bmask,Amask);
	return &MockSurface;
}

int SDL_RenderDrawLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2) {
	printf("SDL_RenderDrawLine(%p,%d,%d,%d,%d)\n",renderer,x1,y1,x2,y2);
	return 0;
}

int SDL_SetRenderDrawColor(SDL_Renderer* renderer, uint8_t r, uint8_t g, uint8_t b, uint8_t a){
	printf("SDL_SetRenderDrawColor(%p,%d,%d,%d,%d)\n",renderer,r,g,b,a);
	return 0;
}

TTF_Font* TTF_OpenFont(const char* file, int ptsize) {
	printf("TTF_OpenFont(%s,%d)\n", file, ptsize);
	return NULL;
}

SDL_Surface* IMG_Load_RW(SDL_RWops* src, int freesrc) {
	printf("IMG_Load_RW(rwops,%d)\n", freesrc);
	return &MockSurface;
}

void Mix_RewindMusic(void) {
	printf("Mix_RewindMusic()\n");
}

void Mix_PauseMusic(void) {
	printf("Mix_PauseMusic()\n");
}

int Mix_FadeInMusic(Mix_Music* music, int loops , int ms) {
	printf("Mix_FadeInMusic(music,%d,%d)\n", loops, ms);
	return 0;
}

void Mix_FreeMusic(Mix_Music* music) {
	printf("Mix_FreeMusic(music)\n");
}

Mix_Music* Mix_LoadMUS_RW(SDL_RWops* src, int freesrc) {
	printf("Mix_LoadMUS_RW(src,%d)\n", freesrc);
	return &MusicMock;
}

int64_t SDL_RWtell(SDL_RWops* context) {
	printf("SDL_RWtell(%p)\n", context);
	return 0;
}

int64_t SDL_RWseek(SDL_RWops *context, int64_t offset, int whence) {
	printf("SDL_RWseek()\n");
	return 0;
}

SDL_RWops* SDL_RWFromMem(void* mem, int size) {
	printf("SDL_RWFromMem(%p,%d)\n", mem, size);
	return NULL;
}

SDL_RWops* SDL_RWFromFile(const char* file, const char* mode) {
	printf("SDL_RWFromFile(%s,%s)\n", file, mode);
	MockRWops.mock_file = 0;
	if (strcmp(file,MockValTestJPG)==0) {
		MockRWops.mock_file = MockValTestJPG;
	}
	if (strcmp(file,MockValScreenPNG)==0) {
		MockRWops.mock_file = MockValScreenPNG;
	}
	return &MockRWops;
}

Mix_Chunk* Mix_LoadWAV_RW(SDL_RWops* src, int freesrc) {
	printf("Mix_LoadWAV_RW(src,%d)\n", freesrc);
	return &MockChunk;
}

int Mix_PlayChannel(int channel, Mix_Chunk* chunk, int loops) {
	printf("Mix_PlayChannel(%d,chunk,%d)\n", channel, loops);
	return 0;
}

void Mix_FreeChunk(Mix_Chunk *chunk) {
	printf("Mix_FreeChunk(chunk)\n");
}

void SDL_RenderPresent(SDL_Renderer * renderer) {
	printf("SDL_RenderPresent(renderer)\n");
}

int SDL_strlen(char* str) {
	return strlen(str);
}

int SDL_SetWindowInputFocus(SDL_Window* window) {
	printf("SDL_SetWindowInputFocus(%p)\n", window);
	return 0;
}

void SDL_RaiseWindow(SDL_Window * window) {
	printf("SDL_RaiseWindow(%p)\n", window);
}

SDL_Window* SDL_GetKeyboardFocus(void) {
	printf("SDL_GetKeyboardFocus()\n");
	return NULL;
}

const char* SDL_GetKeyName(SDL_Keycode key) {
	printf("SDL_GetKeyName(%p)\n", key);
	return "";
}

const char* SDL_GameControllerGetStringForButton(SDL_GameControllerButton button) {
	printf("SDL_GameControllerGetStringForButton(%d)\n", button);
	return "";
}

int MockEventCount = 0;

int SDL_PollEvent(SDL_Event* event) {
	int has_event = 0;
	if (MockEventCount >= 3) {
		printf("SDL_PollEvent(SDL_QUIT)\n");
		event->type = SDL_QUIT;
		has_event = 1;
	}
	MockEventCount++;
	return has_event;
}

int SDL_RenderClear(SDL_Renderer* renderer) {
	printf("SDL_RenderClear(renderer)\n");
	return 0;
}

void SDL_GL_GetDrawableSize(SDL_Window* window, int* w, int* h) {
	printf("SDL_GL_GetDrawableSize(window,640,480)\n");
	*w = 640;
	*h = 480;
}

void SDL_SetWindowPosition(SDL_Window* window, int x, int y) {
	printf("SDL_SetWindowPosition(window, %d, %d)\n",x,y);
}

int SDL_QueryTexture(SDL_Texture* texture, uint32_t* format, int* access, int* w, int* h) {
	printf("SDL_QueryTexture(texture...)\n");
	*w = 640;
	*h = 480;
	return 0;
}

SDL_Texture* IMG_LoadTexture_RW(SDL_Renderer* renderer, SDL_RWops *src, int freesrc) {
	printf("IMG_LoadTexture_RW(renderer,src,%d)\n", freesrc);
	return NULL;
}

void SDL_Delay(uint32_t ms) {
	printf("SDL_Delay(%u)\n", ms);
}

void SDL_SetWindowSize(SDL_Window* window, int w, int h) {
	printf("SDL_SetWindowSize(window, %d, %d)\n", w, h);
}

int SDL_RWclose(SDL_RWops* context) {
	printf("SDL_RWclose(%p)\n", context);
	return 0;
}

size_t SDL_RWread(SDL_RWops* context, void* ptr, size_t size, size_t maxnum) {
	printf("SDL_RWread()\n");
	if (context->mock_file == MockValScreenPNG) {
		printf("Mock: reading file screen.png\n");
		memcpy(ptr, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8); //png magic bytes
		return 8;
	}
	return 0;
}

int SDL_UpdateTexture(SDL_Texture * texture, const SDL_Rect* rect, const void *pixels, int pitch) {
	printf("SDL_UpdateTexture(%p, %p, %p, %d)\n", texture,rect,pixels,pitch);
	return 0;

}

int SDL_RenderCopy(SDL_Renderer* renderer, SDL_Texture* texture, const SDL_Rect* srcrect, const SDL_Rect* dstrect) {
	printf("SDL_RenderCopy(renderer,texture,src,dst)\n");
	return 0;
}

int SDL_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture, const SDL_Rect * srcrect, const SDL_Rect * dstrect, const double angle, const SDL_Point* center, const SDL_RendererFlip flip) {
	printf("SDL_RenderCopyEx(renderer,texture,srcrect,dstrect,angle,center,flip)\n");
	return 0;
}

int SDL_RenderSetLogicalSize(SDL_Renderer * renderer, int w, int h) {
	printf("SDL_RenderSetLogicalSize(%p, %d, %d)\n", renderer, w, h);
	return 0;
}

int SDL_GetTicks() {
	printf("SDL_GetTicks()\n");
	return 0;
}

int SDL_Quit() {
	printf("SDL_Quit()\n");
	return 0;
}

int TTF_GetTextSize(TTF_Font *font, const char *text, int *w, int *h) {
	printf("TTF_SizeUTF8(%p, %s)\n", font, text);
	*w = 10;
	*h = 10;
	return 0;
}

int SDL_HasIntersection(const SDL_Rect * A, const SDL_Rect * B) {
	printf("SDL_HasIntersection(A,B)\n");
	return 0;
}

int SDL_IntersectRect(const SDL_Rect * A, const SDL_Rect * B, SDL_Rect * C) {
	printf("SDL_IntersectRect(A,B,C)\n");
	return 0;
}

int SDL_GetTicks64() {
	printf("SDL_GetTicks64()\n");
	return MockTicks64;
}

int SDL_StartTextInput() {
	printf("SDL_StartTextInput()\n");
	return 0;
}

int SDL_StopTextInput() {
	printf("SDL_StopTextInput()\n");
	return 0;
}

int SDL_GetPerformanceCounter() {
		printf("SDL_StopTextInput()\n");

	return MockPerformanceCounter;
}
int SDL_GetPerformanceFrequency() {
	printf("SDL_StopTextInput()\n");
	return MockPerformanceFrequency;

}

int TTF_CloseFont() {
	printf("TTF_CloseFont()\n");
	return 0;
}
int TTF_SetFontDirection(void** font, int d) {
	printf("TTF_SetFontDirection(%d)\n", d);
	return 0;
}

void** TTF_OpenFontRW(SDL_RWops* fontname, int i, int j) {
	printf("TTF_OpenFontRW()\n");
	return NULL;
}

int SDL_DestroyTexture() {
	printf("SDL_DestroyTexture()\n");
	return 0;
}
int TTF_SetFontScriptName() {
	printf("TTF_SetFontScriptName()\n");
	return 0;
}

SDL_RWops* SDL_RWFromConstMem() {
	printf("SDL_RWFromConstMem()\n");
	return &MockRWops;
}

int SDL_OpenURL(char* str){
	printf("SDL_OpenURL(%s)\n", str);
	return 0;
}