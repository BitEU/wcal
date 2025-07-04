#ifndef STORAGE_H
#define STORAGE_H

#include "appointments.h"
#include "todo.h"

// File formats version
#define STORAGE_VERSION 1

// Storage functions
int save_appointments(AppointmentList *list, const char *filename);
int load_appointments(AppointmentList *list, const char *filename);
int save_todos(TodoList *list, const char *filename);
int load_todos(TodoList *list, const char *filename);

#endif // STORAGE_H