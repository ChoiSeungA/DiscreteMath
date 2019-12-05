all:
	gcc -o count `pkg-config --cflags --libs glib-2.0` count.c `pkg-config --libs glib-2.0`
	gcc -o stemmer -I../include stemmer.c ../lib/libstemmer.o 
	gcc -o PA3 -I../include `pkg-config --cflags --libs glib-2.0` PA3.c `pkg-config --libs glib-2.0` ../lib/libstemmer.o
	gcc -o PA3_1 -I../include `pkg-config --cflags --libs glib-2.0` -lm PA3_1.c `pkg-config --libs glib-2.0` ../lib/libstemmer.o
