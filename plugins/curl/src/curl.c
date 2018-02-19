#include <citrine/citrine.h>
#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define CTR_OBJECT_RESOURCE_CURL 54

/**
 * @internal
 *
 * Throws error with object, message, and a description
 *
 **/
void ctr_curl_internal_error(ctr_object* myself, char* msg, char* desc) {
	char errstr[80];
	sprintf(errstr, "%s ~ %s", msg, desc);
	ctr_argument errArgs;
	errArgs.object = ctr_build_string_from_cstring(errstr);
	ctr_block_error(myself, &errArgs);
}

/**
 * @internal
 *
 * Send `type` message to object and return citrine string
 *
 **/
ctr_object *ctr_internal_otype(ctr_object *o) {
	ctr_argument arg;
	arg.object = ctr_build_string_from_cstring("type");
	return ctr_object_message(o, &arg);
}

/**
 * Curl new.
 *
 * Creates new Curl handle via curl_easy_init
 *
 **/
ctr_object* ctr_curl_new(ctr_object* myself, ctr_argument* argumentList) {

	ctr_object* curlObjectInstance = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	curlObjectInstance->link = myself;

	curlObjectInstance->info.type = CTR_OBJECT_TYPE_OTEX;

	ctr_resource* rsrc = ctr_heap_allocate(sizeof(ctr_resource));
	rsrc->type = CTR_OBJECT_RESOURCE_CURL;
	rsrc->ptr = curl_easy_init();

	curlObjectInstance->value.rvalue = rsrc;

	return curlObjectInstance;
}

/**
 * [Curl] cleanup.
 *
 * Destroy Curl handle in resource
 *
 **/
ctr_object* ctr_curl_cleanup(ctr_object* myself, ctr_argument* argumentList) {

	curl_easy_cleanup((CURL *)myself->value.rvalue->ptr);

	ctr_heap_free(myself->value.rvalue);

	return myself;
}

/**
 * [Curl] type.
 *
 * Returns 'Curl' as the type string
 *
 **/
ctr_object* ctr_curl_type(ctr_object* myself, ctr_argument* argumentList) {

	return ctr_build_string_from_cstring("Curl");
}

/**
 *
 * [Curl] respondTo: [string] with: [Object]
 *
 * Default response sets Curl option
 *
 **/
ctr_object* ctr_curl_respondto(ctr_object* myself, ctr_argument* argumentList) {

	ctr_object* msgObj = ctr_internal_cast2string(argumentList->object);
	char* msg = ctr_heap_allocate_cstring(msgObj);
	msg[strlen(msg)-1] = 0;

	ctr_object *argObj = argumentList->next->object;
	//ctr_object *valObj;
	void* val = NULL;

	char msg_and_args[80];
	CURLoption opt;

	unsigned int cancel = 0;
	unsigned int ctrstr = 0;

	switch (argObj->info.type) {
		case CTR_OBJECT_TYPE_OTNIL:
			val = NULL;
			break;
		case CTR_OBJECT_TYPE_OTBOOL:
			val = &argObj->value.bvalue;
			break;
		case CTR_OBJECT_TYPE_OTNUMBER:
			val = &argObj->value.nvalue;
			break;
		case CTR_OBJECT_TYPE_OTSTRING:
			val = ctr_heap_allocate_cstring(argObj);
			ctrstr = 1;
			break;
		case CTR_OBJECT_TYPE_OTNATFUNC:
		case CTR_OBJECT_TYPE_OTBLOCK:
			argumentList->next->object = ctr_block_run(argObj, NULL, myself);
			ctr_curl_respondto(myself, argumentList);
			break;
		case CTR_OBJECT_TYPE_OTOBJECT:
		case CTR_OBJECT_TYPE_OTARRAY:
		case CTR_OBJECT_TYPE_OTMISC:
		case CTR_OBJECT_TYPE_OTEX:

			sprintf(msg_and_args, "Curl %s: '%s'",
				msg, (char *)ctr_object_type(argObj, NULL)->value.svalue);

			ctr_curl_internal_error(myself, msg_and_args, "Cannot set this value");

			return myself;
	}

	if (val == NULL) { cancel = 1; }
	else if (strcasecmp(msg, "url") == 0) { opt = CURLOPT_URL; }
	else if (strcasecmp(msg, "path_as_is") == 0) { opt = CURLOPT_PATH_AS_IS; }
	else if (strcasecmp(msg, "proxy") == 0) { opt = CURLOPT_PROXY; }
	else if (strcasecmp(msg, "pre_proxy") == 0) { opt = CURLOPT_PRE_PROXY; }
	else if (strcasecmp(msg, "proxyport") == 0) { opt = CURLOPT_PROXYPORT; }
	else if (strcasecmp(msg, "proxytype") == 0) { opt = CURLOPT_PROXYTYPE; }
	else if (strcasecmp(msg, "noproxy") == 0) { opt = CURLOPT_NOPROXY; }
	else if (strcasecmp(msg, "httpproxytunnel") == 0) { opt = CURLOPT_HTTPPROXYTUNNEL; }
	else if (strcasecmp(msg, "connect_to") == 0) { opt = CURLOPT_CONNECT_TO; }
	else if (strcasecmp(msg, "socks5_gssapi_service") == 0) { opt = CURLOPT_SOCKS5_GSSAPI_SERVICE; }
	else if (strcasecmp(msg, "socks5_gssapi_nec") == 0) { opt = CURLOPT_SOCKS5_GSSAPI_NEC; }
	else if (strcasecmp(msg, "proxy_service_name") == 0) { opt = CURLOPT_PROXY_SERVICE_NAME; }
	else if (strcasecmp(msg, "service_name") == 0) { opt = CURLOPT_SERVICE_NAME; }
	else if (strcasecmp(msg, "interface") == 0) { opt = CURLOPT_INTERFACE; }
	else if (strcasecmp(msg, "localport") == 0) { opt = CURLOPT_LOCALPORT; }
	else if (strcasecmp(msg, "localportrange") == 0) { opt = CURLOPT_LOCALPORTRANGE; }
	else if (strcasecmp(msg, "dns_cache_timeout") == 0) { opt = CURLOPT_DNS_CACHE_TIMEOUT; }
	else if (strcasecmp(msg, "dns_use_global_cache") == 0) { opt = CURLOPT_DNS_USE_GLOBAL_CACHE; }
	else if (strcasecmp(msg, "buffersize") == 0) { opt = CURLOPT_BUFFERSIZE; }
	else if (strcasecmp(msg, "port") == 0) { opt = CURLOPT_PORT; }
	else if (strcasecmp(msg, "tcp_fastopen") == 0) { opt = CURLOPT_TCP_FASTOPEN; }
	else if (strcasecmp(msg, "tcp_nodelay") == 0) { opt = CURLOPT_TCP_NODELAY; }
	else if (strcasecmp(msg, "address_scope") == 0) { opt = CURLOPT_ADDRESS_SCOPE; }
	else if (strcasecmp(msg, "tcp_keepalive") == 0) { opt = CURLOPT_TCP_KEEPALIVE; }
	else if (strcasecmp(msg, "tcp_keepidle") == 0) { opt = CURLOPT_TCP_KEEPIDLE; }
	else if (strcasecmp(msg, "tcp_keepintvl") == 0) { opt = CURLOPT_TCP_KEEPINTVL; }
	else if (strcasecmp(msg, "unix_socket_path") == 0) { opt = CURLOPT_UNIX_SOCKET_PATH; }
	else if (strcasecmp(msg, "abstract_unix_socket") == 0) { opt = CURLOPT_ABSTRACT_UNIX_SOCKET; }
	else if (strcasecmp(msg, "netrc") == 0) { opt = CURLOPT_NETRC; }
	else if (strcasecmp(msg, "netrc_file") == 0) { opt = CURLOPT_NETRC_FILE; }
	else if (strcasecmp(msg, "userpwd") == 0) { opt = CURLOPT_USERPWD; }
	else if (strcasecmp(msg, "proxyuserpwd") == 0) { opt = CURLOPT_PROXYUSERPWD; }
	else if (strcasecmp(msg, "username") == 0) { opt = CURLOPT_USERNAME; }
	else if (strcasecmp(msg, "password") == 0) { opt = CURLOPT_PASSWORD; }
	else if (strcasecmp(msg, "login_options") == 0) { opt = CURLOPT_LOGIN_OPTIONS; }
	else if (strcasecmp(msg, "proxyusername") == 0) { opt = CURLOPT_PROXYUSERNAME; }
	else if (strcasecmp(msg, "proxypassword") == 0) { opt = CURLOPT_PROXYPASSWORD; }
	else if (strcasecmp(msg, "httpauth") == 0) { opt = CURLOPT_HTTPAUTH; }
	else if (strcasecmp(msg, "tlsauth_username") == 0) { opt = CURLOPT_TLSAUTH_USERNAME; }
	else if (strcasecmp(msg, "proxy_tlsauth_username") == 0) { opt = CURLOPT_PROXY_TLSAUTH_USERNAME; }
	else if (strcasecmp(msg, "tlsauth_password") == 0) { opt = CURLOPT_TLSAUTH_PASSWORD; }
	else if (strcasecmp(msg, "proxy_tlsauth_password") == 0) { opt = CURLOPT_PROXY_TLSAUTH_PASSWORD; }
	else if (strcasecmp(msg, "tlsauth_type") == 0) { opt = CURLOPT_TLSAUTH_TYPE; }
	else if (strcasecmp(msg, "proxy_tlsauth_type") == 0) { opt = CURLOPT_PROXY_TLSAUTH_TYPE; }
	else if (strcasecmp(msg, "proxyauth") == 0) { opt = CURLOPT_PROXYAUTH; }
	else if (strcasecmp(msg, "sasl_ir") == 0) { opt = CURLOPT_SASL_IR; }
	else if (strcasecmp(msg, "xoauth2_bearer") == 0) { opt = CURLOPT_XOAUTH2_BEARER; }
	else if (strcasecmp(msg, "autoreferer") == 0) { opt = CURLOPT_AUTOREFERER; }
	else if (strcasecmp(msg, "accept_encoding") == 0) { opt = CURLOPT_ACCEPT_ENCODING; }
	else if (strcasecmp(msg, "transfer_encoding") == 0) { opt = CURLOPT_TRANSFER_ENCODING; }
	else if (strcasecmp(msg, "followlocation") == 0) { opt = CURLOPT_FOLLOWLOCATION; }
	else if (strcasecmp(msg, "unrestricted_auth") == 0) { opt = CURLOPT_UNRESTRICTED_AUTH; }
	else if (strcasecmp(msg, "maxredirs") == 0) { opt = CURLOPT_MAXREDIRS; }
	else if (strcasecmp(msg, "postredir") == 0) { opt = CURLOPT_POSTREDIR; }
	else if (strcasecmp(msg, "put") == 0) { opt = CURLOPT_PUT; }
	else if (strcasecmp(msg, "post") == 0) { opt = CURLOPT_POST; }
	else if (strcasecmp(msg, "postfields") == 0) { opt = CURLOPT_POSTFIELDS; }
	else if (strcasecmp(msg, "postfieldsize") == 0) { opt = CURLOPT_POSTFIELDSIZE; }
	else if (strcasecmp(msg, "postfieldsize_large") == 0) { opt = CURLOPT_POSTFIELDSIZE_LARGE; }
	else if (strcasecmp(msg, "copypostfields") == 0) { opt = CURLOPT_COPYPOSTFIELDS; }
	else if (strcasecmp(msg, "httppost") == 0) { opt = CURLOPT_HTTPPOST; }
	else if (strcasecmp(msg, "referer") == 0) { opt = CURLOPT_REFERER; }
	else if (strcasecmp(msg, "useragent") == 0) { opt = CURLOPT_USERAGENT; }
	else if (strcasecmp(msg, "httpheader") == 0) { opt = CURLOPT_HTTPHEADER; }
	else if (strcasecmp(msg, "headeropt") == 0) { opt = CURLOPT_HEADEROPT; }
	else if (strcasecmp(msg, "proxyheader") == 0) { opt = CURLOPT_PROXYHEADER; }
	else if (strcasecmp(msg, "http200aliases") == 0) { opt = CURLOPT_HTTP200ALIASES; }
	else if (strcasecmp(msg, "cookie") == 0) { opt = CURLOPT_COOKIE; }
	else if (strcasecmp(msg, "cookiefile") == 0) { opt = CURLOPT_COOKIEFILE; }
	else if (strcasecmp(msg, "cookiejar") == 0) { opt = CURLOPT_COOKIEJAR; }
	else if (strcasecmp(msg, "cookiesession") == 0) { opt = CURLOPT_COOKIESESSION; }
	else if (strcasecmp(msg, "cookielist") == 0) { opt = CURLOPT_COOKIELIST; }
	else if (strcasecmp(msg, "httpget") == 0) { opt = CURLOPT_HTTPGET; }
	else if (strcasecmp(msg, "http_version") == 0) { opt = CURLOPT_HTTP_VERSION; }
	else if (strcasecmp(msg, "ignore_content_length") == 0) { opt = CURLOPT_IGNORE_CONTENT_LENGTH; }
	else if (strcasecmp(msg, "http_content_decoding") == 0) { opt = CURLOPT_HTTP_CONTENT_DECODING; }
	else if (strcasecmp(msg, "http_transfer_decoding") == 0) { opt = CURLOPT_HTTP_TRANSFER_DECODING; }
	else if (strcasecmp(msg, "expect_100_timeout_ms") == 0) { opt = CURLOPT_EXPECT_100_TIMEOUT_MS; }
	else if (strcasecmp(msg, "pipewait") == 0) { opt = CURLOPT_PIPEWAIT; }
	else if (strcasecmp(msg, "stream_depends") == 0) { opt = CURLOPT_STREAM_DEPENDS; }
	else if (strcasecmp(msg, "stream_depends_e") == 0) { opt = CURLOPT_STREAM_DEPENDS_E; }
	else if (strcasecmp(msg, "stream_weight") == 0) { opt = CURLOPT_STREAM_WEIGHT; }
	else if (strcasecmp(msg, "mail_from") == 0) { opt = CURLOPT_MAIL_FROM; }
	else if (strcasecmp(msg, "mail_rcpt") == 0) { opt = CURLOPT_MAIL_RCPT; }
	else if (strcasecmp(msg, "mail_auth") == 0) { opt = CURLOPT_MAIL_AUTH; }
	else if (strcasecmp(msg, "tftp_blksize") == 0) { opt = CURLOPT_TFTP_BLKSIZE; }
	else if (strcasecmp(msg, "tftp_no_options") == 0) { opt = CURLOPT_TFTP_NO_OPTIONS; }
	else if (strcasecmp(msg, "ftpport") == 0) { opt = CURLOPT_FTPPORT; }
	else if (strcasecmp(msg, "quote") == 0) { opt = CURLOPT_QUOTE; }
	else if (strcasecmp(msg, "postquote") == 0) { opt = CURLOPT_POSTQUOTE; }
	else if (strcasecmp(msg, "prequote") == 0) { opt = CURLOPT_PREQUOTE; }
	else if (strcasecmp(msg, "append") == 0) { opt = CURLOPT_APPEND; }
	else if (strcasecmp(msg, "ftp_use_eprt") == 0) { opt = CURLOPT_FTP_USE_EPRT; }
	else if (strcasecmp(msg, "ftp_use_epsv") == 0) { opt = CURLOPT_FTP_USE_EPSV; }
	else if (strcasecmp(msg, "ftp_use_pret") == 0) { opt = CURLOPT_FTP_USE_PRET; }
	else if (strcasecmp(msg, "ftp_create_missing_dirs") == 0) { opt = CURLOPT_FTP_CREATE_MISSING_DIRS; }
	else if (strcasecmp(msg, "ftp_response_timeout") == 0) { opt = CURLOPT_FTP_RESPONSE_TIMEOUT; }
	else if (strcasecmp(msg, "ftp_alternative_to_user") == 0) { opt = CURLOPT_FTP_ALTERNATIVE_TO_USER; }
	else if (strcasecmp(msg, "ftp_skip_pasv_ip") == 0) { opt = CURLOPT_FTP_SKIP_PASV_IP; }
	else if (strcasecmp(msg, "ftpsslauth") == 0) { opt = CURLOPT_FTPSSLAUTH; }
	else if (strcasecmp(msg, "ftp_ssl_ccc") == 0) { opt = CURLOPT_FTP_SSL_CCC; }
	else if (strcasecmp(msg, "ftp_account") == 0) { opt = CURLOPT_FTP_ACCOUNT; }
	else if (strcasecmp(msg, "ftp_filemethod") == 0) { opt = CURLOPT_FTP_FILEMETHOD; }
       	else if (strcasecmp(msg, "rtsp_request") == 0) { opt = CURLOPT_RTSP_REQUEST; }
	else if (strcasecmp(msg, "rtsp_session_id") == 0) { opt = CURLOPT_RTSP_SESSION_ID; }
	else if (strcasecmp(msg, "rtsp_stream_uri") == 0) { opt = CURLOPT_RTSP_STREAM_URI; }
	else if (strcasecmp(msg, "rtsp_transport") == 0) { opt = CURLOPT_RTSP_TRANSPORT; }
	else if (strcasecmp(msg, "rtsp_client_cseq") == 0) { opt = CURLOPT_RTSP_CLIENT_CSEQ; }
	else if (strcasecmp(msg, "rtsp_server_cseq") == 0) { opt = CURLOPT_RTSP_SERVER_CSEQ; }
       	else if (strcasecmp(msg, "transfertext") == 0) { opt = CURLOPT_TRANSFERTEXT; }
	else if (strcasecmp(msg, "proxy_transfer_mode") == 0) { opt = CURLOPT_PROXY_TRANSFER_MODE; }
	else if (strcasecmp(msg, "crlf") == 0) { opt = CURLOPT_CRLF; }
	else if (strcasecmp(msg, "range") == 0) { opt = CURLOPT_RANGE; }
	else if (strcasecmp(msg, "resume_from") == 0) { opt = CURLOPT_RESUME_FROM; }
	else if (strcasecmp(msg, "resume_from_large") == 0) { opt = CURLOPT_RESUME_FROM_LARGE; }
	else if (strcasecmp(msg, "customrequest") == 0) { opt = CURLOPT_CUSTOMREQUEST; }
	else if (strcasecmp(msg, "filetime") == 0) { opt = CURLOPT_FILETIME; }
	else if (strcasecmp(msg, "dirlistonly") == 0) { opt = CURLOPT_DIRLISTONLY; }
	else if (strcasecmp(msg, "nobody") == 0) { opt = CURLOPT_NOBODY; }
	else if (strcasecmp(msg, "infilesize") == 0) { opt = CURLOPT_INFILESIZE; }
	else if (strcasecmp(msg, "infilesize_large") == 0) { opt = CURLOPT_INFILESIZE_LARGE; }
	else if (strcasecmp(msg, "upload") == 0) { opt = CURLOPT_UPLOAD; }
	else if (strcasecmp(msg, "maxfilesize") == 0) { opt = CURLOPT_MAXFILESIZE; }
	else if (strcasecmp(msg, "maxfilesize_large") == 0) { opt = CURLOPT_MAXFILESIZE_LARGE; }
	else if (strcasecmp(msg, "timecondition") == 0) { opt = CURLOPT_TIMECONDITION; }
	else if (strcasecmp(msg, "timevalue") == 0) { opt = CURLOPT_TIMEVALUE; }
       	else if (strcasecmp(msg, "timeout") == 0) { opt = CURLOPT_TIMEOUT; }
	else if (strcasecmp(msg, "timeout_ms") == 0) { opt = CURLOPT_TIMEOUT_MS; }
	else if (strcasecmp(msg, "low_speed_limit") == 0) { opt = CURLOPT_LOW_SPEED_LIMIT; }
	else if (strcasecmp(msg, "low_speed_time") == 0) { opt = CURLOPT_LOW_SPEED_TIME; }
	else if (strcasecmp(msg, "max_send_speed_large") == 0) { opt = CURLOPT_MAX_SEND_SPEED_LARGE; }
	else if (strcasecmp(msg, "max_recv_speed_large") == 0) { opt = CURLOPT_MAX_RECV_SPEED_LARGE; }
	else if (strcasecmp(msg, "maxconnects") == 0) { opt = CURLOPT_MAXCONNECTS; }
	else if (strcasecmp(msg, "fresh_connect") == 0) { opt = CURLOPT_FRESH_CONNECT; }
	else if (strcasecmp(msg, "forbid_reuse") == 0) { opt = CURLOPT_FORBID_REUSE; }
	else if (strcasecmp(msg, "connecttimeout") == 0) { opt = CURLOPT_CONNECTTIMEOUT; }
	else if (strcasecmp(msg, "connecttimeout_ms") == 0) { opt = CURLOPT_CONNECTTIMEOUT_MS; }
	else if (strcasecmp(msg, "ipresolve") == 0) { opt = CURLOPT_IPRESOLVE; }
	else if (strcasecmp(msg, "connect_only") == 0) { opt = CURLOPT_CONNECT_ONLY; }
	else if (strcasecmp(msg, "use_ssl") == 0) { opt = CURLOPT_USE_SSL; }
	else if (strcasecmp(msg, "resolve") == 0) { opt = CURLOPT_RESOLVE; }
	else if (strcasecmp(msg, "dns_interface") == 0) { opt = CURLOPT_DNS_INTERFACE; }
	else if (strcasecmp(msg, "dns_local_ip4") == 0) { opt = CURLOPT_DNS_LOCAL_IP4; }
	else if (strcasecmp(msg, "dns_local_ip6") == 0) { opt = CURLOPT_DNS_LOCAL_IP6; }
	else if (strcasecmp(msg, "dns_servers") == 0) { opt = CURLOPT_DNS_SERVERS; }
	else if (strcasecmp(msg, "accepttimeout_ms") == 0) { opt = CURLOPT_ACCEPTTIMEOUT_MS; }
       	else if (strcasecmp(msg, "sslcert") == 0) { opt = CURLOPT_SSLCERT; }
	else if (strcasecmp(msg, "proxy_sslcert") == 0) { opt = CURLOPT_PROXY_SSLCERT; }
	else if (strcasecmp(msg, "sslcerttype") == 0) { opt = CURLOPT_SSLCERTTYPE; }
	else if (strcasecmp(msg, "proxy_sslcerttype") == 0) { opt = CURLOPT_PROXY_SSLCERTTYPE; }
	else if (strcasecmp(msg, "sslkey") == 0) { opt = CURLOPT_SSLKEY; }
	else if (strcasecmp(msg, "proxy_sslkey") == 0) { opt = CURLOPT_PROXY_SSLKEY; }
	else if (strcasecmp(msg, "sslkeytype") == 0) { opt = CURLOPT_SSLKEYTYPE; }
	else if (strcasecmp(msg, "proxy_sslkeytype") == 0) { opt = CURLOPT_PROXY_SSLKEYTYPE; }
	else if (strcasecmp(msg, "keypasswd") == 0) { opt = CURLOPT_KEYPASSWD; }
	else if (strcasecmp(msg, "proxy_keypasswd") == 0) { opt = CURLOPT_PROXY_KEYPASSWD; }
	else if (strcasecmp(msg, "ssl_enable_alpn") == 0) { opt = CURLOPT_SSL_ENABLE_ALPN; }
	else if (strcasecmp(msg, "ssl_enable_npn") == 0) { opt = CURLOPT_SSL_ENABLE_NPN; }
	else if (strcasecmp(msg, "sslengine") == 0) { opt = CURLOPT_SSLENGINE; }
	else if (strcasecmp(msg, "sslengine_default") == 0) { opt = CURLOPT_SSLENGINE_DEFAULT; }
	else if (strcasecmp(msg, "ssl_falsestart") == 0) { opt = CURLOPT_SSL_FALSESTART; }
	else if (strcasecmp(msg, "sslversion") == 0) { opt = CURLOPT_SSLVERSION; }
	else if (strcasecmp(msg, "proxy_sslversion") == 0) { opt = CURLOPT_PROXY_SSLVERSION; }
	else if (strcasecmp(msg, "ssl_verifyhost") == 0) { opt = CURLOPT_SSL_VERIFYHOST; }
	else if (strcasecmp(msg, "proxy_ssl_verifyhost") == 0) { opt = CURLOPT_PROXY_SSL_VERIFYHOST; }
	else if (strcasecmp(msg, "ssl_verifypeer") == 0) { opt = CURLOPT_SSL_VERIFYPEER; }
	else if (strcasecmp(msg, "proxy_ssl_verifypeer") == 0) { opt = CURLOPT_PROXY_SSL_VERIFYPEER; }
	else if (strcasecmp(msg, "ssl_verifystatus") == 0) { opt = CURLOPT_SSL_VERIFYSTATUS; }
	else if (strcasecmp(msg, "cainfo") == 0) { opt = CURLOPT_CAINFO; }
	else if (strcasecmp(msg, "proxy_cainfo") == 0) { opt = CURLOPT_PROXY_CAINFO; }
	else if (strcasecmp(msg, "issuercert") == 0) { opt = CURLOPT_ISSUERCERT; }
	else if (strcasecmp(msg, "capath") == 0) { opt = CURLOPT_CAPATH; }
	else if (strcasecmp(msg, "proxy_capath") == 0) { opt = CURLOPT_PROXY_CAPATH; }
	else if (strcasecmp(msg, "crlfile") == 0) { opt = CURLOPT_CRLFILE; }
	else if (strcasecmp(msg, "proxy_crlfile") == 0) { opt = CURLOPT_PROXY_CRLFILE; }
	else if (strcasecmp(msg, "certinfo") == 0) { opt = CURLOPT_CERTINFO; }
	else if (strcasecmp(msg, "pinnedpublickey") == 0) { opt = CURLOPT_PINNEDPUBLICKEY; }
	else if (strcasecmp(msg, "proxy_pinnedpublickey") == 0) { opt = CURLOPT_PROXY_PINNEDPUBLICKEY; }
	else if (strcasecmp(msg, "random_file") == 0) { opt = CURLOPT_RANDOM_FILE; }
	else if (strcasecmp(msg, "egdsocket") == 0) { opt = CURLOPT_EGDSOCKET; }
	else if (strcasecmp(msg, "ssl_cipher_list") == 0) { opt = CURLOPT_SSL_CIPHER_LIST; }
	else if (strcasecmp(msg, "proxy_ssl_cipher_list") == 0) { opt = CURLOPT_PROXY_SSL_CIPHER_LIST; }
	else if (strcasecmp(msg, "ssl_sessionid_cache") == 0) { opt = CURLOPT_SSL_SESSIONID_CACHE; }
	else if (strcasecmp(msg, "ssl_options") == 0) { opt = CURLOPT_SSL_OPTIONS; }
	else if (strcasecmp(msg, "proxy_ssl_options") == 0) { opt = CURLOPT_PROXY_SSL_OPTIONS; }
	else if (strcasecmp(msg, "krblevel") == 0) { opt = CURLOPT_KRBLEVEL; }
	else if (strcasecmp(msg, "gssapi_delegation") == 0) { opt = CURLOPT_GSSAPI_DELEGATION; }
       	else if (strcasecmp(msg, "ssh_auth_types") == 0) { opt = CURLOPT_SSH_AUTH_TYPES; }
	else if (strcasecmp(msg, "ssh_host_public_key_md5") == 0) { opt = CURLOPT_SSH_HOST_PUBLIC_KEY_MD5; }
	else if (strcasecmp(msg, "ssh_public_keyfile") == 0) { opt = CURLOPT_SSH_PUBLIC_KEYFILE; }
	else if (strcasecmp(msg, "ssh_private_keyfile") == 0) { opt = CURLOPT_SSH_PRIVATE_KEYFILE; }
	else if (strcasecmp(msg, "ssh_knownhosts") == 0) { opt = CURLOPT_SSH_KNOWNHOSTS; }
	else if (strcasecmp(msg, "ssh_keyfunction") == 0) { opt = CURLOPT_SSH_KEYFUNCTION; }
	else if (strcasecmp(msg, "ssh_keydata") == 0) { opt = CURLOPT_SSH_KEYDATA; }
       	else if (strcasecmp(msg, "private") == 0) { opt = CURLOPT_PRIVATE; }
	else if (strcasecmp(msg, "share") == 0) { opt = CURLOPT_SHARE; }
	else if (strcasecmp(msg, "new_file_perms") == 0) { opt = CURLOPT_NEW_FILE_PERMS; }
	else if (strcasecmp(msg, "new_directory_perms") == 0) { opt = CURLOPT_NEW_DIRECTORY_PERMS; }
       	else if (strcasecmp(msg, "telnetoptions") == 0) { opt = CURLOPT_TELNETOPTIONS; }
       	else {
		sprintf(msg_and_args, "Curl %s:", msg);
		ctr_curl_internal_error(myself, msg_and_args, "Option invalid");
		cancel = 1;
       	}

	if (!cancel) {
	       	CURLcode res = curl_easy_setopt(myself->value.rvalue->ptr, opt, val);
		if (res != CURLE_OK) {
			sprintf(msg_and_args, "Curl %s `%s`", msg, (char *)ctr_object_type(argObj, NULL)->value.svalue);
			ctr_curl_internal_error(myself, msg_and_args, (char *)curl_easy_strerror(res));
		}
	}

	ctr_heap_free(msg);
	if (ctrstr) ctr_heap_free(val);

	return myself;
}

/**
 * [Curl] perform.
 *
 * Performs a blocking file transfer
 *
 **/
ctr_object* ctr_curl_perform(ctr_object* myself, ctr_argument* argumentList) {

	FILE *temp = tmpfile();
	curl_easy_setopt(myself->value.rvalue->ptr, CURLOPT_WRITEDATA, temp);

	CURLcode code = curl_easy_perform(myself->value.rvalue->ptr);

	if (code != CURLE_OK)
		ctr_curl_internal_error(myself, "Curl perform", "Received Curl error code");

	fseek(temp, 0, SEEK_END);
	ctr_size fileLen = ftell(temp);
	fseek(temp, 0, SEEK_SET);
	char *buffer = (char *)ctr_heap_allocate(fileLen + 1);
	if (!buffer) {
		printf("Out of memory\n");
		fclose(temp);
		exit(1);
	}
	fread(buffer, fileLen, 1, temp);
	fclose(temp);
	ctr_object *str = ctr_build_string(buffer, fileLen);
	ctr_heap_free(buffer);

	return str;
}

/* Loading */

/**
 * @internal
 *
 * Adds the Curl object into the world
 **/
void begin(){
	ctr_object*  CtrStdHandle = ctr_internal_create_object(CTR_OBJECT_TYPE_OTOBJECT);
	ctr_internal_create_func(CtrStdHandle, ctr_build_string_from_cstring( "new" ), &ctr_curl_new );
	ctr_internal_create_func(CtrStdHandle, ctr_build_string_from_cstring( "type" ), &ctr_curl_type );
	ctr_internal_create_func(CtrStdHandle, ctr_build_string_from_cstring( "cleanup" ), &ctr_curl_cleanup );
	ctr_internal_create_func(CtrStdHandle, ctr_build_string_from_cstring( "perform" ), &ctr_curl_perform );
	ctr_internal_create_func(CtrStdHandle, ctr_build_string_from_cstring( "respondTo:and:"), &ctr_curl_respondto );
	ctr_internal_object_add_property(CtrStdWorld, ctr_build_string_from_cstring( "Curl" ), CtrStdHandle, CTR_CATEGORY_PUBLIC_PROPERTY);
	CtrStdHandle->link = CtrStdObject;
	CtrStdHandle->info.sticky = 1;
}
