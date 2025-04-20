// This code is part of the project "Ligra: A Lightweight Graph Processing
// Framework for Shared Memory", presented at Principles and Practice of
// Parallel Programming, 2013.
// Copyright (c) 2013 Julian Shun and Guy Blelloch
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include <chrono>
#include <omp.h>
#include <thread>
#include <vector>
#include "../ligra/ligra.h"
#include <algorithm>
#include <atomic>
#include <condition_variable>

template<class vertex>
struct TaskInfo {
    std::mutex lock;
    std::condition_variable cond;

    alignas(64) atomic<bool> finalExit{false};
    alignas(64) atomic<long> sourceStart{0llu};
    alignas(64) atomic<long> sourceEnd{0llu};
    graph<vertex> &GA;

    explicit TaskInfo(graph<vertex> &GA) : GA(GA) {}
};

struct BFS_F {
    uintE *Parents;

    BFS_F(uintE *_Parents) : Parents(_Parents) {
    }

    inline bool update(uintE s, uintE d) {
        //Update
        if (Parents[d] == UINT_E_MAX) {
            Parents[d] = s;
            return 1;
        } else return 0;
    }

    inline bool updateAtomic(uintE s, uintE d) {
        //atomic version of Update
        return (CAS(&Parents[d], UINT_E_MAX, s));
    }

    //cond function checks if vertex has been visited yet
    inline bool cond(uintE d) { return (Parents[d] == UINT_E_MAX); }
};

template<class vertex>
void workerThreadFunc(TaskInfo<vertex> &info) {
    auto &mutex = info.lock;
    auto &cv = info.cond;
    std::unique_lock<std::mutex> lck(mutex, std::defer_lock);
    while (true) {
        lck.lock();
        cv.wait(lck, [&] {
            if (info.finalExit.load(std::memory_order_acquire)) {
                return true;
            }
            if (info.sourceStart.load(std::memory_order_acquire)
                != info.sourceEnd.load(std::memory_order_acquire)) {
                return true;
            }
            return false;
        });
        lck.unlock();
        if (info.finalExit.load(std::memory_order_acquire)) {
            return;
        }
        auto currSource = 0u;
        while ((currSource = info.sourceStart.fetch_add(1u)) < info.sourceEnd.load(memory_order_acquire)) {
            long n = info.GA.n;
            //creates Parents array, initialized to all -1, except for start
            uintE *Parents = newA(uintE, n);
            parallel_for (long i = 0; i < n; i++) Parents[i] = UINT_E_MAX;
            Parents[currSource] = currSource;
            vertexSubset Frontier(n, currSource); //creates initial frontier
            int level = 0;
            while (!Frontier.isEmpty()) {
                vertexSubset output = edgeMap(info.GA, Frontier, BFS_F(Parents));
                Frontier.del();
                Frontier = output; //set new frontier
                level++;
                printf("source: %u | level: %d | size: %lu\n", currSource, level, Frontier.size());
            }
            Frontier.del();
            free(Parents);
        }
        cv.notify_all();
    }
}

template<class vertex>
void Compute(graph<vertex> &GA, commandLine P) {
    // hardcoding 'k' value, base case is 1 source
    auto k = 1;
    auto workerThreads = std::vector<std::thread>();
    TaskInfo<vertex> taskInfo {GA};
    for (auto i = 0u; i < k; i++) {
        workerThreads.emplace_back([&] {
            workerThreadFunc(taskInfo);
        });
    }
    std::unique_lock<std::mutex> lck(taskInfo.lock, std::defer_lock);
    for (auto round = 0u; round < 2u; round++) {
        auto duration = std::chrono::system_clock::now().time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        unsigned sourceStart = P.getOptionLongValue("-sourceStart", 0);
        unsigned sourceEnd = P.getOptionLongValue("-sourceEnd", 0);
        auto source = sourceStart;
        while (true) {
            auto end = std::min(source + k, sourceEnd);
            taskInfo.lock.lock();
            taskInfo.sourceStart.store(source, memory_order_release);
            taskInfo.sourceEnd.store(end, memory_order_release);
            taskInfo.lock.unlock();
            taskInfo.cond.notify_all();
            lck.lock();
            taskInfo.cond.wait(lck, [&] {
               return taskInfo.sourceStart.load(std::memory_order_acquire) >= end;
            });
            lck.unlock();
            if (end == sourceEnd) {
                break;
            }
            source = end;
        }
        auto duration1 = std::chrono::system_clock::now().time_since_epoch();
        auto millis1 = std::chrono::duration_cast<std::chrono::milliseconds>(duration1).count();
        printf("total time: %lu\n", millis1 - millis);
    }
    taskInfo.lock.lock();
    taskInfo.finalExit.store(true, memory_order_release);
    taskInfo.lock.unlock();
    taskInfo.cond.notify_all();
    for (auto &thread : workerThreads) {
        thread.join();
    }
}
