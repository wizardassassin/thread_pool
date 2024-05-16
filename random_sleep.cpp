#include <chrono>
#include <iostream>
#include <random>
#include <thread>

#include "thread_pool.hpp"

int main(int argc, char const *argv[]) {
    (void)argc;
    (void)argv;

    ThreadPool::ThreadPool pool;

    for (size_t i = 0; i < 100; i++) {
        pool.addJob([](int64_t threadId) {
            (void)threadId;
            std::random_device rd;
            std::mt19937 mt(rd());
            std::uniform_int_distribution<int> dist(0, 9);
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 * dist(mt)));
        });
    }

    pool.start();
    pool.wait(std::cout);
    pool.stop();

    return 0;
}
