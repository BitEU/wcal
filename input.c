#include "input.h"
#include <stdio.h>
#include <windows.h>
#include <conio.h>

InputAction process_input(int key, UIState *state) {
    // Global keys that work in any view
    switch (key) {
        case 'q':
        case 'Q':
            return ACTION_QUIT;
            
        case 'h':
        case 'H':
            return ACTION_HELP;
            
        case 'a':
        case 'A':
            if (state->selected_view == VIEW_CALENDAR || state->selected_view == VIEW_APPOINTMENTS) {
                return ACTION_ADD_APPOINTMENT;
            } else if (state->selected_view == VIEW_TODO) {
                return ACTION_ADD_TODO;
            }
            break;
            
        case 'd':
        case 'D':
            return ACTION_DELETE;
            
        case 'e':
        case 'E':
            return ACTION_EDIT;
            
        case KEY_TAB:
            // Cycle through views
            state->selected_view = (state->selected_view + 1) % 3;
            state->cursor_x = 0;
            state->cursor_y = 0;
            return ACTION_REDRAW;
            
        case KEY_SPACE:
            // Toggle completion for todos
            if (state->selected_view == VIEW_TODO) {
                return ACTION_EDIT;  // Will be handled as toggle
            }
            break;
    }
    
    // View-specific navigation
    switch (state->selected_view) {
        case VIEW_CALENDAR:
            navigate_calendar(key, state);
            break;
            
        case VIEW_APPOINTMENTS:
            navigate_appointments(key, state, NULL);
            break;
            
        case VIEW_TODO:
            navigate_todos(key, state, NULL);
            break;
    }
    
    return ACTION_REDRAW;
}

void navigate_calendar(int key, UIState *state) {
    int days_in_month = get_days_in_month(state->selected_date.year, state->selected_date.month);
    
    switch (key) {
        case KEY_LEFT:
            state->selected_date.day--;
            if (state->selected_date.day < 1) {
                // Go to previous month
                state->selected_date.month--;
                if (state->selected_date.month < 1) {
                    state->selected_date.month = 12;
                    state->selected_date.year--;
                }
                state->selected_date.day = get_days_in_month(state->selected_date.year, state->selected_date.month);
            }
            break;
            
        case KEY_RIGHT:
            state->selected_date.day++;
            if (state->selected_date.day > days_in_month) {
                // Go to next month
                state->selected_date.day = 1;
                state->selected_date.month++;
                if (state->selected_date.month > 12) {
                    state->selected_date.month = 1;
                    state->selected_date.year++;
                }
            }
            break;
            
        case KEY_UP:
            state->selected_date.day -= 7;
            if (state->selected_date.day < 1) {
                // Go to previous month
                state->selected_date.month--;
                if (state->selected_date.month < 1) {
                    state->selected_date.month = 12;
                    state->selected_date.year--;
                }
                int prev_days = get_days_in_month(state->selected_date.year, state->selected_date.month);
                state->selected_date.day = prev_days + state->selected_date.day;
            }
            break;
            
        case KEY_DOWN:
            state->selected_date.day += 7;
            if (state->selected_date.day > days_in_month) {
                // Go to next month
                state->selected_date.day = state->selected_date.day - days_in_month;
                state->selected_date.month++;
                if (state->selected_date.month > 12) {
                    state->selected_date.month = 1;
                    state->selected_date.year++;
                }
            }
            break;
            
        case KEY_PGUP:
            // Previous month
            state->selected_date.month--;
            if (state->selected_date.month < 1) {
                state->selected_date.month = 12;
                state->selected_date.year--;
            }
            // Adjust day if necessary
            days_in_month = get_days_in_month(state->selected_date.year, state->selected_date.month);
            if (state->selected_date.day > days_in_month) {
                state->selected_date.day = days_in_month;
            }
            break;
            
        case KEY_PGDN:
            // Next month
            state->selected_date.month++;
            if (state->selected_date.month > 12) {
                state->selected_date.month = 1;
                state->selected_date.year++;
            }
            // Adjust day if necessary
            days_in_month = get_days_in_month(state->selected_date.year, state->selected_date.month);
            if (state->selected_date.day > days_in_month) {
                state->selected_date.day = days_in_month;
            }
            break;
            
        case KEY_HOME:
            // Go to today
            state->selected_date = state->current_date;
            break;
    }
}

void navigate_appointments(int key, UIState *state, AppointmentList *appointments) {
    switch (key) {
        case KEY_UP:
            if (state->cursor_y > 0) {
                state->cursor_y--;
            }
            break;
            
        case KEY_DOWN:
            state->cursor_y++;
            // TODO: Check against actual appointment count for the day
            break;
            
        case KEY_PGUP:
            state->appointment_scroll -= 5;
            if (state->appointment_scroll < 0) {
                state->appointment_scroll = 0;
            }
            break;
            
        case KEY_PGDN:
            state->appointment_scroll += 5;
            break;
    }
}

void navigate_todos(int key, UIState *state, TodoList *todos) {
    switch (key) {
        case KEY_UP:
            if (state->cursor_y > 0) {
                state->cursor_y--;
            } else if (state->todo_scroll > 0) {
                state->todo_scroll--;
            }
            break;
            
        case KEY_DOWN:
            state->cursor_y++;
            // TODO: Check against actual todo count
            break;
            
        case KEY_PGUP:
            state->todo_scroll -= 5;
            if (state->todo_scroll < 0) {
                state->todo_scroll = 0;
            }
            break;
            
        case KEY_PGDN:
            state->todo_scroll += 5;
            break;
    }
}

void delete_selected_item(UIState *state, AppointmentList *appointments, TodoList *todos) {
    // Confirmation dialog
    int window_width, window_height;
    update_console_size(state);
    window_width = state->window_width;
    window_height = state->window_height;
    
    int dialog_x = window_width / 2 - 20;
    int dialog_y = window_height / 2 - 3;
    
    // Clear the background area first
    clear_area(dialog_x, dialog_y, 40, 6);
    
    draw_box(dialog_x, dialog_y, 40, 6, "Confirm Delete");
    
    set_color(NORMAL_FG, NORMAL_BG);
    gotoxy(dialog_x + 2, dialog_y + 2);
    printf("Delete selected item? (y/n)");
    
    int ch = _getch();
    if (ch == 'y' || ch == 'Y') {
        switch (state->selected_view) {
            case VIEW_APPOINTMENTS:
                // TODO: Find the actual appointment index based on cursor position
                break;
                
            case VIEW_TODO:
                if (state->cursor_y + state->todo_scroll < todos->count) {
                    delete_todo(todos, state->cursor_y + state->todo_scroll);
                }
                break;
        }
    }
}

void edit_selected_item(UIState *state, AppointmentList *appointments, TodoList *todos) {
    switch (state->selected_view) {
        case VIEW_APPOINTMENTS:
            // TODO: Find the actual appointment index and edit it
            break;
            
        case VIEW_TODO:
            if (state->cursor_y + state->todo_scroll < todos->count) {
                // For space key, just toggle completion
                toggle_todo_completion(todos, state->cursor_y + state->todo_scroll);
            }
            break;
    }
}