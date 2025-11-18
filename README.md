# qtlinq — LINQ for Qt (Header‑Only, C++17)

![License](https://img.shields.io/badge/License-GPL-blue.svg)
![C++](https://img.shields.io/badge/C%2B%2B-17-orange.svg)

A **header-only LINQ-style query library for Qt**, designed for `QList<T>`, providing a clean, fluent, expressive C#-style querying API in modern **C++17**.

## ✨ Features

- **Header-only**, single include  
- **Qt-friendly (`QList<T>`)**  
- **C++17 compatible**  
- **LINQ-like fluent API**  
- **Stable ordering for all operations**  
- **Automatic multi-strategy `distinct()`**  
- **Polymorphic filtering (`ofType<T>()`)**  
- **Range-based iteration**  
- **Optional-safe first element APIs**  
- **QtTest unit tests**  
- **Benchmark suite with huge datasets**

## 🚀 Quick Example

```cpp
#include <qtlinq/qtlinq.h>

QList<int> xs{1,2,3,4,5,6};

auto result =
    qlinq::from(xs)
        .where([](int x){ return x % 2 == 0; })
        .select([](int x){ return x * 10; })
        .toList();
// → { 20, 40, 60 }
```

## 📚 API Summary

### Querying
- `where(pred)`
- `select(func)`
- `selectMany(func)`
- `take(n)`
- `skip(n)`
- `reverse()`
- `distinct()`

### Ordering
- `orderBy(key)`
- `orderByDescending(key)`

### First elements
- `first()`
- `first(pred)`
- `firstOrDefault()`
- `firstOrDefault(pred)`

### Aggregation
- `any(pred)`
- `all(pred)`
- `count()` / `count(value)`
- `min()` / `max()`
- `min(selector)` / `max(selector)`
- `minBy(selector)` / `maxBy(selector)`
- `sum(selector)`
- `aggregate(seed, func)`

### Polymorphic type filtering
- `ofType<U>()`

## 🛠 Installation (CMake)

```cmake
target_include_directories(your_target PRIVATE path/to/qtlinq/include)
```

## 🤝 Contributing

Pull requests and issues are welcome.  
Open an issue before large changes.

## 📄 License

Project uses GPLv3 License.