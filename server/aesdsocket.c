#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>

#define PORT 9000
#define DATA_BUFFER_SIZE 1024
#define DATA_FILE "/var/tmp/aesdsocketdata"

volatile sig_atomic_t stop_server = 0;

void signal_handler(int sig) {
    stop_server = 1;
}

void clean_up(int sockfd, FILE *fp) {
    if (sockfd >= 0) {
        close(sockfd);
    }
    if (fp) {
        fclose(fp);
        remove(DATA_FILE);
    }
    closelog();
}

// This function will read the entire file and send its contents to the client
void send_file_contents(int sockfd, FILE *fp) {
    char buffer[DATA_BUFFER_SIZE];
    size_t bytes_read;

    rewind(fp); // Go to the start of the file
    while ((bytes_read = fread(buffer, 1, DATA_BUFFER_SIZE, fp)) > 0) {
        if (write(sockfd, buffer, bytes_read) < 0) {
            syslog(LOG_ERR, "Error sending file contents to socket: %s", strerror(errno));
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;
    char buffer[DATA_BUFFER_SIZE];
    ssize_t read_size;
    FILE *fp = NULL;
    int ret;
    int daemon_mode = 0;

    // Argument parsing to check for daemon mode
    int opt;
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                daemon_mode = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-d]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Set up logging
    openlog("aesdsocket", LOG_PID, LOG_USER);

    // Register signal_handler for SIGINT and SIGTERM
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        syslog(LOG_ERR, "Error opening socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Initialize socket structure
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // Bind the host address
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        syslog(LOG_ERR, "Error on binding: %s", strerror(errno));
        clean_up(sockfd, fp);
        exit(EXIT_FAILURE);
    }

    // Daemonize if requested
    if (daemon_mode) {
        pid_t pid = fork();
        if (pid > 0) {
            // Parent process, should exit
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            // Fork failed
            syslog(LOG_ERR, "Fork failed: %s", strerror(errno));
            exit(EXIT_FAILURE);
        }

        // Child process continues, this is now the daemon
        setsid(); // Start a new session

        // Close standard file descriptors
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }

    // Start listening for the clients
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    // Loop to accept incoming connections
    while (!stop_server) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            if (errno == EINTR) continue;
            syslog(LOG_ERR, "Error on accept: %s", strerror(errno));
            break;
        }

        syslog(LOG_INFO, "Accepted connection from %s", inet_ntoa(cli_addr.sin_addr));

        // Open or create the file for appending
        fp = fopen(DATA_FILE, "a+");
        if (fp == NULL) {
            syslog(LOG_ERR, "Error opening file: %s", strerror(errno));
            close(newsockfd);
            continue;
        }

        // Receive data in a loop until newline or end of stream
        do {
            memset(buffer, 0, DATA_BUFFER_SIZE);
            read_size = read(newsockfd, buffer, DATA_BUFFER_SIZE - 1);
            if (read_size <= 0) {
                syslog(LOG_ERR, "Error reading from socket: %s", strerror(errno));
                break;
            }

            // Write to file
            ret = fwrite(buffer, sizeof(char), read_size, fp);
            if (ret < read_size) {
                syslog(LOG_ERR, "Error writing to file: %s", strerror(errno));
                break;
            }
            fflush(fp);
        } while (buffer[read_size - 1] != '\n');

        // Send the file contents back to the client
        send_file_contents(newsockfd, fp);

        // Close the file and the socket for this connection
        fclose(fp);
        fp = NULL;
        close(newsockfd);
        syslog(LOG_INFO, "Closed connection from %s", inet_ntoa(cli_addr.sin_addr));
    }

    // Clean up before exiting
    clean_up(sockfd, fp);

    if (stop_server) {
        syslog(LOG_INFO, "Daemon exiting, received signal");
    }

    return 0;
}