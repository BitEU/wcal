#include "storage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// For ZIP functionality - using Windows native ZIP API through PowerShell commands
// This is a simple approach that works cross-platform

// Helper function to execute PowerShell commands for ZIP operations
static int execute_powershell_command(const char *command) {
    return system(command);
}

// Helper function to escape CSV fields
static void escape_csv_field(const char *input, char *output, size_t output_size) {
    size_t j = 0;
    output[j++] = '"'; // Start with quote
    
    for (size_t i = 0; input[i] && j < output_size - 3; i++) {
        if (input[i] == '"') {
            output[j++] = '"'; // Escape quote with double quote
            output[j++] = '"';
        } else {
            output[j++] = input[i];
        }
    }
    
    output[j++] = '"'; // End with quote
    output[j] = '\0';
}

// Helper function to format ICS date-time
static void format_ics_datetime(DateTime dt, char *buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%04d%02d%02dT%02d%02d00", 
             dt.year, dt.month, dt.day, dt.hour, dt.minute);
}

// Helper function to generate ICS UID
static void generate_ics_uid(const Appointment *appt, char *uid, size_t uid_size) {
    snprintf(uid, uid_size, "%04d%02d%02d%02d%02d-%s@wcal.local",
             appt->date_time.year, appt->date_time.month, appt->date_time.day,
             appt->date_time.hour, appt->date_time.minute, 
             appt->description);
    
    // Replace spaces and special characters in UID with hyphens
    for (size_t i = 0; uid[i]; i++) {
        if (uid[i] == ' ' || uid[i] == '\t' || uid[i] == '\n' || uid[i] == '@') {
            uid[i] = '-';
        }
    }
}

int save_appointments_as_ics(AppointmentList *list, const char *filename) {
    FILE *file;
    if (fopen_s(&file, filename, "w") != 0) return 0;
    
    // Write ICS header
    fprintf(file, "BEGIN:VCALENDAR\r\n");
    fprintf(file, "VERSION:2.0\r\n");
    fprintf(file, "PRODID:-//WCAL//Calendar Application//EN\r\n");
    fprintf(file, "CALSCALE:GREGORIAN\r\n");
    
    // Write appointments as VEVENT entries
    for (int i = 0; i < list->count; i++) {
        char start_time[32], end_time[32], uid[512];
        DateTime end_dt = list->items[i].date_time;
        
        // Calculate end time
        end_dt.minute += list->items[i].duration_minutes;
        while (end_dt.minute >= 60) {
            end_dt.hour++;
            end_dt.minute -= 60;
        }
        while (end_dt.hour >= 24) {
            end_dt.day++;
            end_dt.hour -= 24;
            // Note: This is simplified - proper date arithmetic would handle month/year rollover
        }
        
        format_ics_datetime(list->items[i].date_time, start_time, sizeof(start_time));
        format_ics_datetime(end_dt, end_time, sizeof(end_time));
        generate_ics_uid(&list->items[i], uid, sizeof(uid));
        
        fprintf(file, "BEGIN:VEVENT\r\n");
        fprintf(file, "UID:%s\r\n", uid);
        fprintf(file, "DTSTART:%s\r\n", start_time);
        fprintf(file, "DTEND:%s\r\n", end_time);
        fprintf(file, "SUMMARY:%s\r\n", list->items[i].description);
        fprintf(file, "DESCRIPTION:Duration: %d minutes\r\n", list->items[i].duration_minutes);
        fprintf(file, "END:VEVENT\r\n");
    }
    
    // Write ICS footer
    fprintf(file, "END:VCALENDAR\r\n");
    
    fclose(file);
    return 1;
}

int save_todos_as_csv(TodoList *list, const char *filename) {
    FILE *file;
    if (fopen_s(&file, filename, "w") != 0) return 0;
    
    // Write CSV header
    fprintf(file, "Description,Priority,Completed\n");
    
    // Write todo items
    for (int i = 0; i < list->count; i++) {
        char escaped_desc[MAX_TODO_DESCRIPTION * 2 + 4];
        escape_csv_field(list->items[i].description, escaped_desc, sizeof(escaped_desc));
        
        const char *priority_str;
        switch (list->items[i].priority) {
            case 1: priority_str = "High"; break;
            case 2: priority_str = "Urgent"; break;
            default: priority_str = "Normal"; break;
        }
        
        fprintf(file, "%s,%s,%s\n", 
                escaped_desc, 
                priority_str,
                list->items[i].completed ? "Yes" : "No");
    }
    
    fclose(file);
    return 1;
}

int load_appointments_from_ics(AppointmentList *list, const char *filename) {
    FILE *file;
    if (fopen_s(&file, filename, "r") != 0) return 0;
    
    char line[512];
    Appointment current_appt;
    int in_event = 0;
    int event_complete = 0;
    
    // Clear the list
    list->count = 0;
    
    while (fgets(line, sizeof(line), file)) {
        // Remove trailing whitespace
        char *end = line + strlen(line) - 1;
        while (end > line && (*end == '\n' || *end == '\r' || *end == ' ' || *end == '\t')) {
            *end-- = '\0';
        }
        
        if (strcmp(line, "BEGIN:VEVENT") == 0) {
            in_event = 1;
            event_complete = 0;
            memset(&current_appt, 0, sizeof(current_appt));
        } else if (strcmp(line, "END:VEVENT") == 0 && in_event) {
            if (event_complete) {
                if (list->count < list->capacity) {
                    list->items[list->count++] = current_appt;
                }
            }
            in_event = 0;
        } else if (in_event && strncmp(line, "DTSTART:", 8) == 0) {
            // Parse DTSTART: YYYYMMDDTHHMMSS
            char *datetime_str = line + 8;
            if (strlen(datetime_str) >= 15) {
                sscanf(datetime_str, "%4d%2d%2dT%2d%2d",
                       &current_appt.date_time.year,
                       &current_appt.date_time.month,
                       &current_appt.date_time.day,
                       &current_appt.date_time.hour,
                       &current_appt.date_time.minute);
            }
        } else if (in_event && strncmp(line, "DTEND:", 6) == 0) {
            // Parse DTEND to calculate duration
            char *datetime_str = line + 6;
            if (strlen(datetime_str) >= 15) {
                DateTime end_dt;
                sscanf(datetime_str, "%4d%2d%2dT%2d%2d",
                       &end_dt.year, &end_dt.month, &end_dt.day,
                       &end_dt.hour, &end_dt.minute);
                
                // Simple duration calculation (assumes same day)
                int start_minutes = current_appt.date_time.hour * 60 + current_appt.date_time.minute;
                int end_minutes = end_dt.hour * 60 + end_dt.minute;
                current_appt.duration_minutes = end_minutes - start_minutes;
                if (current_appt.duration_minutes <= 0) {
                    current_appt.duration_minutes = 60; // Default to 1 hour
                }
            }
        } else if (in_event && strncmp(line, "SUMMARY:", 8) == 0) {
            strncpy(current_appt.description, line + 8, MAX_DESCRIPTION_LENGTH - 1);
            current_appt.description[MAX_DESCRIPTION_LENGTH - 1] = '\0';
            event_complete = 1;
        }
    }
    
    fclose(file);
    sort_appointments(list);
    return 1;
}

int load_todos_from_csv(TodoList *list, const char *filename) {
    FILE *file;
    if (fopen_s(&file, filename, "r") != 0) return 0;
    
    char line[512];
    int first_line = 1;
    
    // Clear the list
    list->count = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (first_line) {
            first_line = 0;
            continue; // Skip header
        }
        
        // Remove trailing whitespace
        char *end = line + strlen(line) - 1;
        while (end > line && (*end == '\n' || *end == '\r')) {
            *end-- = '\0';
        }
        
        if (strlen(line) == 0) continue;
        
        TodoItem todo;
        memset(&todo, 0, sizeof(todo));
        
        // Simple CSV parsing (assumes quotes around description)
        char *desc_start = strchr(line, '"');
        if (desc_start) {
            desc_start++; // Skip opening quote
            char *desc_end = strchr(desc_start, '"');
            if (desc_end) {
                size_t desc_len = desc_end - desc_start;
                if (desc_len < MAX_TODO_DESCRIPTION) {
                    strncpy(todo.description, desc_start, desc_len);
                    todo.description[desc_len] = '\0';
                    
                    // Parse priority and completion
                    char *rest = desc_end + 2; // Skip quote and comma
                    if (strncmp(rest, "High", 4) == 0) {
                        todo.priority = 1;
                    } else if (strncmp(rest, "Urgent", 6) == 0) {
                        todo.priority = 2;
                    } else {
                        todo.priority = 0;
                    }
                    
                    char *completed_start = strrchr(rest, ',');
                    if (completed_start) {
                        completed_start++;
                        todo.completed = (strncmp(completed_start, "Yes", 3) == 0) ? 1 : 0;
                    }
                    
                    if (list->count < list->capacity) {
                        list->items[list->count++] = todo;
                    }
                }
            }
        }
    }
    
    fclose(file);
    sort_todos(list);
    return 1;
}

int save_data_to_zip(AppointmentList *appointments, TodoList *todos) {
    char command[1024];
    
    // Save appointments as ICS
    if (!save_appointments_as_ics(appointments, TEMP_ICS_FILE)) {
        return 0;
    }
    
    // Save todos as CSV
    if (!save_todos_as_csv(todos, TEMP_CSV_FILE)) {
        remove(TEMP_ICS_FILE);
        return 0;
    }
    
    // Remove existing archive if it exists
    remove(ARCHIVE_NAME);
    
    // Create ZIP archive using PowerShell
    snprintf(command, sizeof(command),
             "powershell -Command \"Compress-Archive -Path '%s','%s' -DestinationPath '%s' -Force\"",
             TEMP_ICS_FILE, TEMP_CSV_FILE, ARCHIVE_NAME);
    
    int result = execute_powershell_command(command);
    
    // Clean up temporary files
    remove(TEMP_ICS_FILE);
    remove(TEMP_CSV_FILE);
    
    return (result == 0);
}

int load_data_from_zip(AppointmentList *appointments, TodoList *todos) {
    char command[1024];
    
    // Check if archive exists
    FILE *test_file;
    if (fopen_s(&test_file, ARCHIVE_NAME, "r") != 0) {
        return 0; // Archive doesn't exist
    }
    fclose(test_file);
    
    // Extract files from ZIP using PowerShell
    snprintf(command, sizeof(command),
             "powershell -Command \"Expand-Archive -Path '%s' -DestinationPath '.' -Force\"",
             ARCHIVE_NAME);
    
    if (execute_powershell_command(command) != 0) {
        return 0;
    }
    
    // Load appointments from ICS
    int appt_result = load_appointments_from_ics(appointments, TEMP_ICS_FILE);
    
    // Load todos from CSV
    int todo_result = load_todos_from_csv(todos, TEMP_CSV_FILE);
    
    // Clean up extracted files
    remove(TEMP_ICS_FILE);
    remove(TEMP_CSV_FILE);
    
    return (appt_result && todo_result);
}

// Keep original functions for backward compatibility
int save_appointments(AppointmentList *list, const char *filename) {
    return save_appointments_as_ics(list, filename);
}

int load_appointments(AppointmentList *list, const char *filename) {
    return load_appointments_from_ics(list, filename);
}

int save_todos(TodoList *list, const char *filename) {
    return save_todos_as_csv(list, filename);
}

int load_todos(TodoList *list, const char *filename) {
    return load_todos_from_csv(list, filename);
}