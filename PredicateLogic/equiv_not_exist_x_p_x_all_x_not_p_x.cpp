#include "PredicateLogic.hpp"

template <class x, class P> using NotAll = Not<All<x, P>>;
template <class x, class P> using NotExist = Not<Exist<x, P>>;
template <class x> using P = Pred<0, x>;
template <class x> using NotP = Not<P<x>>;
using x = Bound<0>;

consteval Equiv<NotExist<x, P<x>>, All<x, NotP<x>>> solve() {
    return {
        [&](NotExist<x, P<x>> not_exist_x_p_x) -> All<x, NotP<x>> {
            return [&]<class a>() -> NotP<a> {
                return [&](P<a> p_a) -> False {
                    return not_exist_x_p_x({a(), p_a});
                };
            };
        },
        [&](All<x, NotP<x>> all_x_not_p_x) -> NotExist<x, P<x>> {
            return [&](Exist<x, P<x>> exist_x_p_x) -> False {
                return exist_x_p_x.elim(
                    [&]<class a>(P<a> p_a) -> False {
                        return all_x_not_p_x.elim<a>()(p_a);
                    }
                );
            };
        }
    };
}

int main() {
    solve();
}