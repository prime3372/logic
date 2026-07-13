#include "PropositionalLogic/PropositionalLogic.hpp"

using P = Prop<0>;
using Q = Prop<1>;
using R = Prop<2>;

consteval Impl<Impl<P, Impl<Q, R>>, Impl<Impl<P, Q>, Impl<P, R>>> solve() {
    return [&](Impl<P, Impl<Q, R>> impl_p_impl_q_r) -> Impl<Impl<P, Q>, Impl<P, R>> {
        return [&](Impl<P, Q> impl_p_q) -> Impl<P, R> {
            return [&](P p) -> R {
                return impl_p_impl_q_r(p)(impl_p_q(p));
            };
        };
    };
}

int main() {
    solve();
}