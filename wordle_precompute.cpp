#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <stdfloat>
#include <tuple>
#include <utility>

#include "thread_pool.hpp"
#include "wordle_calculations.hpp"

template <typename T>
void shuffle_array(std::vector<T> &arr) {
    std::random_device rd;
    // std::mt19937 mt(rd());
    std::mt19937 mt(0);
    std::shuffle(arr.begin(), arr.end(), mt);
}

void test_code() {
    using Wordle::FILTER_STATE::CORRECT_SPOT;
    using Wordle::FILTER_STATE::NO_SPOT;
    using Wordle::FILTER_STATE::WRONG_SPOT;

    std::initializer_list<std::tuple<std::string, std::string, std::vector<Wordle::FILTER_STATE>>> filterTests = {
        {"tidal", "embed", {NO_SPOT, NO_SPOT, NO_SPOT, NO_SPOT, WRONG_SPOT}},
        {"tidal", "lares", {WRONG_SPOT, WRONG_SPOT, NO_SPOT, NO_SPOT, NO_SPOT}},
        {"tidal", "crane", {NO_SPOT, NO_SPOT, WRONG_SPOT, NO_SPOT, NO_SPOT}},
        {"tidal", "reaies", {NO_SPOT, NO_SPOT, WRONG_SPOT, WRONG_SPOT, NO_SPOT}},
        {"tidal", "areos", {WRONG_SPOT, NO_SPOT, NO_SPOT, NO_SPOT, NO_SPOT}},
        {"tidal", "earls", {NO_SPOT, WRONG_SPOT, NO_SPOT, WRONG_SPOT, NO_SPOT}},

        {"boost", "afoot", {NO_SPOT, NO_SPOT, CORRECT_SPOT, WRONG_SPOT, CORRECT_SPOT}},
        {"boost", "achoo", {NO_SPOT, NO_SPOT, NO_SPOT, WRONG_SPOT, WRONG_SPOT}},
        {"boost", "abbot", {NO_SPOT, WRONG_SPOT, NO_SPOT, WRONG_SPOT, CORRECT_SPOT}},
        {"boost", "attap", {NO_SPOT, WRONG_SPOT, NO_SPOT, NO_SPOT, NO_SPOT}},
        {"boost", "oboes", {WRONG_SPOT, WRONG_SPOT, CORRECT_SPOT, NO_SPOT, WRONG_SPOT}},
        {"boost", "orlop", {WRONG_SPOT, NO_SPOT, NO_SPOT, WRONG_SPOT, NO_SPOT}},

        {"birch", "roons", {WRONG_SPOT, NO_SPOT, NO_SPOT, NO_SPOT, NO_SPOT}},
        {"birch", "tucks", {NO_SPOT, NO_SPOT, WRONG_SPOT, NO_SPOT, NO_SPOT}},
        {"birch", "idled", {WRONG_SPOT, NO_SPOT, NO_SPOT, NO_SPOT, NO_SPOT}},
        {"birch", "cutin", {WRONG_SPOT, NO_SPOT, NO_SPOT, WRONG_SPOT, NO_SPOT}},
        {"birch", "snick", {NO_SPOT, NO_SPOT, WRONG_SPOT, CORRECT_SPOT, NO_SPOT}},
        {"birch", "tendu", {NO_SPOT, NO_SPOT, NO_SPOT, NO_SPOT, NO_SPOT}},

        {"antic", "paisa", {NO_SPOT, WRONG_SPOT, WRONG_SPOT, NO_SPOT, NO_SPOT}},
        {"antic", "limba", {NO_SPOT, WRONG_SPOT, NO_SPOT, NO_SPOT, WRONG_SPOT}},
        {"antic", "lamas", {NO_SPOT, WRONG_SPOT, NO_SPOT, NO_SPOT, NO_SPOT}},
        {"antic", "lemes", {NO_SPOT, NO_SPOT, NO_SPOT, NO_SPOT, NO_SPOT}},
        {"antic", "globe", {NO_SPOT, NO_SPOT, NO_SPOT, NO_SPOT, NO_SPOT}},
        {"antic", "igloo", {WRONG_SPOT, NO_SPOT, NO_SPOT, NO_SPOT, NO_SPOT}},
    };

    for (auto const &[real, guess, filter] : filterTests) {
        auto computed_filter = Wordle::getFilterState(real, guess);
        assert(computed_filter == filter);
    }

    auto words_all = Wordle::parseWords("wordle-nyt-words-14855.txt", 14855);
    shuffle_array(words_all);
    std::vector<std::string> words(words_all.begin(), words_all.begin() + 100);

    for (auto const &real_word : words) {
        for (auto const &test_word : words) {
            for (auto const &guess_word : words) {
                auto filter_1 = Wordle::getFilterState(real_word, guess_word);
                auto filter_2 = Wordle::getFilterState(test_word, guess_word);
                bool computed_valid = Wordle::validWord(test_word, guess_word, filter_1);
                bool valid = filter_1 == filter_2;
                assert(computed_valid == valid);
            }
        }
    }

    shuffle_array(words);

    std::chrono::duration<int64_t, std::nano> filter_compare{0};
    std::chrono::duration<int64_t, std::nano> custom_compare{0};

    for (auto const &real_word : words) {
        for (auto const &test_word : words) {
            for (auto const &guess_word : words) {
                auto filter_1 = Wordle::getFilterState(real_word, guess_word);

                auto time1 = std::chrono::steady_clock::now();
                auto filter_2 = Wordle::getFilterState(test_word, guess_word);
                bool valid = filter_1 == filter_2;
                auto time2 = std::chrono::steady_clock::now();
                bool computed_valid = Wordle::validWord(test_word, guess_word, filter_1);
                auto time3 = std::chrono::steady_clock::now();
                filter_compare += time2 - time1;
                custom_compare += time3 - time2;

                assert(computed_valid == valid);
            }
        }
    }

    auto filter_time = std::chrono::duration_cast<std::chrono::microseconds>(filter_compare).count();
    auto custom_time = std::chrono::duration_cast<std::chrono::microseconds>(custom_compare).count() / 1000.0;

    std::cout << "Filter Compare Time: " << filter_time / 1000.0 << " ms\nCustom Compare Time: " << custom_time
              << " ms\n";
}

int main(int argc, char const *argv[]) {
    (void)argc;
    (void)argv;

    using std::float64_t;

    test_code();

    ThreadPool::ThreadPool pool;
    auto words = Wordle::parseWords("wordle-nyt-words-14855.txt", 14855);
    auto dist = Wordle::getWordDistribution(words, words, pool);

    std::ofstream file("wordle_scores.txt");
    for (int64_t score : dist) file << score << "\n";
    assert(words.size() == dist.size());

    std::vector<std::pair<std::string, int64_t>> vect(dist.size());
    for (size_t i = 0; i < dist.size(); i++) vect[i] = std::pair(words[i], dist[i]);
    std::sort(vect.begin(), vect.end(), [](auto &a, auto &b) { return b.second > a.second; });

    std::ofstream file2("wordle_output.txt");
    file2 << std::fixed << std::setprecision(3);
    for (size_t i = 0; i < dist.size(); i++)
        file2 << vect[i].first << " " << vect[i].second / (float64_t)dist.size() << "\n";

    pool.stop();
    return 0;
}
