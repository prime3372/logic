#include "PropositionalLogic/PropositionalLogic.hpp"

consteval False solve() {
    False* fake_ptr = nullptr;
    Or<False, Not<False>>().elim(
        [&](False fal) -> Or<False, Not<False>> {
            fake_ptr = new False(fal);
            return fal;
        },
        [&](Not<False> not_fal) -> Or<False, Not<False>> {
            return not_fal;
        }
    );
    False fake = *fake_ptr;
    delete fake_ptr;
    return fake;
}

int main() {
    solve();
}