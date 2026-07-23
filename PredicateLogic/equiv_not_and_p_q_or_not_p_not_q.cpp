#include "PredicateLogic.hpp"

using P = Prop<0>;
using Q = Prop<1>;

consteval Equiv<Not<And<P, Q>>, Or<Not<P>, Not<Q>>> solve() {
    return {
        [&](Not<And<P, Q>> not_and_p_q) -> Or<Not<P>, Not<Q>> {
            return Or<P, Not<P>>().elim(
                [&](P p) -> Or<Not<P>, Not<Q>> {
                    return Not<Q>([&](Q q) -> False {
                        return not_and_p_q({p, q});
                    });
                },
                [&](Not<P> not_p) -> Or<Not<P>, Not<Q>> {
                    return not_p;
                }
            );
        },
        [&](Or<Not<P>, Not<Q>> or_not_p_not_q) -> Not<And<P, Q>> {
            return [&](And<P, Q> and_p_q) -> False {
                return or_not_p_not_q.elim(
                    [&](Not<P> not_p) -> False {
                        return not_p(and_p_q.left);
                    },
                    [&](Not<Q> not_q) -> False {
                        return not_q(and_p_q.right);
                    }
                );
            };
        }
    };
}

int main() {
    solve();
}