#include "PropositionalLogic.hpp"

using P = Prop<0>;
using Q = Prop<1>;
using R = Prop<2>;

consteval Equiv<Or<Or<P, Q>, R>, Or<P, Or<Q, R>>> solve() {
    return {
        [&](Or<Or<P, Q>, R> or_or_p_q_r) -> Or<P, Or<Q, R>> {
            return or_or_p_q_r.elim(
                [&](Or<P, Q> or_p_q) -> Or<P, Or<Q, R>> {
                    return or_p_q.elim(
                        [&](P p) -> Or<P, Or<Q, R>> {
                            return p;
                        },
                        [&](Q q) -> Or<P, Or<Q, R>> {
                            return Or<Q, R>(q);
                        }
                    );
                },
                [&](R r) -> Or<P, Or<Q, R>> { return Or<Q, R>(r); }
            );
        },
        [&](Or<P, Or<Q, R>> or_p_or_q_r) -> Or<Or<P, Q>, R> {
            return or_p_or_q_r.elim(
                [&](P p) -> Or<Or<P, Q>, R> {
                    return Or<P, Q>(p);
                },
                [&](Or<Q, R> or_q_r) -> Or<Or<P, Q>, R> {
                    return or_q_r.elim(
                        [&](Q q) -> Or<Or<P, Q>, R> {
                            return Or<P, Q>(q);
                        },
                        [&](R r) -> Or<Or<P, Q>, R> {
                            return r;
                        }
                    );
                }
            );
        }
    };
}

int main() {
    solve();
}