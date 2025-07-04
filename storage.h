#ifndef STORAGE_H
#define STORAGE_H

#include "appointments.h"
#include "todo.h"

// File formats version
#define STORAGE_VERSION 1

// Archive and temporary file names
#define ARCHIVE_NAME "wcal_data.zip"
#define TEMP_ICS_FILE "temp_appointments.ics"
#define TEMP_CSV_FILE "temp_todos.csv"
#define ICS_FILE_IN_ZIP "appointments.ics"
#define CSV_FILE_IN_ZIP "todos.csv"

// Storage functions
int save_appointments(AppointmentList *list, const char *filename);
int load_appointments(AppointmentList *list, const char *filename);
int save_todos(TodoList *list, const char *filename);
int load_todos(TodoList *list, const char *filename);

// New ZIP-based storage functions
int save_data_to_zip(AppointmentList *appointments, TodoList *todos);
int load_data_from_zip(AppointmentList *appointments, TodoList *todos);

// Helper functions for format conversion
int save_appointments_as_ics(AppointmentList *list, const char *filename);
int load_appointments_from_ics(AppointmentList *list, const char *filename);
int save_todos_as_csv(TodoList *list, const char *filename);
int load_todos_from_csv(TodoList *list, const char *filename);

#endif // STORAGE_H