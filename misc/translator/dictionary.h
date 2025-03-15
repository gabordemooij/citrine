// Annotated Dictionary Template File

// Instructions:
// 1.  Translate the phrase between the quotes ("")
// 2.  Check that the number of : in the phrase is the same
// 3.  Every phrase must be unique, no duplicates are allowed (also not in a different file)
// 4.  You may not use spaces in the translation, use a dash if necessary. No snake_case or camelCase please!
// 5.  The order of the : must remain the same "add:to:" cannot be translated with "to:add:"
// 7.  You may not put two : after each other ("blahblah::" is not allowed)
// 8.  If a phrase consists of just one character, its translation must also be just one character
// 9.  Please do not use the following symbols in your translation: { } ( ) , := <- except for the dedicated symbols
// 10. Please do not start your translation with a digit
// 11. Please do not use quotes in your translation
// 12. Please do not include an end-of-line symbol as part of your translation (most of the time this is a dot)
// 13. You may use any UTF-8 character you like, not just ASCII (so you can use native characters if needed)
// 14. Clean up: remove all the comments from the file as well as empty lines.
// 15. Do not change the order of the translations!
// 16. Do not omit any translation and do not add additional translations!
// 17. The result should not contain any English words between quotes. Translate every item except the symbols.


// These text fragments are used by Citrine to generate lists and maps,
// for instance after sending a string/text message to such an object, they
// must conform to the translations for map/lists otherwise string representations
// will be invalid and tests will fail.

#define CTR_DICT_CODEGEN_MAP_NEW                 "(List new) "
#define CTR_DICT_CODEGEN_MAP_PUT                 "put:"
#define CTR_DICT_CODEGEN_MAP_PUT_AT              " at:"
#define CTR_DICT_CODEGEN_ARRAY_NEW               "Sequence new "
#define CTR_DICT_CODEGEN_ARRAY_NEW_PUSH          "Sequence ← "

// End of line, 
// how to end a series of Citrine instructions, like a sentence in your native language
// Some languages use a different character to mark the end of a line or sentence.

#define CTR_DICT_END_OF_LINE     "."

// Core Objects

#define CTR_DICT_NIL             "None"                   // Represents nothing, emptyness or None 
#define CTR_DICT_BOOLEAN         "Boolean"               // Represents a the parent of a boolean objects
#define CTR_DICT_TRUE            "True"                  // Represents the boolean value for True or Yes
#define CTR_DICT_FALSE           "False"                 // Represents the boolean value for False or No
#define CTR_DICT_NUMBER          "Number"                // Represents the root object of all numbers
#define CTR_DICT_STRING          "Text"                  // Since Citrine has no concept of bytes, better translate this as Text
#define CTR_DICT_TASK            "Task"                  // Represents a block of code, we prefer the term Task
#define CTR_DICT_OBJECT          "Object"                // The root of all objects
#define CTR_DICT_ARRAY_OBJECT    "Sequence"              // An ordered sequence of elementes (Python: list, PHP: array)
#define CTR_DICT_MAP_OBJECT      "List"                  // An unordered key-value map (Python: dict, PHP: array)
#define CTR_DICT_PROGRAM         "Program"               // A reference to the currently running program itself
#define CTR_DICT_FILE            "File"                  // Represents the parent of all File objects
#define CTR_DICT_MOMENT          "Moment"                // Represents the parent of all time objects, i.e. Moment(s)


// These symbols are probably mostly the same in your language although
// cultural differences may apply. ASCII versions are supplied by the lexer (automatically).
// Some of these used to be icons/pictograms.

#define CTR_DICT_VAR_ICON                       ">>"    // Indicates an assignment to a variable
#define CTR_DICT_ME_ICON                        "self"  // Represents the object itself when sending a message (plays back logic of its own, followed by message)
#define CTR_DICT_MY_ICON                        "own"   // Represents a protected property of the object itself (followed by property name)
#define CTR_DICT_BULLET                          "~"    // A bullet of a list, adds a new element to the list
#define CTR_DICT_SYMBOL_EQUALS                   "="    // Value based comparison (we do not use ==) we use ≔ for assignment
#define CTR_DICT_PLUS                            "+"    // Plus sign
#define CTR_DICT_MINUS                           "-"    // Minus sign
#define CTR_DICT_MULTIPLIER                      "×"    // Multiplication sign (non-ASCII version)
#define CTR_DICT_DIVISION                        "÷"    // Division sign (non-ASCII version)
#define CTR_DICT_GREATER                         ">"    // Greater than sign
#define CTR_DICT_LESS                            "<"    // Less than sign
#define CTR_DICT_AT_SYMBOL                       "?"    // To get the element at a certain position in a list or map (? key or ? index)
#define CTR_DICT_PEN_ICON                        "Out"   // Represents standard output stream of a program, there is often no simple term for this
#define CTR_DICT_NEW_ARRAY_AND_PUSH_SYMBOL       "←"    // Creates a new list and pushes the first element (a ← 1 ; 2)
#define CTR_DICT_GREATER_OR_EQUAL_SYMBOL         "≥"    // Equal or greater than sign 
#define CTR_DICT_LESS_OR_EQUAL_SYMBOL            "≤"    // Less or equal than sign
#define CTR_DICT_UNEQUALS_SYMBOL                 "≠"    // Unequal sign


// Other messages

#define CTR_DICT_NEW             "new"                  // New-messages, creates a new instance (constructor)
#define CTR_DICT_EQUALS          "equals:"              // Checks equality of identity of objects (i.e. same object, not just same value)
#define CTR_DICT_AND             "and:"                 // Returns True if X and: Y are both True
#define CTR_DICT_OR              "or:"                  // Returns True if X or: Y are True
#define CTR_DICT_MODULO          "modulo:"              // 6 modulo: 5 = 1 (like % in JavaScript)
#define CTR_DICT_NOR             "nor:"                 // Returns True if X nor: Y (both X and Y are False)
#define CTR_DICT_ONDO            "on:do:"               // Adds a "method" (task) to an object, attached to a message (i.e., like: upon X do Y)
#define CTR_DICT_TYPE            "type"                 // Inquires about the type of object
#define CTR_DICT_ISNIL           "Nil?"                 // Returns True if recipient is Nil, otherwise False
#define CTR_DICT_MYSELF          "myself"               // Reference to the object itself
#define CTR_DICT_DO              "do"                   // Start a sequence of messages where the return values are ignored in favour of chaining
#define CTR_DICT_DONE            "done"                 // End the chain of messages (return values no longer ignored)
#define CTR_DICT_IFFALSE         "false:"               // Execute specified task if recipient is False (this is how IF-statements work in Citrine)
#define CTR_DICT_IFTRUE          "true:"                // Execute specified task if recipient is True (this is how IF-statements work in Citrine)
#define CTR_DICT_WHILE           "while:"               // Perform task until recipient task evaluates to False
#define CTR_DICT_MESSAGEARGS     "message:arguments:"   // Send message and arguments programmatically
#define CTR_DICT_MESSAGE         "message:"             // Send message without arguments programmatically
#define CTR_DICT_LEARN           "learn:means:"         // Teaches the recipient object that message X means Y (aliasing)
#define CTR_DICT_BREAK           "break"                // Breaks out of a loop
#define CTR_DICT_CONTINUE        "continue"             // Skips the current iteration of a loop
#define CTR_DICT_ELSE            "else:"                // Part of IF-statement, technically same as "false:" - but for readability
#define CTR_DICT_NOT             "not"                  // Negates the recipient value, i.e. False not = True
#define CTR_DICT_TONUMBER        "number"               // Converts recipient to a number, like int() in Python, intval() in PHP or cast: (int)
#define CTR_DICT_ITONUMBER       "international-number" // Convert recipient international number to national notation
#define CTR_DICT_TOSTRING        "string"               // Convert recipient to text object (or string, but we don't do bytes), like toString()
#define CTR_DICT_CHAR_AT_SET     "character:"           // Return the letter/character at the specified index in text/string
#define CTR_DICT_EITHEROR        "either:or:"           // Return one of two specified values depending on True/False recipient
#define CTR_DICT_BY_SET          "by:"                  // Combine two lists (cities by: people), like combine() (PHP) or zip() (Python)
#define CTR_DICT_FLOOR           "floor"                // Round to the lower number (2.5 floor --> 2)
#define CTR_DICT_CEIL            "ceil"                 // Round to the higher number (2.2 ceil --> 3 )
#define CTR_DICT_ROUND           "round"                // Round the number (2.5 round --> 3)
#define CTR_DICT_ABS             "absolute"             // Returns the absolute number, without sign
#define CTR_DICT_SQRT            "square-root"          // Square root of the number
#define CTR_DICT_POWER           "power:"               // 2 power: 8 = 256 (Math.pow(2,8) in JavaScript)
#define CTR_DICT_MIN             "minimum"              // Returns the lowest value in a list
#define CTR_DICT_MAX             "maximum"              // Returns the highest value in a list 
#define CTR_DICT_ODD             "odd?"                 // Returns True if recipient is an odd number
#define CTR_DICT_EVEN            "even?"                // Returns False if recipient is an even number
#define CTR_DICT_POS             "positive?"            // Returns True if recipient is a positive number
#define CTR_DICT_NEG             "negative?"            // Returns True if recipient is a negative number
#define CTR_DICT_TOBOOL          "boolean"              // Convert recipient to boolean value, i.e. True/False
#define CTR_DICT_RANDOM_NUM_BETWEEN  "between:and:"     // Returns a random number between two specified numbers 
#define CTR_DICT_LENGTH	         "length"               // Returns the length of a string/text
#define CTR_DICT_FROM_LENGTH     "from:length:"         // Returns a fragment of a string/text, starting from X with length Y
#define CTR_DICT_TRIM            "remove surrounding spaces" // Remove spaces, like trim()
#define CTR_DICT_AT              "at:"                  // Get/set element at position in list/map
#define CTR_DICT_POSITION_SET    "position:"            // Get/set element at position in list
#define CTR_DICT_INDEX_OF        "find:"                // Find in text (like indexOf() in JavaScript, find() in Python)
#define CTR_DICT_LAST_INDEX_OF   "last:"                // Find last index of subtext in text (lastIndexOf())
#define CTR_DICT_REPLACE_WITH    "replace:with:"        // Replace part of string/text with other string/text
#define CTR_DICT_SPLIT           "split:"               // Split string/text in multiple strings, like explode() in PHP or .split()
#define CTR_DICT_SKIP            "offset:"              // Returns substring starting at specified position
#define CTR_DICT_RUN             "run"                  // Starts the execution of the task you send this to
#define CTR_DICT_APPLY_TO        "apply:"               // Applies the receiving task to the specified object
#define CTR_DICT_APPLY_TO_AND    "apply:and:"           // Applies task to several objects
#define CTR_DICT_VALUE_SET       "set:value:"           // Inject value in variable in task
#define CTR_DICT_ERROR	         "error:"               // Emits an error (throws exception)
#define CTR_DICT_CATCH           "catch:"               // Catch error or exception
#define CTR_DICT_PUSH_SYMBOL     ";"                    // Add element to array/list
#define CTR_DICT_SHIFT           "shift"                // Shift element off list (takes the first element off the list and returns it)
#define CTR_DICT_COUNT           "count"                // Counts number of elements in the list
#define CTR_DICT_JOIN            "join:"                // Glues list together to form a text/string using specified string as glue (like implode())
#define CTR_DICT_POP             "pop"                  // Pops off last part of list, takes it away and returns it
#define CTR_DICT_SORT            "sort:"                // Sorts a list using specified task
#define CTR_DICT_PUT_AT          "put:at:"              // Put a value in a map, has to be like: put: object at: location
#define CTR_DICT_MAP             "map:"                 // Iterates over list and applies task, mapping
#define CTR_DICT_EACH            "each:"                // Same, like foreach()
#define CTR_DICT_WRITE           "write:"               // Writes to screen or disk
#define CTR_DICT_PATH            "path"                 // Returns path of file
#define CTR_DICT_READ            "read"                 // Reads from input or file (like file_get_contents())
#define CTR_DICT_APPEND          "append:"              // Adds specified object to the END of a list, sometimes translated as suffix:
#define CTR_DICT_PREPEND         "prepend:"             // Adds specified object to the BEGINNING of a list, sometimes translated as prefix:
#define CTR_DICT_EXISTS          "exists"               // Returns True if thing (file) exists
#define CTR_DICT_SIZE            "size"                 // Returns size of thing (file)
#define CTR_DICT_DELETE          "delete"               // Deletes thing (file or something else)
#define CTR_DICT_USE_SET         "use:"                 // Make use of external code/file/library (like import in Python), include(), require()
#define CTR_DICT_ARRAY           "list:"                // Create a list from/using specified object, i.e. File list: "/tmp"
#define CTR_DICT_END             "end"                  // End something, stop it, halt, like exit(), quit() (end of program)
#define CTR_DICT_ARGUMENT        "argument:"            // Obtains CLI argument by number
#define CTR_DICT_ARGUMENT_COUNT  "arguments"            // Get the number of CLI arguments
#define CTR_DICT_WAIT_FOR_PASSW  "ask password"         // Ask user for password (not showing typed chars)
#define CTR_DICT_WAIT_FOR_INPUT  "ask"                  // Ask user for input on CLI (and echo)
#define CTR_DICT_INPUT           "input"                // Get external input (stdin)
#define CTR_DICT_FLUSH           "flush"                // Flush output buffers
#define CTR_DICT_WAIT            "wait:"                // Wait for X seconds, like sleep(..) in Python
#define CTR_DICT_TIME            "time"                 // Return UNIX Epoch Time in seconds (if send to Moment-object)
#define CTR_DICT_RESPOND_TO      "respond:"             // Respond to unknown message, magic method like __call__ in Python
#define CTR_DICT_RESPOND_TO_AND  "respond:and:"         // Respond to unknown message, magic method, with 1 argument
#define CTR_DICT_SHELL           "shell:"               // Execute via shell/os, like exec(), shell_exec(), subprocess.run()
#define CTR_DICT_SWEEP           "clean memory"         // Perform manual garbage collection round
#define CTR_DICT_MEMORY_LIMIT    "memory:"              // Set upper memory limit
#define CTR_DICT_MEMORY          "memory"               // Get current memory limit
#define CTR_DICT_GC_MODE         "tidiness:"            // Change garbage collection mode, how aggressive?
#define CTR_DICT_HASH_WITH_KEY   "hash:"                // Return siphash of object (like sha, but always integer)
#define CTR_DICT_CHARACTERS      "characters"           // Return a list of chars from a string/text
#define CTR_DICT_QUALIFIER_SET   "qualifier:"           // Attach a qualification text/string object to a number, i.e. 2 "apples"
#define CTR_DICT_QUALIFIER       "qualifier"            // Return qualifier of a number (numbers can have qualifier, i.e. currency)
#define CTR_DICT_NEW_SET         "new:"                 // New with argument
#define CTR_DICT_HOUR            "hour"                 // Get the hour component of a Moment-object
#define CTR_DICT_HOUR_SET        "hour:"                // Set the hour component of a Moment-object
#define CTR_DICT_MINUTE_SET      "minute:"              // Set the minute component of a Moment-object
#define CTR_DICT_MINUTE          "minute"               // Get the minute component of a Moment-object
#define CTR_DICT_SECOND_SET      "second:"              // Set the second component of a Moment-object
#define CTR_DICT_SECOND          "second"               // Get the second component of a Moment-object
#define CTR_DICT_DAY             "day"                  // Get the day component of a Moment-object
#define CTR_DICT_DAY_SET         "day:"                 // Set the day component of a Moment-object
#define CTR_DICT_WEEK            "week"                 // Get the week component of a Moment-object
#define CTR_DICT_WEEK_SET        "week:"                // Set the week component of a Moment-object
#define CTR_DICT_MONTH           "month"                // Get the month component of a Moment-object
#define CTR_DICT_MONTH_SET       "month:"               // Set the month component of a Moment-object
#define CTR_DICT_YEAR            "year"                 // Get the year component of a Moment-object
#define CTR_DICT_RAW             "raw"                  // Get number without formatting, i.e. 2001 instead of 2.001 or 2,001
#define CTR_DICT_YEAR_SET        "year:"                // Set the year component of a Moment-object
#define CTR_DICT_WEEK_DAY        "weekday"              // Get weekday number, i.e. 1-7 (monday, tuesday..., depending on locale) 
#define CTR_DICT_YEAR_DAY        "yearday"              // Get day of the year
#define CTR_DICT_ZONE            "zone"                 // Get timezone, i.e. Europe/Amsterdam
#define CTR_DICT_ZONE_SET        "zone:"                // Set timezone
#define CTR_DICT_ADD_SET         "add:"                 // Add something to a number, changes the number itself instead of returning new number
#define CTR_DICT_SUBTRACT_SET    "subtract:"            // Subtract, changes the number itself instead of returning new number
#define CTR_DICT_MULTIPLIER_SET  "multiply by:"         // Multiply, changes the number itself instead of returning new number
#define CTR_DICT_DIVIDER_SET     "divide by:"           // Divide, changes the number itself instead of returning new number
#define CTR_DICT_LAST            "last"                 // Returns the last item of a list
#define CTR_DICT_FIRST           "first"                // Returns the first item of a list
#define CTR_DICT_SECOND_LAST     "second last"          // Returns the item before the last item in a list
#define CTR_DICT_FILL_WITH       "fill:with:"           // Fill elements of a list with objects of a certain kind
#define CTR_DICT_SPLICE          "replace:length:with:" // Replace a fragment of a list with something else, like splice()
#define CTR_DICT_VALUES          "values"               // Return values of a map as a list
#define CTR_DICT_ENTRIES         "entries"              // Return keys of a map as a list
#define CTR_DICT_COMPARE_SET     "compare:"             // Compare two objects, like strncmp()
#define CTR_DICT_HAS             "has:"                 // Returns True if recipient list contains specified element
#define CTR_DICT_COPY            "copy"                 // Make a copy of an object
#define CTR_DICT_CASE_DO         "case:do:"             // Like a switch() case... statement
#define CTR_DICT_STOP            "stop"                 // Add a newline to a text/string (telegram style)
#define CTR_DICT_ASCII_UPPER_CASE        "uppercase"    // Convert to UPPERCASE
#define CTR_DICT_ASCII_LOWER_CASE        "lowercase"    // Convert to lowercase
#define CTR_DICT_CONTAINS                "contains:"    // Like has: but with other words
#define CTR_DICT_APPLY_TO_AND_AND      "apply:and:and:" // Apply task to objects
#define CTR_DICT_ENVIRONMENT_VARIABLE    "setting:"     // Get value of environment variable
#define CTR_DICT_SET_ENVIRONMENT_VARIABLE"setting:value:"// Set value of environment variable
#define CTR_DICT_RESPOND_TO_AND_AND   "respond:and:and:" // Magic method, multiple arguments
#define CTR_DICT_RESPOND_TO_AND_AND_AND "respond:and:and:and:"// Magic method, multiple arguments
#define CTR_DICT_CURRENT_TASK   "this-task"       // Refers to current task/code block
#define CTR_DICT_NUM_DEC_SEP      "."                    // Decimal separator
#define CTR_DICT_NUM_THO_SEP      ","                    // Thousands separator
#define CTR_DICT_QUOT_OPEN        "‘"                    // Start quote to begin a string/text literal
#define CTR_DICT_QUOT_CLOSE       "’"                    // End quote to end a string/text literal, must be different
#define CTR_DICT_MESSAGE_CHAIN    ","                    // Comma or symbol to connect multiple statements, chaining
#define CTR_DICT_ASSIGN           "≔"                    // Assignment symbol ≔, do not use =, we use = to compare, we do not use ==
#define CTR_DICT_PAREN_OPEN       "("                    // (...) takes precedence in expressions (not translatable yet)
#define CTR_DICT_PAREN_CLOSE      ")"                    // (...) takes precedence in expressions (not translatable yet)
#define CTR_DICT_BLOCK_START      "{"                    // Start of a literal task/code block (not translatable yet)
#define CTR_DICT_BLOCK_END        "}"                    // End of a literal task/code block (not translatable yet)
#define CTR_DICT_PARAMETER_PREFIX ":"                    // Prefix for parameters in Citrine, change if natural language uses : as EOL (8-bit limit)
#define CTR_DICT_RETURN           "<-"                    // Return symbol (like ret or return in other languages)
#define CTR_DICT_CODE             "code"                 // Serialize string to re-executable code
#define CTR_DICT_PROCEDURE        "procedure"            // Execute loop only 1 time, like × 1
#define CTR_DICT_TOOBJECT         "object"               // Eval string/text in recipient object
#define CTR_DICT_PATH_OBJECT      "Path"                 // Allows for platform independent file path notation: Path my documents -> my/documents
#define CTR_DICT_CMD_OBJECT       "Command"              // Allows for platform independent, stringless CLI commands: Command ls mydir -> ls mydir
#define CTR_DICT_RECURSIVE        "recursive"            // Allows a recursive operation in object, otherwise protection kicks in

// Math
#define CTR_DICT_MATH_SIN                        "sin"   // sinus function (math)
#define CTR_DICT_MATH_COS                        "cos"   // cosinus function (math)
#define CTR_DICT_MATH_TAN                        "tan"   // tangus function (math)
#define CTR_DICT_MATH_ATAN                       "atan"  // atangus function (math)
#define CTR_DICT_MATH_LOG                        "log"   // logarithm function (math)