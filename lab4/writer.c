#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define DEVICE "/dev/my_dev"  // Driver device file path

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <student Name>\n", argv[0]);
        return 1;
    }

    int fd;
    char *studentName = argv[1];
    // printf("%s\n",studentName);

    fd = open(DEVICE, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }
 
    // Process each char of the student name one by one
    for (int i = 0; i < strlen(studentName); i++) {
        char letter = studentName[i]; 
        // printf("%c",letter);
 
        if (write(fd, &letter, 1) < 0) {
            perror("Failed to write to the device");
            close(fd);
            return errno;
        }
        fflush(stdout);
        sleep(1);
    }

    // Close device file
    close(fd);
    return 0;
}
