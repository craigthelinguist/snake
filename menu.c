
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define KEY_ESC 27
#define KEY_ENTER 10

enum item_type { SLIDER, TEXT };

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
};

enum event_type { EXIT, SLIDER_RETURN, TEXT_RETURN };

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
  menu->indent_size = 3;
  menu->window = window;
  
  menu->items = items;
  menu->num_items = num_items;
  menu->selection = 0;
  
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
  int top_offset = 2;
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
  for (i=0; i != sz; i++) {
    
    struct menu_item *elem = elems[i];
    top_offset++;
    
    switch (elem->tag) {

        case TEXT:
          item_text = elem->item.text;
          mvwprintw(window, top_offset, left_offset, item_text->text);
          break;
          
        case SLIDER:
          item_slider = elem->item.slider;
          
          // Draw slider text.
          mvwprintw(window, top_offset, left_offset, item_slider->text);
          top_offset++;
          
          // Draw slider.
          mvwaddch(window, top_offset, left_offset + (int)(0.5*left_offset), '[');
          int j;
          for (j=0; j < item_slider->length; j++) {
            mvwaddch(window, top_offset, left_offset + (int)(0.5*left_offset)+1+j, '-');
          }
          break;
          
    }
  }
  
  // Refresh window.
  wrefresh(window);
  
  while (1);
  
}

  /*
    Convenience function. Makes an exit event.
  */
struct menu_event *
_EXIT_EVENT (struct menu_item *elem)
{
  struct menu_event *menu_event = malloc (sizeof (struct menu_event));
  menu_event->tag = EXIT;
  menu_event->elem = elem;
  menu_event->int_value = 0;
  return menu_event;
}

  /*
    Convenience function. Make an event which contains text.

    str:
      Text to be contained in the event.
  */
struct menu_event *
_TEXT_EVENT (struct menu_item *elem)
{
  
  struct menu_event *menu_event = malloc (sizeof (struct menu_event));
  menu_event->tag = TEXT_RETURN;
  menu_event->elem = elem;
  menu_event->int_value = 0;
  return menu_event;
}

  /*
    Convenience function. Make an event which contains an int.

    amt:
      int to be contained in the event.
  */
struct menu_event *
_SLIDER_EVENT (struct menu_item *elem)
{
  struct menu_event *menu_event = malloc (sizeof (struct menu_event));
  menu_event->tag = SLIDER_RETURN;
  menu_event->elem = elem;
  menu_event->int_value = elem->item.slider->pos;
  return menu_event;
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
run_menu (struct menu *menu, struct menu_event *menu_event)
{
  
  // Index of current selection.
  int i = 0;
  
  // Whether you're engaged on the currently selected menu element.
  // This only makes sense for some types, e.g.: sliders.
  int engaged = 0;
  
  // For use below.
  struct menu_item *elem;
  struct item_text *item_text;
  struct item_slider *item_slider;
  
  int c;
  while (1) {
    c = getch();
    elem = menu->items[i];
    enum item_type type = elem->tag;
    
    // Press "esc".
    if (c == KEY_ESC) {
      menu_event = _EXIT_EVENT(NULL);
      break;
    }
    
    // Press "enter" on a text item.
    else if (c == KEY_ENTER && type == TEXT) {
      item_text = elem->item.text;
      if (item_text->exit) menu_event = _EXIT_EVENT(elem);
      else menu_event = _TEXT_EVENT(elem);
      break;
    }
    
    // Press "enter" on a slider item.
    else if (c == KEY_ENTER && type == SLIDER) {
      item_slider = elem->item.slider;
      if (!engaged) {
        engaged = 1;
      }
      else {
        engaged = 0;
        menu_event = _SLIDER_EVENT(elem);
        break;
      }
    }
    
    // Press "left" on an engaged slider.
    else if (c == KEY_LEFT && type == SLIDER && engaged) {
      item_slider = elem->item.slider;
      int new_pos = item_slider->pos;
      if (new_pos > 0) {
        new_pos--;
        item_slider->pos = new_pos;
      }
    }
    
    // Press "right" on an engaged slider.
    else if (c == KEY_RIGHT && type == SLIDER && engaged) {
      item_slider = elem->item.slider;
      int new_pos = item_slider->pos;
      if (new_pos < item_slider->length) {
        new_pos++;
        item_slider->pos = new_pos;
      }
    }
    
    // Press "down".
    else if (c == KEY_DOWN && !engaged) {
      if (i < menu->num_items) i++;
    }
    
    // Press "up".
    else if (c == KEY_UP && !engaged) {
      if (i > 0) i--;
    }
    
  }
  
  menu->selection = i;
  
}


int
main (int argc, char *argv[])
{
	
  // establish terminal, remove cursor.
  initscr();
  noecho();
  
  // allow for function keys to be registered by ncurses.
  keypad(stdscr, TRUE);

  // set input delay so it is non-blocking.
  timeout(0);

  
  nodelay(stdscr, 0);
  curs_set(FALSE);
  
  // Make items.
  struct menu_item *item1 = malloc(sizeof (struct menu_item));
  make_item_text(item1, "Item 1");
  struct menu_item *item2 = malloc(sizeof (struct menu_item));
  make_item_exit(item2, "Exit");
  struct menu_item *items[] = { item1, item2 };
  
  // Make the window.
  WINDOW *window;
  window = newwin(30, 30, 0, 0);
  
  // Make menu.
  struct menu *menu = malloc(sizeof (struct menu));
  int num_items = sizeof(items) / sizeof(items[0]);
  make_menu(menu, window, items, num_items);
  
  // Run the menu.
  struct menu_event *event = malloc(sizeof (struct menu_event));
  while(1) {
    menu_refresh(menu);
    run_menu(menu, event);
  }
    
}
