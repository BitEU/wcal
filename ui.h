#ifndef UI_H
#define UI_H

#include <windows.h>
#include "calendar.h"
#include "appointments.h"
#include "todo.h"

// Color definitions
#define COLOR_BLACK     0
#define COLOR_BLUE      1
#define COLOR_GREEN     2
#define COLOR_CYAN      3
#define COLOR_RED       4
#define COLOR_MAGENTA   5
#define COLOR_YELLOW    6
#define COLOR_WHITE     7
#define COLOR_GRAY      8
#define COLOR_BRIGHT    8

// Theme colors
#define BORDER_FG       (COLOR_CYAN | COLOR_BRIGHT)
#define BORDER_BG       COLOR_BLACK
#define HEADER_FG       (COLOR_YELLOW | COLOR_BRIGHT)
#define HEADER_BG       COLOR_BLACK
#define SELECTED_FG     COLOR_BLACK
#define SELECTED_BG     COLOR_WHITE
#define TODAY_FG        (COLOR_RED | COLOR_BRIGHT)
#define TODAY_BG        COLOR_BLACK
#define NORMAL_FG       COLOR_WHITE
#define NORMAL_BG       COLOR_BLACK

// View types
typedef enum {
    VIEW_CALENDAR,
    VIEW_APPOINTMENTS,
    VIEW_TODO
} ViewType;

// UI State
typedef struct {
    int cursor_x, cursor_y;
    ViewType selected_view;
    Date current_date;
    Date selected_date;
    int window_width, window_height;
    int appointment_scroll;
    int todo_scroll;
    HANDLE console_output;
    HANDLE console_input;
    CONSOLE_SCREEN_BUFFER_INFO original_console_info;
} UIState;

// Box drawing characters (using ASCII for compatibility)
#define BOX_HORIZONTAL      '\xC4'
#define BOX_VERTICAL        '\xB3'
#define BOX_TOP_LEFT        '\xDA'
#define BOX_TOP_RIGHT       '\xBF'
#define BOX_BOTTOM_LEFT     '\xC0'
#define BOX_BOTTOM_RIGHT    '\xD9'
#define BOX_CROSS           '\xC5'
#define BOX_T_DOWN          '\xC2'
#define BOX_T_UP            '\xC1'
#define BOX_T_RIGHT         '\xC3'
#define BOX_T_LEFT          '\xB4'

// Function declarations
void init_console(void);
void restore_console(void);
void update_console_size(UIState *state);
void clear_screen(void);
void gotoxy(int x, int y);
void set_color(int foreground, int background);
void draw_box(int x, int y, int width, int height, const char *title);
void draw_ui(UIState *state, AppointmentList *appointments, TodoList *todos);
void draw_calendar_panel(UIState *state, int x, int y, int width, int height);
void draw_appointments_panel(UIState *state, AppointmentList *appointments, int x, int y, int width, int height);
void draw_todo_panel(UIState *state, TodoList *todos, int x, int y, int width, int height);
void draw_status_bar(UIState *state, int y, int width);
void draw_help_screen(UIState *state);

#endif // UI_H