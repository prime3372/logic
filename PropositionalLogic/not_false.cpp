#include "PropositionalLogic.hpp"

consteval Not<False> solve() {
    return [=](False fal) -> False { return fal; };
};

int main() {
    solve();
}