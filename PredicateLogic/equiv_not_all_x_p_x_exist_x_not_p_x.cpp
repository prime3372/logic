#include "PredicateLogic.hpp"

template <class x, class P> using NotAll = Not<All<x, P>>;
template <class x, class P> using NotExist = Not<Exist<x, P>>;
template <class x> using P = Pred<0, x>;
template <class x> using NotP = Not<P<x>>;
using x = Bound<0>;

consteval Equiv<NotAll<x, P<x>>, Exist<x, NotP<x>>> solve() {
    return {
        [&](NotAll<x, P<x>> not_all_x_p_x) -> Exist<x, NotP<x>> {
            return Or<Exist<x, NotP<x>>, NotExist<x, NotP<x>>>().elim(
                [&](Exist<x, NotP<x>> exist_x_not_p_x) -> Exist<x, NotP<x>> {
                    return exist_x_not_p_x;
                },
                [&](NotExist<x, NotP<x>> not_exist_x_not_p_x) -> Exist<x, NotP<x>> {
                    return not_all_x_p_x(
                        [&]<class a>() -> P<a> {
                            return Or<P<a>, NotP<a>>().elim(
                                [&](P<a> p_a) -> P<a> { 
                                    return p_a;
                                },
                                [&](NotP<a> not_p_a) -> P<a> {
                                    return not_exist_x_not_p_x(
                                        {a(), not_p_a}
                                    ).template explode<P<a>>();
                                }
                            );
                        }
                    ).explode<Exist<x, NotP<x>>>();
                }
            );
        },
        [&](Exist<x, NotP<x>> exist_x_not_p_x) -> NotAll<x, P<x>> {
            return exist_x_not_p_x.elim(
                [&]<class a>(NotP<a> not_p_a) -> NotAll<x, P<x>> {
                    return [&](All<x, P<x>> all_x_p_x) -> False {
                        return not_p_a(all_x_p_x.elim<a>());
                    };
                }
            );
        }
    };
}

int main() {
    solve();
}