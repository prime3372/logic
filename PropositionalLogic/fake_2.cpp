#include "PropositionalLogic/PropositionalLogic.hpp"
#include <memory>

union Fake {
    char dummy;
    False value;
    constexpr Fake() : dummy(0) {}
};

consteval False solve() {
    Fake fake;
    Or<False, Not<False>>().elim(
        [&](False fal) -> Or<False, Not<False>> {
            std::construct_at(&fake.value, fal);
            return fal;
        },
        [&](Not<False> not_fal) -> Or<False, Not<False>> { return not_fal; }
    );
    return fake.value;
}

int main() {
    solve();
}