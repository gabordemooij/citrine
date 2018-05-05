#clean the workspace
rm -rf /tmp/opack
mkdir /tmp/opack
mkdir /tmp/opack/mods
mkdir /tmp/opack/mods/percolator
mkdir /tmp/opack/mods/pg
mkdir /tmp/opack/mods/request
mkdir /tmp/opack/mods/password
mkdir /tmp/opack/mods/curl
mkdir /tmp/opack/mods/json


#copy the OBSD binaries
cp ../ctr /tmp/opack/
cp ../ctrnl /tmp/opack/
cp ../mods/percolator/*.so /tmp/opack/mods/percolator/
cp ../mods/pg/*.so /tmp/opack/mods/pg/
cp ../mods/request/*.so /tmp/opack/mods/request/
cp ../mods/password/*.so /tmp/opack/mods/password/
cp ../mods/curl/*.so /tmp/opack/mods/curl/
cp ../mods/json/*.so /tmp/opack/mods/json/
