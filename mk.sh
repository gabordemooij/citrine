
#First compile UTF-8Proc with gmake
cd lib/utf8proc-1.3.1
rm *.o
gmake

#Switch back to main directory
cd ..
cd ..

#Now run make file for Citrine itself
make
rm *.o
