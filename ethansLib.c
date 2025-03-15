//Author: Ethan Roppel

#include "ethansLib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global variables
FileEntry file_table[MAX_FILES];  // File table to track files
int file_count = 0;               // Number of files in the system
int open_file_index = -1;         // Track the currently open file (only one at a time)

// Create a new file (Dynamic Entry Handling)
int fileCreate(const char *filename) {
    // Check if the file is already in file_table
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_table[i].filename, filename) == 0) {
            printf("Error: File '%s' already exists.\n", filename);
            return -1;
        }
    }

    // Create the file on disk
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error: Unable to create file '%s'.\n", filename);
        return -1;
    }
    fclose(file);

    // Add the file to the table dynamically
    if (file_count < MAX_FILES) {
        strcpy(file_table[file_count].filename, filename);
        file_table[file_count].size = 0;
        file_table[file_count].is_open = 0;
        file_count++;
        printf("File '%s' created successfully.\n", filename);
        return 0;
    }

    printf("Error: File table is full, cannot track new file.\n");
    return -1;
}

// Open a file (Handles external files dynamically)
// The two changes I made to this were allowing external files and updating index accordingly
int fileOpen(const char *filename) {

    // Check for errors
    if (open_file_index != -1) {
        printf("Error: Another file is already open. Please close it first.\n");
        return -1;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: File '%s' does not exist on disk.\n", filename);
        return -1;
    }
    fclose(file);

    // Check if file is already tracked
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_table[i].filename, filename) == 0) {
            if (file_table[i].is_open) {
                printf("Error: File '%s' is already open.\n", filename);
                return -1;
            }
            file_table[i].is_open = 1;
            open_file_index = i;
            printf("File '%s' opened successfully.\n", filename);
            return i;
        }
    }

    // Add file dynamically, explicitly ensuring proper initialization
    if (file_count < MAX_FILES) {
        strcpy(file_table[file_count].filename, filename);
        file_table[file_count].size = 0;
        file_table[file_count].is_open = 1;
        
        open_file_index = file_count;  // Update the open file index immediately
        file_count++;  // Increment after successful initialization
        
        printf("File '%s' was found on disk and added to the file table.\n", filename);
        return open_file_index;
    }

    printf("Error: File table is full, cannot open '%s'.\n", filename);
    return -1;
}

// Write data to a file
int fileWrite(int file_index, const char *data) {

    // Check for errors
    if (file_index != open_file_index || file_index < 0 || file_index >= file_count) {
        printf("Error: No file is open or wrong file reference.\n");
        return -1;
    }

    FILE *file = fopen(file_table[file_index].filename, "w");
    if (!file) {
        printf("Error: Unable to open file '%s' for writing.\n", file_table[file_index].filename);
        return -1;
    }
    
    int data_size = strlen(data);
    fwrite(data, 1, data_size, file);
    fclose(file);
    file_table[file_index].size = data_size;
    
    printf("Data written to file '%s' successfully.\n", file_table[file_index].filename);
    return 0;
}

// Read data from a file
int fileRead(int file_index, char *buffer, int buffer_size) {

    // Check for errors
    if (file_index != open_file_index || file_index < 0 || file_index >= file_count) {
        printf("Error: No file is open or wrong file reference.\n");
        return -1;
    }

    // Try to open file from table aray
    FILE *file = fopen(file_table[file_index].filename, "r");
    if (!file) {
        printf("Error: Unable to open file '%s' for reading.\n", file_table[file_index].filename);
        return -1;
    }

    //
    int read_size = fread(buffer, 1, buffer_size - 1, file);
    buffer[read_size] = '\0';
    fclose(file);
    
    printf("Data read from file '%s' successfully.\n", file_table[file_index].filename);
    return read_size;
}

// Close a file
int fileClose(int file_index) {
    
    // Check for errors
    if (file_index != open_file_index || file_index < 0 || file_index >= file_count) {
        printf("Error: No file is open or wrong file reference.\n");
        return -1;
    }

    //Set is_open to 0 and reset index
    file_table[file_index].is_open = 0;
    open_file_index = -1;
    
    printf("File '%s' closed successfully.\n", file_table[file_index].filename);
    return 0;
}

// Delete a file (Handles external files dynamically)
int fileDelete(const char *filename) {
    
    int found = -1;

    // Search for file in file_table
    for (int i = 0; i < file_count; i++) {
        if (strcmp(file_table[i].filename, filename) == 0) {
            found = i;
            break;
        }
    }

    // Prevent deleting an open file and catch error
    if (found != -1 && file_table[found].is_open) {
        printf("Error: Cannot delete an open file. Close it first.\n");
        return -1;
    }

    // Check if the file exists on disk
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: File '%s' does not exist on disk.\n", filename);
        return -1;
    }
    fclose(file);

    // Try to delete the file from disk
    if (remove(filename) != 0) {
        printf("Error: Unable to delete file '%s'.\n", filename);
        return -1;
    }

    // Remove from file_table if successful
    // and update the table with new state
    if (found != -1) {
        for (int j = found; j < file_count - 1; j++) {
            file_table[j] = file_table[j + 1];
        }
        file_count--;
    }

    printf("File '%s' deleted successfully.\n", filename);
    return 0;
}

// Cleanup before exiting the program
void fileExit() {

    //Close items before exiting
    if (open_file_index != -1) {
        file_table[open_file_index].is_open = 0;
        printf("File '%s' was open. It has been closed before exiting.\n", file_table[open_file_index].filename);
    }
    printf("Exiting program safely.\n");
    exit(0);
}