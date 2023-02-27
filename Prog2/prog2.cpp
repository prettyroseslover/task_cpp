//
// Created by prettyroseslover on 25/01/23.
//
#include <iostream>
#include <thread>
#include <cstring>
#include <cstdlib>

#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "../common.h"

using namespace std;

void get_info(int sockfd) {
    while (true) {
        char str_recieved[MAX];
        bzero(str_recieved, MAX);

        size_t recv_len = recv(sockfd, str_recieved, sizeof(str_recieved), 0);
        if (recv_len == -1) {
            cout << "\033[1;31mERROR recv: " << strerror(errno) << "\033[0m" << endl;
            return;
        }
        if (recv_len == 0) {
            return;
        }
        // cout << "Received " << recv_len << " bytes: " << str_recieved << endl;
        int num_recieved = atoi(str_recieved);

        if (num_recieved > 99 && num_recieved % 32 == 0) {
            // 55555555555555555555555553 - строка, удовлетворяющая условиям
            cout << "\033[1;32mData is recieved successfully!\033[0m" << endl;
            cout << num_recieved << endl;
        } else if (num_recieved < 99 || num_recieved % 32 != 0) {
            cout << "\033[1;33mData is recieved, but doesn't meet the criteria\033[0m" << endl;
        }
    }
}

int main() {
    cout << "\033[1;32mWelcome to Program #2!\033[0m" << endl;

    int sockfd, connfd, len;
    struct sockaddr_in servaddr{}, cli{};

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        cout << "\033[1;31mSocket creation failed: " << strerror(errno) << "\033[0m" << endl;
    }

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);


    if ((bind(sockfd, (sockaddr *) &servaddr, sizeof(servaddr))) != 0) {
        cout << "\033[1;31mSocket binding failed: " << strerror(errno) << "\033[0m" << endl;
    }

    if ((listen(sockfd, 5)) != 0) {
        cout << "\033[1;31mListen() failed: " << strerror(errno) << "\033[0m" << endl;
    } else {
        cout << "\033[1;32mServer's listening\033[0m" << endl;
    }

    len = sizeof(cli);

    while (true) {
        connfd = accept(sockfd, (sockaddr *) &cli, (socklen_t *) &len);
        if (connfd < 0) {
            cout << "\033[1;31mAccept() failed: " << strerror(errno) << "\033[0m" << endl;
        } else {
            cout << "\033[1;32mClient accepted\033[0m" << endl;

        }

        get_info(connfd);
        close(connfd);
    }

    close(sockfd);
    return 0;
}