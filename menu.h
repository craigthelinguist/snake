
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
void init_colours();

