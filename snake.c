

// ------------------------------------------------------------
// Imports.
// ------------------------------------------------------------

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>



// ------------------------------------------------------------
// Macros.
// ------------------------------------------------------------

// colour pairs used in drawing
#define COLOR_SNAKE COLOR_PAIR(1)
#define COLOR_WALL COLOR_PAIR(2)
#define COLOR_FOOD COLOR_PAIR(3)

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
  struct point *loc;
  struct snake *next;
} snake_t;

typedef struct point {
  int row;
  int col;
} point;

// ------------------------------------------------------------
// Function declarations.
// ------------------------------------------------------------

// Drawing functions.
void draw_snake (snake_t *snake);
void draw_wall ();
void draw_food (point *p);

// Snake-related functions.
snake_t *init_snake (snake_t *prev, int row, int col);
void move_snake (snake_t **head, Direction dir, int grow_snake);
int touching (snake_t *head, point *p);

// Food-related functions.
point randomise_food ();

// Time-related functions.
long int timems (void);
long int update_delay(void);

// Input-related functions.
int process_input(Direction snake_dir, Direction *queued_dir);

// Gui-related functions.
void setup_gui(void);



// ------------------------------------------------------------
// Globals.
// ------------------------------------------------------------

// dimensions of the screen.
int ROWS = 0;
int COLS = 0;
int DIFFICULTY = 0;


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
  long int delay = 300 - 20*DIFFICULTY;
  if (delay < 20) return 20;
  return delay;
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
  init_pair(3, COLOR_RED, COLOR_RED); // food colour

  // store dimensions of the screen.
  getmaxyx(stdscr, ROWS, COLS);

  // allow for function keys to be registered by ncurses.
  keypad(stdscr, TRUE);

  // set input delay so it is non-blocking.
  timeout(0);

  // seed rand
  srand(time(NULL));

}

/*  Draw the snake on the default ncurses window. */
void draw_snake (snake_t *snake)
{
  attron(COLOR_SNAKE);
  while (snake != NULL) {
    mvaddch(snake->loc->row, snake->loc->col, '*');
    snake = snake->next;
  }
  attroff(COLOR_SNAKE);
}

void draw_food (point *pt)
{
  attron(COLOR_FOOD);
  mvaddch(pt->row, pt->col, '*');
  attroff(COLOR_FOOD);
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

/*  Draw the queued direction of the snake. */
void draw_direction (Direction queued_dir)
{
    char c;
    switch (queued_dir) {
      case NORTH:
        c = 'N';
        break;
      case SOUTH:
        c = 'S';
        break;
      case WEST:
        c = 'W';
        break;
      case EAST:
        c = 'E';
        break;
    }
    mvaddch(2, 45, c);
}



// ------------------------------------------------------------
// Snake functions.
// ------------------------------------------------------------

/*  Create and allocate a segment of snake at the specified (row,col)
    position. If next is specified, then that will be the next pointer
    in the initialised segment of snake. */
snake_t *init_snake (snake_t *next, int row, int col)
{
  snake_t *snake = (snake_t *) malloc(sizeof (snake_t));
  point *p = (point *) malloc(sizeof (point));
  p->row = row; p->col = col;
  snake->loc = p;
  if (next != NULL) snake->next = next;
  return snake;
}

/*  Check to see if the body of the snake is touching the specified
    row and col. */
int touching (snake_t *head, point *p)
{
  while (head != NULL) {
    if (head->loc->row == p->row && head->loc->col == p->col) return 1;
    head = head->next;
  }
  return 0;
}

/*  Move the snake one step in the specified direction. Returns a non-zero
    value if the snake moved successfully without touching itself or a wall.
    Otherwise returns zero.  */
void move_snake (snake_t **snake, Direction dir, int should_grow) {
  
  // figure out where the snake should now be.
  snake_t *head = *snake;
  int newRow = head->loc->row;
  int newCol = head->loc->col;
  switch (dir) {
    case NORTH:
      newRow = MAX(head->loc->row-1, 1);
      break;

    case SOUTH:
      newRow = MIN(head->loc->row+1, WALL_HT-2);
      break;

    case WEST:
      newCol = MAX(head->loc->col-1, 1);
      break;

    case EAST:
      newCol = MIN(head->loc->col+1, WALL_WD-2);
      break;
  }
  
  // If the snake has grown, prepend the new segment to the head of the snake.
  if (should_grow) {
    snake_t *new_head = init_snake(head, newRow, newCol);
    *snake = new_head;
    return;
  }
  
  // Otherwise, shift each segment of the snake along one.
  int currRow, currCol;
  while (head != NULL) {
    currRow = head->loc->row;
    currCol = head->loc->col;
    head->loc->row = newRow;
    head->loc->col = newCol;
    newRow = currRow;
    newCol = currCol;
    head = head->next;
  }

}

/*  Construct a random point within the boundaries of the game. */
point randomise_food (snake_t *head)
{
  int row, col;
  point p;
  do {
    int row = rand() % (WALL_HT-2) + 1;
    int col = rand() % (WALL_WD-2) + 1;
    p.row = row; p.col = col;
  } while (touching(head, &p));
  return p;
}


// ------------------------------------------------------------
// Input functions.
// ------------------------------------------------------------

/*  Process user input. Updates the direction of the snake depending
    on what key the user presses. Will return a non-zero value if the
    program should terminate. Otherwise it will return zero. */
int process_input(Direction snake_dir, Direction *queued_dir)
{
    // get key code
    int in = getch();

    // check if user pushed escape
    if (in == KEY_ESC) return 1;

    // get direction the user pushed
    Direction d2;
    if (in == KEY_UP) d2 = NORTH;
    else if (in == KEY_DOWN) d2 = SOUTH;
    else if (in == KEY_LEFT) d2 = WEST;
    else if (in == KEY_RIGHT) d2 = EAST;
    else return 0;

    // update queued direction, if snake can turn in that direction
    if (!opposites(snake_dir, d2)) *queued_dir = d2;
    return 0;
}

int opposites (Direction d1, Direction d2)
{
  if (d1 == NORTH && d2 == SOUTH) return 1;
  if (d1 == SOUTH && d2 == NORTH) return 1;
  if (d1 == WEST && d2 == EAST) return 1;
  if (d1 == EAST && d2 == WEST) return 1;
  return 0;
}



// ------------------------------------------------------------
// Main.
// ------------------------------------------------------------

void parse_args (int argc, char *argv[])
{
  // default difficulty
  if (argc == 1) {
    DIFFICULTY = 1;
    return;  
  }

  // parse difficulty
  if (argc == 2) {
    DIFFICULTY = atoi(argv[1]);
    printf("diff: %d\n", DIFFICULTY);
    return;
  }

  // error.
  printf("Bad number of arguments.\n");
  abort();
}

int main (int argc, char *argv[])
{

  parse_args(argc, argv);
  setup_gui();

  // initialise snake.
  snake_t *snake = init_snake(NULL, WALL_HT/2, WALL_WD/2);
  snake = init_snake(snake, WALL_HT/2 + 1, WALL_WD/2);
  snake = init_snake(snake, WALL_HT/2 + 1, WALL_WD/2);
  
  // remember direction of snake, last time step.
  long int lastUpdate = timems();
  Direction snake_dir = NORTH;
  Direction queued_dir = NORTH;

  // position of food
  point food = randomise_food(snake);
  int ate_food = 0;

  while (1) {

    // process user input. If non-zero value is returned, then the programme
    // should terminate.
    if (process_input(snake_dir, &queued_dir)) break;
    draw_direction(queued_dir);

    // check if enough time has elapsed to warrant updating the snake.
    long int currTime = timems();
    if (currTime - lastUpdate < update_delay()) continue;
    lastUpdate = currTime;

    // change direction and move the snake.
    snake_dir = queued_dir;
    move_snake(&snake, snake_dir, ate_food);
    ate_food = 0;

    // if the snake would be inside of its body or a wall, game over.
    // nb: if the snake tries to move into a wall it ends up moving onto itself
    // so this handles both cases.
    if (touching(snake->next, snake->loc)) break;

    // check if snake has eaten food.
    if (touching(snake, &food)) {
      food = randomise_food(snake);
      ate_food = 1;
    }

    // clear screen and redraw.
    clear();
    draw_snake(snake);
    draw_food(&food);
    draw_wall();
    refresh();


    // if head of snake is over food, eat food, add to body of snake,
    // respawn a new piece of food

  }
  endwin();
  return 0;
}
