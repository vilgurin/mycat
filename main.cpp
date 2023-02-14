// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include "options_parser.h"
#include <vector>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

enum ERRORS {
    OPEN_ERROR = -1,
    READ_ERROR = -2,
    WRITE_ERROR = -3,
    CLOSE_ERROR = -4
};

int myopen(char *file, std::vector<int> *fds);
int myclose(int fd);
size_t read_buffer(int fd, char *buffer, int size_buf);
size_t write_buffer(int fd, char *buffer, size_t size_buff);
int flag_support(char* buffer, char* buffer2, size_t size_buff);
void close_fds(std::vector<int> fds);

int main(int argc, char *argv[]) {
    if (argc <= 1){
        std::cout << "Please enter correct number of arguments" << std::endl;
        exit(0);
    }
    std::vector<int> fds;
    const constexpr size_t SIZE_BUF = 4096;
    int flag = 0;

    for (int i = 1; i < argc; ++i){
        std::string name = argv[i];
        if (name == "--help" or name == "-h"){
            std::cout << "You need to enter ./mycat [-A] <filename1> <filename2> ... " << std::endl;
            exit(0);
        } else if (name == "-A"){
            flag = 1;
        }
    }

    for (int i = 1; i < argc; ++i) {
        std::string temp;
        temp = argv[i];
        if (temp != "-A" && myopen(argv[i], &fds) == OPEN_ERROR) {
            std::cerr <<"here2\n";
            close_fds(fds);
            return OPEN_ERROR;
        }
    }


    char buffer[SIZE_BUF];
    char buffer2[SIZE_BUF*4];

    for (size_t i = 0; i < fds.size(); ++i){
        while (true) {
            auto read_res = read_buffer(fds[i], buffer, SIZE_BUF);
            if (read_res == READ_ERROR) {
                std::cerr << "here\n";
                close_fds(fds);
                exit(READ_ERROR);
            } else if (read_res == 0) {
                break;
            }
            if (flag == 1) {
                auto read_size_flag = flag_support(buffer, buffer2, read_res);
                if (write_buffer(1, buffer2, read_size_flag) == WRITE_ERROR) {
                    close_fds(fds);
                    return WRITE_ERROR;
                }
                memset(buffer, 0, SIZE_BUF);
                memset(buffer2, 0, read_size_flag);
            } else {
                if (write_buffer(1,buffer, read_res) == WRITE_ERROR) {
                    close_fds(fds);
                    return WRITE_ERROR;
                }
                memset(buffer, 0, SIZE_BUF);
            }
        }
        myclose(fds[i]);
    }

    return 0;
}


int myopen(char *file, std::vector<int> *fds) {
    while (true){
        int fd;
        if ((fd = open(file, O_RDWR)) < 0) {
            if (fd == -1){
                if (errno == EINTR){
                    continue;
                }
                else{
                    return OPEN_ERROR;
                }
            }
        } else {
            fds->push_back(fd);
            break;
        }
    }
    return 0;
}

int myclose(int fd){
    int close_res;
    while (true){
        if ((close_res = (close(fd))) < 0){
            if (close_res == -1){
                if (errno == EINTR){
                    continue;
                } else{
                    return CLOSE_ERROR;
                }
            }

        }else{
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
                std::cerr << "ERROR";
                return WRITE_ERROR;
            }
        } else {
            written_bytes += written_now;
        }
    }
    return 0;
}

int flag_support(char* buffer, char* buffer2, size_t size_buff){
    int i2 = 0;
    for (int i = 0; i < size_buff; ++i) {

        if (isspace(buffer[i]) or isprint(buffer[i])){
            buffer2[i2] = buffer[i];
            ++i2;
        } else {
            std::ostringstream oss;
            oss << "\\x" << std::hex << static_cast<int>(buffer[i]);
            std::string temp = oss.str();
            temp = temp.erase(2,6);
            temp[2] = toupper(temp[2]);
            temp[3] = toupper(temp[3]);

            for (auto s: temp){
                buffer2[i2] = s;
                ++i2;
            }
        }
    }
    return i2;
}
void close_fds(std::vector<int> fds){
    for (size_t i = 0; i < fds.size(); ++i){
        myclose(fds[i]);
    }
}