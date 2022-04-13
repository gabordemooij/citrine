#!/bin/bash
#set -x
#set -v

CITRINE_MEMORY_LIMIT_MB=8
CITRINE_MEMORY_MODE=1
export CITRINE_MEMORY_LIMIT_MB
export CITRINE_MEMORY_MODE

#Determine which makefile to use
OS=$(uname -s)
LDFLAGS='-shared'
if [ "$OS" = "OpenBSD" -o "$OS" = "FreeBSD" ]; then
	MAKEFILE=makefile.bsd
elif [ "$OS" = "Darwin" ]; then
	MAKEFILE=makefile.bsd
	LDFLAGS='-shared -undefined dynamic_lookup'
elif [ "$OS" = "Haiku" ]; then
	MAKEFILE=makefile.haiku
else
	MAKEFILE=makefile
fi

#Remove .so
find . -name "*.so" -exec rm {} +

#Run makefiles of plugins
make plugin PACKAGE="request" NAME="libctrrequest.so" LDFLAGS=${LDFLAGS}
make plugin PACKAGE="request" NAME="libctrverzoek.so" LDFLAGS=${LDFLAGS}
make plugin PACKAGE="mock/percolator" NAME="libctrpercolator.so" LDFLAGS=${LDFLAGS}
make plugin PACKAGE="jsmn" NAME="libctrjson.so" LDFLAGS=${LDFLAGS}
make plugin PACKAGE="jsmn" NAME="libctrjsonnl.so" LDFLAGS=${LDFLAGS}
make plugin PACKAGE="jsmn" NAME="libctrjsonhy.so" LDFLAGS=${LDFLAGS}

#Remove old dicts
rm dict/*

#Run regular makefiles through build script
./mk.sh
cp bin/${OS}/ctrxx bin/Generic/ctr
cd tests/test-o-mat
../../bin/${OS}/ctren_us runtests.ctr
