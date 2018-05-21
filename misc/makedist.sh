rm -rf /tmp/dist

#Create folder structure
mkdir /tmp/dist
mkdir /tmp/dist/src
mkdir /tmp/dist/bin
mkdir /tmp/dist/docs
mkdir /tmp/dist/examples
mkdir /tmp/dist/fonts
mkdir /tmp/dist/dictionaries
mkdir /tmp/dist/bin/openbsd
mkdir /tmp/dist/bin/linux

#Copy license
cp ../LICENSE /tmp/dist/

#Copy 'welcome and greetings'
cp ../docs/info.txt /tmp/dist/

#Copy dictionaries
cp ../misc/dictionaries/*.dict /tmp/dist/dictionaries/

#Copy examples
cp ../examples/fibonacci.ctr /tmp/dist/examples/

#Copy documentation
cp ../docs/ctr.1 /tmp/dist/docs/ctr.1

#Copy fonts and font-macros
cp ../fonts/Citrine.ttf /tmp/dist/fonts/
cp ../misc/ide/geany/macros.ini /tmp/dist/fonts/geany_macros.ini

#Copy previously prepared sets
cp -R /tmp/spack/* /tmp/dist/src/
rm -rf /tmp/dist/src/.git
cp -R /tmp/opack/* /tmp/dist/bin/openbsd/
cp -R /tmp/linpack/* /tmp/dist/bin/linux/

#List the contents of the package
tree /tmp/dist -I test*

#Produce tarball
rm /tmp/citrine$1.tar.gz
rm /tmp/checksum$1.txt
tar cvzf /tmp/citrine$1.tar.gz -C /tmp dist
sha256sum /tmp/citrine$1.tar.gz > /tmp/checksum$1.txt
