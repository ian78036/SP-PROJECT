#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>

// Function declarations
void menu_system();
void file_operations(int mode, char *arg1, char *arg2);
void directory_operations(int mode, char *arg1);
void keylogger_operations(char *logfile);
void print_menu();

int main(int argc, char *argv[]) {
    if (argc == 1) {
        // Menu-based operation
        menu_system();
    } else {
        // Command-line arguments
        if (strcmp(argv[1], "-m") == 0) {
            int mode = atoi(argv[2]); // Mode (1: File, 2: Directory, 3: Keylogger)
            switch (mode) {
                case 1:
                    file_operations(atoi(argv[3]), argv[4], argv[5]);
                    break;
                case 2:
                    directory_operations(atoi(argv[3]), argv[4]);
                    break;
                case 3:
                    keylogger_operations(argv[3]);
                    break;
                default:
                    printf("Invalid mode selected!\n");
            }
        } else {
            printf("Invalid arguments. Use the following format:\n");
            printf("./supercommand -m <mode> <operation> <args>\n");
        }
    }
    return 0;
}

// Menu-based operations
void menu_system() {
    int choice;
    while (1) {
        print_menu();
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: {
                int sub_choice;
                char arg1[100], arg2[100];
                printf("File Operations:\n1. Create File\n2. Change Permission\n3. Read File\n4. Write File\n5. Delete File\n");
                printf("Enter your choice: ");
                scanf("%d", &sub_choice);
                printf("Enter file name (and path if required): ");
                scanf("%s", arg1);
                if (sub_choice == 2) {
                    printf("Enter permission (e.g., 0644): ");
                    scanf("%s", arg2);
                }
                file_operations(sub_choice, arg1, sub_choice == 2 ? arg2 : NULL);
                break;
            }
            case 2: {
                int sub_choice;
                char arg1[100];
                printf("Directory Operations:\n1. Create Directory\n2. Remove Directory\n3. Print Current Directory\n4. List Directory\n");
                printf("Enter your choice: ");
                scanf("%d", &sub_choice);
                if (sub_choice <= 2) {
                    printf("Enter directory name (and path if required): ");
                    scanf("%s", arg1);
                }
                directory_operations(sub_choice, sub_choice <= 2 ? arg1 : NULL);
                break;
            }
            case 3: {
                char logfile[100];
                printf("Keylogger Operations:\nEnter log file name (e.g., keylog.txt): ");
                scanf("%s", logfile);
                keylogger_operations(logfile);
                break;
            }
            case 0:
                printf("Exiting...\n");
                return;
            default:
                printf("Invalid choice!\n");
        }
    }
}

// File operations
void file_operations(int mode, char *arg1, char *arg2) {
    int fd;
    switch (mode) {
        case 1: // Create/Open file
            fd = open(arg1, O_CREAT | O_WRONLY, 0644);
            if (fd == -1) perror("Error creating/opening file");
            else printf("File %s created/opened successfully.\n", arg1);
            close(fd);
            break;
        case 2: // Change file permissions
            if (chmod(arg1, strtol(arg2, NULL, 8)) == -1)
                perror("Error changing file permissions");
            else
                printf("Permissions of file %s changed to %s.\n", arg1, arg2);
            break;
        case 3: // Read file
            fd = open(arg1, O_RDONLY);
            if (fd == -1) perror("Error opening file for reading");
            else {
                char buffer[1024];
                ssize_t bytes;
                while ((bytes = read(fd, buffer, sizeof(buffer))) > 0) {
                    write(STDOUT_FILENO, buffer, bytes);
                }
                printf("\n");
            }
            close(fd);
            break;
        case 4: // Write to file
            fd = open(arg1, O_WRONLY | O_APPEND);
            if (fd == -1) perror("Error opening file for writing");
            else {
                printf("Enter text to write to the file: ");
                char buffer[1024];
                scanf(" %[^\n]", buffer);
                write(fd, buffer, strlen(buffer));
                printf("Text written to file %s.\n", arg1);
            }
            close(fd);
            break;
        case 5: // Delete file
            if (unlink(arg1) == -1)
                perror("Error deleting file");
            else
                printf("File %s deleted successfully.\n", arg1);
            break;
        default:
            printf("Invalid file operation!\n");
    }
}

// Directory operations
void directory_operations(int mode, char *arg1) {
    switch (mode) {
        case 1: // Create directory
            if (mkdir(arg1, 0755) == -1)
                perror("Error creating directory");
            else
                printf("Directory %s created successfully.\n", arg1);
            break;
        case 2: // Remove directory
            if (rmdir(arg1) == -1)
                perror("Error removing directory");
            else
                printf("Directory %s removed successfully.\n", arg1);
            break;
        case 3: { // Print current directory
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
                printf("Current Directory: %s\n", cwd);
            else
                perror("Error getting current directory");
            break;
        }
        case 4: { // List directory
            DIR *dir;
            struct dirent *entry;
            if ((dir = opendir(arg1 ? arg1 : ".")) == NULL)
                perror("Error opening directory");
            else {
                printf("Directory contents:\n");
                while ((entry = readdir(dir)) != NULL) {
                    printf("%s\n", entry->d_name);
                }
                closedir(dir);
            }
            break;
        }
        default:
            printf("Invalid directory operation!\n");
    }
}

// Keylogger operations
void keylogger_operations(char *logfile) {
    printf("Keylogger started (mock implementation).\n");
    // Implementation for logging keystrokes to logfile will go here
    // This requires more advanced handling and permissions
}

// Print the menu
void print_menu() {
    printf("\nMain Menu:\n");
    printf("1. File Operations\n");
    printf("2. Directory Operations\n");
    printf("3. Keylogger\n");
    printf("0. Exit\n");
}
