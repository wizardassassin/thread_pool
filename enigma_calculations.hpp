#include <algorithm>
#include <limits>
#include <random>
#include <utility>
#include <vector>

namespace Enigma {

const size_t rotor_size = 26;
const size_t rotor_count = 3;
const size_t plugboard_count = 10;

void generateMap(std::mt19937& mt, std::vector<int>& shuffle_arr) {
    std::iota(shuffle_arr.begin(), shuffle_arr.end(), 0);
    std::shuffle(shuffle_arr.begin(), shuffle_arr.end(), mt);
}

bool hasSelfMap(const std::vector<int>& shuffle_arr) {
    return std::any_of(shuffle_arr.begin(), shuffle_arr.end(), [idx = 0](int val) mutable { return val == idx++; });
}

void generateReflectorMap(std::mt19937& mt, std::vector<int>& vect, size_t vect_size) {
    // TODO
}

void generatePlugboardMap(std::mt19937& mt, std::vector<int>& vect, size_t vect_size, int plug_count) {
    // TODO
}

class Rotor {
   private:
    std::vector<int64_t> forward_map;
    std::vector<int64_t> backward_map;
    size_t offset;

   public:
    Rotor() : forward_map(rotor_size), backward_map(rotor_size), offset{0} {}
    ~Rotor() {}
    void setRotor(const std::vector<int64_t>& the_map) {
        for (size_t i = 0; i < rotor_size; i++) {
            forward_map[i] = the_map[i] - i;
            backward_map[the_map[i]] = i - the_map[i];
        }
    }
    void setOffset(size_t the_offset) { offset = the_offset; }
    int64_t forwardEncodeVal(int64_t input) {
        return (input + forward_map[(input + offset) % rotor_size] + rotor_size) % rotor_size;
    }
    int64_t backwardEncodeVal(int64_t input) {
        return (input + backward_map[(input + offset) % rotor_size] + rotor_size) % rotor_size;
    }
    bool increment() { return 0 == (offset = (offset + 1) % rotor_size); }
};

class Rotorboard {
   private:
    std::vector<Rotor> rotors;
    Rotor reflector;

   public:
    Rotorboard() : rotors(rotor_count), reflector() {}
    ~Rotorboard() {}
    void setRotorboard(const std::vector<std::vector<int64_t>>& the_map) {
        for (size_t i = 0; i < rotor_count; i++) {
            rotors[i].setRotor(the_map[i]);
        }
    }
    void setReflector(const std::vector<int64_t>& the_map) { reflector.setRotor(the_map); }
    int64_t encodeVal(int64_t val) {
        for (size_t i = 0; i < rotor_count; i++) {
            val = rotors[i].forwardEncodeVal(val);
        }
        val = reflector.forwardEncodeVal(val);
        for (size_t i = rotor_count; i > 0; i--) {
            val = rotors[i - 1].backwardEncodeVal(val);
        }
        return val;
    }
    bool increment() {
        bool did_rollover = true;
        for (size_t i = 0; i < rotor_count; i++) {
            if (did_rollover) did_rollover = rotors[i].increment();
        }
        return did_rollover;
    }
};

class EnigmaMachine {
   private:
    Rotorboard rotorboard;
    Rotor plugboard;

   public:
    EnigmaMachine() : rotorboard(), plugboard() {}
    ~EnigmaMachine() {}
    void setEnigmaMachine() {
        plugboard.setRotor(
            {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25});
        rotorboard.setReflector(
            {13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12});
        rotorboard.setRotorboard(
            {{7, 2, 0, 8, 16, 5, 18, 22, 25, 12, 15, 10, 17, 3, 9, 21, 11, 13, 20, 6, 4, 24, 1, 14, 19, 23},
             {1, 19, 12, 18, 5, 25, 0, 22, 2, 10, 14, 24, 13, 6, 3, 11, 4, 8, 16, 7, 15, 23, 21, 20, 9, 17},
             {7, 8, 25, 16, 15, 1, 24, 9, 21, 18, 20, 10, 3, 12, 19, 23, 17, 14, 5, 0, 6, 22, 4, 13, 2, 11}});
    }
    int64_t encodeVal(int64_t val) {
        rotorboard.increment();
        val = plugboard.forwardEncodeVal(val);
        val = rotorboard.encodeVal(val);
        val = plugboard.forwardEncodeVal(val);
        return val;
    }
    std::string encodeString(const std::string& str) {
        std::string new_str = "";
        for (char c : str) new_str += 'a' + encodeVal(c - 'a');
        return new_str;
    }
};

}  // namespace Enigma
