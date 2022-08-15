main:
	rm -rf webpage
	gcc main.c urlProcessed.c -o web_client

exec:
	./web_client https://www.openfind.com.tw webpage/ 5
