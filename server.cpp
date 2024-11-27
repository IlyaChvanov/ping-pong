#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <string.h>
#include <thread>
#include <sys/stat.h>

#define FIFO_PATH "/tmp/pingpong_fifo"
#define BUFFER_SIZE 1024

int main() {
    char buffer[BUFFER_SIZE];

    if (!std::filesystem::exists(FIFO_PATH)) {
        mkfifo(FIFO_PATH, 0666);
    }

    while (true) {
        // Ожидание сообщений от клиента
        int fifo_fd = open(FIFO_PATH, O_RDONLY);
        if (fifo_fd == -1) {
            perror("Error while opening FIFO for reading");
            exit(1);
        }

        ssize_t bytes_read = read(fifo_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read == -1) {
            perror("Error while reading from FIFO");
            close(fifo_fd);
            exit(1);
        }

        if (bytes_read == 0) {
            printf("FIFO is empty, waiting for data...\n");
            close(fifo_fd);
            continue;
        }

        buffer[bytes_read] = '\0';
        std::cout << "Received request: " << buffer;

        try {
            if (buffer == std::string("ping")) {
                int fifo_fd_write = open(FIFO_PATH, O_WRONLY);
                if (fifo_fd_write == -1) {
                    perror("Error while opening FIFO for writing");
                    close(fifo_fd);
                    exit(1);
                }

                const char *response = "pong";
                ssize_t bytes_written = write(fifo_fd_write, response, strlen(response));
                if (bytes_written == -1) {
                    perror("Error while writing to FIFO");
                    close(fifo_fd);
                    close(fifo_fd_write);
                    exit(1);
                }

                printf("Sent: \"%s\"\n", response);
                close(fifo_fd_write);
            } else {
                throw std::runtime_error("message is not ping");
            }
        } catch (const std::exception &e) {
            std::cout << e.what() << std::endl;
            continue;
        }
        close(fifo_fd);
    }

    return 0;
}
