#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <string.h>
#include "ui.h"
#include "calendar.h"
#include "appointments.h"
#include "todo.h"
#include "input.h"
#include "storage.h"

// Global state
UIState g_ui_state;
AppointmentList g_appointments;
TodoList g_todos;

void initialize_app(void) {
    // Initialize console
    init_console();
    
    // Initialize UI state
    g_ui_state.selected_view = VIEW_CALENDAR;
    g_ui_state.cursor_x = 0;
    g_ui_state.cursor_y = 0;
    
    // Get current date
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    g_ui_state.current_date.year = tm->tm_year + 1900;
    g_ui_state.current_date.month = tm->tm_mon + 1;
    g_ui_state.current_date.day = tm->tm_mday;
    g_ui_state.selected_date = g_ui_state.current_date;
    
    // Initialize data structures
    init_appointments(&g_appointments);
    init_todos(&g_todos);
    
    // Load saved data
    load_appointments(&g_appointments, "appointments.dat");
    load_todos(&g_todos, "todos.dat");
}

void cleanup_app(void) {
    // Save data
    save_appointments(&g_appointments, "appointments.dat");
    save_todos(&g_todos, "todos.dat");
    
    // Clean up memory
    free_appointments(&g_appointments);
    free_todos(&g_todos);
    
    // Restore console
    restore_console();
}

void main_loop(void) {
    int running = 1;
    int needs_redraw = 1;
    
    while (running) {
        // Update console size
        update_console_size(&g_ui_state);
        
        // Redraw if needed
        if (needs_redraw) {
            draw_ui(&g_ui_state, &g_appointments, &g_todos);
            needs_redraw = 0;
        }
        
        // Handle input
        if (_kbhit()) {
            int key = _getch();
            
            // Special keys (arrows, function keys) send two bytes
            if (key == 0 || key == 224) {
                key = _getch();
                key += 256; // Offset special keys
            }
            
            InputAction action = process_input(key, &g_ui_state);
            
            switch (action) {
                case ACTION_QUIT:
                    running = 0;
                    break;
                    
                case ACTION_REDRAW:
                    needs_redraw = 1;
                    break;
                    
                case ACTION_ADD_APPOINTMENT:
                    add_appointment_interactive(&g_appointments, &g_ui_state);
                    needs_redraw = 1;
                    break;
                    
                case ACTION_ADD_TODO:
                    add_todo_interactive(&g_todos);
                    needs_redraw = 1;
                    break;
                    
                case ACTION_DELETE:
                    delete_selected_item(&g_ui_state, &g_appointments, &g_todos);
                    needs_redraw = 1;
                    break;
                    
                case ACTION_EDIT:
                    edit_selected_item(&g_ui_state, &g_appointments, &g_todos);
                    needs_redraw = 1;
                    break;
                    
                case ACTION_HELP:
                    draw_help_screen(&g_ui_state);
                    needs_redraw = 1;
                    break;
                    
                case ACTION_NONE:
                default:
                    break;
            }
        }
        
        // Small delay to reduce CPU usage
        Sleep(50);
    }
}

int main(void) {
    initialize_app();
    main_loop();
    cleanup_app();
    
    return 0;
}