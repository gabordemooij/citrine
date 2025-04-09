#include "citrine.h"

/**
 * UTF8Size
 *
 * measures the size of character
 */
int ctr_utf8size(char c) {
	if ((c & CTR_UTF8_BYTE3) == CTR_UTF8_BYTE3) return 4;
	if ((c & CTR_UTF8_BYTE2) == CTR_UTF8_BYTE2) return 3;
	if ((c & CTR_UTF8_BYTE1) == CTR_UTF8_BYTE1) return 2;
	return 1;
}

/**
 * GetUTF8Length
 *
 * measures the length of an utf8 string in utf8 chars
 */
ctr_size ctr_getutf8len(char* strval, ctr_size max) {
	ctr_size i;
	ctr_size j = 0;
	ctr_size s = 0;
	for(i = 0; i < max; i++) {
		s = ctr_utf8size(strval[i]);
		j += (s - 1);
	}
	return (i-j);
}

/**
 * GetBytesForUTF8String
 */
ctr_size getBytesUtf8(char* strval, long startByte, ctr_size lenUChar) {
	long i = 0;
	long bytes = 0;
	int s = 0;
	ctr_size x = 0;
	long index = 0;
	char c;
	while(x < lenUChar) {
		index = startByte + i;
		c = strval[index];
		s = ctr_utf8size(c);
		bytes = bytes + s;
		i = i + s;
		x ++;
	}
	return bytes;
}
