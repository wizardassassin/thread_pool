#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <tuple>
#include <utility>

#include "wordle_calculations.hpp"

int main(int argc, char const *argv[]) {
    (void)argc;
    (void)argv;

    ThreadPool::ThreadPool pool;
    auto words = Wordle::parseWords("wordle-nyt-words-14855.txt", 14855);
    auto matrix = Wordle::getWordMatrix(words, words, pool);

    std::ofstream file("wordle_matrix.txt");
    for (size_t i = 0; i < words.size(); i++) {
        for (size_t j = 0; j < words.size(); j++) {
            file << matrix[i][j] << " ";
        }
        file << "\n";
    }

    pool.stop();
    return 0;
}