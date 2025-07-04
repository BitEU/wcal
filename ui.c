#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <conio.h>

void init_console(void) {
    // Get console handles
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    
    // Save original console state
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    
    // Set console window size to be wider but reasonable height
    COORD bufferSize = {140, 30};  // Wider but more reasonable height
    SetConsoleScreenBufferSize(hOut, bufferSize);
    
    SMALL_RECT windowSize = {0, 0, 139, 29};  // 140x30 window
    SetConsoleWindowInfo(hOut, TRUE, &windowSize);
    
    // Set console mode
    DWORD mode;
    GetConsoleMode(hIn, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE; // Disable quick edit
    mode |= ENABLE_EXTENDED_FLAGS;
    SetConsoleMode(hIn, mode);
    
    // Set console code page for box drawing characters
    SetConsoleOutputCP(437);
    
    // Hide cursor
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
    
    // Clear screen
    clear_screen();
}

void restore_console(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    
    // Show cursor
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
    
    // Reset colors
    set_color(COLOR_WHITE, COLOR_BLACK);
    
    // Clear screen
    clear_screen();
}

void update_console_size(UIState *state) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    state->window_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    state->window_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

void clear_screen(void) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD topLeft = {0, 0};
    DWORD written;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    
    GetConsoleScreenBufferInfo(hOut, &csbi);
    DWORD consoleSize = csbi.dwSize.X * csbi.dwSize.Y;
    
    FillConsoleOutputCharacter(hOut, ' ', consoleSize, topLeft, &written);
    FillConsoleOutputAttribute(hOut, csbi.wAttributes, consoleSize, topLeft, &written);
    SetConsoleCursorPosition(hOut, topLeft);
}

void clear_area(int x, int y, int width, int height) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;
    
    // Clear the area line by line
    for (int row = 0; row < height; row++) {
        COORD coord = {x, y + row};
        FillConsoleOutputCharacter(hOut, ' ', width, coord, &written);
        FillConsoleOutputAttribute(hOut, NORMAL_FG | (NORMAL_BG << 4), width, coord, &written);
    }
}

void gotoxy(int x, int y) {
    COORD coord = {x, y};
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void set_color(int foreground, int background) {
    WORD color = (background << 4) | foreground;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void draw_box(int x, int y, int width, int height, const char *title) {
    set_color(BORDER_FG, BORDER_BG);
    
    // Top border
    gotoxy(x, y);
    printf("%c", BOX_TOP_LEFT);
    for (int i = 1; i < width - 1; i++) {
        printf("%c", BOX_HORIZONTAL);
    }
    printf("%c", BOX_TOP_RIGHT);
    
    // Title
    if (title != NULL && strlen(title) > 0) {
        int title_len = strlen(title);
        int title_pos = x + (width - title_len - 2) / 2;
        gotoxy(title_pos, y);
        set_color(HEADER_FG, HEADER_BG);
        printf(" %s ", title);
        set_color(BORDER_FG, BORDER_BG);
    }
    
    // Side borders
    for (int i = 1; i < height - 1; i++) {
        gotoxy(x, y + i);
        printf("%c", BOX_VERTICAL);
        gotoxy(x + width - 1, y + i);
        printf("%c", BOX_VERTICAL);
    }
    
    // Bottom border
    gotoxy(x, y + height - 1);
    printf("%c", BOX_BOTTOM_LEFT);
    for (int i = 1; i < width - 1; i++) {
        printf("%c", BOX_HORIZONTAL);
    }
    printf("%c", BOX_BOTTOM_RIGHT);
}

void draw_ui(UIState *state, AppointmentList *appointments, TodoList *todos) {
    clear_screen();
    
    // Calculate panel dimensions - give appointments panel extra 10 characters
    int appointments_width = (state->window_width / 3) + 10;
    int todo_width = state->window_width / 3;
    int calendar_width = state->window_width - appointments_width - todo_width;
    int panel_height = state->window_height - 3; // Leave room for status bar
    
    // Draw panels
    draw_appointments_panel(state, appointments, 0, 0, appointments_width, panel_height);
    draw_calendar_panel(state, appointments_width, 0, calendar_width, panel_height, appointments);
    draw_todo_panel(state, todos, appointments_width + calendar_width, 0, todo_width, panel_height);
    
    // Draw status bar
    draw_status_bar(state, state->window_height - 2, state->window_width);
}

void draw_calendar_panel(UIState *state, int x, int y, int width, int height, AppointmentList *appointments) {
    char title[32];
    sprintf_s(title, sizeof(title), "Calendar");
    draw_box(x, y, width, height, title);
    
    // Calendar content area
    int content_x = x + 2;
    int content_y = y + 2;
    int content_width = width - 4;
    int content_height = height - 4;
    
    // Month and year header
    set_color(HEADER_FG, HEADER_BG);
    gotoxy(content_x + (content_width - 20) / 2, content_y);
    printf("%s %d", get_month_name(state->selected_date.month), state->selected_date.year);
    
    // Day headers
    set_color(NORMAL_FG, NORMAL_BG);
    gotoxy(content_x + 4, content_y + 2);
    printf("Sun Mon Tue Wed Thu Fri Sat");
    
    // Calculate first day of month
    int first_day = get_first_day_of_month(state->selected_date.year, state->selected_date.month);
    int days_in_month = get_days_in_month(state->selected_date.year, state->selected_date.month);
    
    // Draw calendar days
    int day = 1;
    int week = 0;
    
    while (day <= days_in_month) {
        gotoxy(content_x, content_y + 4 + week * 2);
        
        // Print week number
        set_color(COLOR_GRAY, NORMAL_BG);
        printf("%2d ", get_week_number(state->selected_date.year, state->selected_date.month, day));
        
        // Print days
        for (int dow = 0; dow < 7; dow++) {
            if (week == 0 && dow < first_day) {
                printf("    ");
            } else if (day <= days_in_month) {
                // Create date for this day
                Date current_day = {state->selected_date.year, state->selected_date.month, day};
                
                // Check if this day has appointments
                int has_appointments = has_appointment_on_date(appointments, current_day);
                
                // Check if this is today
                if (day == state->current_date.day &&
                    state->selected_date.month == state->current_date.month &&
                    state->selected_date.year == state->current_date.year) {
                    set_color(TODAY_FG, TODAY_BG);
                }
                // Check if this is selected
                else if (day == state->selected_date.day && state->selected_view == VIEW_CALENDAR) {
                    set_color(SELECTED_FG, SELECTED_BG);
                } else {
                    set_color(NORMAL_FG, NORMAL_BG);
                }
                
                // Print day with appointment indicator
                if (has_appointments) {
                    printf(" %2d*", day);  // Add asterisk for appointments
                } else {
                    printf(" %2d ", day);
                }
                day++;
            } else {
                printf("    ");
            }
        }
        week++;
    }
    
    set_color(NORMAL_FG, NORMAL_BG);
}

void draw_appointments_panel(UIState *state, AppointmentList *appointments, int x, int y, int width, int height) {
    draw_box(x, y, width, height, "Appointments");
    
    // Content area
    int content_x = x + 2;
    int content_y = y + 2;
    int content_width = width - 4;
    
    // Show date
    set_color(HEADER_FG, HEADER_BG);
    gotoxy(content_x, content_y);
    printf("%s %d, %d", get_month_name(state->selected_date.month), 
           state->selected_date.day, state->selected_date.year);
    
    // Show appointments for selected date
    set_color(NORMAL_FG, NORMAL_BG);
    int line = 0;
    int found = 0;
    int appointment_indices[100];  // Store actual appointment indices
    int appointment_count = 0;
    
    // Find all appointments that span the selected date using the existing function
    appointment_count = find_appointments_by_date(appointments, state->selected_date, appointment_indices, 100);
    
    for (int idx = 0; idx < appointment_count; idx++) {
        int i = appointment_indices[idx];  // Actual appointment index
        
        if (line >= state->appointment_scroll && line - state->appointment_scroll < height - 6) {
            gotoxy(content_x, content_y + 2 + (line - state->appointment_scroll));
            
            // Highlight if selected
            if (state->selected_view == VIEW_APPOINTMENTS && line == state->cursor_y) {
                set_color(SELECTED_FG, SELECTED_BG);
            }
            
            // Check if this is a multi-day event
            int is_multiday = 0;
            DateTime end_time = appointments->items[i].date_time;
            
            if (appointments->items[i].duration_minutes > 0) {
                // Calculate end date and time
                int total_minutes = end_time.minute + appointments->items[i].duration_minutes;
                
                end_time.minute = total_minutes % 60;
                int total_hours = end_time.hour + (total_minutes / 60);
                
                end_time.hour = total_hours % 24;
                int total_days = total_hours / 24;
                
                // Add days
                end_time.day += total_days;
                
                // Handle month/year overflow (simplified)
                while (end_time.day > get_days_in_month(end_time.year, end_time.month)) {
                    end_time.day -= get_days_in_month(end_time.year, end_time.month);
                    end_time.month++;
                    if (end_time.month > 12) {
                        end_time.month = 1;
                        end_time.year++;
                    }
                }
                
                // Check if it spans multiple days
                is_multiday = (appointments->items[i].date_time.year != end_time.year ||
                              appointments->items[i].date_time.month != end_time.month ||
                              appointments->items[i].date_time.day != end_time.day);
            }
            
            if (is_multiday && appointments->items[i].duration_minutes > 0) {
                // Multi-day format: "Jul 21, 2025 01:00 -> Jul 24, 2025 10:00"
                const char* months[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
                printf("- %s %d, %d %02d:%02d -> %s %d, %d %02d:%02d",
                       months[appointments->items[i].date_time.month],
                       appointments->items[i].date_time.day,
                       appointments->items[i].date_time.year,
                       appointments->items[i].date_time.hour,
                       appointments->items[i].date_time.minute,
                       months[end_time.month],
                       end_time.day,
                       end_time.year,
                       end_time.hour,
                       end_time.minute);
            } else {
                // Single day format: "01:00 -> 15:00" or just "01:00"
                printf("- %02d:%02d", 
                       appointments->items[i].date_time.hour,
                       appointments->items[i].date_time.minute);
                
                if (appointments->items[i].duration_minutes > 0) {
                    printf(" -> %02d:%02d", end_time.hour, end_time.minute);
                }
            }
            
            set_color(NORMAL_FG, NORMAL_BG);
            
            // Description on next line
            gotoxy(content_x + 2, content_y + 3 + (line - state->appointment_scroll));
            printf("%.30s", appointments->items[i].description);
            
            line += 2;
        } else {
            line += 2; // Still increment line count for appointments that are scrolled off
        }
        found = 1;
    }
    
    if (!found) {
        gotoxy(content_x, content_y + 2);
        printf("(none)");
    }
}

void draw_todo_panel(UIState *state, TodoList *todos, int x, int y, int width, int height) {
    draw_box(x, y, width, height, "TODO");
    
    // Content area
    int content_x = x + 2;
    int content_y = y + 2;
    int content_width = width - 4;
    
    set_color(NORMAL_FG, NORMAL_BG);
    
    int visible_todos = 0;
    for (int i = 0; i < todos->count && visible_todos < height - 4; i++) {
        if (i >= state->todo_scroll) {
            gotoxy(content_x, content_y + visible_todos);
            
            // Highlight if selected
            if (state->selected_view == VIEW_TODO && visible_todos == state->cursor_y) {
                set_color(SELECTED_FG, SELECTED_BG);
            }
            
            // Priority indicator
            char priority_char = ' ';
            if (todos->items[i].priority == 1) priority_char = '!';
            else if (todos->items[i].priority == 2) priority_char = '*';
            
            // Completion status
            char status = todos->items[i].completed ? 'X' : ' ';
            
            printf("%d. [%c] %c %.25s", i + 1, status, priority_char, todos->items[i].description);
            
            set_color(NORMAL_FG, NORMAL_BG);
            visible_todos++;
        }
    }
}

void draw_status_bar(UIState *state, int y, int width) {
    // Clear status bar area
    gotoxy(0, y);
    set_color(NORMAL_FG, NORMAL_BG);
    for (int i = 0; i < width; i++) printf(" ");
    
    // Draw status bar content
    gotoxy(2, y);
    printf("Help:h  Quit:q  Add:a  Delete:d  Edit:e  Tab:Switch View");
    
    // Show current view
    gotoxy(width - 20, y);
    switch (state->selected_view) {
        case VIEW_CALENDAR:
            printf("[Calendar View]");
            break;
        case VIEW_APPOINTMENTS:
            printf("[Appointments]");
            break;
        case VIEW_TODO:
            printf("[TODO List]");
            break;
    }
}

void draw_help_screen(UIState *state) {
    clear_screen();
    
    int help_x = state->window_width / 2 - 35;
    int help_y = 2;
    
    // Clear the background area first
    clear_area(help_x, help_y, 70, 20);
    
    draw_box(help_x, help_y, 70, 20, "Help");
    
    set_color(HEADER_FG, HEADER_BG);
    gotoxy(help_x + 2, help_y + 2);
    printf("KEYBOARD SHORTCUTS");
    
    set_color(NORMAL_FG, NORMAL_BG);
    
    // Navigation section
    gotoxy(help_x + 2, help_y + 4);
    printf("Navigation:");
    gotoxy(help_x + 4, help_y + 5);
    printf("Arrow Keys     Move cursor/Navigate calendar");
    gotoxy(help_x + 4, help_y + 6);
    printf("Tab            Switch between panels");
    gotoxy(help_x + 4, help_y + 7);
    printf("PgUp/PgDn      Previous/Next month or scroll");
    gotoxy(help_x + 4, help_y + 8);
    printf("Home           Jump to today");
    
    // Actions section
    gotoxy(help_x + 2, help_y + 10);
    printf("Actions:");
    gotoxy(help_x + 4, help_y + 11);
    printf("a              Add appointment or todo");
    gotoxy(help_x + 4, help_y + 12);
    printf("d              Delete selected item");
    gotoxy(help_x + 4, help_y + 13);
    printf("e              Edit selected item");
    gotoxy(help_x + 4, help_y + 14);
    printf("Space          Toggle todo completion");
    gotoxy(help_x + 4, help_y + 15);
    printf("q              Quit and save");
    
    gotoxy(help_x + 2, help_y + 17);
    set_color(HEADER_FG, HEADER_BG);
    printf("Press any key to return...");
    
    set_color(NORMAL_FG, NORMAL_BG);
    _getch();
}