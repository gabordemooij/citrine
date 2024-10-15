// Annotated System Message Template File

#define CTR_MSG_LANG_CODE "en_us"                                                       // Language code for this version of Citrine
#define CTR_MSG_WELCOME   "Citrine Programming Language EN/US\n"                        // Text for Welcome Screen
#define CTR_MSG_COPYRIGHT "Written by Gabor de Mooij © copyright 2019, Licensed BSD.\n" // Text for Welcome Screen
#define CTR_MSG_USAGE_G   "Not enough arguments. Usage: ctr -g file1.h file2.h"         // Error message, not enough arguments to generate dictionary from two headers
#define CTR_MSG_USAGE_T   "Not enough arguments. Usage: ctr -t d.dict program.ctr"      // Error message, not enough arguments to translate code file using dictionary
#define CTR_ERR_LEX       "%s on line: %d. \n"                                          // Lexer error, syntax error/other error %s on line %d
#define CTR_ERR_TOKBUFF   "Token Buffer Exausted. Tokens may not exceed 255 bytes"      // Error, a Citrine keyword or token was longer than 255 bytes
#define CTR_ERR_OOM       "Out of memory."                                              // Out of memory
#define CTR_ERR_SYNTAX    "Parse error, unexpected %s ( %s: %d )\n"                     // Parsing error, parser expected certain token %s but got %s on line %d
#define CTR_ERR_LONG      "Message too long.\n"                                         // Message to object was too long
#define CTR_ERR_EXP_COLON "Expected colon.\n"                                           // Parsing error, parser expected a colon (or translated symbol) but got something else
#define CTR_ERR_EXP_MSG   "Expected message.\n"                                         // Parsing error, parser expected a message but got something else
#define CTR_ERR_EXP_PCLS  "Expected ).\n"                                               // Parsing error, parser expected a ) but got something else
#define CTR_ERR_EXP_DOT   "Expected a dot (.).\n"                                       // Parsing error, parser expected a end-of-line symbol but got something else
#define CTR_ERR_EXP_KEY   "Key should always be followed by a property name!\n"         // A key symbol (or translation) was not followed by a property name
#define CTR_ERR_EXP_VAR   "Pointing hand should always be followed by variable!\n"      // A pointing finger (or translation) was not followed by an assignment statement
#define CTR_ERR_EXP_RCP   "Expected a message recipient.\n"                             // Expected an object to send a message to but got something else
#define CTR_ERR_EXP_MSG2  "Recipient cannot be followed by colon.\n"                    // An object recipient was followed by a comma directly, i.e. Object: (instead of Object message:)
#define CTR_ERR_INV_LAS   "Invalid left-hand assignment.\n"                             // User tried to assign a value to something that is not assignable (i.e. expression)
#define CTR_ERR_EXP_BLK   "Expected block."                                             // Citrine expected a literal task/block, but got something else
#define CTR_ERR_EXP_ARR   "Expected list."                                              // Citrine expected a literal list, but got something else
#define CTR_ERR_EXP_NUM   "Expected number."                                            // Citrine expected a literal number, but got something else
#define CTR_ERR_EXP_STR   "Expected string."                                            // Citrine expected a literal string/text, but got something else
#define CTR_ERR_DIVZERO   "Division by zero."                                           // User tried to divide a number by zero
#define CTR_ERR_BOUNDS    "Index out of bounds."                                        // User tried to access an element outside the scope of the object/list/map/file or other collection
#define CTR_ERR_REGEX     "Could not compile regular expression."                       // Not in use (yet)
#define CTR_ERR_SIPHKEY   "Key must be exactly 16 bytes long."                          // Siphash key needs to be 16 bytes (user applied hash: with wrong key)
#define CTR_SYM_OBJECT    "[Object]"                                                    // Default string/text representation of an object
#define CTR_SYM_BLOCK     "[Block]"                                                     // Default string/text representation of a task/block of code
#define CTR_SYM_FILE      "[File (no path)]"                                            // Default string/text representation of a File object with no path
#define CTR_ERR_OPEN      "Unable to open: %s."                                         // Unable to open file/device %s
#define CTR_ERR_DELETE    "Unable to delete."                                           // Unable to delete file/object/device
#define CTR_ERR_FOPENED   "File has already been opened."                               // File has already been opened
#define CTR_ERR_SEEK      "Seek failed."                                                // File seek operation failed
#define CTR_ERR_LOCK      "Unable to lock."                                             // Unable to acquire a lock on object
#define CTR_ERR_RET       "Invalid return expression.\n"                                // The return statement was invalid
#define CTR_ERR_SEND      "Cannot send message to receiver of type: %d \n"              // Invalid message to certain object
#define CTR_ERR_KEYINX    "Key index exhausted."                                        // Key index exhausted (too many arguments in message)
#define CTR_ERR_ANOMALY   "Detected message level anomaly.\n"                           // Structure of message chain was corrupted
#define CTR_ERR_UNCAUGHT  "Uncatched error has occurred.\n"                             // Unhandled Exception
#define CTR_ERR_NODE      "Runtime Error. Invalid parse node: %d %s \n"                 // Unknown type of node encountered, probably corrupt AST in memory
#define CTR_ERR_MISSING   "Missing parse node\n"                                        // Corrupted memory, parse node is missing
#define CTR_ERR_FOPEN     "Error while opening the file.\n"                             // An error occurred during the opening of a file/device/object
#define CTR_ERR_RNUM      "Must return a number."                                       // The object/message requires the return of a Number
#define CTR_ERR_RSTR      "Must return a string."                                       // The object/message requires the return of a String/Text
#define CTR_ERR_RBOOL     "Must return a boolean."                                      // The object/message requires the return of a Boolean value
#define CTR_ERR_NESTING   "Too many nested calls."                                      // Too many nested calls, prob. recursion error
#define CTR_ERR_KNF       "Key not found: "                                             // Undefined var/key
#define CTR_ERR_ASSIGN    "Cannot assign to undefined variable: "                       // Undefined var/key
#define CTR_ERR_EXEC      "Unable to execute command."                                  // Error during execution of command
#define CTR_MSG_DSC_FILE  "file"                                                        // Filesystem description of a file
#define CTR_MSG_DSC_FLDR  "folder"                                                      // Filesystem description of a folder/directory
#define CTR_MSG_DSC_SLNK  "symbolic link"                                               // Filesystem description of a symbolic link
#define CTR_MSG_DSC_CDEV  "character device"                                            // Filesystem description of a character device file
#define CTR_MSG_DSC_BDEV  "block device"                                                // Filesystem description of a block device file
#define CTR_MSG_DSC_SOCK  "socket"                                                      // Filesystem description of a UNIX domain socket file
#define CTR_MSG_DSC_NPIP  "named pipe"                                                  // Filesystem description of a named pipe
#define CTR_MSG_DSC_OTHR  "other"                                                       // Filesystem description of a something else/other
#define CTR_MSG_DSC_TYPE  "type"                                                        // Filesystem file type
#define CTR_TERR_LMISMAT  "Language mismatch."                                          // The system expected an object/plugin/module in a diff. lang
#define CTR_TERR_LONG     "Translation error, message too long."                        // Translated message is too long
#define CTR_TERR_DICT     "Error opening dictionary."                                   // Unable to open a directory
#define CTR_TERR_KMISMAT  "Error: key mismatch %s %s on line %d\n"                      // Translation key misaligned in file
#define CTR_TERR_ELONG    "Dictionary entry too long."                                  // Dictionary entry is too long
#define CTR_TERR_AMWORD   "Ambiguous word in dictionary: %s."                           // Used the same entry/key multiple times
#define CTR_TERR_AMTRANS  "Ambiguous translation in dictionary: %s."                    // Used the same translation multiple times
#define CTR_TERR_TMISMAT  "Keyword/Binary mismatch:"                                    // Translated a keyword message as a binary message or vice-versa
#define CTR_TERR_BUFF     "Unable to copy translation to buffer."                       // Something went wrong while copying translation to buffer
#define CTR_TERR_WARN     "Warning: Not translated: "                                   // Missing translation
#define CTR_TERR_TOK      "Token length exceeds maximum buffer size."                   // Word/translation is too long
#define CTR_TERR_PART     "Part of keyword message token exceeds buffer limit."         // Part of word/translation (in a keyword message) is too long
#define CTR_TERR_COLONS   "Different no. of colons."                                    // Translated keyword message has a different no. of args than original
#define CTR_MSG_ERROR     "Error."                                                      // Generic error message
#define CTR_MERR_OOM      "Out of memory. Failed to allocate %lu bytes.\n"              // Out of memory with specification
#define CTR_MERR_MALLOC   "Out of memory. Failed to allocate %lu bytes (malloc failed). \n" // Out of memory with specification, indicating low-level issue
#define CTR_MERR_POOL     "Unable to allocate memory pool.\n"                           // Shared memory pool failed
#define CTR_STDDATEFRMT   "%m/%d/%Y %I:%M:%S %p"                                        // How to display a date + time in your native language
#define CTR_STDTIMEZONE   "UTC"                                                         // Default time zone for your language or country