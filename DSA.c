#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

typedef struct ToDo todo;

struct ToDo {
    int priority;
    char description[101];
    char deadline[11];
    char category[11];
    todo* next;
    int count;
};

todo* start = NULL;

GtkWidget *window;
GtkWidget *text_view;
GtkTextBuffer *buffer;

void seetodo(GtkMenuItem *menu_item, gpointer user_data);
void createtodo(GtkMenuItem *menu_item, gpointer user_data);
void deletetodo(GtkMenuItem *menu_item, gpointer user_data);
void filterByPriority(GtkMenuItem *menu_item, gpointer user_data);
void filterByCategory(GtkMenuItem *menu_item, const gchar *category);
void adjustcount();
void showDueDateFilterDialog();
void filterByDueDate(const char *due_date);
void interface();


void seetodo(GtkMenuItem *menu_item, gpointer user_data) {
    todo* temp;
    temp = start;
    GtkTextIter iter;
    if (start == NULL) {
        gtk_text_buffer_set_text(buffer, "No tasks in your TODO list.", -1);
    } else {
        gtk_text_buffer_set_text(buffer, "Your TODO List:\n", -1);
        while (temp != NULL) {
            char task[256];
            snprintf(task, sizeof(task), "\nTask %d (Priority: %d, Deadline: %s, Category: %s): %s\n",
                     temp->count, temp->priority, temp->deadline, temp->category, temp->description);
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, task, -1);
            temp = temp->next;
        }
    }
}

void createtodo(GtkMenuItem *menu_item, gpointer user_data) {
    // Create a new dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Add a new task",
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        "Cancel",
        GTK_RESPONSE_CANCEL,
        "OK",
        GTK_RESPONSE_OK,
        NULL
    );

    // Create a content area within the dialog
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Create labels and entry fields for task description, deadline date, and category
    GtkWidget *description_label = gtk_label_new("Task Description:");
    GtkWidget *description_entry = gtk_entry_new();
    GtkWidget *deadline_label = gtk_label_new("Deadline Date (YYYY-MM-DD):");
    GtkWidget *deadline_entry = gtk_entry_new();
    GtkWidget *category_label = gtk_label_new("Task Category:");
    GtkWidget *category_entry = gtk_entry_new();

    // Create a label and a combo box for selecting the priority
    GtkWidget *priority_label = gtk_label_new("Priority:");
    GtkWidget *priority_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priority_combo), "High");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priority_combo), "Medium");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX(priority_combo), "Low");
    gtk_combo_box_set_active(GTK_COMBO_BOX(priority_combo), 0);

    // Pack widgets into the content area
    gtk_container_add(GTK_CONTAINER(content_area), description_label);
    gtk_container_add(GTK_CONTAINER(content_area), description_entry);
    gtk_container_add(GTK_CONTAINER(content_area), deadline_label);
    gtk_container_add(GTK_CONTAINER(content_area), deadline_entry);
    gtk_container_add(GTK_CONTAINER(content_area), category_label);
    gtk_container_add(GTK_CONTAINER(content_area), category_entry);
    gtk_container_add(GTK_CONTAINER(content_area), priority_label);
    gtk_container_add(GTK_CONTAINER(content_area), priority_combo);

    // Show all widgets in the dialog
    gtk_widget_show_all(dialog);

    // Run the dialog and wait for a response
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_OK) {
        // User clicked "OK," so retrieve the user input
        const gchar *description = gtk_entry_get_text(GTK_ENTRY(description_entry));
        const gchar *deadline = gtk_entry_get_text(GTK_ENTRY(deadline_entry));
        const gchar *category = gtk_entry_get_text(GTK_ENTRY(category_entry));
        const gchar *priority_text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(priority_combo));
        int priority = 1; // Default to high priority

        if (priority_text != NULL) {
            // Convert the selected priority to an integer
            if (strcmp(priority_text, "Medium") == 0) {
                priority = 2;
            } else if (strcmp(priority_text, "Low") == 0) {
                priority = 3;
            }

            // Create a new task and add it to the list
            todo* add = (todo*)calloc(1, sizeof(todo));
            add->priority = priority;
            strncpy(add->description, description, sizeof(add->description) - 1);
            strncpy(add->deadline, deadline, sizeof(add->deadline) - 1);
            strncpy(add->category, category, sizeof(add->category) - 1);
            add->count = 1;

            if (start == NULL) {
                start = add;
                add->next = NULL;
            } else {
                todo* temp = start;
                while (temp->next != NULL) {
                    temp = temp->next;
                }
                temp->next = add;
                add->next = NULL;
                add->count = temp->count + 1;
            }

            adjustcount();
        }
    }

    // Destroy the dialog
    gtk_widget_destroy(dialog);
}

void deletetodo(GtkMenuItem *menu_item, gpointer user_data) {
    if (start == NULL) {
        // Show a message dialog indicating there are no tasks
        GtkWidget *message_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "No tasks in your TODO list.");
        gtk_window_set_title(GTK_WINDOW(message_dialog), "No Tasks");
        gtk_dialog_run(GTK_DIALOG(message_dialog));
        gtk_widget_destroy(message_dialog);
    } else {
        GtkWidget *dialog;
        GtkWidget *content_area;
        GtkWidget *entry;
        const gchar *input;
        int taskNumber;
        todo *del, *temp;

        // Create a dialog for task number input
        dialog = gtk_dialog_new_with_buttons(
            "Delete a task",
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            "Cancel",
            GTK_RESPONSE_CANCEL,
            "Delete",
            GTK_RESPONSE_OK,
            NULL
        );

        // Create an entry field for the task number
        content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        entry = gtk_entry_new();
        gtk_container_add(GTK_CONTAINER(content_area), entry);

        // Show all widgets in the dialog
        gtk_widget_show_all(dialog);

        // Run the dialog and wait for a response
        gint response = gtk_dialog_run(GTK_DIALOG(dialog));
        if (response == GTK_RESPONSE_OK) {
            // Get the task number from the entry field
            input = gtk_entry_get_text(GTK_ENTRY(entry));
            taskNumber = atoi(input);

            // Remove the task with the specified task number
            if (start == NULL) {
                // List is empty
                g_print("No tasks in your TODO list.\n");
            } else {
                if (start->count == taskNumber) {
                    // Remove the first task
                    del = start;
                    start = start->next;
                    free(del);
                    adjustcount();
                    g_print("Task removed.\n");
                } else {
                    // Find and remove the task
                    del = start;
                    temp = start->next;

                    while (temp != NULL) {
                        if (temp->count == taskNumber) {
                            del->next = temp->next;
                            free(temp);
                            adjustcount();
                            g_print("Task removed.\n");
                            break;
                        } else {
                            del = temp;
                            temp = temp->next;
                        }
                    }

                    if (temp == NULL) {
                        g_print("Task number not found.\n");
                    }
                }
            }
        }

        // Destroy the dialog
        gtk_widget_destroy(dialog);
    }
}


void filterByPriority(GtkMenuItem *menu_item, gpointer user_data) {
	
        if (start == NULL) {
        // Show a message dialog indicating there are no tasks
        GtkWidget *message_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "No tasks in your TODO list.");
        gtk_window_set_title(GTK_WINDOW(message_dialog), "No Tasks");
        gtk_dialog_run(GTK_DIALOG(message_dialog));
        gtk_widget_destroy(message_dialog);
    } else{// Create a new dialog
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Filter tasks by priority",
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        "Cancel",
        GTK_RESPONSE_CANCEL,
        "Filter",
        GTK_RESPONSE_OK,
        NULL
    );

    // Create a content area within the dialog
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Create a label and a combo box for selecting the priority
    GtkWidget *priority_label = gtk_label_new("Select Priority:");
    GtkWidget *priority_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priority_combo), "High");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priority_combo), "Medium");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(priority_combo), "Low");
    gtk_combo_box_set_active(GTK_COMBO_BOX(priority_combo), 0);

    // Pack widgets into the content area
    gtk_container_add(GTK_CONTAINER(content_area), priority_label);
    gtk_container_add(GTK_CONTAINER(content_area), priority_combo);

    // Show all widgets in the dialog
    gtk_widget_show_all(dialog);

    // Run the dialog and wait for a response
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));

    if (response == GTK_RESPONSE_OK) {
        // User clicked "Filter," so retrieve the selected priority
        const gchar *priority_text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(priority_combo));
        int desiredPriority = 1; // Default to high priority

        if (priority_text != NULL) {
            // Convert the selected priority to an integer
            if (strcmp(priority_text, "Medium") == 0) {
                desiredPriority = 2;
            } else if (strcmp(priority_text, "Low") == 0) {
                desiredPriority = 3;
            }

            // Create a new text buffer to display the filtered tasks
            GtkTextBuffer *filtered_buffer = gtk_text_buffer_new(NULL);

            // Iterate through the tasks and add those with the desired priority to the filtered text buffer
            todo* temp = start;
            while (temp != NULL) {
                if (temp->priority == desiredPriority) {
                    char task[256];
                    snprintf(task, sizeof(task), "Task %d (Priority: %d, Deadline: %s, Category: %s): %s\n",
                             temp->count, temp->priority, temp->deadline, temp->category, temp->description);
                    GtkTextIter iter;
                    gtk_text_buffer_get_end_iter(filtered_buffer, &iter);
                    gtk_text_buffer_insert(filtered_buffer, &iter, task, -1);
                }
                temp = temp->next;
            }

            // Create a new window to display the filtered tasks
            GtkWidget *filtered_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
            gtk_window_set_title(GTK_WINDOW(filtered_window), "Filtered Tasks");
            gtk_window_set_default_size(GTK_WINDOW(filtered_window), 400, 300);

            GtkWidget *filtered_text_view = gtk_text_view_new();
            gtk_text_view_set_buffer(GTK_TEXT_VIEW(filtered_text_view), filtered_buffer);

            gtk_container_add(GTK_CONTAINER(filtered_window), filtered_text_view);
            gtk_widget_show_all(filtered_window);
        }
    }

    // Destroy the dialog
    gtk_widget_destroy(dialog);
}
}
void filterByCategory(GtkMenuItem *menu_item, const gchar *category) {
    // Implementation for filtering by category
    GtkTextIter iter;
    // Create a new text buffer to display the filtered tasks
    GtkTextBuffer *filtered_buffer = gtk_text_buffer_new(NULL);

    // Iterate through the tasks and add those with the desired category to the filtered text buffer
    todo* temp = start;
    while (temp != NULL) {
        if (strcmp(temp->category, category) == 0) {
            char task[256];
            snprintf(task, sizeof(task), "Task %d (Priority: %d, Deadline: %s, Category: %s): %s\n",
                     temp->count, temp->priority, temp->deadline, temp->category, temp->description);
            gtk_text_buffer_insert_at_cursor(filtered_buffer, task, -1);
        }
        temp = temp->next;
    }

    // Create a new window to display the filtered tasks
    GtkWidget *filtered_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(filtered_window), "Filtered Tasks");
    gtk_window_set_default_size(GTK_WINDOW(filtered_window), 400, 300);

    GtkWidget *filtered_text_view = gtk_text_view_new();
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(filtered_text_view), filtered_buffer);

    gtk_container_add(GTK_CONTAINER(filtered_window), filtered_text_view);
    gtk_widget_show_all(filtered_window);
}

void filterByDueDate(const char *due_date) {
// Create a new text buffer to display the filtered tasks
    GtkTextBuffer *filtered_buffer = gtk_text_buffer_new(NULL);

    // Iterate through the tasks and add those with the matching due date to the filtered text buffer
    todo *temp = start;
    while (temp != NULL) {
        if (strcmp(temp->deadline, due_date) == 0) {
            char task[256];
            snprintf(task, sizeof(task), "Task %d (Priority: %d, Deadline: %s, Category: %s): %s\n",
                     temp->count, temp->priority, temp->deadline, temp->category, temp->description);
            gtk_text_buffer_insert_at_cursor(filtered_buffer, task, -1);
        }
        temp = temp->next;
    }

    // Create a new window to display the filtered tasks
    GtkWidget *filtered_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(filtered_window), "Filtered Tasks by Due Date");
    gtk_window_set_default_size(GTK_WINDOW(filtered_window), 400, 300);

    GtkWidget *filtered_text_view = gtk_text_view_new();
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(filtered_text_view), filtered_buffer);

    gtk_container_add(GTK_CONTAINER(filtered_window), filtered_text_view);
    gtk_widget_show_all(filtered_window);
}


void showDueDateFilterDialog() {
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *entry;
    const gchar *due_date;
    
        if (start == NULL) {
        // Show a message dialog indicating there are no tasks
        GtkWidget *message_dialog = gtk_message_dialog_new(GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "No tasks in your TODO list.");
        gtk_window_set_title(GTK_WINDOW(message_dialog), "No Tasks");
        gtk_dialog_run(GTK_DIALOG(message_dialog));
        gtk_widget_destroy(message_dialog);
    }else{
    // Create a dialog for due date input
    dialog = gtk_dialog_new_with_buttons(
        "Filter tasks by Due Date",
        GTK_WINDOW(window),
        GTK_DIALOG_MODAL,
        "Cancel",
        GTK_RESPONSE_CANCEL,
        "Filter",
        GTK_RESPONSE_OK,
        NULL
    );

    // Create an entry field for due date
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(content_area), entry);

    // Show all widgets in the dialog
    gtk_widget_show_all(dialog);

    // Run the dialog and wait for a response
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_OK) {
        // Get the due date from the entry field
        due_date = gtk_entry_get_text(GTK_ENTRY(entry));

        // Filter tasks by due date
        filterByDueDate(due_date);
    }

    // Destroy the dialog
    gtk_widget_destroy(dialog);
}
}

void adjustcount() {
    int count = 1;  // Start with task number 1
    todo* temp = start;

    while (temp != NULL) {
        temp->count = count;
        count++;
        temp = temp->next;
    }
}

void interface() {
    gtk_init(NULL, NULL);

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "TODO App");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *menu = gtk_menu_bar_new();
    GtkWidget *menu_item;

    menu_item = gtk_menu_item_new_with_label("View TODO List");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(seetodo), NULL);

    menu_item = gtk_menu_item_new_with_label("Add a new task");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(createtodo), NULL);

    menu_item = gtk_menu_item_new_with_label("Remove a task");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(deletetodo), NULL);

    menu_item = gtk_menu_item_new_with_label("Filter tasks by priority");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(filterByPriority), NULL);

    menu_item = gtk_menu_item_new_with_label("Filter by Due Date");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(showDueDateFilterDialog), NULL);

    // Create a "Categories" menu item with submenu
    GtkWidget *categories_menu_item = gtk_menu_item_new_with_label("Categories");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), categories_menu_item);

    // Create a submenu for the "Categories" menu
    GtkWidget *categories_submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(categories_menu_item), categories_submenu);

    // Create menu items for categories
    menu_item = gtk_menu_item_new_with_label("Filter by WT");
    g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(filterByCategory), "WT");
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_submenu), menu_item);

    menu_item = gtk_menu_item_new_with_label("Filter by SDCA");
    g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(filterByCategory), "SDCA");
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_submenu), menu_item);

    // Add more category items as needed
    menu_item = gtk_menu_item_new_with_label("Filter by DSA");
    g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(filterByCategory), "DSA");
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_submenu), menu_item);

    menu_item = gtk_menu_item_new_with_label("Filter by AFLL");
    g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(filterByCategory), "AFLL");
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_submenu), menu_item);

    menu_item = gtk_menu_item_new_with_label("Filter by SDS");
    g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(filterByCategory), "SDS");
    gtk_menu_shell_append(GTK_MENU_SHELL(categories_submenu), menu_item);

    menu_item = gtk_menu_item_new_with_label("Exit");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), menu_item);
    g_signal_connect(G_OBJECT(menu_item), "activate", G_CALLBACK(gtk_main_quit), NULL);

    gtk_box_pack_start(GTK_BOX(vbox), menu, FALSE, FALSE, 0);

    text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text_view), GTK_WRAP_WORD);
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view));
    gtk_text_buffer_set_text(buffer, "", -1);

    gtk_box_pack_start(GTK_BOX(vbox), text_view, TRUE, TRUE, 0);

    gtk_widget_show_all(window);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    gtk_main();
}


int main(int argc, char *argv[]) {
    interface();
    return 0;
}
