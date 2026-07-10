#include "PropositionalLogic.hpp"

using P = Prop<0>;
using Q = Prop<1>;

consteval Equiv<Not<Or<P, Q>>, And<Not<P>, Not<Q>>> solve() {
    return {
        [&](Not<Or<P, Q>> not_or_p_q) -> And<Not<P>, Not<Q>> {
            return {
                [&](P p) -> False { return not_or_p_q.elim(p); },
                [&](Q q) -> False { return not_or_p_q.elim(q); }
            };
        },
        [&](And<Not<P>, Not<Q>> and_not_p_not_q) -> Not<Or<P, Q>> {
            return [&](Or<P, Q> or_p_q) -> False {
                return or_p_q.elim(
                    [&](P p) -> False { return and_not_p_not_q.left().elim(p); },
                    [&](Q q) -> False { return and_not_p_not_q.right().elim(q); }
                );
            };
        }
    };
}

int main() {
    solve();
}