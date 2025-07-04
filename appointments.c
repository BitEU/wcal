#include "appointments.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

void init_appointments(AppointmentList *list) {
    list->capacity = 100;
    list->count = 0;
    list->items = (Appointment*)malloc(sizeof(Appointment) * list->capacity);
}

void free_appointments(AppointmentList *list) {
    if (list->items) {
        free(list->items);
        list->items = NULL;
    }
    list->count = 0;
    list->capacity = 0;
}

int add_appointment(AppointmentList *list, Appointment *appointment) {
    // Resize if necessary
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        Appointment *new_items = (Appointment*)realloc(list->items, sizeof(Appointment) * list->capacity);
        if (!new_items) return 0;
        list->items = new_items;
    }
    
    list->items[list->count] = *appointment;
    list->count++;
    
    // Keep sorted
    sort_appointments(list);
    
    return 1;
}

int delete_appointment(AppointmentList *list, int index) {
    if (index < 0 || index >= list->count) return 0;
    
    // Shift items
    for (int i = index; i < list->count - 1; i++) {
        list->items[i] = list->items[i + 1];
    }
    
    list->count--;
    return 1;
}

int edit_appointment(AppointmentList *list, int index, Appointment *new_appointment) {
    if (index < 0 || index >= list->count) return 0;
    
    list->items[index] = *new_appointment;
    sort_appointments(list);
    
    return 1;
}

// Comparison function for sorting
static int compare_appointments(const void *a, const void *b) {
    Appointment *app1 = (Appointment*)a;
    Appointment *app2 = (Appointment*)b;
    return compare_datetimes(app1->date_time, app2->date_time);
}

void sort_appointments(AppointmentList *list) {
    qsort(list->items, list->count, sizeof(Appointment), compare_appointments);
}

int find_appointments_by_date(AppointmentList *list, Date date, int *indices, int max_indices) {
    int count = 0;
    
    for (int i = 0; i < list->count && count < max_indices; i++) {
        Appointment *app = &list->items[i];
        
        // Calculate end date/time for multi-day appointments
        DateTime end_time = app->date_time;
        int remaining_minutes = app->duration_minutes;
        
        // Add minutes to get end time
        end_time.minute += remaining_minutes;
        while (end_time.minute >= 60) {
            end_time.hour++;
            end_time.minute -= 60;
        }
        while (end_time.hour >= 24) {
            end_time.day++;
            end_time.hour -= 24;
            
            // Handle month/year overflow
            int days_in_month = get_days_in_month(end_time.year, end_time.month);
            if (end_time.day > days_in_month) {
                end_time.day -= days_in_month;
                end_time.month++;
                if (end_time.month > 12) {
                    end_time.month = 1;
                    end_time.year++;
                }
            }
        }
        
        // Check if the given date falls within the appointment's span
        Date start_date = {app->date_time.year, app->date_time.month, app->date_time.day};
        Date end_date = {end_time.year, end_time.month, end_time.day};
        
        if (compare_dates(date, start_date) >= 0 && compare_dates(date, end_date) <= 0) {
            indices[count++] = i;
        }
    }
    
    return count;
}

int has_appointment_on_date(AppointmentList *list, Date date) {
    int indices[1];
    return find_appointments_by_date(list, date, indices, 1) > 0;
}

// Function to format duration in compact XdYhZm format
static void format_duration_compact(int total_minutes, char *buffer, int buffer_size) {
    buffer[0] = '\0';  // Start with empty string
    
    if (total_minutes == 0) {
        strcpy_s(buffer, buffer_size, "0m");
        return;
    }
    
    int days = total_minutes / (24 * 60);
    int remaining = total_minutes % (24 * 60);
    int hours = remaining / 60;
    int minutes = remaining % 60;
    
    char temp[32];
    
    if (days > 0) {
        sprintf_s(temp, sizeof(temp), "%dd", days);
        strcat_s(buffer, buffer_size, temp);
    }
    
    if (hours > 0) {
        sprintf_s(temp, sizeof(temp), "%dh", hours);
        strcat_s(buffer, buffer_size, temp);
    }
    
    if (minutes > 0) {
        sprintf_s(temp, sizeof(temp), "%dm", minutes);
        strcat_s(buffer, buffer_size, temp);
    }
}

// Helper function to read a line with visual feedback
static void read_line_visual(char *buffer, int max_len, int x, int y) {
    int pos = 0;
    char ch;
    
    // Show cursor
    CONSOLE_CURSOR_INFO cursorInfo;
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(hOut, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
    
    gotoxy(x, y);
    
    while (1) {
        ch = _getch();
        
        if (ch == '\r' || ch == '\n') {  // Enter
            buffer[pos] = '\0';
            break;
        } else if (ch == '\b' && pos > 0) {  // Backspace
            pos--;
            gotoxy(x + pos, y);
            printf(" ");
            gotoxy(x + pos, y);
        } else if (ch == 27) {  // Escape
            buffer[0] = '\0';
            break;
        } else if (ch >= 32 && ch < 127 && pos < max_len - 1) {  // Printable chars
            buffer[pos++] = ch;
            printf("%c", ch);
        }
    }
    
    // Hide cursor again
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
}

void add_appointment_interactive(AppointmentList *list, struct UIState *state) {
    UIState *ui_state = (UIState *)state;  // Cast to the full type
    Appointment new_app;
    char buffer[256];
    
    // Clear a section for input
    int input_y = ui_state->window_height / 2 - 5;
    int input_x = ui_state->window_width / 2 - 30;
    
    // Clear the background area first
    clear_area(input_x, input_y, 60, 10);
    
    // Draw input box
    draw_box(input_x, input_y, 60, 10, "Add Appointment");
    
    // Get date (default to selected date)
    new_app.date_time.year = ui_state->selected_date.year;
    new_app.date_time.month = ui_state->selected_date.month;
    new_app.date_time.day = ui_state->selected_date.day;
    
    set_color(NORMAL_FG, NORMAL_BG);
    
    // Get time
    gotoxy(input_x + 2, input_y + 2);
    printf("Time (HH:MM): ");
    read_line_visual(buffer, 10, input_x + 16, input_y + 2);
    
    if (strlen(buffer) == 0) return;  // Cancelled
    
    // Parse time
    int hour, minute;
    if (sscanf_s(buffer, "%d:%d", &hour, &minute) != 2) {
        return;  // Invalid format
    }
    
    new_app.date_time.hour = hour;
    new_app.date_time.minute = minute;
    
    // Get duration
    gotoxy(input_x + 2, input_y + 3);
    printf("Duration (e.g., 30m, 4h, 3d2h30m): ");
    read_line_visual(buffer, 20, input_x + 37, input_y + 3);
    
    // Parse duration string
    new_app.duration_minutes = 0;
    char *p = buffer;
    int num = 0;
    while (*p) {
        if (*p >= '0' && *p <= '9') {
            num = num * 10 + (*p - '0');
        } else if (*p == 'd' || *p == 'D') {
            new_app.duration_minutes += num * 24 * 60;
            num = 0;
        } else if (*p == 'h' || *p == 'H') {
            new_app.duration_minutes += num * 60;
            num = 0;
        } else if (*p == 'm' || *p == 'M') {
            new_app.duration_minutes += num;
            num = 0;
        }
        p++;
    }
    // If there's a number left without a suffix, assume minutes
    if (num > 0) {
        new_app.duration_minutes += num;
    }
    
    // Get description
    gotoxy(input_x + 2, input_y + 4);
    printf("Description: ");
    read_line_visual(new_app.description, MAX_DESCRIPTION_LENGTH - 1, input_x + 15, input_y + 4);
    
    if (strlen(new_app.description) == 0) return;  // Cancelled
    
    // Add the appointment
    add_appointment(list, &new_app);
}

void edit_appointment_interactive(AppointmentList *list, int index) {
    if (index < 0 || index >= list->count) return;
    
    Appointment *app = &list->items[index];
    Appointment new_app = *app;  // Copy current appointment
    char buffer[256];
    
    // Get window dimensions for centering
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int window_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int window_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    
    // Clear a section for input
    int input_y = window_height / 2 - 5;
    int input_x = window_width / 2 - 30;
    
    // Clear the background area first
    clear_area(input_x, input_y, 60, 12);
    
    // Draw input box
    draw_box(input_x, input_y, 60, 12, "Edit Appointment");
    
    set_color(NORMAL_FG, NORMAL_BG);
    
    // Show current values and get new ones
    
    // Time
    gotoxy(input_x + 2, input_y + 2);
    printf("Time (HH:MM) [%02d:%02d]: ", app->date_time.hour, app->date_time.minute);
    read_line_visual(buffer, 10, input_x + 26, input_y + 2);
    
    if (strlen(buffer) > 0) {
        int hour, minute;
        if (sscanf_s(buffer, "%d:%d", &hour, &minute) == 2) {
            new_app.date_time.hour = hour;
            new_app.date_time.minute = minute;
        }
    }
    
    // Duration
    gotoxy(input_x + 2, input_y + 3);
    char duration_str[32];
    format_duration_compact(app->duration_minutes, duration_str, sizeof(duration_str));
    printf("Duration [%s]: ", duration_str);
    // Calculate correct cursor position based on actual text length
    int cursor_x = input_x + 2 + 10 + (int)strlen(duration_str) + 3; // "Duration [" + duration + "]: "
    read_line_visual(buffer, 20, cursor_x, input_y + 3);
    
    if (strlen(buffer) > 0) {
        // Parse duration string (same as add)
        new_app.duration_minutes = 0;
        char *p = buffer;
        int num = 0;
        while (*p) {
            if (*p >= '0' && *p <= '9') {
                num = num * 10 + (*p - '0');
            } else if (*p == 'd' || *p == 'D') {
                new_app.duration_minutes += num * 24 * 60;
                num = 0;
            } else if (*p == 'h' || *p == 'H') {
                new_app.duration_minutes += num * 60;
                num = 0;
            } else if (*p == 'm' || *p == 'M') {
                new_app.duration_minutes += num;
                num = 0;
            }
            p++;
        }
        if (num > 0) {
            new_app.duration_minutes += num;
        }
    }
    
    // Description
    gotoxy(input_x + 2, input_y + 4);
    printf("Description:");
    gotoxy(input_x + 2, input_y + 5);
    printf("[%.50s]", app->description);
    gotoxy(input_x + 2, input_y + 6);
    printf("New: ");
    read_line_visual(buffer, MAX_DESCRIPTION_LENGTH - 1, input_x + 7, input_y + 6);
    
    if (strlen(buffer) > 0) {
        strcpy_s(new_app.description, MAX_DESCRIPTION_LENGTH, buffer);
    }
    
    // Update the appointment
    edit_appointment(list, index, &new_app);
}

int get_appointment_index_for_display(AppointmentList *list, Date date, int display_index) {
    int indices[100];  // Max appointments per day
    int count = find_appointments_by_date(list, date, indices, 100);
    
    if (display_index >= 0 && display_index < count) {
        return indices[display_index];
    }
    
    return -1;
}