#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstring>
#include <csignal>
#include <atomic>

#include <netinet/in.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "../common.h"

using namespace std;

#define MAX_LENGTH 64

// 1234567890123456789012345678901234567890123456789012345678901234

atomic<bool> is_connected = false;

class SyncBuffer {
private:
    string buffer;
    mutex mtx;
    condition_variable cv;

public:
    void set(string str);
    string get();
    void clean();

};

void SyncBuffer::set(string str) {
    buffer = str;
    cv.notify_one();
}

string SyncBuffer::get() {
    unique_lock<mutex> ulm(mtx);
    cv.wait(ulm, [this] { return !buffer.empty(); });
    return buffer;
}

void SyncBuffer::clean() {
    buffer.clear();
}

bool is_digits(const string &str) {
    return str.find_first_not_of("0123456789") == string::npos;
}

void couting_sort(vector<int> &vec) {
    auto pair = minmax_element(vec.begin(), vec.end());
    int min = *pair.first;
    int dist = *pair.second - min + 1;

    vector<int> t(dist, 0);

    for (auto i: vec) t[i - min]++;

    for (int i = t.size() - 1, idx = 0; i >= 0; --i) {
        for (int j = 0; j < t[i]; ++j) {
            vec[idx++] = i + min;
        }
    }
}

string final_string_to_buffer(const vector<int> &vec) {
    string output;
    for (int i: vec) {
        if (i % 2 == 0) {
            output += "KB";
        } else {
            output += to_string(i);
        }
    }
    return output;
}

[[noreturn]] void thread_1(SyncBuffer &s) {
    while (true) {
        string input;

        cout << "Please input a string of numbers, no longer than 64:" << endl;

        while (true) {
            cin >> input;
            if (input.length() > MAX_LENGTH || !is_digits(input)) {
                cout << "Try again!" << endl;
            } else {
                break;
            }
        }

        vector<int> vec;
        for (char i : input) {
            vec.push_back(i - '0');
        }

        couting_sort(vec);
        string result = final_string_to_buffer(vec);
        s.set(result); // записали в буффер
    }
}

[[noreturn]] void thread_2(SyncBuffer &s) {

    int sockfd;
    struct sockaddr_in servaddr{};
    // создание сокета

    bzero(&servaddr, sizeof(servaddr));
    // назначаем ip-адрес и порт
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    while (true) {
        string result = s.get();
        s.clean(); // очищаем буфер

        cout << "String from a buffer: " << result << endl; // вывод содержимого буфера
        int sum = 0;
        for (char i : result) {
            if (!(i == 'K' || i == 'B')) {
                sum += i - '0';
            }
        }
        cout << "Sum: " << sum << endl;

        // проверяем подключение
        if (!is_connected.load(memory_order_seq_cst)) {
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                cout << "\033[1;31mSocket creation failed: " << strerror(errno) << "\033[0m" << endl;
                continue;
            }
            if (connect(sockfd, (sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
                cout << "\033[1;31mUnable to connect to the server: " << strerror(errno) << endl;
                cout << "You cannot use Program #2!\033[0m" << endl;
                is_connected.store(false, memory_order_seq_cst);
                close(sockfd);
                continue;
            } else {
                is_connected.store(true, memory_order_seq_cst);
            }
        }

        // отправляем данные программе 2
        char sum_to_send[MAX];
        bzero(sum_to_send, sizeof(sum_to_send));
        sprintf(sum_to_send, "%d", sum);

        if (send(sockfd, sum_to_send, sizeof(sum_to_send), 0) == -1) {
            cout << "\033[1;31mSEND ERROR: " << strerror(errno) << "\033[0m" << endl;
            is_connected.store(false, memory_order_seq_cst);
            close(sockfd);
        }
    }
}

void signalHandler(int signum) {
    cout << "\033[1;31mInterrupt signal SIGPIPE received.\033[0m" << endl;
    is_connected.store(false, memory_order_seq_cst);
}

int main() {
    cout << "\033[1;32mWelcome to Program #1!\033[0m" << endl;
    // ловим сигнал SIGPIPE
    signal(SIGPIPE, signalHandler);

    auto info = SyncBuffer();

    thread t1(thread_1, ref(info));
    thread t2(thread_2, ref(info));

    t1.join();
    t2.join();

    return 0;
}
