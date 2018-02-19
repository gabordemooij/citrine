name = curl

all:
	gcc -g -c src/$(name).c -Wall -Werror -fpic -o $(name).o
	gcc -shared -o mods/curl/libctr$(name).so $(name).o -l$(name)
install:
	sh runtests.sh
clean:
	rm mods/$(name)/*

