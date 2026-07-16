#include "PredicateLogic.hpp"

template <class x> using P = Pred<0, x>;
using x = Bound<0>;
using Px = P<x>;
using NotPx = Not<P<x>>;

consteval Equiv<NotExist<x, Px>, All<x, NotPx>> solve() {
    return {
        [&](NotExist<x, Px> notexist_x_px) -> All<x, NotPx> {
            return [&]<TakeAny a>() -> Not<P<a>> {
                return [&](P<a> pa) -> False { return notexist_x_px({a(), pa}); };
            };
        },
        [&](All<x, NotPx> all_x_not_px) -> NotExist<x, Px> {
            return [&](Exist<x, Px> exist_x_px) -> False {
                return exist_x_px.elim(
                    [&]<TakeSome a>(P<a> pa) -> False {
                        return all_x_not_px.elim<a>()(pa);
                    }
                );
            };
        }
    };
}

int main() {
    solve();
}