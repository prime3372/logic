#include "PredicateLogic.hpp"

template <class x, class y> using P = Prop<0, x, y>;
using x = Bound<0>;
using y = Bound<1>;
using Pxy = P<x, y>;

consteval Impl<Exist<x, All<y, Pxy>>, All<y, Exist<x, Pxy>>> solve() {
    return [&](Exist<x, All<y, Pxy>> exist_x_all_y_pxy) -> All<y, Exist<x, Pxy>> {
        return exist_x_all_y_pxy.elim(
            [&]<TakeSome a>(All<y, P<a, y>> all_y_pay) -> All<y, Exist<x, Pxy>> {
                return [&]<TakeAny b>() -> Exist<x, P<x, b>> {
                    return {a(), all_y_pay.elim(b())};
                };
            }
        );
    };
}

int main() {
    solve();
}