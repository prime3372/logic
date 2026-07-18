# C++で形式的証明

## 1. はじめに

本稿では述語論理の形式的証明ツールをC\+\+で実装します。形式的証明のコンパイラをC\+\+で書くという意味ではなく、C\+\+の言語機能をそのまま形式的証明の文法に利用し、C\+\+のコードとして形式的証明をコンパイルすることを目指します。正しい証明に対してコンパイルが通るようにすることはもちろん、常識的な範囲の間違いを含む証明に対してコンパイルエラーを出すことも目標にします。

動作確認はすべてg\+\+で行っています。他の環境で動くかは確かめていません。  
実際にコードを動かすときは次のコンパイルオプションを付けてください。
```powershell
-fsyntax-only
-std=c++23
```
- -fsyntax-only：構文チェックのみを行うオプションです。実行ファイルが出力されなくなります。
- -std=c\+\+23：バージョンとしてC\+\+23を指定します。

## 2. 命題論理の公理・推論規則

まずは述語論理の土台である命題論理を構築します。今回は記号として $\bot,\lnot,\land,\lor,\rightarrow,\leftrightarrow$ を使うこととし、公理および推論規則として以下を採用します。

- 公理
    - **排中律**
    $$P\lor\lnot P$$
- 推論規則
    - **爆発律**
    $$\bot\vdash P$$
    - **$\lnot$ 導入則**
    $$[P]\cdots\bot\vdash\lnot P$$
    - **$\lnot$ 除去則**
    $$P,\lnot P\vdash\bot$$
    - **$\land$ 導入則**
    $$P, Q \vdash P\land Q$$
    - **$\land$ 除去則**
    $$P\land Q\vdash P$$
    $$P\land Q\vdash Q$$
    - **$\lor$ 導入則**
    $$P\vdash P\lor Q$$
    $$Q\vdash P\lor Q$$
    - **$\lor$ 除去則**
    $$P\lor Q,[P]\cdots R,[Q]\cdots R\vdash R$$
    - **$\rightarrow$ 導入則**
    $$[P]\cdots Q\vdash P\rightarrow Q$$
    - **$\rightarrow$ 除去則**
    $$P,P\rightarrow Q\vdash Q$$
    - **$\leftrightarrow$ 導入則**
    $$[P]\cdots Q,[Q]\cdots P\vdash P\leftrightarrow Q$$
    - **$\leftrightarrow$ 除去則**
    $$P,P\leftrightarrow Q\vdash Q$$
    $$Q,P\leftrightarrow Q\vdash P$$
※ $[X]\cdots Y$ で $X$ を仮定すれば $Y$ を導出できることを表します。

## 3. 命題論理の実装

### 3.1. 基本方針

プログラミング上で数学的な証明を取り扱うために、次のような対応関係を考えます。
| 数学 | プログラミング |
| ----- | ----- |
| 命題 | クラス |
| 証明 | オブジェクト |
| 証明する | クラスのコンストラクタを呼ぶ |
| 証明した命題を使う | オブジェクトからメンバ関数を呼ぶ |
| 仮定する | 仮定を引数として受け取るラムダ式を定義する |
| 仮定付きで証明する | 仮定を引数として受け取るラムダ式から目的の命題を返す |

この対応関係の下で、論理記号 $\bot, \lnot,\land,\lor,\rightarrow,\leftrightarrow$ を以下のように実装することにします。

| 論理記号 | コンストラクタ | メンバ |
| ------ | ----- | ----- |
| $\bot$ | なし | 任意の命題を返す（**爆発律**） |
| $\lnot A$ | $A$ を受け取って $\bot$ を返すラムダ式を要求（**$\lnot$ 導入則**） | $A$ を受け取って $\bot$ を返す（**$\lnot$ 除去則**） |
| $A\land B$ | $A,B$ を両方とも要求（**$\land$ 導入則**） | $A, B$ をそれぞれ返す（**$\land$ 除去則**） |
| $A\lor B$ | $A$ または $B$ の一方を要求（**$\lor$ 導入則**） | 「$A$ を受け取って $C$ を返すラムダ式」「$B$ を受け取って $C$ を返すラムダ式」の2つを受け取って $C$ を返す（**$\lor$ 除去則**） |
| $A\rightarrow B$ | $A$ を受け取って $B$ を返すラムダ式を要求（**$\rightarrow$ 導入則**） | $A$ を受け取って $B$ を返す（**$\rightarrow$ 除去則**） | 
| $A\leftrightarrow B$ | 「$A$ を受け取って $B$ を返すラムダ式」「$B$ を受け取って $A$ を返すラムダ式」の2つを要求（**$\leftrightarrow$ 導入則**） | $A, B$ の一方を受け取って他方を返す（**$\leftrightarrow$ 除去則**） | 

$A\lor\lnot A$ および $\lnot A\lor A$ はデフォルトコンストラクタを持つものとします。（**排中律**）  
命題変数 $A,B,C,\ldots$ もクラスとして実装します。したがって $\bot$ 以外の論理記号はクラステンプレートとなります。

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
    consteval P explode() const { return PropBase::object<P>; }

private:
    friend class PropBase;
    consteval False() : initialized(true) {}
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
        initialized = true;
    }

    consteval False operator()(P) const { return PropBase::object<False>; }

private:
    friend class PropBase;
    consteval Not() : initialized(true) {}
};
```

`Not` クラステンプレートは論理記号 $\lnot$ に対応します。

コンストラクタ `Not(auto f)` は $\lnot$ 導入則を表しています。ラムダ式 `f` に `P` クラスのオブジェクトを渡して `False` クラスのオブジェクトが返ってくるか検証します。  

`operator()` は $\lnot$ 除去則を表しています。 `P` クラスのオブジェクトを受け取って `False` クラスのオブジェクトを返します。
 
※ `Not<P>` の機能は `Impl<P, False>` と本質的に同じなので、エイリアステンプレートを用いて
```cpp
template <class P>
using Not = Impl<P, False>
```
としても `Not` の実装として十分です。`Impl` の実装については3.7節を参照してください。

### 3.5. `And`

```cpp
template <PropType P, PropType Q>
class And final : PropBase {
public:
    consteval And(P, Q) : initialized(true) {}

    const P left = PropBase::object<P>;
    const Q right = PropBase::object<Q>;

private:
    friend class PropBase;
    consteval And() : initialized(true) {}
};
```

`And` クラステンプレートは論理記号 $\land$ に対応します。

コンストラクタ `And(P, Q)` は $\land$ 導入則を表しています。`P`, `Q` のオブジェクトから `And<P, Q>` のオブジェクトを構築できます。

メンバ変数 `left`, `right`は  $\land$ 除去則を表しています。

### 3.6. `Or`

```cpp
template <PropType P, PropType Q>
class Or final : PropBase {
public:
    consteval Or() requires (std::same_as<Q, Not<P>> || std::same_as<P, Not<Q>>) : initialized(true) {}
    consteval Or(P) : initialized(true) {}
    consteval Or(Q) requires (!std::same_as<P, Q>) : initialized(true) {}

    consteval auto elim(auto f, auto g) const {
        auto rf(f(PropBase::object<P>));
        auto rg(g(PropBase::object<Q>));
        static_assert(std::same_as<decltype(rf), decltype(rg)>);
        return PropBase::object<decltype(rf)>;
    }

private:
    friend class PropBase;
    consteval Or() : initialized(true) {}
};
```

`Or` クラステンプレートは論理記号 $\lor$ に対応します。

デフォルトコンストラクタは排中律を表しています。`Q` と `Not<P>` がクラスとして等しいとき、または `P` と `Not<Q>` がクラスとして等しいとき、デフォルトコンストラクタを呼び出すことができます。

2, 3番目のコンストラクタは $\lor$ 導入則を表しています。`P` または `Q` のオブジェクトから `Or<P, Q>` のオブジェクトを構築できます。

メンバ関数 `elim` は $\lor$ 除去則を表しています。2つのラムダ式 `f`, `g` にそれぞれ `P`, `Q` のオブジェクトを渡して同じクラスのオブジェクトが返ってくるか検証します。戻り値としてそのオブジェクトを返します。

### 3.7. `Impl`

```cpp
template <PropType P, PropType Q>
class Impl final : PropBase {
public:
    consteval Impl(auto f) {
        Q q = f(PropBase::object<P>);
        initialized = true;
    }

    consteval Q operator()(P) const { return PropBase::object<Q>; }

private:
    friend class PropBase;
    consteval Impl() : initialized(true) {}
};
```

`Impl` クラステンプレートは論理記号 $\rightarrow$ に対応します。

コンストラクタ `Impl(auto f)` は $\rightarrow$ 導入則を表しています。ラムダ式 `f` に `P` のオブジェクトを渡して `Q` のオブジェクトが返ってくるか検証します。

`operator()` は $\rightarrow$ 導入則を表しています。`P` のオブジェクトを受け取って `Q` のオブジェクトを返します。

### 3.8. `Equiv`

```cpp
template <PropType P, PropType Q>
class Equiv final : PropBase {
public:
    consteval Equiv(auto f, auto g) {
        Q q = f(PropBase::object<P>);
        P p = g(PropBase::object<Q>);
        initialized = true;
    }

    consteval Q operator()(P) const { return PropBase::object<Q>; }

    consteval P operator()(Q) const requires (!std::same_as<P, Q>) {
        return PropBase::object<P>;
    }

private:
    friend class PropBase;
    consteval Equiv() : initialized(true) {}
};
```

`Impl` クラステンプレートは論理記号 $\leftrightarrow$ に対応します。

コンストラクタ `Impl(auto f)` は $\leftrightarrow$ 導入則を表しています。ラムダ式 `f` に `P` のオブジェクトを渡して `Q` のオブジェクトが返ってくるか、ラムダ式 `g` に `Q` のオブジェクトを渡して `P` のオブジェクトが返ってくるか検証します。

`operator()` は $\leftrightarrow$ 導入則を表しています。`P`, `Q` のオブジェクトのうち一方を受け取って他方を返します。

### 3.9. 補足

上で挙げた実装ではコピーコンストラクタをデフォルト定義していましたが、実はこのままだと問題が生じます。例えば次のような初期化がコンパイルを通過します。
```
False fal(fal);
```
上のような自己初期化はC\+\+では定義された動作であり、初期化されていない `fal` を `fal` にコピーする挙動になります。こういった不正な初期化を防ぐには、コピーコンストラクタ内に　`fal` のメンバ変数を読み取る処理を加えればよいです。
```cpp
class False final : PropBase {
public:
    consteval False(const False& other) {
        initialized = other.initialized;
    }

    template <PropType P>
    consteval P explode() const { return PropBase::object<P>; }

private:
    friend class PropBase;
    consteval False() : initialized(true) {}

    bool initialized;
};
```
`other` が正しく初期化されていない場合、コンパイラが未初期化の `other.initialized` を読み取ったことを検知してエラーを出してくれます。

## 4. 命題論理の証明例

### 4.1. 対偶律

$(P\rightarrow Q)\leftrightarrow(\lnot Q\rightarrow\lnot P)$
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
                    [&](Q q) -> Q { return q; },
                    [&](Not<Q> not_q) -> Q { return impl_not_q_not_p(not_q)(p).explode<Q>(); }
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
                [&](Not<P> not_p) -> Or<Not<P>, Not<Q>> { return not_p; }
            );
        },
        [&](Or<Not<P>, Not<Q>> or_not_p_not_q) -> Not<And<P, Q>> {
            return [&](And<P, Q> and_p_q) -> False {
                return or_not_p_not_q.elim(
                    [&](Not<P> not_p) -> False { return not_p(and_p_q.left); },
                    [&](Not<Q> not_q) -> False { return not_q(and_p_q.right); }
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
                [&](P p) -> False { return not_or_p_q(p); },
                [&](Q q) -> False { return not_or_p_q(q); }
            };
        },
        [&](And<Not<P>, Not<Q>> and_not_p_not_q) -> Not<Or<P, Q>> {
            return [&](Or<P, Q> or_p_q) -> False {
                return or_p_q.elim(
                    [&](P p) -> False { return and_not_p_not_q.left(p); },
                    [&](Q q) -> False { return and_not_p_not_q.right(q); }
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
                [&](P p) -> P { return p; },
                [&](Not<P> not_p) -> P { return not_not_p(not_p).explode<P>(); }
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
        [&](P p) -> P { return p; },
        [&](P p) -> P { return p; }
    };
}

int main() {
    solve();
}
```

## 5. 述語論理の推論


## 6. 述語論理の実装


## 7. 述語論理の証明例

## 8. 実装の全体像

完成した述語論理の形式的証明ツールを以下に示します。

## 9. 参考資料