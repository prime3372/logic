#include "PropositionalLogic.hpp"

using P = Prop<0>;

consteval Equiv<P, P> solve() {
    return {
        [&](P p) -> P { return p; },
        [&](P p) -> P { return p; }
    };
}

int main() {
    solve();
}