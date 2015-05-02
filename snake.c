

// ------------------------------------------------------------
// Imports.
// ------------------------------------------------------------

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>



// ------------------------------------------------------------
// Macros.
// ------------------------------------------------------------

// colour pairs used in drawing
#define COLOR_SNAKE COLOR_PAIR(1)
#define COLOR_WALL COLOR_PAIR(2)

// boundaries of the game
#define WALL_WD 40
#define WALL_HT 20

// key constants
#define KEY_ESC 27

#define MAX(X,Y) X>Y?X:Y
#define MIN(X,Y) X<Y?X:Y



// ------------------------------------------------------------
// Typedefs, enums.
// ------------------------------------------------------------

// Represents the direction of the snake.
typedef enum {NORTH, EAST, SOUTH, WEST} Direction;

// Each piece of the snake is the element of a linked list.
typedef struct snake {
  int row;
  int col;
  struct snake *next;
} snake_t;



// ------------------------------------------------------------
// Function declarations.
// ------------------------------------------------------------

// Drawing functions.
void draw_snake (snake_t *snake);
void draw_wall ();

// Snake-related functions.
snake_t *init_snake (snake_t *prev, int row, int col);
int move_snake (snake_t *head, Direction dir);

// Time-related functions.
long int timems (void);
long int update_delay(void);

// Input-related functions.
void update_direction (Direction *dir);

// Gui-related functions.
void setup_gui(void);



// ------------------------------------------------------------
// Globals.
// ------------------------------------------------------------

// dimensions of the screen.
int ROWS = 0;
int COLS = 0;



// ------------------------------------------------------------
// Time functions.
// ------------------------------------------------------------

/*  Get the number of time elapsed since the epoch in
    milliseconds. */
long int timems (void)
{
  struct timeval tp;
  gettimeofday(&tp, NULL);
  return tp.tv_sec * 1000 + tp.tv_usec / 1000;
}

/*  Get the amount of time to wait between steps of the game world. This should
    increase with difficulty. */
long int update_delay () {
  return 250;
}



// ------------------------------------------------------------
// Drawing functions.
// ------------------------------------------------------------

/*  Set up and configure the gui. */
void setup_gui ()
{

  // establish terminal, remove cursor.
  initscr();
  noecho();
  curs_set(FALSE);

  // set up colours used in programme.
  start_color();
  init_pair(1, COLOR_WHITE, COLOR_WHITE); // snake colour
  init_pair(2, COLOR_CYAN, COLOR_CYAN); // wall colour

  // store dimensions of the screen.
  getmaxyx(stdscr, ROWS, COLS);

  // allow for function keys to be registered by ncurses.
  keypad(stdscr, TRUE);

  // set input delay so it is non-blocking.
  timeout(1);

}
/*  Draw the snake on the default ncurses window. */
void draw_snake (snake_t *snake)
{
  attron(COLOR_SNAKE);
  while (snake != NULL) {
    mvaddch(snake->row, snake->col, '*');
    snake = snake->next;
  }
  attroff(COLOR_SNAKE);
}

/*  Draw the wall on the default ncurses window. */
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



// ------------------------------------------------------------
// Snake functions.
// ------------------------------------------------------------

/*  Create and allocate a segment of snake at the specified (row,col)
    position. */
snake_t *init_snake(snake_t *prev, int row, int col)
{
  snake_t *snake = (snake_t *) malloc(sizeof (snake_t));
  snake->row = row; snake->col = col;
  if (prev != NULL) prev->next = snake;
  return snake;
}

/*  Move the snake one step in the specified direction. Returns a non-zero
    value if the snake moved successfully without touching itself or a wall.
    Otherwise returns zero.  */
int move_snake(snake_t *head, Direction dir) {

  // figure out where the snake should now be.
  int newRow = head->row;
  int newCol = head->col;
  switch (dir) {
    case NORTH:
      newRow = MAX(head->row-1, 1);
      break;

    case SOUTH:
      newRow = MIN(head->row+1, WALL_HT-2);
      break;

    case WEST:
      newCol = MAX(head->col-1, 1);
      break;

    case EAST:
      newCol = MIN(head->col+1, WALL_WD-2);
      break;
  }

  // update each segment of the head/
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

  return 1;
}



// ------------------------------------------------------------
// Input functions.
// ------------------------------------------------------------

/*  Process user input. Updates the direction of the snake depending
    on what key the user presses. Will return a non-zero value if the
    program should terminate. Otherwise it will return zero. */
int process_input(Direction *dir)
{
    int in = getch();
    switch (in) {

      case KEY_UP:
        if (*dir != SOUTH) *dir = NORTH;
        break;

      case KEY_DOWN:
        if (*dir != NORTH) *dir = SOUTH;
        break;

      case KEY_RIGHT:
        if (*dir != WEST) *dir = EAST;
        break;

      case KEY_LEFT:
        if (*dir != EAST) *dir = WEST;
        break;

      case KEY_ESC:
        return 1;

    }
    return 0;

}



// ------------------------------------------------------------
// Main.
// ------------------------------------------------------------

int main (int argc, char *argv[])
{

  setup_gui();

  // initialise snake.
  snake_t *snake = init_snake(NULL, WALL_HT/2, WALL_WD/2);
  snake_t *seg2 = init_snake(snake, snake->row+1, snake->col);
  snake_t *seg3 = init_snake(seg2, seg2->row+1, seg2->col);

  // remember direction of snake, last time step.
  long int lastUpdate = timems();
  Direction dir = NORTH;

  while (1) {

    // process user input. If non-zero value is returned, then the programme
    // should terminate.
    if (process_input(&dir)) break;

    // check if enough time has elapsed to warrant updating the snake.
    long int currTime = timems();
    if (currTime - lastUpdate < update_delay()) continue;
    lastUpdate = currTime;

    // create screen. move the snake. redraw the scene.
    clear();
    if (move_snake(snake, dir));
    draw_snake(snake);
    draw_wall();
    refresh();

    // if head of snake would be inside of its body, game over
    // if head of snake would be inside of a wall, game over

    // if head of snake is over food, eat food, add to body of snake,
    // respawn a new piece of food

  }
  endwin();
  return 0;
}
