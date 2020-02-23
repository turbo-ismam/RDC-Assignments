all : superserver.exe

superserver.exe : superserver.o
	gcc -o superserver.exe superserver.o

superserver.o : superserver.c
	gcc -c superserver.c


.PHONY: clean makec maked

clean:
	rm -f superserver.exe superserver.o

makec:
	make -f MakefileSETUP

maked:
	make -f MakefileSETUP clean
