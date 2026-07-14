#include "PropositionalLogic.hpp"

using P = Prop<0>;

consteval Equiv<P, Not<Not<P>>> solve() {
    return {
        [&](P p) -> Not<Not<P>> {
            return [&](Not<P> not_p) -> False {
                return not_p(p);
            };
        },
        [&](Not<Not<P>> not_not_p) -> P {
            return Or<P, Not<P>>().elim(
                [&](P p) -> P { return p; },
                [&](Not<P> not_p) -> P { return not_not_p(not_p).explode<P>(); }
            );
        }
    };
}

int main() {
    solve();
}