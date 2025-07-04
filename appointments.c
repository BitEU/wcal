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
        if (list->items[i].date_time.year == date.year &&
            list->items[i].date_time.month == date.month &&
            list->items[i].date_time.day == date.day) {
            indices[count++] = i;
        }
    }
    
    return count;
}

int has_appointment_on_date(AppointmentList *list, Date date) {
    for (int i = 0; i < list->count; i++) {
        if (list->items[i].date_time.year == date.year &&
            list->items[i].date_time.month == date.month &&
            list->items[i].date_time.day == date.day) {
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
    printf("Duration (minutes, 0 for no end): ");
    read_line_visual(buffer, 10, input_x + 36, input_y + 3);
    
    new_app.duration_minutes = atoi(buffer);
    
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