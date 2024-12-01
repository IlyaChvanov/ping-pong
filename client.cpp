#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <sys/stat.h> // Для mkfifo
#include <filesystem>
#include <string>

#define FIFO_PATH "/tmp/pingpong_fifo"
#define BUFFER_SIZE 1024

int main()
{
    char buffer[BUFFER_SIZE];

    // Проверяем существование FIFO и создаем его, если не существует
    if (!std::filesystem::exists(FIFO_PATH))
    {
        if (mkfifo(FIFO_PATH, 0666) == -1 && errno != EEXIST)
        {
            std::cerr << "Could not create FIFO" << std::endl;
            return -1;
        }
    }

    // Открываем FIFO на запись
    int fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd == -1)
    {
        std::cerr << "Error while opening FIFO for writing" << std::endl;
        return -1;
    }

    // Отправляем сообщение "ping"
    const char *message = "ping";
    ssize_t bytes_written = write(fifo_fd, message, strlen(message));
    if (bytes_written == -1)
    {
        std::cerr << "Error while writing to FIFO" << std::endl;
        close(fifo_fd);
        return -1;
    }
    std::cout << "Sent: \"" << message << "\"" << std::endl;

    // Закрываем FIFO после отправки
    close(fifo_fd);

    // Открываем FIFO на чтение
    int fifo_fd_read = open(FIFO_PATH, O_RDONLY);
    if (fifo_fd_read == -1)
    {
        std::cerr << "Error while opening FIFO for reading" << std::endl;
        return -1;
    }

    // Читаем ответ
    ssize_t bytes_read = read(fifo_fd_read, buffer, BUFFER_SIZE - 1);
    if (bytes_read == -1)
    {
        std::cerr << "Error while reading from FIFO" << std::endl;
        close(fifo_fd_read);
        return -1;
    }
    if (bytes_read == 0)
    {
        std::cerr << "FIFO is empty, no response received." << std::endl;
        close(fifo_fd_read);
        return -1;
    }

    buffer[bytes_read] = '\0'; // Null-terminate the received string
    std::cout << "Received: \"" << buffer << "\"" << std::endl;

    // Закрываем FIFO после чтения
    close(fifo_fd_read);

    return 0;
}
