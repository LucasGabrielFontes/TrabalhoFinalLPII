// C++
#include "tslogger.hpp"
#include <thread>
#include <vector>
#include <chrono>
#include <random>
#include <string>
#include <algorithm>
#include <cstdlib>

int main(int argc, char** argv) {
    int threads = 8;
    int lines = 200;
    if (argc >= 2) threads = std::max(1, std::atoi(argv[1]));
    if (argc >= 3) lines   = std::max(1, std::atoi(argv[2]));

    TSLog::Config cfg;
    cfg.path = "logs/demo.log";
    cfg.min_level = TSLog::level_from_env(); // usa TSLOG_LEVEL se setado
    cfg.also_stderr = true;
    TSLog::init(cfg);

    TSLog::info("Iniciando demo de logging concorrente: threads=%d, linhas=%d", threads, lines);

    std::vector<std::thread> pool;
    pool.reserve(threads);
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, 3);

    for (int t = 0; t < threads; ++t) {
        pool.emplace_back([t, lines, &rng, &dist] {
            for (int i = 0; i < lines; ++i) {
                int sel = dist(rng);
                switch (sel) {
                    case 0: TSLog::debug("T%02d linha %04d: debug", t, i); break;
                    case 1: TSLog::info ("T%02d linha %04d: info",  t, i); break;
                    case 2: TSLog::warn ("T%02d linha %04d: warn",  t, i); break;
                    default: TSLog::error("T%02d linha %04d: erro simulado %d", t, i, i%7); break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    for (auto& th : pool) th.join();

    TSLog::info("Finalizando demo");
    TSLog::flush();
    TSLog::close();
    return 0;
}