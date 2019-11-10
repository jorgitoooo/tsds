target:
	gcc -o exec main.c ./src/bintree.c ./src/utils.c
memtest: target
	valgrind --leak-check=full ./exec
clean:
	rm exec
