#ifndef INPUT_H
#define INPUT_H

#include "ui.h"
#include "appointments.h"
#include "todo.h"

// Key codes
#define KEY_UP      (72 + 256)
#define KEY_DOWN    (80 + 256)
#define KEY_LEFT    (75 + 256)
#define KEY_RIGHT   (77 + 256)
#define KEY_PGUP    (73 + 256)
#define KEY_PGDN    (81 + 256)
#define KEY_HOME    (71 + 256)
#define KEY_END     (79 + 256)
#define KEY_TAB     9
#define KEY_ENTER   13
#define KEY_ESC     27
#define KEY_SPACE   32

// Input actions
typedef enum {
    ACTION_NONE,
    ACTION_QUIT,
    ACTION_REDRAW,
    ACTION_ADD_APPOINTMENT,
    ACTION_ADD_TODO,
    ACTION_DELETE,
    ACTION_EDIT,
    ACTION_HELP
} InputAction;

// Input handling functions
InputAction process_input(int key, UIState *state, AppointmentList *appointments, TodoList *todos);
void navigate_calendar(int key, UIState *state);
void navigate_appointments(int key, UIState *state, AppointmentList *appointments);
void navigate_todos(int key, UIState *state, TodoList *todos);
void delete_selected_item(UIState *state, AppointmentList *appointments, TodoList *todos);
void edit_selected_item(UIState *state, AppointmentList *appointments, TodoList *todos);

#endif // INPUT_H