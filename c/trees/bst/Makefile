target:
	gcc -o bt main.c bintree.c 
memtest: target
	valgrind --leak-check=full ./bt
clean:
	rm bt
