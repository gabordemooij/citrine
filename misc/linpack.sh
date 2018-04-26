#clean the workspace
rm -rf /tmp/linpack
mkdir /tmp/linpack
mkdir /tmp/linpack/mods
mkdir /tmp/linpack/mods/percolator
mkdir /tmp/linpack/mods/pg
mkdir /tmp/linpack/mods/request
mkdir /tmp/linpack/mods/password
mkdir /tmp/linpack/mods/curl
mkdir /tmp/linpack/mods/json


#copy the Linux binaries
cp ../ctr /tmp/linpack/
cp ../ctrnl /tmp/linpack/
cp ../mods/percolator/*.so /tmp/linpack/mods/percolator/
cp ../mods/pg/*.so /tmp/linpack/mods/pg/
cp ../mods/request/*.so /tmp/linpack/mods/request/
cp ../mods/password/*.so /tmp/linpack/mods/password/
cp ../mods/curl/*.so /tmp/linpack/mods/curl/
cp ../mods/json/*.so /tmp/linpack/mods/json/
