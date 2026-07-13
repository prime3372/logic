#include "PropositionalLogic/PropositionalLogic.hpp"

using P = Prop<0>;
using Q = Prop<1>;
using R = Prop<2>;

consteval Equiv<And<And<P, Q>, R>, And<P, And<Q, R>>> solve() {
    return {
        [&](And<And<P, Q>, R> and_and_p_q_r) -> And<P, And<Q, R>> {
            return {
                and_and_p_q_r.left.left,
                {and_and_p_q_r.left.right, and_and_p_q_r.right}
            };
        },
        [&](And<P, And<Q, R>> and_p_and_q_r) -> And<And<P, Q>, R> {
            return {
                {and_p_and_q_r.left, and_p_and_q_r.right.left},
                and_p_and_q_r.right.right
            };
        }
    };
}

int main() {
    solve();
}