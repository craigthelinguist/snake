
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
  int engaged;
};

enum event_type { EXIT, SLIDER_DISENGAGE, SLIDER_ENGAGE, SLIDER_MOVE, TEXT_RETURN, NAVIGATE };

struct menu_event {
  enum event_type tag;
  struct menu_item *elem;
  int int_value;
};


typedef enum event_type EVENT_TYPE;
typedef struct menu_event EVENT;
typedef struct menu MENU;
typedef struct menu_item ITEM;

void make_item_text (ITEM *item, char *text);
void make_item_exit (ITEM *item, char *text);
void make_item_slider (ITEM *item, char *text, int length);
void make_menu (MENU *menu, WINDOW *window, ITEM **items, int num_items);
void free_elem (ITEM *item);
void free_menu (MENU *menu);
void menu_run (MENU *menu, EVENT *event);
void menu_refresh (MENU *menu);
void menu_init_colours();

int slider_value(ITEM *item);

EVENT_TYPE event_type(EVENT *event);
ITEM *event_item(EVENT *event);

















