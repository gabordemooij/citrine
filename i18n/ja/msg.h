#define CTR_MSG_LANG_CODE "ja"
#define CTR_MSG_WELCOME   "Citrine/JA\n"
#define CTR_MSG_COPYRIGHT "Gabor de Mooij ©, Licensed BSD\n"
#define CTR_MSG_USAGE_G   "引数が足りません。使用法：: ctr -g file1.h file2.h"
#define CTR_MSG_USAGE_T   "引数が足りません。使用法：: ctr -t d.dict program.ctr"
#define CTR_ERR_LEX        "%sはオンラインです：%d。"
#define CTR_ERR_TOKBUFF    "トークンバッファが不足しています。トークンは255バイトを超えることはできません"
#define CTR_ERR_OOM        "メモリ不足です。"
#define CTR_ERR_SYNTAX     "解析エラー、予期しない%s（%s：%d）"
#define CTR_ERR_LONG       "メッセージが長すぎます。"
#define CTR_ERR_EXP_COLON "期待される [:].\n"
#define CTR_ERR_EXP_MSG "期待される [メッセージ：__].\n"
#define CTR_ERR_EXP_PCLS  "期待される [)].\n"
#define CTR_ERR_EXP_DOT    "予想される行末"
#define CTR_ERR_EXP_KEY    "キーの後には必ずプロパティ名を付ける必要があります！"
#define CTR_ERR_EXP_VAR    "ポインティングハンドの後には常に変数が必要です！"
#define CTR_ERR_EXP_RCP    "メッセージの受信者が必要です。"
#define CTR_ERR_EXP_MSG2   "受信者の後にコロンを続けることはできません。"
#define CTR_ERR_INV_LAS    "左側の割り当てが無効です。"
#define CTR_ERR_EXP_BLK   "期待される [コード]."
#define CTR_ERR_EXP_ARR   "期待される [リスト]."
#define CTR_ERR_EXP_NUM   "期待される [_数]."
#define CTR_ERR_EXP_STR   "期待される [ストリング]."
#define CTR_ERR_DIVZERO    "ゼロによる除算。"
#define CTR_ERR_BOUNDS     "範囲外のインデックス。"
#define CTR_ERR_REGEX      "正規表現をコンパイルできませんでした。"
#define CTR_ERR_SIPHKEY    "キーは正確に16バイトの長さでなければなりません。"
#define CTR_SYM_OBJECT    "[Object]"
#define CTR_SYM_BLOCK     "[Block]"
#define CTR_SYM_FILE      "[File (no path)]"
#define CTR_ERR_OPEN       "開けません：%s。"
#define CTR_ERR_DELETE     "削除できません。"
#define CTR_ERR_FOPENED    "ファイルは既に開かれています。"
#define CTR_ERR_SEEK       "シークに失敗しました。"
#define CTR_ERR_LOCK       "ロックできません。"
#define CTR_ERR_RET        "無効な戻り式です。"
#define CTR_ERR_SEND       "タイプ%dの受信者にメッセージを送信できません"
#define CTR_ERR_KEYINX     "キーインデックスが使い果たされました。"
#define CTR_ERR_ANOMALY    "メッセージレベルの異常を検出しました。"
#define CTR_ERR_UNCAUGHT   "キャッチされないエラーが発生しました。"
#define CTR_ERR_NODE       "ランタイムエラー。無効な解析ノード：%d%s"
#define CTR_ERR_MISSING    "解析ノードがありません"
#define CTR_ERR_FOPEN      "ファイルを開くときにエラーが発生しました。"
#define CTR_ERR_RNUM  "戻る必要があります [_数].\n"
#define CTR_ERR_RSTR  "戻る必要があります [ストリング].\n"
#define CTR_ERR_RBOOL "戻る必要があります [ブール].\n"
#define CTR_ERR_NESTING    "ネストされた呼び出しが多すぎます。"
#define CTR_ERR_KNF        "キーが見つかりません："
#define CTR_ERR_ASSIGN     "未定義の変数に割り当てることはできません："
#define CTR_ERR_EXEC       "コマンドを実行できません。"
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
#define CTR_TERR_PART      "キーワードメッセージトークンの一部がバッファ制限を超えています。"
#define CTR_TERR_COLONS   "Different no. of colons."
#define CTR_MSG_ERROR      "エラー。"
#define CTR_MERR_OOM      "メモリ不足です。 %luバイトの割り当てに失敗しました。\n"
#define CTR_MERR_MALLOC   "メモリ不足です。 %luバイトの割り当てに失敗しました。 (malloc). \n"
#define CTR_MERR_POOL     "Unable to allocate memory pool.\n"
#define CTR_STDDATEFRMT   "%Y-%m-%d %H:%M:%S"
#define CTR_STDTIMEZONE   "UTC"