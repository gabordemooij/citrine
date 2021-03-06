#define CTR_MSG_LANG_CODE "ne"
#define CTR_MSG_WELCOME   "Citrine/NE\n"
#define CTR_MSG_COPYRIGHT "Gabor de Mooij ©, Licensed BSD\n"
#define CTR_MSG_USAGE_G   "पर्याप्त तर्कहरू छैनन्। उपयोग:: ctr -g file1.h file2.h"
#define CTR_MSG_USAGE_T   "पर्याप्त तर्कहरू छैनन्। उपयोग:: ctr -t d.dict program.ctr"
#define CTR_ERR_LEX        " %s  लाईनमा: %d ।"
#define CTR_ERR_TOKBUFF    "टोकन बफर बाहिर टोकनहरू २55 बाइट भन्दा बढी हुन सक्दैन"
#define CTR_ERR_OOM        "मेमोरी सकियो।"
#define CTR_ERR_SYNTAX     "पार्स त्रुटि अप्रत्याशित %s  ( %s : %d )"
#define CTR_ERR_LONG       "सन्देश धेरै लामो छ।"
#define CTR_ERR_EXP_COLON "अपेक्षित [:].\n"
#define CTR_ERR_EXP_MSG "अपेक्षित [सन्देश].\n"
#define CTR_ERR_EXP_PCLS  "अपेक्षित [)].\n"
#define CTR_ERR_EXP_DOT    "लाइनको अन्त्य हुने आशा"
#define CTR_ERR_EXP_KEY    "कुञ्जी सँधै सम्पत्ती नाममा पछ्याउनुपर्दछ!"
#define CTR_ERR_EXP_VAR    "पोइन्टि hand हात सँधै भेरिएबलको अनुसरण गर्नुपर्छ"
#define CTR_ERR_EXP_RCP    "सन्देश प्राप्तकर्ताको अपेक्षित"
#define CTR_ERR_EXP_MSG2   "प्रापकलाई कालोन पछि लाग्न सक्दैन।"
#define CTR_ERR_INV_LAS    "अवैध बायाँ कार्य"
#define CTR_ERR_EXP_BLK   "अपेक्षित [कोड]."
#define CTR_ERR_EXP_ARR   "अपेक्षित [सूची]."
#define CTR_ERR_EXP_NUM   "अपेक्षित [संख्या]."
#define CTR_ERR_EXP_STR   "अपेक्षित [स्ट्रिंग]."
#define CTR_ERR_DIVZERO    "शून्य बाट भाग।"
#define CTR_ERR_BOUNDS     "सीमा बाहिर सूचकांक।"
#define CTR_ERR_REGEX      "नियमित अभिव्यक्ति कम्पाइल गर्न सकेन।"
#define CTR_ERR_SIPHKEY    "कुञ्जी ठ्याक्कै १ by बाइट लामो हुनुपर्छ।"
#define CTR_SYM_OBJECT    "[Object]"
#define CTR_SYM_BLOCK     "[Block]"
#define CTR_SYM_FILE      "[File (no path)]"
#define CTR_ERR_OPEN       "खोल्न असमर्थ: %s ।"
#define CTR_ERR_DELETE     "हटाउन असमर्थ।"
#define CTR_ERR_FOPENED    "फाइल पहिले नै खोलिएको छ।"
#define CTR_ERR_SEEK       "खोजी असफल भयो।"
#define CTR_ERR_LOCK       "लक गर्न असमर्थ।"
#define CTR_ERR_RET        "अवैध फिर्ती अभिव्यक्ति।"
#define CTR_ERR_SEND       "प्रकारको प्रापकलाई सन्देश पठाउन सक्दैन: %d "
#define CTR_ERR_KEYINX     "कुञ्जी सूचकांक समाप्त।"
#define CTR_ERR_ANOMALY    "विसंगति पत्ता लाग्यो सन्देश स्तर।"
#define CTR_ERR_UNCAUGHT   "अनचेड त्रुटि देखा पर्‍यो।"
#define CTR_ERR_NODE       "रनटाइम त्रुटि अवैध पार्स नोड: %d  %s "
#define CTR_ERR_MISSING    "छुटेको पार्स नोड"
#define CTR_ERR_FOPEN      "फाइल खोल्दा त्रुटि भयो।"
#define CTR_ERR_RNUM  "फर्कनु पर्छ [संख्या].\n"
#define CTR_ERR_RSTR  "फर्कनु पर्छ [स्ट्रिंग].\n"
#define CTR_ERR_RBOOL "फर्कनु पर्छ [बुलियन].\n"
#define CTR_ERR_NESTING    "धेरै नै नेस्ट गरिएको कलहरू।"
#define CTR_ERR_KNF        "कुञ्जी भेटिएन:"
#define CTR_ERR_ASSIGN     "अपरिभाषित चरमा असाइन गर्न सकिँदैन:"
#define CTR_ERR_EXEC       "आदेश कार्यान्वयन गर्न असमर्थ।"
#define CTR_MSG_DSC_FILE  "file"
#define CTR_MSG_DSC_FLDR  "folder"
#define CTR_MSG_DSC_SLNK  "symbolic link"
#define CTR_MSG_DSC_CDEV  "character device"
#define CTR_MSG_DSC_BDEV  "block device"
#define CTR_MSG_DSC_SOCK  "socket"
#define CTR_MSG_DSC_NPIP  "named pipe"
#define CTR_MSG_DSC_OTHR  "other"
#define CTR_MSG_DSC_TYPE  "type"
#define CTR_TERR_LMISMAT  "Language mismatch."
#define CTR_TERR_LONG     "Translation error, message too long."
#define CTR_TERR_DICT     "Error opening dictionary."
#define CTR_TERR_KMISMAT  "Error: key mismatch %s %s on line %d\n"
#define CTR_TERR_ELONG    "Dictionary entry too long."
#define CTR_TERR_AMWORD   "Ambiguous word in dictionary: %s."
#define CTR_TERR_AMTRANS  "Ambiguous translation in dictionary: %s."
#define CTR_TERR_TMISMAT  "Keyword/Binary mismatch:"
#define CTR_TERR_BUFF     "Unable to copy translation to buffer."
#define CTR_TERR_WARN     "Warning: Not translated: "
#define CTR_TERR_TOK      "Token length exceeds maximum buffer size."
#define CTR_TERR_PART      "कीवर्ड सन्देश टोकनको अंशले बफर सीमा नाघ्यो।"
#define CTR_TERR_COLONS   "Different no. of colons."
#define CTR_MSG_ERROR      "त्रुटि"
#define CTR_MERR_OOM      "मेमोरी सकियो।  %lu  बाइट्स बाँडफाँड गर्न असफल।\n"
#define CTR_MERR_MALLOC   "मेमोरी सकियो।  %lu  बाइट्स बाँडफाँड गर्न असफल। (malloc). \n"
#define CTR_MERR_POOL     "Unable to allocate memory pool.\n"
#define CTR_STDDATEFRMT   "%Y-%m-%d %H:%M:%S"
#define CTR_STDTIMEZONE   "UTC"