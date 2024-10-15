// Part 1 : Objects
// In Citrine, you write a program by sending messages to objects.
// The Media Plugin adds the following objects to the world of Citrine:

#define CTR_DICT_NETWORK_OBJECT             "Network"              // The Object representing the Internet and/or local Network
#define CTR_DICT_COLOR_OBJECT               "Color"                // The Object that represents the concept of a color
#define CTR_DICT_POINT_OBJECT               "Point"                // The Object that represents a point in space visually
#define CTR_DICT_LINE_OBJECT                "Line"                 // The Object that represents a visual line between two Points
#define CTR_DICT_IMAGE_OBJECT               "Image"                // The Object that represents an visual image from a file like PNG or JPEG
#define CTR_DICT_FONT_OBJECT                "Font"                 // The Object that represents a Font
#define CTR_DICT_AUDIO_OBJECT               "Audio"                // The Object that represents audible data 
#define CTR_DICT_SOUND_OBJECT               "Sound"                // The Object that represents a sound like boom!
#define CTR_DICT_MUSIC_OBJECT               "Music"                // The Object that represents a piece of music that can be played back
#define CTR_DICT_PACKAGE_OBJECT             "Package"              // The Object that represents an archive that may contain assets for your app or game
#define CTR_DICT_BLOB_OBJECT                "Blob"                 // The Object that represents a memory blob (for foreign function interface)

// Part 2: Events

// Event handlers are blocks of code that can be attached to events that happen during
// the app or game. Citrine will notify you about these events and you can attach tasks to
// them. These tasks will then execute upon the event.

#define CTR_DICT_ON_STEP                    "step"                 // Event handler for each single step in the game
#define CTR_DICT_ON_KEY_UP                  "key:"                 // Event handler for key up
#define CTR_DICT_ON_KEY_DOWN                "key-down:"            // Event handler for key down
#define CTR_DICT_ON_TIMER                   "timer:"               // Event handler for time-based events
#define CTR_DICT_ON_GAMEPAD_DOWN            "gamepad-down:"        // Event handler for gamepad/joystick/gamekey button down
#define CTR_DICT_ON_GAMEPAD_UP              "gamepad:"             // Event handler for gamepad/joystick/gamekey button up
#define CTR_DICT_ON_CLICK                   "click"                // Event handler for clicks/touches on an image
#define CTR_DICT_ON_HOVER                   "hover:"               // Event handler for hovering over an image
#define CTR_DICT_ON_CLICK_XY                "click-x:y:"           // Event handler for click/touch on certain position on screen
#define CTR_DICT_STOP_AT_SET                "destination"          // Event handler called when image reaches destination x/y
#define CTR_DICT_COLLISION_SET              "collision:"           // Event handler for collisions of active images
#define CTR_DICT_ON_EVENT                   "event:"               // Event handler for other/misc/future events


// Part 3: Messages
// Message can be send to objects to make them do things.

#define CTR_DICT_SCREEN                     "screen:"              // Displays something on the screen (i.e. in the window)
#define CTR_DICT_CLIPBOARD                  "clipboard"            // Retrieves the contents of the clipboard
#define CTR_DICT_TIMER_SET                  "timer:after:"         // Sets a timer (that will cause a timer event) like settimeout in js
#define CTR_DICT_LINK_SET                   "link:"                // Connects to a Package to retrieve assets from there instead of using disk files

// The "link:" command can also be used to connect to a system library, to access
// functionality outside of Citrine. The general idea is that you 'link' or 'connect' to
// a certain file to extract functions or other resources.

#define CTR_DICT_SAY_SET                    "say:"                 // Sends text to a pre-configured text-to-speech synthesizer
#define CTR_DICT_SELECTION                  "selected"               // Returns the selected text in an editable image that can function as a text field
#define CTR_DICT_SEND_TO_SET                "send:to:"             // Sends/returns data over network (Network send: "blah" to: "POST https://...")
#define CTR_DICT_RED_GREEN_BLUE_SET         "red:green:blue:"      // Creates a new color by setting the amount of red, green and blue
#define CTR_DICT_TRANSPARENCY               "transparency:"        // Sets the transparancy of an image
#define CTR_DICT_IMAGE_SET                  "image:"               // Sets the image / updates the image
#define CTR_DICT_CONTROLLABLE               "controllable:"        // Makes the image controllable by joystick/keys/gamepad 1/Yes = top-down/platform 2=Horizontal 3=Vertical 4=Radius-style
#define CTR_DICT_SOLID_SET                  "solid:"               // Toggles the solidness of an image in a game
#define CTR_DICT_ACTIVE_SET                 "active:"              // Toggles event activation of an image in a game (whether events are fired)
#define CTR_DICT_GRAVITY_SET                "gravity:"             // Sets the gravity of an image
#define CTR_DICT_SPEED_SET                  "speed:"               // Sets the motion speed of an image
#define CTR_DICT_FRICTION_SET               "friction:"            // Sets the friction level of an image
#define CTR_DICT_ANIMATIONS_SET             "animations:"          // Sets the number of sub images in a single image to perform animation
#define CTR_DICT_CUT                        "cut"                  // Cuts selected text an editable image that functions as a text field
#define CTR_DICT_EDITABLE_SET               "editable:"            // Toggles editability of an image, if Yes then you can write text in the image
#define CTR_DICT_FONT_TYPE_SIZE_SET         "font:size:"           // Sets the font and size for an image
#define CTR_DICT_COLOR_SET                  "color:"               // Sets the color of the text in an image
#define CTR_DICT_BACKGROUND_COLOR_SET       "background-color:"    // Sets the background color
#define CTR_DICT_DRAW_COLOR_SET             "draw:color:"          // Sets the color for drawing on an image
#define CTR_DICT_ACCELERATE_SET             "accelerate:"          // Sets acceleration of an image
#define CTR_DICT_JUMPHEIGHT_SET             "jump-height:"         // Sets the jump height of an image
#define CTR_DICT_BOUNCE_SET                 "bounce:"              // Toggles whether an image bounces or not
#define CTR_DICT_AUDIO_PLAY                 "play"                 // Plays music or sound
#define CTR_DICT_AUDIO_SILENCE              "silence"              // Stops music or sound
#define CTR_DICT_AUDIO_REWIND               "rewind"               // Rewinds music
#define CTR_DICT_ALIGN_XY_SET               "align-x:y:"           // Align text in image on x/y
#define CTR_DICT_MOVE_TO_XY_SET             "to-x:y:"              // Moves images to position x/y using specified speed
#define CTR_DICT_CLIPBOARD_SET              "clipboard:"           // Sets the contents of the clipboard
#define CTR_DICT_FROM_TO_SET                "from:to:"             // Draws a line between two points
#define CTR_DICT_WIDTH_HEIGHT               "width:height:"        // Sets the width and height of a media object (camera)
#define CTR_DICT_REEL_SET                   "reel:speed:"          // Treat image as a movie with X frames on the reel and apply speed Y
#define CTR_DICT_REEL_AUTOPLAY_SET          "autoplay:"            // Autoplay movie-image
#define CTR_DICT_LINEHEIGHT_SET             "line-height:"         // Set line height of text in image
#define CTR_DICT_GHOST_SET                  "ghost:"               // Allow image to pass through walls (solid images)
#define CTR_DICT_STATIC                     "static:"              // Position image regardless of camera position
#define CTR_DICT_NODIRANI_SET               "fixate:"              // Fixate image, do not apply automatic animations for directions

 
#define CTR_DICT_X                          "x?"                   // Get x position of image
#define CTR_DICT_Y                          "y?"                   // Get y position of image
#define CTR_DICT_SOURCE_SIZE_SET            "source:size:"         // Set source and size of font
#define CTR_DICT_FONTSCRIPT_TXTDIR_SET      "text-style:direction:" // Set Harfbuzz text shaping and writing direction (RTL)
#define CTR_DICT_DIALOG_SET                 "show:"                 // Show a dialog with text ...
#define CTR_DICT_BYTES_SET                  "bytes:"                // Set bytes in a blob of memory (FFI)
#define CTR_DICT_FREE                       "free"                  // Free a memory blob (FFI)
#define CTR_DICT_FREE_STRUCT                "structfree"            // Free a struct (FFI)
#define CTR_DICT_STRUCT_SET                 "struct:"               // Fill a struct (FFI)
#define CTR_DICT_DEREF                      "deref"                 // Dereference a pointer (FFI)
#define CTR_DICT_UTF8_SET                   "utf8:"                 // Fill a memory blob with utf8 encoded data (FFI)
#define CTR_DICT_NEW_TYPE_SET               "new:type:"             // Create a new memory blob of a certain type (FFI)
#define CTR_DICT_FX                         "effect:options:"       // Apply a special effect
#define CTR_DICT_FREEZE_SET                 "freeze:"               // Disable control of image
 
 