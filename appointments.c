#include "appointments.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <ctype.h>

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

// Helper function to check if an appointment spans across a given date
static int appointment_spans_date(Appointment *appointment, Date date) {
    if (appointment->duration_minutes <= 0) {
        // No duration, only check start date
        return (appointment->date_time.year == date.year &&
                appointment->date_time.month == date.month &&
                appointment->date_time.day == date.day);
    }
    
    // Calculate end date and time
    DateTime end_time = appointment->date_time;
    int total_minutes = end_time.minute + appointment->duration_minutes;
    
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
    
    // Check if the given date falls between start and end dates (inclusive)
    DateTime check_date = {date.year, date.month, date.day, 0, 0};
    
    // Convert dates to comparable values (simplified)
    long start_value = appointment->date_time.year * 10000L + appointment->date_time.month * 100L + appointment->date_time.day;
    long end_value = end_time.year * 10000L + end_time.month * 100L + end_time.day;
    long check_value = check_date.year * 10000L + check_date.month * 100L + check_date.day;
    
    return (check_value >= start_value && check_value <= end_value);
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
        if (appointment_spans_date(&list->items[i], date)) {
            indices[count++] = i;
        }
    }
    
    return count;
}

int has_appointment_on_date(AppointmentList *list, Date date) {
    for (int i = 0; i < list->count; i++) {
        if (appointment_spans_date(&list->items[i], date)) {
            return 1;
        }
    }
    return 0;
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
    int input_x = ui_state->window_width / 2 - 35;
    
    // Clear the background area first
    clear_area(input_x, input_y, 70, 12);
    
    // Draw input box
    draw_box(input_x, input_y, 70, 12, "Add Appointment");
    
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
    printf("Duration (e.g., 3d2h30m, 4h, 30m, 0 for no end): ");
    read_line_visual(buffer, 20, input_x + 49, input_y + 3);
    
    if (strlen(buffer) == 0) return;  // Cancelled
    
    // Check if it's just "0" for no end time
    if (strcmp(buffer, "0") == 0) {
        new_app.duration_minutes = 0;
    } else {
        // Parse the new duration format
        int parsed_duration = parse_duration_string(buffer);
        if (parsed_duration < 0) {
            // Invalid format - show error and return
            gotoxy(input_x + 2, input_y + 5);
            printf("Invalid duration format! Press any key...");
            _getch();
            return;
        }
        new_app.duration_minutes = parsed_duration;
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
    // TODO: Implement editing functionality
    (void)app; // Suppress unused variable warning
}

// Parse duration string in format like "3d2h30m", "4h", "30m", etc.
// Returns total duration in minutes, or -1 if invalid format
int parse_duration_string(const char *duration_str) {
    if (!duration_str || strlen(duration_str) == 0) {
        return 0;  // Empty string means 0 duration
    }
    
    int total_minutes = 0;
    int current_number = 0;
    int has_number = 0;
    
    for (int i = 0; duration_str[i] != '\0'; i++) {
        char c = duration_str[i];
        
        if (isdigit(c)) {
            current_number = current_number * 10 + (c - '0');
            has_number = 1;
        } else if (c == 'd' || c == 'D') {
            if (!has_number) return -1;  // Invalid: no number before 'd'
            total_minutes += current_number * 24 * 60;  // days to minutes
            current_number = 0;
            has_number = 0;
        } else if (c == 'h' || c == 'H') {
            if (!has_number) return -1;  // Invalid: no number before 'h'
            total_minutes += current_number * 60;  // hours to minutes
            current_number = 0;
            has_number = 0;
        } else if (c == 'm' || c == 'M') {
            if (!has_number) return -1;  // Invalid: no number before 'm'
            total_minutes += current_number;  // minutes
            current_number = 0;
            has_number = 0;
        } else if (isspace(c)) {
            // Skip whitespace
            continue;
        } else {
            // Invalid character
            return -1;
        }
    }
    
    // If we ended with a number but no unit, that's invalid
    if (has_number) {
        return -1;
    }
    
    return total_minutes;
}