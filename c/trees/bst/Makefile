target:
	gcc -o bt main.c ./src/bintree.c ./src/utils.c
memtest: target
	valgrind --leak-check=full ./bt
clean:
	rm bt
