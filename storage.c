#include "storage.h"
#include <stdio.h>
#include <stdlib.h>

int save_appointments(AppointmentList *list, const char *filename) {
    FILE *file;
    if (fopen_s(&file, filename, "wb") != 0) return 0;
    
    // Write header
    int version = STORAGE_VERSION;
    fwrite(&version, sizeof(int), 1, file);
    
    // Write count
    fwrite(&list->count, sizeof(int), 1, file);
    
    // Write appointments
    for (int i = 0; i < list->count; i++) {
        fwrite(&list->items[i], sizeof(Appointment), 1, file);
    }
    
    fclose(file);
    return 1;
}

int load_appointments(AppointmentList *list, const char *filename) {
    FILE *file;
    if (fopen_s(&file, filename, "rb") != 0) return 0;
    
    // Read header
    int version;
    if (fread(&version, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    if (version != STORAGE_VERSION) {
        fclose(file);
        return 0;  // Version mismatch
    }
    
    // Read count
    int count;
    if (fread(&count, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    // Ensure capacity
    if (count > list->capacity) {
        list->capacity = count * 2;
        Appointment *new_items = (Appointment*)realloc(list->items, sizeof(Appointment) * list->capacity);
        if (!new_items) {
            fclose(file);
            return 0;
        }
        list->items = new_items;
    }
    
    // Read appointments
    list->count = 0;
    for (int i = 0; i < count; i++) {
        if (fread(&list->items[i], sizeof(Appointment), 1, file) == 1) {
            list->count++;
        }
    }
    
    fclose(file);
    
    // Sort to ensure proper order
    sort_appointments(list);
    
    return 1;
}

int save_todos(TodoList *list, const char *filename) {
    FILE *file;
    if (fopen_s(&file, filename, "wb") != 0) return 0;
    
    // Write header
    int version = STORAGE_VERSION;
    fwrite(&version, sizeof(int), 1, file);
    
    // Write count
    fwrite(&list->count, sizeof(int), 1, file);
    
    // Write todos
    for (int i = 0; i < list->count; i++) {
        fwrite(&list->items[i], sizeof(TodoItem), 1, file);
    }
    
    fclose(file);
    return 1;
}

int load_todos(TodoList *list, const char *filename) {
    FILE *file;
    if (fopen_s(&file, filename, "rb") != 0) return 0;
    
    // Read header
    int version;
    if (fread(&version, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    if (version != STORAGE_VERSION) {
        fclose(file);
        return 0;  // Version mismatch
    }
    
    // Read count
    int count;
    if (fread(&count, sizeof(int), 1, file) != 1) {
        fclose(file);
        return 0;
    }
    
    // Ensure capacity
    if (count > list->capacity) {
        list->capacity = count * 2;
        TodoItem *new_items = (TodoItem*)realloc(list->items, sizeof(TodoItem) * list->capacity);
        if (!new_items) {
            fclose(file);
            return 0;
        }
        list->items = new_items;
    }
    
    // Read todos
    list->count = 0;
    for (int i = 0; i < count; i++) {
        if (fread(&list->items[i], sizeof(TodoItem), 1, file) == 1) {
            list->count++;
        }
    }
    
    fclose(file);
    
    // Sort to ensure proper order
    sort_todos(list);
    
    return 1;
}