#include "todo.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

void init_todos(TodoList *list) {
    list->capacity = 50;
    list->count = 0;
    list->items = (TodoItem*)malloc(sizeof(TodoItem) * list->capacity);
}

void free_todos(TodoList *list) {
    if (list->items) {
        free(list->items);
        list->items = NULL;
    }
    list->count = 0;
    list->capacity = 0;
}

int add_todo(TodoList *list, TodoItem *todo) {
    // Resize if necessary
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        TodoItem *new_items = (TodoItem*)realloc(list->items, sizeof(TodoItem) * list->capacity);
        if (!new_items) return 0;
        list->items = new_items;
    }
    
    list->items[list->count] = *todo;
    list->count++;
    
    // Keep sorted
    sort_todos(list);
    
    return 1;
}

int delete_todo(TodoList *list, int index) {
    if (index < 0 || index >= list->count) return 0;
    
    // Shift items
    for (int i = index; i < list->count - 1; i++) {
        list->items[i] = list->items[i + 1];
    }
    
    list->count--;
    return 1;
}

int edit_todo(TodoList *list, int index, TodoItem *new_todo) {
    if (index < 0 || index >= list->count) return 0;
    
    list->items[index] = *new_todo;
    sort_todos(list);
    
    return 1;
}

void toggle_todo_completion(TodoList *list, int index) {
    if (index >= 0 && index < list->count) {
        list->items[index].completed = !list->items[index].completed;
    }
}

// Comparison function for sorting todos
static int compare_todos(const void *a, const void *b) {
    TodoItem *todo1 = (TodoItem*)a;
    TodoItem *todo2 = (TodoItem*)b;
    
    // Sort by: completed status (uncompleted first), then priority (higher first)
    if (todo1->completed != todo2->completed) {
        return todo1->completed - todo2->completed;
    }
    
    return todo2->priority - todo1->priority;
}

void sort_todos(TodoList *list) {
    qsort(list->items, list->count, sizeof(TodoItem), compare_todos);
}

// Helper function to read a line with visual feedback (same as in appointments.c)
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

void add_todo_interactive(TodoList *list) {
    TodoItem new_todo;
    char buffer[256];
    
    // Get window dimensions for centering
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int window_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int window_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    
    // Clear a section for input
    int input_y = window_height / 2 - 4;
    int input_x = window_width / 2 - 30;
    
    // Clear the background area first
    clear_area(input_x, input_y, 60, 8);
    
    // Draw input box
    draw_box(input_x, input_y, 60, 8, "Add TODO");
    
    set_color(NORMAL_FG, NORMAL_BG);
    
    // Get description
    gotoxy(input_x + 2, input_y + 2);
    printf("Description: ");
    read_line_visual(new_todo.description, MAX_TODO_DESCRIPTION - 1, input_x + 15, input_y + 2);
    
    if (strlen(new_todo.description) == 0) return;  // Cancelled
    
    // Get priority
    gotoxy(input_x + 2, input_y + 3);
    printf("Priority (0=Normal, 1=High, 2=Urgent): ");
    read_line_visual(buffer, 10, input_x + 41, input_y + 3);
    
    new_todo.priority = atoi(buffer);
    if (new_todo.priority < 0) new_todo.priority = 0;
    if (new_todo.priority > 2) new_todo.priority = 2;
    
    new_todo.completed = 0;
    
    // Add the todo
    add_todo(list, &new_todo);
}

void edit_todo_interactive(TodoList *list, int index) {
    if (index < 0 || index >= list->count) return;
    
    TodoItem *todo = &list->items[index];
    char buffer[256];
    
    // Similar implementation to add_todo_interactive
    // but pre-fill with existing values...
}