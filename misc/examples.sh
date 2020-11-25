#!/bin/bash
#set -x
#set -v
OS=`uname -s`
for ISO in $(ls ../i18n)
do
	if [ $ISO != 'xx' ]; then
		cat ../dict/xx${ISO}.dict exampledicts/xx${ISO}.dict > /tmp/tmp.dict
		../bin/${OS}/ctrxx -t /tmp/tmp.dict examples/example1.ctr 1>../examples/${ISO}/example1.ctr 2>/dev/null
		../bin/${OS}/ctrxx -t /tmp/tmp.dict examples/example2.ctr 1>../examples/${ISO}/example2.ctr 2>/dev/null
		../bin/${OS}/ctrxx -t /tmp/tmp.dict examples/example3.ctr 1>../examples/${ISO}/example3.ctr 2>/dev/null
	fi
done