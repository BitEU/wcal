#ifndef APPOINTMENTS_H
#define APPOINTMENTS_H

#include "calendar.h"

#define MAX_APPOINTMENTS 1000
#define MAX_DESCRIPTION_LENGTH 256

// Forward declaration
struct UIState;

// Appointment structure
typedef struct {
    DateTime date_time;
    char description[MAX_DESCRIPTION_LENGTH];
    int duration_minutes;
} Appointment;

// Appointment list
typedef struct {
    Appointment *items;
    int count;
    int capacity;
} AppointmentList;

// Appointment functions
void init_appointments(AppointmentList *list);
void free_appointments(AppointmentList *list);
int add_appointment(AppointmentList *list, Appointment *appointment);
int delete_appointment(AppointmentList *list, int index);
int edit_appointment(AppointmentList *list, int index, Appointment *new_appointment);
void sort_appointments(AppointmentList *list);
int find_appointments_by_date(AppointmentList *list, Date date, int *indices, int max_indices);
int has_appointment_on_date(AppointmentList *list, Date date);

// Interactive functions
void add_appointment_interactive(AppointmentList *list, struct UIState *state);
void edit_appointment_interactive(AppointmentList *list, int index);

#endif // APPOINTMENTS_H