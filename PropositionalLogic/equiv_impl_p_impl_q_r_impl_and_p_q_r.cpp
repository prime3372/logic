#include "PropositionalLogic/PropositionalLogic.hpp"

using P = Prop<0>;
using Q = Prop<1>;
using R = Prop<2>;

consteval Equiv<Impl<P, Impl<Q, R>>, Impl<And<P, Q>, R>> solve() {
    return {
        [&](Impl<P, Impl<Q, R>> impl_p_impl_q_r) -> Impl<And<P, Q>, R> {
            return [&](And<P, Q> and_p_q) -> R {
                return impl_p_impl_q_r(and_p_q.left)(and_p_q.right);
            };
        },
        [&](Impl<And<P, Q>, R> impl_and_p_q_r) -> Impl<P, Impl<Q, R>> {
            return [&](P p) -> Impl<Q, R> {
                return [&](Q q) -> R {
                    return impl_and_p_q_r({p, q});
                };
            };
        }
    };
}

int main() {
    solve();
}