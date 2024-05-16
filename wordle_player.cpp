#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <stdfloat>
#include <tuple>
#include <utility>

#include "wordle_calculations.hpp"

std::vector<std::pair<std::string, int64_t>> sort_scores(const std::vector<std::string> &words,
                                                         const std::vector<int64_t> &dist) {
    std::vector<std::pair<std::string, int64_t>> vect(dist.size());
    for (size_t i = 0; i < dist.size(); i++) vect[i] = std::pair(words[i], dist[i]);
    std::sort(vect.begin(), vect.end(), [](auto &a, auto &b) { return b.second > a.second; });
    return vect;
}

void print_scores(const std::vector<std::pair<std::string, int64_t>> &sorted_scores, size_t remain_count,
                  size_t count = 3) {
    using std::float64_t;
    std::streamsize precision = std::cout.precision();
    std::cout << std::fixed << std::setprecision(3);
    for (size_t i = 0; i < std::min(sorted_scores.size(), count); i++)
        std::cout << sorted_scores[i].first << " - " << sorted_scores[i].second / (float64_t)remain_count << "\n";
    std::cout << std::defaultfloat << std::setprecision(precision);
}

void read_word(std::string &the_word, std::vector<Wordle::FILTER_STATE> &the_filter) {
    std::string word, filter;
    std::cin >> word >> filter;
    assert(word.size() == Wordle::word_length);
    assert(filter.size() == Wordle::word_length);
    for (int64_t i = 0; i < Wordle::word_length; i++) {
        char word_c = word[i];
        assert(isalpha(word_c));
        word_c = tolower(word_c);
        the_word[i] = word_c;
        char filter_c = filter[i];
        assert(isalpha(filter_c));
        filter_c = tolower(filter_c);
        assert(filter_c == 'w' || filter_c == 'c' || filter_c == 'n');
        switch (filter_c) {
            case 'w':
                the_filter[i] = Wordle::WRONG_SPOT;
                break;
            case 'c':
                the_filter[i] = Wordle::CORRECT_SPOT;
                break;
            case 'n':
                the_filter[i] = Wordle::NO_SPOT;
                break;
        }
    }
}

int main(int argc, char const *argv[]) {
    (void)argc;
    (void)argv;

    ThreadPool::ThreadPool pool;
    auto all_words = Wordle::parseWords("wordle-nyt-words-14855.txt", 14855);
    auto valid_words = all_words;
    auto scores = Wordle::parseScores("wordle_scores.txt", 14855);
    std::string the_word = "abcde";
    std::vector<Wordle::FILTER_STATE> the_filter(Wordle::word_length, Wordle::INVALID_SPOT);
    assert(the_word.size() == Wordle::word_length);
    assert(the_filter.size() == Wordle::word_length);

    std::cout << all_words.size() << " words:\n";
    while (valid_words.size() != 1) {
        auto sorted_scores = sort_scores(all_words, scores);
        print_scores(sorted_scores, valid_words.size());
        read_word(the_word, the_filter);
        valid_words = Wordle::filterWords(valid_words, the_word, the_filter);
        std::cout << valid_words.size() << " words:\n";
        scores = Wordle::getWordDistribution(valid_words, all_words, pool);
    }
    std::cout << "The Word: " << valid_words[0] << "\n";

    pool.stop();

    return 0;
}
