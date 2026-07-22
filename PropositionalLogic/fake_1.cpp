#include "PropositionalLogic/PropositionalLogic.hpp"

consteval False solve() {
    False* fal_ptr = nullptr;
    Or<False, Not<False>>().elim(
        [&](False fal) -> Or<False, Not<False>> {
            fal_ptr = new False(fal);
            return fal;
        },
        [&](Not<False> not_fal) -> Or<False, Not<False>> {
            return not_fal;
        }
    );
    False fal = *fal_ptr;
    delete fal_ptr;
    return fal;
}

int main() {
    solve();
}