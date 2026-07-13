#include "PredicateLogic.hpp"

template <class x> using P = Prop<0, x>;
using x = Bound<0>;
using Px = P<x>;
using NotPx = Not<P<x>>;

consteval Equiv<NotAll<x, Px>, Exist<x, NotPx>> solve() {
    return {
        [&](NotAll<x, Px> notall_x_px) -> Exist<x, NotPx> {
            return Or<Exist<x, NotPx>, NotExist<x, NotPx>>().elim(
                [&](Exist<x, NotPx> exist_x_not_px) -> Exist<x, NotPx> { return exist_x_not_px; },
                [&](NotExist<x, NotPx> notexist_x_not_px) -> Exist<x, NotPx> {
                    return notall_x_px.elim(
                        [&]<TakeAny a>() -> P<a> {
                            return Or<P<a>, Not<P<a>>>().elim(
                                [&](P<a> pa) -> P<a> { return pa; },
                                [&](Not<P<a>> not_pa) -> P<a> {
                                    return notexist_x_not_px.elim(
                                        {a(), not_pa}
                                    ).template explode<P<a>>();
                                }
                            );
                        }
                    ).explode<Exist<x, NotPx>>();
                }
            );
        },
        [&](Exist<x, NotPx> exist_x_not_px) -> NotAll<x, Px> {
            return exist_x_not_px.elim(
                [&]<TakeSome a>(Not<P<a>> not_pa) -> NotAll<x, Px> {
                    return [&](All<x, Px> all_x_px) -> False {
                        return not_pa.elim(all_x_px.elim(a()));
                    };
                }
            );
        }
    };
}

int main() {
    solve();
}