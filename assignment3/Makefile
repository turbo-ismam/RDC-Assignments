all : tcpClient.exe tcpServer.exe

tcpServer.exe : tcpServer.o
	gcc -o tcpServer.exe tcpServer.o

tcpClient.exe : tcpClient.o
	gcc -o tcpClient.exe tcpClient.o

tcpClient.o : tcpClient.c
	gcc -c tcpClient.c

tcpServer.o : tcpServer.c
	gcc -c tcpServer.c

.PHONY: clean

clean:
	rm -f tcpClient.exe tcpServer.exe tcpClient.o tcpServer.o
