# C++で形式的証明

## 目次

1. はじめに
2. 命題論理の公理・推論規則
3. 命題論理の実装
4. 命題論理の証明例
5. 述語論理の公理・推論規則
6. 述語論理の実装
7. 述語論理の証明例
8. 実装の全体像
9. 参考資料

## 1. はじめに

本稿では述語論理の形式的証明ツールをC\+\+で実装します。形式的証明のコンパイラをC\+\+で書くという意味ではなく、C\+\+の言語機能をそのまま形式的証明の文法に利用し、C\+\+のコードとして形式的証明をコンパイルすることを目指します。正しい証明に対してコンパイルが通るようにすることはもちろん、（常識的な）間違いを含む証明に対してコンパイルエラーを出すことも目標にします。

動作確認はすべてg\+\+で行っています。他の環境で動くかは確かめていません。  
実際にコードを動かすときは次のコンパイルオプションを付けてください。
```powershell
-fconstexpr-depth=2147483647
-fconstexpr-loop-limit=2147483647
-fconstexpr-ops-limit=2147483647
-fsyntax-only
-std=c++23
```

自力を証明を書く場合は次のオプションも付けた方がよいです。
```powershell
-Wmissing-template-keyword
-Wreturn-type
-fdiagnostics-color=always
```

## 2. 命題論理の公理・推論規則

まずは述語論理の土台である命題論理を構築します。今回は記号として $\bot,\lnot,\land,\lor,\rightarrow,\leftrightarrow$ を使うこととし、公理および推論規則として以下を採用します。

- 公理
    - **排中律**
    $$A\lor\lnot A$$
- 推論規則
    - **爆発律**
    $$\bot\vdash A$$
    - **$\lnot$ 導入則**
    $$[A]\cdots\bot\vdash\lnot A$$
    - **$\lnot$ 除去則**
    $$A,\lnot A\vdash$$
    - **$\land$ 導入則**
    $$A, B \vdash A\land B$$
    - **$\land$ 除去則**
    $$A\land B\vdash A$$
    $$A\land B\vdash B$$
    - **$\lor$ 導入則**
    $$A\vdash A\lor B$$
    $$B\vdash A\lor B$$
    - **$\lor$ 除去則**
    $$A\lor B,[A]\cdots C,[B]\cdots C\vdash C$$
    - **$\rightarrow$ 導入則**
    $$[A]\cdots B\vdash A\rightarrow B$$
    - **$\rightarrow$ 除去則**
    $$A,A\rightarrow B\vdash B$$
    - **$\leftrightarrow$ 導入則**
    $$[A]\cdots B,[B]\cdots A\vdash A\leftrightarrow B$$
    - **$\leftrightarrow$ 除去則**
    $$A,A\leftrightarrow B\vdash B$$
    $$B,A\leftrightarrow B\vdash A$$
※ $[X]\cdots Y$ で $X$ を仮定すれば $Y$ を導出できることを表します。

## 3. 命題論理の実装

### 3.1. 基本方針

プログラミング上で数学的な証明を取り扱うために、次のような対応関係を考えます。
| 数学 | プログラミング |
| ----- | ----- |
| 命題 | クラス |
| 証明 | オブジェクト |
| 証明する | クラスのコンストラクタを呼ぶ |
| 仮定する | 仮定に対応する命題クラスを引数とするラムダ式を定義する |
| 仮定付きで証明する | ラムダ式から目的の命題を返す |
| 証明した命題を使う | オブジェクトからメンバ関数を呼ぶ |

※ 表のような対応関係をカリー・ハワード同型対応と呼ぶらしいです。

以上の議論を踏まえると、論理記号 $\bot, \lnot,\land,\lor,\rightarrow,\leftrightarrow$ の実装方針として次が立ちます。

| 論理記号 | コンストラクタ | メンバ |
| ------ | ----- | ----- |
| $\bot$ | なし | 任意の命題を返す（**爆発律**） |
| $\lnot A$ | $A$ を受け取って $\bot$ を返すラムダ式を要求（**$\lnot$ 導入則**） | $A$ を受け取って $\bot$ を返す（**$\lnot$ 除去則**） |
| $A\land B$ | $A,B$ の両方を要求（**$\land$ 導入則**） | $A, B$ をそれぞれ返す（**$\land$ 除去則**） |
| $A\lor B$ | $A,B$ の一方を要求（**$\lor$ 導入則**） | 「$A$ を受け取って $C$ を返すラムダ式」「$B$ を受け取って $C$ を返すラムダ式」の2つを受け取って $C$ を返す（**$\lor$ 除去則**） |
| $A\rightarrow B$ | $A$ を受け取って $B$ を返すラムダ式を要求（**$\rightarrow$ 導入則**） | $A$ を受け取って $B$ を返す（**$\rightarrow$ 除去則**） | 
| $A\leftrightarrow B$ | 「$A$ を受け取って $B$ を返すラムダ式」と「$B$ を受け取って $A$ を返すラムダ式」の2つを要求（**$\leftrightarrow$ 導入則**） | $A, B$ の一方を受け取って他方を返す（**$\leftrightarrow$ 除去則**） | 

$A\lor\lnot A$ および $\lnot A\lor A$ はデフォルトコンストラクタを持つものとします。（**排中律**）  
命題変数 $A,B,C,\ldots$ は コンストラクタを持たないクラスとして実装します。したがって $\bot$ 以外の論理記号はクラステンプレートとなります。

### 3.2. 基底クラス

前節で述べたように、命題クラスのコンストラクタは公理・推論規則と整合しなければなりません。一方で、除去則を実装するからには、命題クラスどうしでオブジェクトを自由にやりとりできる必要があります。そこで、すべての命題クラスの基底クラス `PropBase` を定義し、その中で命題クラスのオブジェクトを一括管理することにします。

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

※ `PropType` は命題クラスを表すコンセプトです。`std::is_base_of_v` を用いて `PropBase` の派生クラスであるか判定しています。

※ `PropBase()` に付いている `consteval` はコンパイル時評価を強制するための修飾子です。`constexpr` と異なり、コンパイル時評価できなかった場合にはエラーが出ます。本稿の形式的証明ツールはコンパイルのみで完結するので、すべての関数に例外なく `consteval` を付けています。

### 3.3. `False` の実装

```cpp
class False final : PropBase {
public:
    consteval False(const False& other) {
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

    template <PropType P>
    consteval P explode() const { return PropBase::object<P>; }

private:
    friend class PropBase;
    consteval False() : initialized(true) {}

    bool initialized = false;
};
```

`False` クラスは論理記号 $\bot$ に対応します。

最初に書かれている `False(const False& other)` はコピーコンストラクタです。命題は一度証明されてしまえばそれまでなので、一度生成できたオブジェクトは自由にコピーできるとしてよいです。  
コピーコンストラクタ内部で書かれている処理は不正な初期化を防ぐためのものです。これがないと、例えば
```
False fal(fal);
```
のような初期化（自己初期化）が通ってしまいます。  
コピーコンストラクタの実装はすべての命題クラスで共通なので、以降説明を省略します。

メンバ関数 `explode` は爆発律を実装したものです。一度 `False` のオブジェクトを作ることができれば、`explode` を使ってあらゆる命題を証明できます。

`private:` セクション内に書かれている `friend` 宣言とデフォルトコンストラクタは、前節で説明した通り `PropBase` 内で `False` のオブジェクトを作れるようにしたものです。この部分の実装もすべての命題クラスで共通なので、以降説明を省略します。

### 3.4 `Not` の実装

```cpp
template <PropType P>
class Not final : PropBase {
public:
    consteval Not(auto f) {
        False q = f(PropBase::object<P>);
        initialized = true;
    }
    consteval Not(const Not& other) {
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

    consteval False elim(P) const { return PropBase::object<False>; }
    consteval False operator()(P p) const { return elim(p); }

private:
    friend class PropBase;
    consteval Not() : initialized(true) {}

    bool initialized = false;
};
```

`Not` テンプレートは論理記号 $\lnot$ に対応します。

コンストラクタ `Not(auto f)` は $\lnot$ 導入則を表しています。ラムダ式 `f` に `P` のオブジェクトを渡して `False` のオブジェクトが返ってくるか検証します。  
※ `consteval` 修飾子の効果によりラムダ式 `f` は定数式として実行されます。定数式として実行できるのは副作用のない関数だけなので、引数として渡した `P` クラスのオブジェクトが不正に使われる心配はありません。

メンバ関数 `elim` は $\lnot$ 除去則を実装したものです。 `P` クラスのオブジェクトを受け取って `False` クラスのオブジェクトを返します。
`operator()` は `elim` と同等の効果です。これにより `Not` は `P` を受け取って `False` を返すラムダ式と同じように使えます。
 
### 3.5 `And` の実装

```cpp
template <PropType P, PropType Q>
class And final : PropBase {
public:
    consteval And(P, Q) : initialized(true) {}
    consteval And(const And& other) {
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

    const P left = PropBase::object<P>;
    const Q right = PropBase::object<Q>;

private:
    friend class PropBase;
    consteval And() : initialized(true) {}

    bool initialized = false;
};
```

`And` テンプレートは論理記号 $\land$ に対応します。

コンストラクタ `And(P, Q)` は $\land$ 導入則を表しています。`P`, `Q` のオブジェクトから `And<P, Q>` のオブジェクトを生成できます。

メンバ変数 `left` `right`は  $\land$ 除去則に対応します。

### 3.6 `Or` の実装

```cpp
template <PropType P, PropType Q>
class Or final : PropBase {
public:
    consteval Or() requires (std::same_as<Q, Not<P>> || std::same_as<P, Not<Q>>) : initialized(true) {}
    consteval Or(P) : initialized(true) {}
    consteval Or(Q) requires (!std::same_as<P, Q>) : initialized(true) {}
    consteval Or(const Or& other) {
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

    consteval auto elim(auto f, auto g) const {
        auto rf(f(PropBase::object<P>));
        auto rg(g(PropBase::object<Q>));
        if (!std::same_as<decltype(rf), decltype(rg)>) throw;
        return PropBase::object<decltype(rf)>;
    }

private:
    friend class PropBase;
    consteval Or() : initialized(true) {}

    bool initialized = false;
};
```

`Or` テンプレートは論理記号 $\lor$ に対応します。

デフォルトコンストラクタは排中律を表しています。`Q` と `Not<P>` がクラスとして等しいとき、または `P` と `Not<Q>` がクラスとして等しいとき、デフォルトコンストラクタを呼び出すことができます。

2, 3番目のコンストラクタにより、`P` または `Q` のオブジェクトから `Or<P, Q>` のオブジェクトを生成できます。これは $\lor$ 導入則に対応します。

メンバ関数 `elim` は $\lor$ 除去則を実装したものです。2つのラムダ式 `f`, `g` にそれぞれ `P`, `Q` のオブジェクトを渡して同じクラスのオブジェクトが返ってくるか検証します。戻り値としてそのオブジェクトを返します。

### 3.7 `Impl` の実装

```cpp
template <PropType P, PropType Q>
class Impl final : PropBase {
public:
    consteval Impl(auto f) {
        Q q = f(PropBase::object<P>);
        initialized = true;
    }
    consteval Impl(const Impl& other) {
        if (!other.initialized) throw; // prevent illegal initialization
        initialized = true;
    }

    consteval Q operator()(P) const { return PropBase::object<Q>; }

private:
    friend class PropBase;
    consteval Impl() : initialized(true) {}

    bool initialized = false;
};
```

`Impl` テンプレートは論理記号 $\rightarrow$ に対応します。

コンストラクタ `Impl(auto f)` は $\rightarrow$ 導入則を表しています。ラムダ式 `f` に `P` のオブジェクトを渡して `Q` のオブジェクトが返ってくるか検証します。




## 4. 述語論理の推論


## 5. 述語論理の実装


## 6. まとめ

