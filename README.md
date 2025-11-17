# qtlinq

A **header-only LINQ-style query library for Qt**, designed specifically for `QList<T>`.  
It provides a fluent, expressive API similar to C# LINQ, while staying lightweight and dependency-free (besides Qt itself).

```cpp
#include <qtlinq/qtlinq.h>

QList<int> xs{1, 2, 3, 4, 5, 6};

auto evensTimes10 = qlinq::from(xs)
    .where([](int x){ return x % 2 == 0; })
    .select([](int x){ return x * 10; })
    .toList();
// { 20, 40, 60 }
```

## ✨ Features

- Header-only (`#include <qtlinq/qtlinq.h>`)
- Works directly with `QList<T>`
- Fluent LINQ-like API
- Strongly typed, safe, exception-aware
- Supports filtering, mapping, flattening, ordering, aggregations…
- Supports polymorphic `ofType<T>()`
- Range-based for loop compatible
- Fully unit-tested using Qt Test framework

## 🚀 Getting Started

### Add to your project

```cpp
#include <qtlinq/qtlinq.h>
```

That's it. The entire library lives in a single header.

## 📚 Basic Usage

### Filtering + Projection

```cpp
auto values = qlinq::from(xs)
    .where([](int x){ return x > 10; })
    .select([](int x){ return QString("Value=%1").arg(x); })
    .toList();
```

### Flattening (`selectMany`)

```cpp
QList<QList<int>> nested{{1,2},{3},{4,5}};

auto flat = qlinq::from(nested)
    .selectMany([](const QList<int>& l){ return l; })
    .toList();
// {1,2,3,4,5}
```

### Ordering

```cpp
auto sorted = qlinq::from(people)
    .orderBy([](const Person& p){ return p.age; })
    .toList();
```

### First / FirstOrDefault

```cpp
auto v = qlinq::from(xs).first();                 // throws if empty
auto o = qlinq::from(xs).firstOrDefault();        // std::optional
auto f = qlinq::from(xs).first([](int x){return x % 2 == 0;});
```

### Min / Max (element)

```cpp
auto minElement = qlinq::from(xs).min();
auto maxElement = qlinq::from(xs).max();
```

### Min / Max (selector → returns selector value)

```cpp
auto maxAge = qlinq::from(people).max([](auto& p){ return p.age; });
```

### Polymorphic Type Filtering

```cpp
auto dogs = qlinq::from(animals).ofType<Dog*>();
```

### Range-based for loop

```cpp
for (int x : qlinq::from(xs).where([](int n){ return n % 2 == 0; })) {
    qDebug() << x;
}
```

## 🧩 API Overview

| Operation | Description |
|----------|-------------|
| `where(pred)` | Filter items |
| `select(func)` | Projection |
| `selectMany(func)` | Flatten sequences |
| `take(n)` / `skip(n)` | Slice sequence |
| `orderBy(key)` / `orderByDescending(key)` | Sort |
| `any(pred)` / `all(pred)` | Quantifiers |
| `first()` / `first(pred)` | Return first element, throw if not found |
| `firstOrDefault()` | Return optional element |
| `min()` / `max()` | Extremes (elements) |
| `min(selector)` / `max(selector)` | Selector-based extremes (returns key) |
| `distinct()` | Unique values |
| `reverse()` | Reverse order |
| `sum(selector)` | Sum projections |
| `count()` / `count(value)` | Counting |
| `aggregate()` | General fold |
| `for_each(func)` | Apply function |
| `ofType<U>()` | Dynamic type filtering |

## 🧪 Running Tests

```bash
mkdir build && cd build
cmake .. -DQTLINQ_BUILD_TESTS=ON
cmake --build .
ctest
```

## 🤝 Contributing

Issues and PRs are welcomed.

## 📄 License

See `LICENSE.md`.