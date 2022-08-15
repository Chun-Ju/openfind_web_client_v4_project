main:
	rm -rf webpage
	gcc main.c urlProcessed.c socket.c -o web_client -D_TEST_ -lssl -lcrypto

exec:
	./web_client https://www.openfind.com.tw webpage/ 5
