#include "PredicateLogic.hpp"

template <class x> using P = Pred<0, x>;
template <class x> using Q = Pred<1, x>;
using x = Bound<0>;

Impl<Exist<x, And<P<x>, Q<x>>>, And<Exist<x, P<x>>, Exist<x, Q<x>>>> solve() {
    return [&](Exist<x, And<P<x>, Q<x>>> exist_x_and_p_x_q_x) -> And<Exist<x, P<x>>, Exist<x, Q<x>>> {
        return exist_x_and_p_x_q_x.elim(
            [&]<class a>(And<P<a>, Q<a>> and_p_a_q_a) -> And<Exist<x, P<x>>, Exist<x, Q<x>>> {
                return {
                    {a(), and_p_a_q_a.left},
                    {a(), and_p_a_q_a.right}
                };
            }
        );
    };
}

int main() {
    solve();
}