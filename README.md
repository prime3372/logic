# C++で形式的証明

## 目次

- **1. はじめに**
- **2. 命題論理の公理・推論規則**
- **3. 命題論理の実装**
   - **3.1. 基本方針**
   - **3.2. 基底クラス**
   - **3.3. `False`**
   - **3.4. `Not`**
   - **3.5. `And`**
   - **3.6. `Or`**
   - **3.7. `Impl`**
   - **3.8. `Equiv`** 
   - **3.9. `Prop`** 
   - **3.10. 補足**  
- **4. 命題論理の証明例**
- **5. 述語論理の公理・推論規則**
- **6. 述語論理の実装**
  - **6.1. メタ関数**
  - **6.2. 基底クラス**
  - **6.3. `All` **
  - **6.4. `Exist` **
  - **6.5. `Pred` **
  -  
- **7. 述語論理の証明例**
- **8. 実装の全体像**
- **9.  アピールポイント**

## 1. はじめに

本稿では述語論理の形式的証明ツールをC\+\+で実装します。形式的証明のコンパイラをC\+\+で書くという意味ではなく、C\+\+の言語機能をそのまま形式的証明の文法に利用し、C\+\+のコードとして形式的証明をコンパイルすることを目指します。正しい証明に対してコンパイルが通るようにすることはもちろん、常識的な範囲の間違いを含む証明に対してコンパイルエラーを出すことも目標にします。

動作確認はすべてg\+\+で行っています。他の環境で動くかは確かめていません。  
実際にコードを動かすときは次のコンパイルオプションを付けてください。
```shell
-fsyntax-only
-std=c++23
```
- -fsyntax-only：構文チェックのみを行うオプションです。実行ファイルが出力されなくなります。
- -std=c\+\+23：バージョンとしてC\+\+23を指定します。

## 2. 命題論理の公理・推論規則

まずは述語論理の土台である命題論理を構築します。今回は記号として $\bot,\lnot,\land,\lor,\to,\leftrightarrow$ を使うこととし、公理および推論規則として以下を採用します。

- 公理
    - **排中律**
    $$P\lor\lnot P$$
- 推論規則
    - **爆発律**
    $$\frac{\begin{array}{c}\bot\end{array}}{P}$$
    - **$\lnot$ 導入則**
    $$\frac{\begin{array}{ccc}[P] \\ \vdots \\ \bot\end{array}}{\lnot P}$$
    - **$\lnot$ 除去則**
    $$\frac{\begin{array}{c}P & \lnot P\end{array}}{\bot}$$
    - **$\land$ 導入則**
    $$\frac{\begin{array}{c}P & Q\end{array}}{P\land Q}$$
    - **$\land$ 除去則**
    $$\frac{\begin{array}{c}P\land Q\end{array}}{P}$$
    $$\frac{\begin{array}{c}P\land Q\end{array}}{Q}$$
    - **$\lor$ 導入則**
    $$\frac{\begin{array}{c}P\end{array}}{P\lor Q}$$
    $$\frac{\begin{array}{c}Q\end{array}}{P\lor Q}$$
    - **$\lor$ 除去則**
    $$\frac{\begin{array}{ccc} & [P] & [Q] \\ & \vdots & \vdots \\ P\lor Q & R & R \end{array}}{R}$$
    - **$\to$ 導入則**
    $$\frac{\begin{array}{ccc}[P] \\ \vdots \\ Q\end{array}}{P\to Q}$$
    - **$\to$ 除去則**
    $$\frac{\begin{array}{c}P & P\to Q\end{array}}{Q}$$
    - **$\leftrightarrow$ 導入則**
    $$\frac{\begin{array}{ccc}[P] & [Q] \\ \vdots & \vdots \\ Q & P \end{array}}{P\leftrightarrow Q}$$
    - **$\leftrightarrow$ 除去則**
    $$\frac{\begin{array}{c}P & P\leftrightarrow Q\end{array}}{Q}$$
    $$\frac{\begin{array}{c}Q & P\leftrightarrow Q\end{array}}{P}$$

## 3. 命題論理の実装

### 3.1. 基本方針

プログラミング上で数学的な証明を取り扱うために、次のような対応関係を考えます。
| 数学 | プログラミング (C\+\+) |
| ----- | ----- |
| 命題 | クラス |
| 証明 | オブジェクト |
| 証明する | クラスのコンストラクタを呼ぶ |
| 証明した命題を使う | オブジェクトからメンバ関数を呼ぶ |
| 仮定する | 仮定を引数として受け取るラムダ式を定義する |
| 仮定付きで証明する | 仮定を引数として受け取るラムダ式から目的の命題を返す |

この対応関係の下で、論理記号 $\bot, \lnot,\land,\lor,\to,\leftrightarrow$ を以下のように実装することにします。

| 論理記号 | コンストラクタ | メンバ |
| ------ | ----- | ----- |
| $\bot$ | なし | 任意の命題を返す（**爆発律**） |
| $\lnot P$ | $P$ を受け取って $\bot$ を返すラムダ式を要求（**$\lnot$ 導入則**） | $P$ を受け取って $\bot$ を返す（**$\lnot$ 除去則**） |
| $P\land Q$ | $P,Q$ を両方とも要求（**$\land$ 導入則**） | $P,Q$ をそれぞれ返す（**$\land$ 除去則**） |
| $P\lor Q$ | $P$ または $Q$ の一方を要求（**$\lor$ 導入則**） | 「$P$ を受け取って $R$ を返すラムダ式」「$Q$ を受け取って $R$ を返すラムダ式」の2つを受け取って $R$ を返す（**$\lor$ 除去則**） |
| $P\to Q$ | $P$ を受け取って $Q$ を返すラムダ式を要求（**$\to$ 導入則**） | $P$ を受け取って $Q$ を返す（**$\to$ 除去則**） | 
| $P\leftrightarrow Q$ | 「$P$ を受け取って $Q$ を返すラムダ式」「$Q$ を受け取って $P$ を返すラムダ式」の2つを要求（**$\leftrightarrow$ 導入則**） | $P, Q$ の一方を受け取って他方を返す（**$\leftrightarrow$ 除去則**） | 

$P\lor\lnot P$ および $\lnot P\lor P$ はデフォルトコンストラクタを持つものとします。（**排中律**）  
命題変数 $P,Q,R,\ldots$ もクラスとして実装します。したがって $\bot$ 以外の論理記号はクラステンプレートとなります。

### 3.2. 基底クラス

除去則を実装するためには、命題クラスどうしでオブジェクトをやりとりさせる必要があります。そこで、すべての命題クラスの基底クラス `PropBase` を定義し、その中で命題クラスのオブジェクトを一括管理することにします。

```cpp
class PropBase;

template <class P>
concept PropType = std::is_base_of_v<PropBase, P>;

class PropBase {
protected:
    consteval PropBase() {}

    template <PropType P>
    static constexpr P object = P();
};
```

`PropBase` を継承した命題クラス `P` は、`PropBase` に対してデフォルトコンストラクタ `P()` へのアクセスを許可しつつ、`PropBase::object` を介して必要なオブジェクトを入手するものとします。

`PropType` は命題クラスを表すコンセプトです。`std::is_base_of_v` を用いて `PropBase` の派生クラスであるか判定しています。

※ `PropBase()` に付いている `consteval` はコンパイル時評価を強制するための修飾子です。`constexpr` と異なり、コンパイル時評価できなかった場合にはエラーが出ます。本稿の形式的証明ツールはコンパイルのみで完結するので、すべての関数に例外なく `consteval` を付けています。

### 3.3. `False`

```cpp
class False final : PropBase {
public:
    template <PropType P>
    consteval P explode() const {
        return PropBase::object<P>;
    }

private:
    friend class PropBase;
    consteval False() {}
};
```

`False` クラスは論理記号 $\bot$ に対応します。

メンバ関数 `explode` は爆発律を実装したものです。一度 `False` のオブジェクトを作ることができれば、`explode` を使ってあらゆる命題を証明できます。

`private:` セクション内に書かれている `friend` 宣言とデフォルトコンストラクタは、前節で説明した通り `PropBase` 内で `False` のオブジェクトを作れるようにしたものです。この部分の実装はすべての命題クラスで共通なので、以降説明を省略します。

### 3.4. `Not`

```cpp
template <PropType P>
class Not final : PropBase {
public:
    consteval Not(auto f) {
        False q = f(PropBase::object<P>);
    }

    consteval False operator()(P) const {
        return PropBase::object<False>;
    }

private:
    friend class PropBase;
    consteval Not() {}
};
```

`Not` クラステンプレートは論理記号 $\lnot$ に対応します。

コンストラクタ `Not(auto f)` は $\lnot$ 導入則を表しています。ラムダ式 `f` に `P` クラスのオブジェクトを渡して `False` クラスのオブジェクトが返ってくるか検証します。  

`operator()` は $\lnot$ 除去則を表しています。 `P` クラスのオブジェクトを受け取って `False` クラスのオブジェクトを返します。
 
※ `Not<P>` の機能は `Impl<P, False>` と本質的に同じなので、エイリアステンプレートを用いて次のように書いても `Not` の実装として十分です。（`Impl` の実装については3.7節を参照してください）
```cpp
template <class P>
using Not = Impl<P, False>
```
もっとも、間違った証明を書いたとき、エラーメッセージではエイリアスが元の名前に展開されるので、ただでさえ読みにくいエラーメッセージがさらに読みにくくなります。本稿ではこれを避けるためエイリアスを積極的には使っていません。

### 3.5. `And`

```cpp
template <PropType P, PropType Q>
class And final : PropBase {
public:
    consteval And(P, Q) {}

    const P left = PropBase::object<P>;
    const Q right = PropBase::object<Q>;

private:
    friend class PropBase;
    consteval And() {}
};
```

`And` クラステンプレートは論理記号 $\land$ に対応します。

コンストラクタ `And(P, Q)` は $\land$ 導入則を表しています。`P`, `Q` のオブジェクトから `And<P, Q>` のオブジェクトを構築できます。

メンバ変数 `left`, `right` は  $\land$ 除去則を表しています。

### 3.6. `Or`

```cpp
template <PropType P, PropType Q>
class Or final : PropBase {
public:
    consteval Or() requires (std::same_as<Q, Not<P>> || std::same_as<P, Not<Q>>) {}
    consteval Or(P) {}
    consteval Or(Q) requires (!std::same_as<P, Q>) {}

    consteval auto elim(auto f, auto g) const {
        auto rf = f(PropBase::object<P>));
        auto rg = g(PropBase::object<Q>));
        static_assert(std::same_as<decltype(rf), decltype(rg)>);
        return PropBase::object<decltype(rf)>;
    }

private:
    friend class PropBase;
    consteval Or() {}
};
```

`Or` クラステンプレートは論理記号 $\lor$ に対応します。

デフォルトコンストラクタは排中律を表しています。`Q` と `Not<P>` がクラスとして等しいとき、または `P` と `Not<Q>` がクラスとして等しいとき、デフォルトコンストラクタを呼び出すことができます。

2, 3番目のコンストラクタは $\lor$ 導入則を表しています。`P` または `Q` のオブジェクトから `Or<P, Q>` のオブジェクトを構築できます。

メンバ関数 `elim` は $\lor$ 除去則を表しています。2つのラムダ式 `f`, `g` を引数として受け取り、それぞれ `P`, `Q` のオブジェクトを渡して同じクラスのオブジェクトが返ってくるか検証します。戻り値としてそのオブジェクトを返します。

### 3.7. `Impl`

```cpp
template <PropType P, PropType Q>
class Impl final : PropBase {
public:
    consteval Impl(auto f) {
        Q q = f(PropBase::object<P>);
    }

    consteval Q operator()(P) const {
        return PropBase::object<Q>;
    }

private:
    friend class PropBase;
    consteval Impl() {}
};
```

`Impl` クラステンプレートは論理記号 $\to$ に対応します。

コンストラクタ `Impl(auto f)` は $\to$ 導入則を表しています。ラムダ式 `f` に `P` のオブジェクトを渡して `Q` のオブジェクトが返ってくるか検証します。

`operator()` は $\to$ 導入則を表しています。`P` のオブジェクトを受け取って `Q` のオブジェクトを返します。

### 3.8. `Equiv`

```cpp
template <PropType P, PropType Q>
class Equiv final : PropBase {
public:
    consteval Equiv(auto f, auto g) {
        Q q = f(PropBase::object<P>);
        P p = g(PropBase::object<Q>);
    }

    consteval Q operator()(P) const {
        return PropBase::object<Q>;
    }

    consteval P operator()(Q) const requires (!std::same_as<P, Q>) {
        return PropBase::object<P>;
    }

private:
    friend class PropBase;
    consteval Equiv() {}
};
```

`Impl` クラステンプレートは論理記号 $\leftrightarrow$ に対応します。

コンストラクタ `Equiv(auto f, auto g)` は $\leftrightarrow$ 導入則を表しています。2つのラムダ式 `f`, `g` を受け取り、ラムダ式 `f` に `P` のオブジェクトを渡して `Q` のオブジェクトが返ってくるか、ラムダ式 `g` に `Q` のオブジェクトを渡して `P` のオブジェクトが返ってくるか検証します。

`operator()` は $\leftrightarrow$ 導入則を表しています。`P`, `Q` のオブジェクトのうち一方を受け取って他方を返します。

### 3.9. `Prop`

```cpp
template <size_t id>
class Prop final : PropBase {
private:
    friend class PropBase;
    consteval Prop() {}
};
```

`Prop` クラステンプレートは命題変数 $P,Q,R,\ldots$ に対応します。`id` に異なる非負整数を入れることで命題変数として区別します。`using` ディレクティブを用いて `using P = Prop<0>` などのエイリアスを定義することを想定しています。

### 3.10. 補足

#### ■ 補足1

上で示した実装ではコピーコンストラクタをデフォルト定義していましたが、実はこのままだと問題が生じます。例えば次のような初期化がコンパイルを通過します。
```
False fal(fal);
```
このような自己初期化はC\+\+では定義された動作であり、初期化されていない `fal` を `fal` にコピーする挙動になります。これを防ぐには、コピーコンストラクタ内に `fal` のメンバ変数を読み取る処理を書けばよいです。
```cpp
class False final : PropBase {
public:
    consteval False(const False& other) {
        initialized = other.initialized; // 追加
    }

    template <PropType P>
    consteval P explode() const {
        return PropBase::object<P>;
    }

private:
    friend class PropBase;
    consteval False() : initialized(true) {}

    bool initialized;
};
```
`other` が正しく初期化されていない場合、コンパイラが未初期化の `other.initialized` を読み取ったことを検知してエラーを出してくれます。

#### ■ 補足2

もう一つ問題があります。`Or` の `elim` 関数および `Impl`, `Equiv` のコンストラクタでは、仮定となる命題を直接ラムダ式に渡しています。
```cpp
consteval auto elim(auto f, auto g) const {
    auto rf = f(PropBase::object<P>);
    auto rg = g(PropBase::object<Q>);
    static_assert(std::same_as<decltype(rf), decltype(rg)>);
    return PropBase::object<decltype(rf)>;
}
```
```cpp
consteval Impl(auto f) {
    Q q = f(PropBase::object<P>);
}
```
```cpp
consteval Equiv(auto f, auto g) {
    Q q = f(PropBase::object<P>);
    P p = g(PropBase::object<Q>);
}
```
この仮定はラムダ式の内部のみで使われることが前提ですが、多少工夫すれば仮定を外部に持ち出すコードも書けてしまいます。例えば次のコードがコンパイルを通過します。
```cpp
False* fal_ptr = nullptr;
Or<False, Not<False>>().elim(
    [&](False fal) -> Or<False, Not<False>> {
        fal_ptr = new False(fal);
        return fal;
    },
    [&](Not<False> not_fal) -> Or<False, Not<False>> {
        return not_fal;
    }
);
False fal = *fal_ptr;
delete fal_ptr;
```
`new` 演算子を `delete` 定義することで上のコードはひとまずコンパイルエラーになります。
```cpp
class False final : PropBase {
public:
    consteval False(const False& other) {
        initialized = other.initialized;
    }

    template <PropType P>
    consteval P explode() const {
        return PropBase::object<P>;
    }

    // 追加
    static void* operator new(size_t) = delete;
    static void* operator new[](size_t) = delete;

private:
    friend class PropBase;
    consteval False() : initialized(true) {}

    bool initialized;
};
```
ところが、ポインタの代わりに共用体を使って初期化を遅延し、`std::construct_at` を使ってコピー構築するとコンパイルエラーを回避できてしまいます。
```cpp
union Fake {
    char dummy;
    False fal;
    constexpr Fake() : dummy(0) {}
};

consteval False solve() {
    Fake fake;
    Or<False, Not<False>>().elim(
        [&](False fal) -> Or<False, Not<False>> {
            std::construct_at(&fake.fal, fal);
            return fal;
        },
        [&](Not<False> not_fal) -> Or<False, Not<False>> { return not_fal; }
    );
    return fake.fal;
}

int main() {
    solve();
}
```
定数式の文脈では、あらかじめ確保していおいたメモリ上にオブジェクトを構築すること (配置new) は通常できないのですが、`std::construct_at` はそれを可能にするための特別な組み込み関数が用意されているらしいです。  
このような手法をコンパイルエラーにする方法は残念ながら思い付きませんでした。とはいえ、ユーザー側が不注意で `std::construct_at` を使って初期化を試みる状況はまず考えられないので、この問題には目を瞑ることにします。

## 4. 命題論理の証明例

### 4.1. 対偶律

$(P\to Q)\leftrightarrow(\lnot Q\to\lnot P)$
```cpp
using P = Prop<0>;
using Q = Prop<1>;

consteval Equiv<Impl<P, Q>, Impl<Not<Q>, Not<P>>> solve() {
    return {
        [&](Impl<P, Q> impl_p_q) -> Impl<Not<Q>, Not<P>> {
            return [&](Not<Q> not_q) -> Not<P> {
                return [&](P p) -> False {
                    return not_q(impl_p_q(p));
                };
            };
        },
        [&](Impl<Not<Q>, Not<P>> impl_not_q_not_p) -> Impl<P, Q> {
            return [&](P p) -> Q {
                return Or<Q, Not<Q>>().elim(
                    [&](Q q) -> Q {
                        return q;
                    },
                    [&](Not<Q> not_q) -> Q {
                        return impl_not_q_not_p(not_q)(p).explode<Q>();
                    }
                );
            };
        }
    };
}

int main() {
    solve();
}
```

### 4.2. ド・モルガンの法則

$\lnot(P\land Q)\leftrightarrow\lnot P\lor\lnot Q$
```cpp
using P = Prop<0>;
using Q = Prop<1>;

consteval Equiv<Not<And<P, Q>>, Or<Not<P>, Not<Q>>> solve() {
    return {
        [&](Not<And<P, Q>> not_and_p_q) -> Or<Not<P>, Not<Q>> {
            return Or<P, Not<P>>().elim(
                [&](P p) -> Or<Not<P>, Not<Q>> {
                    return Not<Q>([&](Q q) -> False {
                        return not_and_p_q({p, q});
                    });
                },
                [&](Not<P> not_p) -> Or<Not<P>, Not<Q>> {
                    return not_p;
                }
            );
        },
        [&](Or<Not<P>, Not<Q>> or_not_p_not_q) -> Not<And<P, Q>> {
            return [&](And<P, Q> and_p_q) -> False {
                return or_not_p_not_q.elim(
                    [&](Not<P> not_p) -> False {
                        return not_p(and_p_q.left);
                    },
                    [&](Not<Q> not_q) -> False {
                        return not_q(and_p_q.right);
                    }
                );
            };
        }
    };
}

int main() {
    solve();
}
```

$\lnot(P\lor Q)\leftrightarrow\lnot P\land\lnot Q$
```cpp
using P = Prop<0>;
using Q = Prop<1>;

consteval Equiv<Not<Or<P, Q>>, And<Not<P>, Not<Q>>> solve() {
    return {
        [&](Not<Or<P, Q>> not_or_p_q) -> And<Not<P>, Not<Q>> {
            return {
                [&](P p) -> False {
                    return not_or_p_q(p);
                },
                [&](Q q) -> False {
                    return not_or_p_q(q);
                }
            };
        },
        [&](And<Not<P>, Not<Q>> and_not_p_not_q) -> Not<Or<P, Q>> {
            return [&](Or<P, Q> or_p_q) -> False {
                return or_p_q.elim(
                    [&](P p) -> False {
                        return and_not_p_not_q.left(p);
                    },
                    [&](Q q) -> False {
                        return and_not_p_not_q.right(q);
                    }
                );
            };
        }
    };
}

int main() {
    solve();
}
```

### 4.3. 二重否定の導入・除去

$P\leftrightarrow\lnot\lnot P$
```cpp
using P = Prop<0>;

consteval Equiv<P, Not<Not<P>>> solve() {
    return {
        [&](P p) -> Not<Not<P>> {
            return [&](Not<P> not_p) -> False {
                return not_p(p);
            };
        },
        [&](Not<Not<P>> not_not_p) -> P {
            return Or<P, Not<P>>().elim(
                [&](P p) -> P {
                    return p;
                },
                [&](Not<P> not_p) -> P {
                    return not_not_p(not_p).explode<P>();
                }
            );
        }
    };
}

int main() {
    solve();
}
```

### 4.4. 同一律

$P\leftrightarrow P$
```cpp
using P = Prop<0>;

consteval Equiv<P, P> solve() {
    return {
        [&](P p) -> P {
            return p;
        },
        [&](P p) -> P {
            return p;
        }
    };
}

int main() {
    solve();
}
```

## 5. 述語論理の公理・推論規則

述語論理では、命題論理の記号に加えて新たに量化記号 $\forall,\exist$ を導入し、次の推論規則を追加します。

- **$\forall$ 導入則**
$$\frac{\begin{array}{c}P(a/x)\end{array}}{\forall xP}$$
- **$\forall$ 除去則**
$$\frac{\begin{array}{c}\forall xP\end{array}}{P(t / x)}$$
- **$\exist$ 導入則**
$$\frac{\begin{array}{c}P(t/x)\end{array}}{\exist xP}$$
- **$\exist$ 除去則**
$$\frac{\begin{array}{cc} & P(a/x) \\ & \vdots \\ \exist xP & Q \end{array}}{Q}$$

$\forall$ 導入則および $\exist$ 除去則中の $a$ は自由変数です。

## 6. 述語論理の実装

### 6.1. メタ関数

論理記号の実装に入る前に、変数への代入を行うためのメタ関数を定義します。数学では単に $P(a/x)$ と書けばよいのですが、プログラミングでは命題 `P` の述語としての構造に現れるすべての `x` を洗い出して `a` に置き換えなければなりません。以下では、`ReplaceType` というメタ関数を定義し、`ReplaceType<T, U, V>` でクラス `T` の構造中に現れる `U` を `V` に置き換えたクラスを得ることを目指します。例えば、`T` が
```cpp
Hoge<U, Fuga<V, U>, W>
```
であれば
`ReplaceType<T, U, V>` は
```cpp
Hoge<V, Fuga<V, V>, W>
```
となります。  

大前提として、クラス `T` には元々のテンプレートがどういう構造だったかという情報が全く含まれていないので、まずは `T` 自体にそれらの情報を埋め込む必要があります。具体的には、すべての命題クラスに対して元々のテンプレートをメンバエイリアステンプレート `Template` に格納し、テンプレート引数の一覧を `std::tuple` の形でメンバエイリアス `TemplateArgs` に格納にします。  
`And` クラステンプレートを例にとると次のようになります。

```cpp
template <PropType P, PropType Q>
class And final : PropBase {
public:
    template <PropType T, PropType U>
    using Template = And<T, U>;
    using TemplateArgs = std::tuple<P, Q>;

    // 以下略
};
```
テンプレートではないクラスについても、整合性をとるために引数が空のエイリアステンプレートと `std::tuple` を用意します。`False` クラスを例にとると次のようになります。
```cpp
class False final : PropBase {
public:
    template <class _ = void>
    using Template = False;
    using TemplateArgs = std::tuple<>;

    // 以下略
};
```


この前提の下で、`ReplaceType<T, U, V>` は次のようなメタ関数として実装できます。
- `T` が `U` に一致しているならば `V` を返す。
- そうでなければ、`T::TemplateArgs` が `std::tuple<Args...>` に一致するとして、`T::Template<ReplaceType<Args[0], U, V>, ReplaceType<Args[1], U, V>, ...>` を返す。


言葉で説明しても分かりにくいと思うので実装を載せます。
```cpp
template <class T, class U, class V>
class ReplaceTypeImplementation;

template <template <class...> class Template, class TemplateArgs, class U, class V>
class ReplaceTypeHelper {};

template <template <class...> class Template, class U, class V, class... Args>
class ReplaceTypeHelper<Template, std::tuple<Args...>, U, V> {
public:
    using result = Template<typename ReplaceTypeImplementation<Args, U, V>::result...>;
};

template <class T, class U, class V>
class ReplaceTypeImplementation {
public:
    using result =
        std::conditional_t<
            std::same_as<T, U>,
            V,
            typename ReplaceTypeHelper<T::template Template, typename T::TemplateArgs, U, V>::result
        >;
};

template <class T, class U, class V>
using ReplaceType = typename ReplaceTypeImplementation<T, U, V>::result;
```
複雑そうに見えますがやっていることは単純です。端的に言えばテンプレートの特殊化を利用して `T::TemplateArgs` を `std::tuple<Args...>` にマッチさせ、`T::Template` の引数に `ReplaceType<Args, U, V>...` を代入しています。それを行っているのがヘルパー関数 `ReplaceTypeHelper` で、`ReplaceTypeImplementation` 本体では `T` が `U` に一致するか確認した後、残りの処理を `ReplaceTypeHelper` に投げています。  
再帰の終了条件については、`T` が `U` に一致しているとき `V` が返って終了するのはもちろん、`T` のテンプレート引数が空であるときも `T` 自身が返ることを容易に確認できます。

### 6.2. 基底クラス

命題クラスの基底クラスは命題論理のもの (3.2.節参照) と同じなので省略します。述語論理では、新たに変数クラスの基底クラスを次のように実装します。

```cpp
class VarBase {};

template <class T>
concept VarType = std::is_base_of_v<VarBase, T>;

class BoundVarBase : VarBase {};

template <class T>
concept BoundVarType = std::is_base_of_v<BoundVarBase, T>;

class FreeVarBase : VarBase {};

template <class T>
concept FreeVarType = std::is_base_of_v<FreeVarBase, T>;
```



```cpp
template <class T>
concept TakeSome = FreeVarType<T>;

template <class T>
concept TakeAny = FreeVarType<T>;
```


## 7. 述語論理の証明例

## 8. 実装の全体像

完成した述語論理の形式的証明ツールを以下に示します。

## 9. 参考資料


## 10. アピールポイント