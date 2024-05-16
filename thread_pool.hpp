// Custom thread pool from reading
// https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
#pragma once

#include <chrono>
#include <cmath>
#include <condition_variable>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <stdfloat>
#include <thread>
#include <vector>

namespace Timer {
void printTime(std::ostream& stream, const std::chrono::steady_clock::time_point& startTime,
               const std::chrono::steady_clock::time_point& stopTime) {
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime);
    int64_t time = duration.count();
    if (time < 1000) {
        stream << time / 1000.0 << " ms";
        return;
    }
    if (time < 10 * 1000) {
        stream << time / 10 / 100.0 << " ms";
        return;
    }
    if (time < 100 * 1000) {
        stream << time / 100 / 10.0 << " ms";
        return;
    }
    time /= 1000;
    int64_t sec = 1000;
    int64_t min = 60 * sec;
    int64_t hour = 60 * min;
    int64_t day = 24 * hour;
    if (time < sec) {
        stream << time << " ms";
        return;
    }
    if (time < 10 * sec) {
        stream << time / 10 / 100.0 << " sec";
        return;
    }
    if (time < min) {
        stream << time / 100 / 10.0 << " sec";
        return;
    }
    if (time < 10 * min) {
        stream << time / 600 / 100.0 << " min";
        return;
    }
    if (time < hour) {
        stream << time / 6000 / 10.0 << " min";
        return;
    }
    if (time < 10 * hour) {
        stream << time / 36000 / 100.0 << " hour";
        return;
    }
    if (time < day) {
        stream << time / 360000 / 10.0 << " hour";
        return;
    }
    std::streamsize size = stream.precision();
    stream << std::fixed << std::setprecision(1) << time / 360000 / 10.0 << " hour";
    stream << std::defaultfloat << std::setprecision(size);
    return;
}
}  // namespace Timer

namespace ThreadPool {
// I hope there's no race conditions or unexpected behavior
class ThreadPool {
   private:
    bool stopThreads;
    bool isActive;
    int64_t threadCount;
    int64_t jobCount;
    int64_t activeThreads;
    std::condition_variable mainWait;
    std::mutex threadLock;
    std::condition_variable threadWait;
    std::vector<std::thread> threads;
    std::queue<std::function<void(int64_t)>> jobs;

    void threadLoop(int64_t threadId) {
        while (true) {
            std::unique_lock<std::mutex> lock(this->threadLock);
            this->activeThreads--;
            this->mainWait.notify_all();
            this->threadWait.wait(lock, [this]() { return !this->jobs.empty() || this->stopThreads; });
            if (this->stopThreads) return;
            auto job = this->jobs.front();
            this->jobs.pop();
            this->activeThreads++;
            lock.unlock();
            job(threadId);
        }
    }

   public:
    ThreadPool(int64_t threadCount)
        : stopThreads{false},
          isActive{false},
          threadCount{threadCount},
          jobCount{0},
          activeThreads{0},
          threads(threadCount) {}
    ThreadPool() : ThreadPool((int64_t)std::thread::hardware_concurrency()) {}
    void start() {
        if (this->isActive) return;
        this->activeThreads = this->threadCount;
        for (int64_t i = 0; i < this->threadCount; i++)
            this->threads[i] = std::thread([this, i]() { this->threadLoop(i); });
        this->isActive = true;
    }
    void stop() {
        if (!this->isActive) return;
        std::unique_lock<std::mutex> lock(this->threadLock);
        this->stopThreads = true;
        this->threadWait.notify_all();
        lock.unlock();
        for (std::thread& currThread : this->threads) currThread.join();
        this->stopThreads = false;
        this->threads.clear();
        this->threads.resize(this->threadCount);
        std::queue<std::function<void(int64_t)>> emptyQueue;
        this->jobs = emptyQueue;
        this->jobCount = 0;
        this->isActive = false;
        this->mainWait.notify_all();
    }
    void wait(std::ostream& stream) {
        using std::float64_t;
        std::unique_lock<std::mutex> lock(this->threadLock);
        std::cout << this->threadCount << " Threads:\n";
        auto start = std::chrono::steady_clock::now();
        this->mainWait.wait(lock, [this, &stream, &start] {
            auto stop = std::chrono::steady_clock::now();
            int64_t activeThreads = this->activeThreads;
            int64_t jobCount = this->jobCount;
            int64_t remainingJobs = std::min(jobCount, (int64_t)this->jobs.size() + activeThreads);
            int64_t completedJobs = std::max((int64_t)0, this->jobCount - remainingJobs - activeThreads);
            stream << "\33[2K\r";
            stream << completedJobs << " / " << jobCount << " - ";
            Timer::printTime(stream, start, stop);
            stream << " < ";
            if (completedJobs != 0) {
                auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
                float64_t timePerJob = duration.count() / (float64_t)(completedJobs);
                auto nanoTime = remainingJobs * timePerJob;
                Timer::printTime(stream, stop, stop + std::chrono::nanoseconds((int64_t)nanoTime));
            } else {
                stream << "unknown";
            }
            stream << std::flush;
            return this->jobs.empty() && this->activeThreads == 0;
        });
        stream << std::endl;
        this->jobCount = 0;
    }
    void wait() {
        std::unique_lock<std::mutex> lock(this->threadLock);
        this->mainWait.wait(lock, [this] { return this->jobs.empty() && this->activeThreads == 0; });
    }
    bool isBusy() {
        std::unique_lock<std::mutex> lock(this->threadLock);
        return this->jobs.empty();
    }
    void addJob(const std::function<void(int64_t)>& job) {
        std::unique_lock<std::mutex> lock(this->threadLock);
        this->jobCount++;
        this->jobs.push(job);
        this->threadWait.notify_one();
    }
    int64_t getThreadCount() { return this->threadCount; }
    bool getActiveStatus() { return this->isActive; }
};
}  // namespace ThreadPool
