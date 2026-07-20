#include "PropositionalLogic.hpp"

using P = Prop<0>;
using Q = Prop<1>;

consteval Equiv<Impl<P, Q>, Impl<Not<Q>, Not<P>>> solve() {
    return {
        [&](Impl<P, Q> impl_p_q) -> Impl<Not<Q>, Not<P>> {
            return [&](Not<Q> not_q) -> Not<P> {
                return [&](P p) -> False {
                    return not_q(impl_p_q(p));
                };
            };
        },
        [&](Impl<Not<Q>, Not<P>> impl_not_q_not_p) -> Impl<P, Q> {
            return [&](P p) -> Q {
                return Or<Q, Not<Q>>().elim(
                    [&](Q q) -> Q { return q; },
                    [&](Not<Q> not_q) -> Q { return impl_not_q_not_p(not_q)(p).explode<Q>(); }
                );
            };
        }
    };
}

int main() {
    solve();
}