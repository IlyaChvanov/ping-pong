#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sys/stat.h> // Для mkfifo
#include <filesystem>
#include <string>
#include <thread>

#define FIFO_PATH "/tmp/pingpong_fifo"
#define BUFFER_SIZE 5

int main() {
    char buffer[BUFFER_SIZE];
    if (!std::filesystem::exists(FIFO_PATH))
    {
        if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST)
        {
            std::cerr << "Could not create FIFO" << std::endl;
            return -1;
        }
    }

    while (true) {
        int fifo_fd = open(FIFO_PATH, O_WRONLY);
        if (fifo_fd == -1)
        {
            std::cerr << "Error while opening FIFO for writing" << std::endl;
            return -1;
        }

        const char *message = "ping";
        ssize_t bytes_written = write(fifo_fd, message, strlen(message));
        close(fifo_fd);
        if (bytes_written == -1)
        {
            std::cerr << "Error while writing to FIFO" << std::endl;
            return -1;
        }
        std::cout << "Client Sent: \"" << message << "\"" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        int fifo_fd_read = open(FIFO_PATH, O_RDONLY);
        if (fifo_fd_read == -1)
        {
            std::cerr << "Error while opening FIFO for reading" << std::endl;
            return -1;
        }

        ssize_t bytes_read = read(fifo_fd_read, buffer, BUFFER_SIZE - 1);
        close(fifo_fd_read);
        if (bytes_read == -1)
        {
            std::cerr << "Error while reading from FIFO" << std::endl;
            return -1;
        }
        if (bytes_read == 0)
        {
            std::cerr << "FIFO is empty, no response received." << std::endl;
            return -1;
        }
        if (buffer != std::string("pong")) {
            std::cerr << "Error: the message is not 'pong'" << std::endl;
            continue;
        }

        buffer[bytes_read] = '\0';
        std::cout << "client Received: \"" << buffer << "\"" << std::endl;
    }
    return 0;
}
