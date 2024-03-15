#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Check for the correct number of arguments
    if (argc != 3) {
        printf("Usage: %s <full_path_to_file> <text_string>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Open syslog with LOG_USER facility
    openlog("writer", LOG_PID, LOG_USER);

    // Extract arguments
    char *filepath = argv[1];
    char *text = argv[2];

    // Attempt to open the file for writing
    FILE *file = fopen(filepath, "w");
    if (!file) {
        syslog(LOG_ERR, "Failed to open file: %s", filepath);
        printf("Error: Failed to open file.\n");
        closelog();
        exit(EXIT_FAILURE);
    }

    // Write the text string to the file
    if (fprintf(file, "%s", text) < 0) {
        syslog(LOG_ERR, "Failed to write to file: %s", filepath);
        printf("Error: Failed to write to file.\n");
        fclose(file);
        closelog();
        exit(EXIT_FAILURE);
    }

    // Log the write operation
    syslog(LOG_DEBUG, "Writing '%s' to %s", text, filepath);
    printf("Successfully wrote to the file: %s\n", filepath);

    // Clean up
    fclose(file);
    closelog();

    return EXIT_SUCCESS;
}
