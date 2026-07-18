#include "PropositionalLogic/PropositionalLogic.hpp"
#include <optional>

consteval False solve() {
    std::optional<False> fake;
    Or<False, Not<False>>().elim(
        [&](False fal) -> Or<False, Not<False>> {
            fake.emplace(fal);
            return fal;
        },
        [&](Not<False> not_fal) -> Or<False, Not<False>> { return not_fal; }
    );
    return fake.value();
}

int main() {
    solve();
}