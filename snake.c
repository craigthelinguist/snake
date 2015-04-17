
#include <ncurses.h>
#include <string.h>
#include <stdlib.h>

#define MIN(X,Y) X>Y?Y:X
#define MAX(X,Y) X>Y?X:Y

// colour pairs used in drawing
#define COLOR_SNAKE COLOR_PAIR(1)
#define COLOR_WALL COLOR_PAIR(2)

// boundaries of the game
#define WALL_WD 40
#define WALL_HT 20

// key constants
#define KEY_ESC 27

/** Rows and columns of the screen **/
int ROWS = 0;
int COLS = 0;

typedef struct snake {
  int row;
  int col;
  struct snake *next;
} snake_t;

// function declarations
void draw_snake (snake_t *snake);
int handle_input (snake_t *snake);

void draw_snake (snake_t *snake)
{
  attron(COLOR_SNAKE);
  while (snake != NULL) {
    mvaddch(snake->row, snake->col, '*');
    snake = snake->next;
  }
  attroff(COLOR_SNAKE);
}

void draw_wall ()
{
  attron(COLOR_WALL);
  int i;

  // draw the left and right edges
  for (i=0; i<WALL_WD; i++) {
    mvaddch(0, i, '*');
    mvaddch(WALL_HT-1, i, '*');
  }

  // draw top and bottom edges
  for (i=1; i<WALL_HT-1; i++) {
    mvaddch(i, 0, '*');
    mvaddch(i, WALL_WD-1, '*');
  }

  attroff(COLOR_WALL);
}


snake_t *init_snake(snake_t *prev, int row, int col)
{
  snake_t *snake = (snake_t *) malloc(sizeof (snake_t));
  snake->row = row; snake->col = col;
  if (prev != NULL) prev->next = snake;
  return snake;
}

void move_snake(snake_t *head, int newRow, int newCol) {
  int oldRow, oldCol;
  while (head != NULL) {
    oldRow = head->row;
    oldCol = head->col;
    head->row = newRow;
    head->col = newCol;
    newRow = oldRow;
    newCol = oldCol;
    head = head->next;
  }
}

int main (int argc, char *argv[])
{

  // establish ncurses terminal
  initscr();
  noecho();
  curs_set(FALSE);

  // colours
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_WHITE); // snake colour
  init_pair(2, COLOR_CYAN, COLOR_CYAN); // wall colour

  // get dimensions of the screen
  getmaxyx(stdscr, ROWS, COLS);

  // allow for function keys to be registered by ncurses
  keypad(stdscr, TRUE);

  // initialise snake
  snake_t *snake = init_snake(NULL, WALL_HT/2, WALL_WD/2);
  snake_t *seg2 = init_snake(snake, snake->row+1, snake->col);
  snake_t *seg3 = init_snake(seg2, seg2->row+1, seg2->col);

  while (1) {
    clear();
    draw_snake(snake);
    draw_wall();
    refresh();

    // get input and new location of the head of the snake
    int in = getch();
    int newRow = snake->row;
    int newCol = snake->col;
    if (in == KEY_UP)         newRow = MAX(snake->row-1, 1);
    else if (in == KEY_DOWN)  newRow = MIN(snake->row+1, WALL_HT-2);
    else if (in == KEY_LEFT)  newCol = MAX(snake->col-1, 1);
    else if (in == KEY_RIGHT) newCol = MIN(snake->col+1, WALL_WD-2);
    else if (in == KEY_ESC)   break;

    // if head of snake would be inside of its body, game over
    // if head of snake would be inside of a wall, game over

    // move snake
    if (newRow != snake->row || newCol != snake->col) {
      move_snake(snake, newRow, newCol);
    }

    // if head of snake is over food, eat food, add to body of snake,
    // respawn a new piece of food

  }
  endwin();
  return 0;
}
