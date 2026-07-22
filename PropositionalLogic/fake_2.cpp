#include "PropositionalLogic/PropositionalLogic.hpp"
#include <memory>

union Fake {
    char dummy;
    False fal;
    constexpr Fake() : dummy(0) {}
};

consteval False solve() {
    Fake fake;
    Or<False, Not<False>>().elim(
        [&](False fal) -> Or<False, Not<False>> {
            std::construct_at(&fake.fal, fal);
            return fal;
        },
        [&](Not<False> not_fal) -> Or<False, Not<False>> {
            return not_fal;
        }
    );
    return fake.fal;
}

int main() {
    solve();
}