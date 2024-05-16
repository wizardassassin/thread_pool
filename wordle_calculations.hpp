#pragma once

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "thread_pool.hpp"

// lost in time forever
namespace Wordle {
const int64_t word_length = 5;

enum FILTER_STATE { INVALID_SPOT, WRONG_SPOT, CORRECT_SPOT, NO_SPOT };

std::vector<std::string> parseWords(const std::string& filename, size_t expected_count) {
    std::vector<std::string> words;
    std::ifstream file(filename);
    std::string str;
    while (!(file >> str).fail()) {
        assert(str.size() == word_length);
        words.push_back(str);
    }
    assert(words.size() == expected_count);
    words.shrink_to_fit();
    for (size_t i = 0; i < expected_count; i++) words[i].shrink_to_fit();

    return words;
}

std::vector<int64_t> parseScores(const std::string& filename, size_t expected_count) {
    std::vector<int64_t> scores;
    std::ifstream file(filename);
    int64_t score;
    while (!(file >> score).fail()) {
        assert(score > 0);
        scores.push_back(score);
    }
    assert(scores.size() == expected_count);
    scores.shrink_to_fit();

    return scores;
}

std::vector<FILTER_STATE> getFilterState(const std::string& real_word, const std::string& guess_word) {
    std::vector<FILTER_STATE> filter_state(word_length, INVALID_SPOT);
    std::string real_word_mutate = real_word;
    for (int64_t i = 0; i < word_length; i++) {
        if (real_word[i] == guess_word[i]) {
            filter_state[i] = CORRECT_SPOT;
            real_word_mutate[i] = '_';
        }
    }
    for (int64_t i = 0; i < word_length; i++) {
        if (filter_state[i] == CORRECT_SPOT) continue;
        auto loc = real_word_mutate.find(guess_word[i]);
        if (loc == std::string::npos) {
            filter_state[i] = NO_SPOT;
            continue;
        }
        filter_state[i] = WRONG_SPOT;
        real_word_mutate[loc] = '_';
    }
    return filter_state;
}

bool validWord(const std::string& test_word, const std::string& guess_word,
               const std::vector<FILTER_STATE>& filter_state) {
    std::string test_word_mutate = test_word;
    for (int64_t i = 0; i < word_length; i++) {
        if (test_word[i] == guess_word[i]) {
            if (filter_state[i] != CORRECT_SPOT) return false;
            test_word_mutate[i] = '_';
            continue;
        }
        if (filter_state[i] == CORRECT_SPOT) return false;
    }
    for (int64_t i = 0; i < word_length; i++) {
        if (filter_state[i] == CORRECT_SPOT) continue;
        auto loc = test_word_mutate.find(guess_word[i]);
        if (loc == std::string::npos) {
            if (filter_state[i] != NO_SPOT) return false;
            continue;
        }
        if (filter_state[i] != WRONG_SPOT) return false;
        test_word_mutate[loc] = '_';
    }
    return true;
}

std::vector<std::string> filterWords(const std::vector<std::string>& valid_words, const std::string& guess_word,
                                     const std::vector<FILTER_STATE>& filter_state) {
    std::vector<std::string> new_words;
    for (const std::string& word : valid_words)
        if (validWord(word, guess_word, filter_state)) new_words.push_back(word);
    return new_words;
}

int64_t filterWordsCounter(const std::vector<std::string>& valid_words, const std::string& guess_word,
                           const std::vector<FILTER_STATE>& filter_state) {
    int64_t counter = 0;
    for (const std::string& word : valid_words)
        if (validWord(word, guess_word, filter_state)) counter++;
    return counter;
}

std::vector<int64_t> getGuessDistribution(const std::vector<std::string>& valid_words,
                                          const std::vector<std::string>& guess_words, const std::string& real_word) {
    std::vector<int64_t> dist;
    dist.reserve(guess_words.size());
    for (const std::string& guess : guess_words) {
        auto filter = getFilterState(real_word, guess);
        auto count = filterWordsCounter(valid_words, guess, filter);
        dist.push_back(count);
    }
    return dist;
}

std::vector<int64_t> getWordDistribution(const std::vector<std::string>& valid_words,
                                         const std::vector<std::string>& guess_words, ThreadPool::ThreadPool& pool) {
    std::mutex addLock;
    std::vector<int64_t> guess_dist(guess_words.size(), 0);
    for (const std::string& word : valid_words) {
        pool.addJob([&valid_words, &guess_words, &addLock, &guess_dist, &word](int64_t threadId) {
            (void)threadId;
            auto dist = getGuessDistribution(valid_words, guess_words, word);
            std::unique_lock<std::mutex> lock(addLock);
            for (size_t i = 0; i < guess_words.size(); i++) guess_dist[i] += dist[i];
        });
    }
    pool.start();
    pool.wait(std::cout);
    return guess_dist;
}

std::vector<std::vector<int64_t>> getWordMatrix(const std::vector<std::string>& valid_words,
                                                const std::vector<std::string>& guess_words,
                                                ThreadPool::ThreadPool& pool) {
    std::mutex addLock;
    std::vector<std::vector<int64_t>> guess_matrix(guess_words.size(), std::vector<int64_t>(valid_words.size(), -1));
    for (size_t j = 0; j < valid_words.size(); j++) {
        const std::string& word = valid_words[j];
        pool.addJob([&valid_words, &guess_words, &addLock, &guess_matrix, &word, j](int64_t threadId) {
            (void)threadId;
            auto dist = getGuessDistribution(valid_words, guess_words, word);
            std::unique_lock<std::mutex> lock(addLock);
            for (size_t i = 0; i < guess_words.size(); i++) {
                assert(guess_matrix[i][j] == -1);
                guess_matrix[i][j] = dist[i];
            }
        });
    }
    pool.start();
    pool.wait(std::cout);

    for (size_t i = 0; i < guess_words.size(); i++)
        for (size_t j = 0; j < valid_words.size(); j++) assert(guess_matrix[i][j] != -1);

    return guess_matrix;
}

}  // namespace Wordle
