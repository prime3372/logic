#include "PredicateLogic.hpp"

template <class x> using P = Pred<0, x>;
template <class x> using Q = Pred<1, x>;
using x = Bound<0>;

Impl<Or<All<x, P<x>>, All<x, Q<x>>>, All<x, Or<P<x>, Q<x>>>> solve() {
    return [&](Or<All<x, P<x>>, All<x, Q<x>>> or_all_x_p_x_all_x_q_x) -> All<x, Or<P<x>, Q<x>>> {
        return [&]<class a>() -> Or<P<a>, Q<a>> {
            return or_all_x_p_x_all_x_q_x.elim(
                [&](All<x, P<x>> all_x_p_x) {
                    return Or<P<a>, Q<a>>(all_x_p_x.elim<a>());
                },
                [&](All<x, Q<x>> all_x_q_x) {
                    return Or<P<a>, Q<a>>(all_x_q_x.elim<a>());
                }
            );
        };
    };
}

int main() {
    solve();
}