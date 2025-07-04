#ifndef TODO_H
#define TODO_H

#define MAX_TODOS 500
#define MAX_TODO_DESCRIPTION 256

// TODO item structure
typedef struct {
    char description[MAX_TODO_DESCRIPTION];
    int priority;  // 0 = normal, 1 = high, 2 = urgent
    int completed;
} TodoItem;

// TODO list
typedef struct {
    TodoItem *items;
    int count;
    int capacity;
} TodoList;

// TODO functions
void init_todos(TodoList *list);
void free_todos(TodoList *list);
int add_todo(TodoList *list, TodoItem *todo);
int delete_todo(TodoList *list, int index);
int edit_todo(TodoList *list, int index, TodoItem *new_todo);
void toggle_todo_completion(TodoList *list, int index);
void sort_todos(TodoList *list);

// Interactive functions
void add_todo_interactive(TodoList *list);
void edit_todo_interactive(TodoList *list, int index);

#endif // TODO_H