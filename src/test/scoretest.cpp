#include "../Score.cpp"
// #include "../Score.h"
#include <iostream>

int main(int argc, char const *argv[]) {
    std::cout << std::boolalpha;
    // Score a, b;
    // a.won = true;
    // b.score = 10;
    Score a = {5, false, true, false}; // lost = true
    Score b = {0, true, false,
               false}; // won = true (b should be greater than a!!)
    Score c = {20, false, false, false};
    // std::cout << a.score << std::endl;
    // std::cout << a.won << std::endl;
    // std::cout << a.lost << std::endl;
    std::cout << (c > b) << std::endl;
    std::cout << (a > b) << std::endl;
    std::cout << (b < a) << std::endl;
    std::cout << (a < b) << std::endl;
    return 0;
}
