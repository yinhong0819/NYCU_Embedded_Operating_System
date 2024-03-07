#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define LED_DEVICE "/dev/etx_device"  // Driver device file path

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <student ID>\n", argv[0]);
        return 1;
    }

    int fd;
    char *studentNumber = argv[1];
    char binaryDigit;  // Character used to store each binary digit
    char str[4];

    fd = open(LED_DEVICE, O_WRONLY);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }

    // Process each digit of the student number one by one
    for (int i = 0; i < strlen(studentNumber); i++) {
        int digit = studentNumber[i] - '0'; // Convert characters to integers

        // Convert and output the binary of each number
        for (int j = 0; j < 4; j++) {
            int bit = (digit >> (3 - j)) & 1;
            char binaryDigit = bit + '0';  // Convert to character '0' or '1'
            str[j] = binaryDigit;
        }
        if (write(fd, &str, 4) < 0) {
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
