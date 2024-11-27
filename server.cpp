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

// !!ДЛЯ ДИМЫ!!
// Файл автоматически очищается при чтении данных, так что, когда будешь делать клиента, сразу считывай
// и отправляй ping и ничего не очищай. Также все файлы открыты
// !!ДЛЯ ДИМЫ!!
int main() {
    char buffer[BUFFER_SIZE];
    if (!std::filesystem::exists(FIFO_PATH)) {
        if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST) {
            std::cerr << "Could not create FIFO" << std::endl;
            return -1;
        }
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        int fifo_fd = open(FIFO_PATH, O_RDONLY);
        if (fifo_fd == -1) {
            std::cerr << "Error while opening FIFO for reading" << std::endl;
            exit(1);
        }
        ssize_t bytes_read = read(fifo_fd, buffer, BUFFER_SIZE - 1);
        if (bytes_read == -1) {
            std::cerr << "Error while reading from FIFO" << std::endl;
            close(fifo_fd);
            exit(1);
        }
        if (bytes_read == 0) {
            std::cout << "FIFO is empty, waiting for data..." << std::endl;
            close(fifo_fd);
            continue;
        }

        buffer[bytes_read] = '\0';
        std::cout << "Received request: " << buffer << std::endl;
        if (buffer == std::string("ping")) {
            int fifo_fd_write = open(FIFO_PATH, O_WRONLY);
            if (fifo_fd_write == -1) {
                std::cerr << "Error while opening FIFO for writing" << std::endl;
                close(fifo_fd);
                exit(1);
            }
            const char *response = "pong";
            ssize_t bytes_written = write(fifo_fd_write, response, strlen(response));
            if (bytes_written == -1) {
                std::cerr << "Error while writing to FIFO" << std::endl;
                close(fifo_fd);
                close(fifo_fd_write);
                exit(1);
            }
            std::cout << "Sent: \"" << response << "\"" << std::endl;
            close(fifo_fd_write);
        } else {
            std::cerr << "Error: the message is not 'ping'" << std::endl;
            close(fifo_fd);
            continue;
        }

        close(fifo_fd);
    }
    return 0;
}
