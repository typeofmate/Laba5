#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    std::atomic<size_t> cnt(0);

    auto lol = [&]() { cnt++; };

    std::vector<std::thread> tPool;
    size_t                   thread_cnt = 8;
    for (size_t i = 0; i < thread_cnt; i++) {
        tPool.emplace_back(lol);
    }

    for (auto& th : tPool) {
        th.join();
    }
    std::cout << cnt << std::endl;
}