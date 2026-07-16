#include "PropositionalLogic/PropositionalLogic.hpp"
#include <optional>

using P = Prop<0>;

consteval P solve() {
    std::optional<P> fake;
    Or<P, Not<P>>().elim(
        [&](P p) -> Or<P, Not<P>> {
            fake.emplace(p);
            return p;
        },
        [&](Not<P> not_p) -> Or<P, Not<P>> { return not_p; }
    );
    return fake.value();
}

int main() {
    solve();
}