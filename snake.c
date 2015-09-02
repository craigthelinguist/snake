

// ------------------------------------------------------------
// Imports.
// ------------------------------------------------------------

#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "menu.h"

// ------------------------------------------------------------
// Macros.
// ------------------------------------------------------------

// colour pairs used in drawing
#define COLOR_SNAKE COLOR_PAIR(1)
#define COLOR_WALL COLOR_PAIR(2)
#define COLOR_FOOD COLOR_PAIR(3)

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
struct snake {
  struct point *loc;
  struct snake *next;
};

struct point {
  int row;
  int col;
};

struct game_data {
  int rows;
  int cols;
  int WALL_HT;
  int WALL_WD;
  int difficulty;
};

// ------------------------------------------------------------
// Function declarations.
// ------------------------------------------------------------

// Drawing functions.
void draw_snake (struct snake *, WINDOW *);
void draw_wall (struct game_data *, WINDOW *);
void draw_food (struct point, WINDOW *);
void draw_direction (Direction, WINDOW *);

// Snake-related functions.
struct snake *init_snake (struct snake *prev, int row, int col);
struct point new_pos (struct game_data *, struct snake *, Direction dir);
void move_snake (struct game_data *game, struct snake *head, struct point);
void grow_snake (struct game_data *game, struct snake **head, struct point);
int touching (struct snake *head, struct point *p);
int opposites (Direction d1, Direction d2);

// Food-related functions.
struct point randomise_food (struct game_data *game, struct snake *head);

// Time-related functions.
long int timems (void);
long int update_delay(struct game_data *);

// Input-related functions.
int process_input(Direction snake_dir, Direction *queued_dir);

// Game-related functions.
void play_game (struct game_data *, WINDOW *);



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
long int update_delay (struct game_data *game) {
  long int delay = 300 - 20*game->difficulty;
  if (delay < 20) return 20;
  return delay;
}



// ------------------------------------------------------------
// Drawing functions.
// ------------------------------------------------------------

/*  Draw the snake on the default ncurses window. */
void draw_snake (struct snake *snake, WINDOW *window)
{
  wattron(window, COLOR_SNAKE);
  while (snake != NULL) {
    mvwaddch(window, snake->loc->row, snake->loc->col, '*');
    snake = snake->next;
  }
  wattroff(window, COLOR_SNAKE);
}

void draw_food (struct point pt, WINDOW *window)
{
  wattron(window, COLOR_FOOD);
  mvwaddch(window, pt.row, pt.col, '*');
  wattroff(window, COLOR_FOOD);
}

/*  Draw the wall on the default ncurses window. */
void draw_wall (struct game_data *game, WINDOW *window)
{
  wattron(window, COLOR_WALL);
  int i;

  // draw the left and right edges
  for (i=0; i< game->WALL_WD; i++) {
    mvwaddch(window, 0, i, '*');
    mvwaddch(window, game->WALL_HT-1, i, '*');
  }

  // draw top and bottom edges
  for (i=1; i<game->WALL_HT-1; i++) {
    mvwaddch(window, i, 0, '*');
    mvwaddch(window, i, game->WALL_WD-1, '*');
  }

  wattroff(window, COLOR_WALL);
}

/*  Draw the queued direction of the snake. */
void draw_direction (Direction queued_dir, WINDOW *window)
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
    mvwaddch(window, 2, 45, c);
}



// ------------------------------------------------------------
// Snake functions.
// ------------------------------------------------------------

/*  Create and allocate a segment of snake at the specified (row,col)
    position. If next is specified, then that will be the next pointer
    in the initialised segment of snake. */
struct snake *init_snake (struct snake *next, int row, int col)
{
  struct snake *snake = malloc(sizeof (struct snake));
  struct point *p = malloc(sizeof (struct point));
  p->row = row; p->col = col;
  snake->loc = p;
  snake->next = next;
  return snake;
}

/*  Check to see if the body of the snake is touching the specified
    row and col. */
int touching (struct snake *head, struct point *p)
{
  while (head != NULL) {
    if (head->loc->row == p->row && head->loc->col == p->col) return 1;
    head = head->next;
  }
  return 0;
}

/*  Prepend a new head to the snake. Update the pointer to the head of the
    snake. */
void
grow_snake (struct game_data *game, struct snake **snake, struct point newpos)
{

  // figure out where the snake should now be.
  struct snake *head = *snake;
  int newRow = newpos.row;
  int newCol = newpos.col;

  // If the snake has grown, prepend the new segment to the head of the snake.
  struct snake *new_head = init_snake(head, newRow, newCol);
  *snake = new_head;
  
}

/*  Move the snake one step in the specified direction. Returns a non-zero
    value if the snake moved successfully without touching itself or a wall.
    Otherwise returns zero.  */
void
move_snake (struct game_data *game, struct snake *head, struct point newpos) {
  
  int newRow = newpos.row;
  int newCol = newpos.col;
  
  // Shift each segment of the snake along one.
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

struct point
new_pos (struct game_data *game, struct snake *head, Direction dir)
{
  
  // Figure out where the snake should now be.
  int newRow = head->loc->row;
  int newCol = head->loc->col;
  switch (dir) {
    case NORTH:
      newRow = MAX(head->loc->row-1, 1);
      break;

    case SOUTH:
      newRow = MIN(head->loc->row+1, game->WALL_HT-2);
      break;

    case WEST:
      newCol = MAX(head->loc->col-1, 1);
      break;

    case EAST:
      newCol = MIN(head->loc->col+1, game->WALL_WD-2);
      break;
  }
  
  // Return as a point.
  struct point p = {newRow, newCol};
  return p;

}

/*  Construct a random point within the boundaries of the game. */
struct point
randomise_food (struct game_data *game, struct snake *head)
{
  int row, col;
  struct point p;
  do {
    int row = rand() % (game->WALL_HT-2) + 1;
    int col = rand() % (game->WALL_WD-2) + 1;
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


void play_game (struct game_data *game, WINDOW *window)
{

  // Seed rand and set non-blocking input.
  srand(time(NULL));
  timeout(0);

  // Initialise snake.
  struct snake *snake = init_snake(NULL, game->WALL_HT/2, game->WALL_WD/2);
  snake = init_snake(snake, game->WALL_HT/2 + 1, game->WALL_WD/2);
  snake = init_snake(snake, game->WALL_HT/2 + 1, game->WALL_WD/2);
  
  // remember direction of snake, last time step.
  long int lastUpdate = timems();
  Direction snake_dir = NORTH;
  Direction queued_dir = NORTH;

  // position of food
  struct point food = randomise_food(game, snake);
  int ate_food = 0;

  while (1) {

    // Process input. Update queued directino.
    if (process_input(snake_dir, &queued_dir)) break;
    draw_direction(queued_dir, window);

    // Check if you should update.
    long int currTime = timems();
    if (currTime - lastUpdate < update_delay(game)) continue;
    lastUpdate = currTime;

    // Get direction. Move snake and grow in length.
    snake_dir = queued_dir;
    struct point newpos = new_pos(game, snake, snake_dir);
    if (ate_food) {
      grow_snake(game, &snake, newpos);
      ate_food = 0;        
    }
    else move_snake(game, snake, newpos);
    
    // Snake touching itself? Game over, man!
    if (touching(snake->next, snake->loc)) break;

    // If Snake has eaten food, randomly generate new food.
    if (touching(snake, &food)) {
      food = randomise_food(game, snake);
      ate_food = 1;
    }

    // Clear window and redraw.
    wclear(window);
    draw_snake(snake, window);
    draw_food(food, window);
    draw_wall(game, window);
    wrefresh(window);

  }

  // Clean output.
  wclear(window);

  // Re-enable blocking input.
  timeout(-1);

}


// ------------------------------------------------------------
// Main.
// ------------------------------------------------------------


int
main (int argc, char *argv[])
{
  
  // Establish ncurses.
  initscr();

  // Hide cursor and cursor feedback.
  noecho();
  curs_set(FALSE);

  // Enable function keys to be registered by ncurses.
  keypad(stdscr, TRUE);

  // Enable blocking input.
  timeout(-1);

  // Enable colours.
  if (has_colors() == FALSE) {
    fprintf(stderr, "No colour - exiting.");  
    exit(1);  
  }
  start_color();
  menu_init_colours();
  init_pair(1, COLOR_WHITE, COLOR_WHITE); // snake colour
  init_pair(2, COLOR_CYAN, COLOR_CYAN); // wall colour
  init_pair(3, COLOR_RED, COLOR_RED); // food colour

  // Create windows for menu and game.
  WINDOW *window_menu = newwin(30, 30, 0, 0);
  WINDOW *window_game = newwin(0, 0, 0, 0);

  // Allocate memory for menu items.
  ITEM *item1 = malloc(sizeof (ITEM));
  ITEM *item2 = malloc(sizeof (ITEM));
  ITEM *item3 = malloc(sizeof (ITEM));

  // Create menu items.
  make_item_text(item1, "Play");
  make_item_slider(item2, "Difficulty", 10);
  make_item_exit(item3, "Exit");

  // Allocate memory for menu; create menu.
  ITEM *items[] = { item1, item2, item3 };
  MENU *menu = malloc(sizeof (MENU));
  int num_items = sizeof(items) / sizeof(items[0]);
  make_menu(menu, window_menu, items, num_items);

  // Create and set game data.
  struct game_data *game = malloc(sizeof (struct game_data));
  game->WALL_WD = 20;
  game->WALL_HT = 20;
  game->difficulty = 0;
  int rows, cols;
  getmaxyx(stdscr, rows, cols);
  game->rows = rows;
  game->cols = cols;

  // Display menu, get options.
  while (1) {
    EVENT *event;
    int done = 0;
    while (!done) {
      event = malloc(sizeof (EVENT));
      menu_refresh(menu);
      menu_run(menu, event);
      EVENT_TYPE type = event_type(event);
      if (type == EXIT) done = 1;
      else if (type == TEXT_RETURN) done = 2;
      free(event);
    }

    // User wants to play a game.
    if (done == 2) {
      game->difficulty = slider_value(item2);
      wclear(window_menu);
      play_game(game, window_game);
    }

    // User exited. Tidy up and return.
    if (done == 1) {
      wclear(window_menu);
      wclear(window_game);
      delwin(window_menu);
      delwin(window_game);
      free(game);   
      free_menu(menu);
      endwin();
      return 0;
    }
  }

}

