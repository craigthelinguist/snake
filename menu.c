
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define KEY_ESC 27
#define KEY_NL 10

#define COLOR_NORMAL COLOR_PAIR(10)
#define COLOR_HIGHLIGHT COLOR_PAIR(11)

enum item_type { SLIDER, TEXT, MOVE };

struct item_slider {
  char *text;
  int length;
  int pos;
};

struct item_text {
  char *text;
  int exit;
};

struct menu_item {
  enum item_type tag;
  union {
    struct item_text *text;
    struct item_slider *slider;
  } item;
};

struct menu {
  int indent_size;
  WINDOW *window;
  struct menu_item **items;
  int num_items;
  int selection;
  int engaged;
};

enum event_type { EXIT, SLIDER_RETURN, TEXT_RETURN, NAVIGATE };

struct menu_event {
  enum event_type tag;
  struct menu_item *elem;
  int int_value;
};

  /*
    Make a text item.
    elem:
      A struct ptr which will be used to store the item (your responsiblity
      to free).
    text:
      The text that this item should display. It will be copied into the
      struct.
  */
void
make_item_text (struct menu_item *elem, char *text)
{
  
  // Make the item_text struct.
  struct item_text *item_text = malloc(sizeof (struct item_text));
  item_text->text = text;
  item_text->exit = 0;
  
  // Make the elem tagged union.
  elem->tag = TEXT;
  elem->item.text = item_text;
  
};

  /*
    Make an exit item. This is the same as a text item, but when it is
    engaged by the user it ends the menu session.
    
    elem:
      A struct ptr which will be used to store the item (your responsiblity
      to free).
    text:
      The text that this item should display. It will be copied into the
      struct.
  */
void
make_item_exit (struct menu_item *elem, char *text)
{
   make_item_text(elem, text);
   elem->item.text->exit = 1;
}

  /*
    Make a slider item.

    elem:
      A struct ptr which will be used to store the item (your responsiblity
      to free).
    text:
      The text that this item should display. It will be copied into the
      struct, so you don't have to worry about freeing it.
    length:
      How long the slider is. This should be a positive number.
  */
void
make_item_slider (struct menu_item *elem, char *text, int length)
{
  
  // Copy the string and allocate a new one for internal use.
  int len = strlen(text);
  char *copy = malloc (len + 1);
  strcpy(copy, text);
  copy[len] = '\0';
  
  // Make the item_slider struct.
  struct item_slider *item_slider = malloc(sizeof (struct item_slider));
  item_slider->text = copy;
  item_slider->length = length;
  item_slider->pos = 0;
  
  // Make the elem tagged union.
  elem->tag = SLIDER;
  elem->item.slider = item_slider;

};

  /*
    Make a menu.

    menu:
      A struct ptr which will be used to store the menu (your
      responsiblity to free).
    window:
      The ncurses window that the menu is apart of.
    items:
      Array of pointers to item structs which are apart of your menu.
    num_items:
      Size of the array.
      
  */
void
make_menu (struct menu *menu, WINDOW *window, struct menu_item **items, int num_items)
{
  
  // Make the menu struct.
  menu->indent_size = 5;
  menu->window = window;
  
  menu->items = items;
  menu->num_items = num_items;
  menu->selection = 0;
  menu->engaged = 0;
  
}

  /*
    Refresh the menu by drawing its contents onto its window.

    menu:
      To menu to refresh.
  */
void
menu_refresh (struct menu *menu)
{
  
  // Clear window.
  wclear(menu->window);
  
  // Offsets for displaying.
  int top_offset = 1;
  int left_offset = menu->indent_size;
  
  // Variables for use inside switch statement.
  struct item_text *item_text;
  struct item_slider *item_slider;
  
  // Get the menu elements.
  WINDOW *window = menu->window;
  struct menu_item **elems = menu->items;
  int sz = menu->num_items;
  
  // Go through each menu element, output to window.
  int i;
  
  // Set color.
  attron(COLOR_NORMAL);
  
  // Draw each item on the screen.
  for (i=0; i != sz; i++) {
    struct menu_item *elem = elems[i];
    top_offset++;
    
    switch (elem->tag) {
      
        case TEXT:
    
	  if (i == menu->selection) {
	    wattron(window, COLOR_HIGHLIGHT);
	    mvwprintw(window, top_offset, left_offset-3, "-->");
	    left_offset = left_offset + 1;
	  }

	  item_text = elem->item.text;
          mvwprintw(window, top_offset, left_offset, item_text->text);
          
	  
	  if (i == menu->selection) {
	    wattron(window, COLOR_NORMAL);
	    left_offset = left_offset - 1;
	  }
     
	  break;
          
        case SLIDER:
          item_slider = elem->item.slider;
	  
	  if (i == menu->selection) {
	    wattron(window, COLOR_HIGHLIGHT);
	    mvwprintw(window, top_offset, left_offset-3, "-->");
	    left_offset = left_offset + 1;
	  }

          // Draw slider text.
          mvwprintw(window, top_offset, left_offset, item_slider->text);
          top_offset++;
         
	  if (i == menu->selection) {
	    wattron(window, COLOR_NORMAL);
	    left_offset = left_offset - 1;
	  }
          
          // Draw slider.
          mvwaddch(window, top_offset, left_offset + (int)(0.5*left_offset), '[');
          int j;
          for (j=0; j < item_slider->length; j++) {
            mvwaddch(window, top_offset, left_offset + (int)(0.5*left_offset)+1+j, '-');
          }
          mvwaddch(window, top_offset, left_offset + (0.5*left_offset)+1+item_slider->length, ']');
          break;
          
    }
   
  }
  
  wattroff(window, COLOR_NORMAL);
  // Refresh window.
  wrefresh(window);
  
}

  /*
    Convenience function. Makes an exit event.
  */
void
_EXIT_EVENT (struct menu_event *menu_event, struct menu_item *elem)
{
  menu_event->tag = EXIT;
  menu_event->elem = elem;
  menu_event->int_value = 0;
}

  /*
    Convenience function. Make an event which contains text.

    str:
      Text to be contained in the event.
  */
void
_TEXT_EVENT (struct menu_event *menu_event, struct menu_item *elem)
{
  menu_event->tag = TEXT_RETURN;
  menu_event->elem = elem;
  menu_event->int_value = 0;
}

  /*
    Convenience function. Make an event which contains an int.

    amt:
      int to be contained in the event.
  */
void
_SLIDER_EVENT (struct menu_event *menu_event, struct menu_item *elem)
{
  menu_event->tag = SLIDER_RETURN;
  menu_event->elem = elem;
  menu_event->int_value = elem->item.slider->pos;
}

void
_NAVIGATE_EVENT (struct menu_event *menu_event, struct menu_item *elem)
{
  menu_event->tag = NAVIGATE;
  menu_event->elem = elem;
}

  /*
    Run the menu. All input will yield to the given menu's event loop.
    The loop will stop when a menu event fires, such as the user
    selecting an event item or exiting.

    menu:
      The menu to run.
    menu_event:
      A struct ptr which will point to the location of the event
      created when this function returns.
  */
void
run_menu (struct menu *menu, struct menu_event *event)
{
  
  // Window we're outputting to.
  WINDOW *window = menu->window;
  
  // Whether you're engaged on the currently selected menu element.
  // This only makes sense for some types, e.g.: sliders.
  int engaged = menu->engaged;
  
  // For use below.
  struct menu_item *elem = menu->items[menu->selection];
  struct item_text *item_text;
  struct item_slider *item_slider;
  
  int ch;
  while (1) {
    
    ch = getch();
    
    // Press "esc".
    if (ch == KEY_ESC) {
      _EXIT_EVENT(event, NULL);
      break;
    }
    
    // Press "enter" while disengaged with slider.
    //else if (ch == KEY_ENTER && !engaged && elem->tag == SLIDER) {
    //}
    
    // Press "left" while engaged with slider.
    
    // Press "right" while engaged with slider.
    
    // Press "enter" while engaged with slider.
    
    // Press "enter"; send key event
    else if (ch == KEY_NL) {
      item_text = (elem->item).text;     
      if (item_text->exit) _EXIT_EVENT(event, elem);
      else _TEXT_EVENT(event, elem);
      break;
    }
    
    // Press "down".
    else if (ch == KEY_DOWN && !engaged) {
      fprintf(stderr, "down");
      if (menu->selection < menu->num_items - 1) menu->selection = menu->selection+1;
      _NAVIGATE_EVENT(event, elem);
      break;
    }
    
    // Press "up".
    else if (ch == KEY_UP && !engaged) {
      if (menu->selection > 0) menu->selection = menu->selection-1;
      _NAVIGATE_EVENT(event, elem);
      break;
    }
    
  }
  
}


int
main (int argc, char *argv[])
{
	
  // establish terminal, remove cursor.
  initscr();
  noecho();
  curs_set(FALSE);
  
  // allow for function keys to be registered by ncurses.
  keypad(stdscr, TRUE);
  
  if (has_colors() == FALSE) {
    fprintf(stderr, "no colour");
    exit(1);
    
  }
  
  // Enable colours.
  start_color();
  
  // Create colour pairs.
  init_pair(10, COLOR_WHITE, COLOR_BLACK);
  init_pair(11, COLOR_BLACK, COLOR_WHITE);
  
  // Make items.
  struct menu_item *item1 = malloc(sizeof (struct menu_item));
  make_item_text(item1, "Item 1");
  struct menu_item *item2 = malloc(sizeof (struct menu_item));
  make_item_slider(item2, "Difficulty", 10);
  struct menu_item *item3 = malloc(sizeof (struct menu_item));
  make_item_exit(item3, "Exit");
  
  struct menu_item *items[] = { item1, item2, item3 };
  
  // Make the window.
  WINDOW *window;
  window = newwin(30, 30, 0, 0);
  
  // Make menu.
  struct menu *menu = malloc(sizeof (struct menu));
  int num_items = sizeof(items) / sizeof(items[0]);
  make_menu(menu, window, items, num_items);
  
  // Run the menu.
  struct menu_event *event;
  int done = 0;
  do {
    event = malloc(sizeof (struct menu_event));
    menu_refresh(menu);
    run_menu(menu, event);
    if (event->tag == EXIT) done = 1;
    free(event);
  } while (!done);
  
  // Clean up; should do a whole bunch of frees here too.
  wclear(window);
  endwin();
  return 0;
    
}
