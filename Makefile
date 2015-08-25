all: snake.c
	gcc -o snake snake.c menu.c -l ncurses
