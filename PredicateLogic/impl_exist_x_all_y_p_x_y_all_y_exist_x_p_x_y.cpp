#include "PredicateLogic.hpp"

template <class x, class y> using P = Pred<0, x, y>;
using x = Bound<0>;
using y = Bound<1>;

consteval Impl<Exist<x, All<y, P<x, y>>>, All<y, Exist<x, P<x, y>>>> solve() {
    return [&](Exist<x, All<y, P<x, y>>> exist_x_all_y_p_x_y) -> All<y, Exist<x, P<x, y>>> {
        return exist_x_all_y_p_x_y.elim(
            [&]<TakeSome a>(All<y, P<a, y>> all_y_p_a_y) -> All<y, Exist<x, P<x, y>>> {
                return [&]<TakeAny b>() -> Exist<x, P<x, b>> {
                    return {a(), all_y_p_a_y.template elim<b>()};
                };
            }
        );
    };
}

int main() {
    solve();
}