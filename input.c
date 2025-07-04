#include "input.h"
#include <conio.h>
#include <stdio.h>

// Global to track the last key pressed for context
static int g_last_key_pressed = 0;

InputAction process_input(int key, UIState *state, AppointmentList *appointments, TodoList *todos) {
    g_last_key_pressed = key;  // Track the key that was pressed
    
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
            // Only allow delete in appointments and todo views
            if (state->selected_view == VIEW_APPOINTMENTS || state->selected_view == VIEW_TODO) {
                return ACTION_DELETE;
            }
            break;
            
        case 'e':
        case 'E':
            // Only allow edit in appointments and todo views
            if (state->selected_view == VIEW_APPOINTMENTS || state->selected_view == VIEW_TODO) {
                return ACTION_EDIT;
            }
            break;
            
        case KEY_TAB:
            // Cycle through views
            state->selected_view = (state->selected_view + 1) % 3;
            state->cursor_x = 0;
            state->cursor_y = 0;
            state->appointment_display_index = 0;
            return ACTION_REDRAW;
            
        case KEY_SPACE:
            // Toggle completion for todos only
            if (state->selected_view == VIEW_TODO) {
                return ACTION_EDIT;  // This will be handled specially in edit_selected_item
            }
            break;
    }
    
    // View-specific navigation
    switch (state->selected_view) {
        case VIEW_CALENDAR:
            navigate_calendar(key, state);
            break;
            
        case VIEW_APPOINTMENTS:
            navigate_appointments(key, state, appointments);
            break;
            
        case VIEW_TODO:
            navigate_todos(key, state, todos);
            break;
    }
    
    g_last_key_pressed = key; // Track the last key pressed
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
    
    // Reset appointment display index when date changes
    state->appointment_display_index = 0;
}

void navigate_appointments(int key, UIState *state, AppointmentList *appointments) {
    // Get the count of appointments for the current selected date
    int appointment_indices[100];
    int appointment_count = 0;
    
    if (appointments) {
        appointment_count = find_appointments_by_date(appointments, state->selected_date, appointment_indices, 100);
    }
    
    switch (key) {
        case KEY_UP:
            if (state->cursor_y > 0) {
                state->cursor_y -= 2;  // Each appointment takes 2 lines
                state->appointment_display_index--;
            } else if (state->appointment_scroll > 0) {
                state->appointment_scroll--;
            }
            break;
            
        case KEY_DOWN:
            // Check if we can move down (appointment_display_index is 0-based)
            if (state->appointment_display_index < appointment_count - 1) {
                state->cursor_y += 2;  // Each appointment takes 2 lines
                state->appointment_display_index++;
            }
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
    
    // Ensure cursor_y and appointment_display_index stay in sync
    if (state->appointment_display_index < 0) {
        state->appointment_display_index = 0;
        state->cursor_y = 0;
    } else if (state->appointment_display_index >= appointment_count && appointment_count > 0) {
        state->appointment_display_index = appointment_count - 1;
        state->cursor_y = state->appointment_display_index * 2;
    } else if (appointment_count == 0) {
        state->appointment_display_index = 0;
        state->cursor_y = 0;
    }
}

void navigate_todos(int key, UIState *state, TodoList *todos) {
    int todo_count = todos ? todos->count : 0;
    
    switch (key) {
        case KEY_UP:
            if (state->cursor_y > 0) {
                state->cursor_y--;
            } else if (state->todo_scroll > 0) {
                state->todo_scroll--;
            }
            break;
            
        case KEY_DOWN:
            // Check if we can move down (considering scroll offset)
            if (state->cursor_y + state->todo_scroll < todo_count - 1) {
                state->cursor_y++;
            }
            break;
            
        case KEY_PGUP:
            state->todo_scroll -= 5;
            if (state->todo_scroll < 0) {
                state->todo_scroll = 0;
            }
            break;
            
        case KEY_PGDN:
            if (state->todo_scroll + 5 < todo_count) {
                state->todo_scroll += 5;
            }
            break;
    }
    
    // Ensure cursor position stays within bounds
    if (state->cursor_y < 0) {
        state->cursor_y = 0;
    }
    
    // Ensure we don't go beyond the available todos
    if (state->cursor_y + state->todo_scroll >= todo_count && todo_count > 0) {
        if (todo_count > 0) {
            state->cursor_y = todo_count - 1 - state->todo_scroll;
            if (state->cursor_y < 0) {
                state->todo_scroll = todo_count - 1;
                state->cursor_y = 0;
            }
        } else {
            state->cursor_y = 0;
            state->todo_scroll = 0;
        }
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
                {
                    // Find the actual appointment index based on cursor position
                    int appointment_indices[100];
                    int appointment_count = find_appointments_by_date(
                        appointments, 
                        state->selected_date, 
                        appointment_indices, 
                        100
                    );
                    
                    // Calculate which appointment is selected (each appointment takes 2 lines)
                    int selected_appointment_index = state->cursor_y / 2;
                    
                    if (selected_appointment_index >= 0 && selected_appointment_index < appointment_count) {
                        int actual_index = appointment_indices[selected_appointment_index];
                        delete_appointment(appointments, actual_index);
                        
                        // Adjust cursor if necessary
                        if (state->cursor_y > 0 && selected_appointment_index >= appointment_count - 1) {
                            state->cursor_y -= 2;
                            if (state->cursor_y < 0) state->cursor_y = 0;
                        }
                    }
                }
                break;
                
            case VIEW_TODO:
                if (state->cursor_y + state->todo_scroll < todos->count) {
                    delete_todo(todos, state->cursor_y + state->todo_scroll);
                    // Adjust cursor if at the end
                    if (state->cursor_y > 0 && state->cursor_y + state->todo_scroll >= todos->count - 1) {
                        state->cursor_y--;
                    }
                }
                break;
        }
    }
}

void edit_selected_item(UIState *state, AppointmentList *appointments, TodoList *todos) {
    switch (state->selected_view) {
        case VIEW_APPOINTMENTS:
            {
                // Find the actual appointment index based on cursor position
                int appointment_indices[100];
                int appointment_count = find_appointments_by_date(
                    appointments, 
                    state->selected_date, 
                    appointment_indices, 
                    100
                );
                
                // Calculate which appointment is selected (each appointment takes 2 lines)
                int selected_appointment_index = state->cursor_y / 2;
                
                if (selected_appointment_index >= 0 && selected_appointment_index < appointment_count) {
                    int actual_index = appointment_indices[selected_appointment_index];
                    edit_appointment_interactive(appointments, actual_index);
                }
            }
            break;
            
        case VIEW_TODO:
            if (state->cursor_y + state->todo_scroll < todos->count) {
                // If space was pressed, just toggle completion
                if (g_last_key_pressed == KEY_SPACE) {
                    toggle_todo_completion(todos, state->cursor_y + state->todo_scroll);
                } else {
                    // Otherwise, open edit dialog
                    edit_todo_interactive(todos, state->cursor_y + state->todo_scroll);
                }
            }
            break;
    }
}