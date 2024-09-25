#include <algorithm>
#include <iostream>
#include <random>
#include <vector>

#include "enigma_calculations.hpp"

int main(int argc, char const *argv[]) {
    (void)argc;
    (void)argv;
    std::random_device rd;
    std::mt19937 mt(123456);
    std::string input;
    while (std::cin >> input) {
        Enigma::EnigmaMachine machine;
        machine.setEnigmaMachine();
        std::cout << machine.encodeString(input) << "\n";
    }
    return 0;
}
