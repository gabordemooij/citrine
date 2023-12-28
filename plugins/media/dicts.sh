#!/bin/sh
ISOs=$(ls i18n)

for ISO in $ISOs
do
	echo ; echo $ISO
	if [ -f "dict/xx${ISO}.dict" ]; then
		rm   dict/en${ISO}.dict
	fi
	STRINGKEYS="CTR_DICT_RUN,CTR_DICT_END" ../../bin/Linux/ctrnl -g ../../i18n/en/dictionary.h ../../i18n/${ISO}/dictionary.h > dict/en${ISO}.dict
	STRINGKEYS="CTR_DICT_ON" ../../bin/Linux/ctrnl -g i18n/en/media.h i18n/${ISO}/media.h >> dict/en${ISO}.dict
done

