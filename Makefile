all : tcpServer.exe tcpClient.exe udpServer.exe udpClient.exe superserver.exe

superserver.exe : superserver.o
	gcc -o superserver.exe superserver.o

udpServer.exe : udpServer.o
	gcc -o udpServer.exe udpServer.o

udpClient.exe : udpClient.o
	gcc -o udpServer.exe udpClient.o

tcpServer.exe : tcpServer.o
	gcc -o tcpServer.exe tcpServer.o

tcpClient.exe : tcpClient.o
	gcc -o tcpServer.exe tcpClient.o

superserver.o : superserver.c
	gcc -c superserver.c

tcpClient.o : tcpClient.c
	gcc -c tcpClient.c

tcpServer.o : tcpServer.c
	gcc -c tcpServer.c

udpClient.o : udpClient.c
	gcc -c udpClient.c

udpServer.o : udpServer.c
	gcc -c udpServer.c

.PHONY: clean

clean:
	rm -f tcpServer.exe tcpClient.exe udpServer.exe udpClient.exe superserver.exe tcpServer.o tcpClient.o udpServer.o udpClient.o superserver.o
