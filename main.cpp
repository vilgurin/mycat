// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include "options_parser.h"
#include <vector>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <cstring>
enum ERRORS {
    OPEN_ERROR = -1,
    READ_ERROR = -2,
    WRITE_ERROR = -3
};

int myopen(char *file, std::vector<int> *fds) {
    while (true){
        int fd;
        if ((fd = open(file, 0)) < 0) {
            if (fd == -1){
                if (errno == EINTR){
                    continue;
                }
                else{
                    OPEN_ERROR;
                }
            }
        } else {
            fds->push_back(fd);
            break;
        }
    }
    return 0;
}

size_t read_buffer(int fd, char *buffer, int size_buf) {
    ssize_t read_bytes = 0;
    while (read_bytes < size_buf) {
        ssize_t read_now = read(fd, buffer + read_bytes, size_buf - read_bytes);
        if (read_now == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                return READ_ERROR;
            }
        } else if (read_now == 0) {
            return read_bytes;
        } else {
            read_bytes += read_now;
        }
    }
    return read_bytes;
}


size_t write_buffer(int fd, char *buffer, size_t size_buff) {
    if (size_buff == 0){
        return 0;
    }
    size_t written_bytes = 0;
    while (written_bytes < size_buff) {
        ssize_t written_now = write(fd, buffer + written_bytes, size_buff - written_bytes);
        if (written_now == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                return WRITE_ERROR;
            }
        } else {
            written_bytes += written_now;
        }
    }
    return 0;
}



int main(int argc, char *argv[]) {
    std::vector<int> fds;
    std::vector<std::string> arguments;

    const constexpr size_t SIZE_BUF = 4096;
    char buffer[SIZE_BUF];
    int flag = 0;


    for (int i = 0; i < argc; ++i){
        std::string name = argv[i];
        if (argc <= 1){
            std::cout << "Please enter correct number of arguments" << std::endl;
            exit(0);
        }
        if (name == "--help" or name == "-h"){
            std::cout << "You need to enter ./mycat [-A] <filename1> <filename2> ... " << std::endl;
            exit(0);
        }
        if (name == "-A"){
            flag = 1;
        }
    }


    for (int i = 1; i < argc; ++i) {
        if (myopen(argv[i], &fds) == OPEN_ERROR) {
            return READ_ERROR;
        }
    }

    std::cout << fds.size() << "\n" << std::endl;
    int read_res = 1;
    for (size_t i = 0; i < fds.size(); ++i){
        while (read_res != 0) {
            read_res = read_buffer(fds[i], buffer, SIZE_BUF);
            write_buffer(1, buffer, read_res);
            memset(buffer, 0, SIZE_BUF);
        }
        read_res = 1;
        close(fds[i]);
    }

    return 0;
}