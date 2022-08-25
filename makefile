main:
	rm -rf web_client
	rm -rf webpage
	rm -rf diskhash
	rm -rf url.txt
	gcc main.c urlNormalized.c socket.c simpleDiskHash.c directoryOperation.c -o web_client -lssl -lcrypto

test:
	rm -rf web_client
	rm -rf webpage
	rm -rf diskhash
	rm -rf url.txt
	gcc main.c urlNormalized.c socket.c simpleDiskHash.c directoryOperation.c -o web_client -D_TEST_ -lssl -lcrypto

exec:
	./web_client https://www.openfind.com.tw webpage/ 5
