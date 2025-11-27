// For license of this file, see <project-root-folder>/LICENSE.md.

#pragma once

#include <algorithm>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <QList>
#include <QSet>

namespace qlinq {

  template <typename T>
  class Query {
    public:
      using value_type = T;

      Query() = default;
      explicit Query(const QList<T>& list) : _data(list) {}
      explicit Query(QList<T>&& list) : _data(std::move(list)) {}

      // Factory.
      static Query<T> from(const QList<T>& list) {
        return Query<T>(list);
      }

      static Query<T> from(QList<T>&& list) {
        return Query<T>(std::move(list));
      }

      // Basic iteration.
      auto begin() {
        return _data.begin();
      }
      auto end() {
        return _data.end();
      }
      auto begin() const {
        return _data.begin();
      }
      auto end() const {
        return _data.end();
      }

      // Materialization.
      QList<T> toList() const {
        return _data;
      }

      int size() const {
        return _data.size();
      }
      int count() const {
        return _data.size();
      }
      bool isEmpty() const {
        return _data.isEmpty();
      }

      // ofType<U>: dynamic_cast filter.
      template <typename U>
      Query<U> ofType() const {
        static_assert(std::is_pointer<T>::value, "ofType<U>(): Source sequence type must be a pointer.");
        static_assert(std::is_pointer<U>::value, "ofType<U>(): Target sequence type must be a pointer.");

        using FromPointee = typename std::remove_pointer<T>::type;
        using ToPointee = typename std::remove_pointer<U>::type;

        static_assert(std::is_base_of<FromPointee, ToPointee>::value || std::is_base_of<ToPointee, FromPointee>::value,
                      "ofType<U>() requires convertible pointer types");

        QList<U> result;
        result.reserve(_data.size());

        for (auto ptr : _data) {
          if (auto casted = dynamic_cast<U>(ptr)) {
            result.append(casted);
          }
        }

        return Query<U>(std::move(result));
      }

      // where: filter items.
      template <typename Pred>
      Query<T> where(Pred pred) const {
        QList<T> result;
        result.reserve(_data.size());

        for (const auto& item : _data) {
          if (pred(item)) {
            result.append(item);
          }
        }

        return Query<T>(std::move(result));
      }

      // select: project items.
      template <typename Func>
      auto select(Func func) const -> Query<typename std::decay<decltype(func(std::declval<T>()))>::type> {
        using ResultType = typename std::decay<decltype(func(std::declval<T>()))>::type;

        QList<ResultType> result;
        result.reserve(_data.size());

        for (const auto& item : _data) {
          result.append(func(item));
        }

        return Query<ResultType>(std::move(result));
      }

      // selectMany: flatten projected sequences (optimized: 2-pass reserve)
      template <typename Func>
      auto selectMany(Func func) const -> Query<typename std::decay<decltype(*func(std::declval<T>()).begin())>::type> {
        using InnerList = decltype(func(std::declval<T>()));
        using InnerType = typename std::decay<decltype(*std::declval<InnerList>().begin())>::type;

        QList<InnerType> result;

        // Pass 1: total size
        int total = 0;
        for (const auto& item : _data) {
          const InnerList& inner = func(item);
          total += inner.size();
        }
        result.reserve(total);

        // Pass 2: append
        for (const auto& item : _data) {
          const InnerList& inner = func(item);
          for (const auto& x : inner) {
            result.append(x);
          }
        }

        return Query<InnerType>(std::move(result));
      }

      // take: first n items.
      Query<T> take(int n) const {
        if (n <= 0) {
          return Query<T>(QList<T>{});
        }
        if (n >= _data.size()) {
          return *this;
        }

        QList<T> result;
        result.reserve(n);

        auto it = _data.constBegin();
        auto end = it + n;
        for (; it != end; ++it) {
          result.append(*it);
        }

        return Query<T>(std::move(result));
      }

      // skip: skip first n items.
      Query<T> skip(int n) const {
        if (n <= 0) {
          return *this;
        }
        if (n >= _data.size()) {
          return Query<T>(QList<T>{});
        }

        QList<T> result;
        result.reserve(_data.size() - n);

        auto it = _data.constBegin() + n;
        for (; it != _data.constEnd(); ++it) {
          result.append(*it);
        }

        return Query<T>(std::move(result));
      }

      // orderBy: sort ascending.
      template <typename KeySelector>
      Query<T> orderBy(KeySelector keySelector) const {
        QList<T> result(_data); // direct ctor → faster than assignment
        std::sort(result.begin(), result.end(), [&](const T& a, const T& b) {
          return keySelector(a) < keySelector(b);
        });
        return Query<T>(std::move(result));
      }

      // orderByDescending: sort descending.
      template <typename KeySelector>
      Query<T> orderByDescending(KeySelector keySelector) const {
        QList<T> result(_data);
        std::sort(result.begin(), result.end(), [&](const T& a, const T& b) {
          return keySelector(a) > keySelector(b);
        });
        return Query<T>(std::move(result));
      }

      // any: does any element match?
      template <typename Pred>
      bool any(Pred pred) const {
        for (const auto& item : _data) {
          if (pred(item)) {
            return true;
          }
        }
        return false;
      }

      // all: do all elements match?
      template <typename Pred>
      bool all(Pred pred) const {
        for (const auto& item : _data) {
          if (!pred(item)) {
            return false;
          }
        }
        return true;
      }

      // min: return smallest element.
      std::optional<T> min() const {
        if (_data.isEmpty()) {
          return std::nullopt;
        }

        const T* best = &_data.first();
        for (const auto& item : _data) {
          if (item < *best) {
            best = &item;
          }
        }

        return *best;
      }

      // min(selector)
      template <typename KeySelector>
      auto min(KeySelector keySelector) const
        -> std::optional<typename std::decay<decltype(keySelector(std::declval<T>()))>::type> {
        using R = typename std::decay<decltype(keySelector(std::declval<T>()))>::type;

        if (_data.isEmpty()) {
          return std::nullopt;
        }

        std::optional<R> bestValue;
        for (const auto& item : _data) {
          R k = keySelector(item);
          if (!bestValue.has_value() || k < *bestValue) {
            bestValue = k;
          }
        }
        return bestValue;
      }

      // minBy
      template <typename KeySelector>
      std::optional<T> minBy(KeySelector keySelector) const {
        if (_data.isEmpty()) {
          return std::nullopt;
        }

        const T* best = &_data.first();
        auto bestKey = keySelector(*best);

        for (const auto& item : _data) {
          auto k = keySelector(item);
          if (k < bestKey) {
            best = &item;
            bestKey = k;
          }
        }
        return *best;
      }

      // max
      std::optional<T> max() const {
        if (_data.isEmpty()) {
          return std::nullopt;
        }

        const T* best = &_data.first();
        for (const auto& item : _data) {
          if (*best < item) {
            best = &item;
          }
        }

        return *best;
      }

      // max(selector)
      template <typename KeySelector>
      auto max(KeySelector keySelector) const
        -> std::optional<typename std::decay<decltype(keySelector(std::declval<T>()))>::type> {
        using R = typename std::decay<decltype(keySelector(std::declval<T>()))>::type;

        if (_data.isEmpty()) {
          return std::nullopt;
        }

        std::optional<R> bestValue;
        for (const auto& item : _data) {
          R k = keySelector(item);
          if (!bestValue.has_value() || *bestValue < k) {
            bestValue = k;
          }
        }
        return bestValue;
      }

      // maxBy
      template <typename KeySelector>
      std::optional<T> maxBy(KeySelector keySelector) const {
        if (_data.isEmpty()) {
          return std::nullopt;
        }

        const T* best = &_data.first();
        auto bestKey = keySelector(*best);

        for (const auto& item : _data) {
          auto k = keySelector(item);
          if (bestKey < k) {
            best = &item;
            bestKey = k;
          }
        }
        return *best;
      }

      // count(value)
      int count(const T& value) const {
        int c = 0;
        for (const auto& item : _data) {
          if (item == value) {
            ++c;
          }
        }
        return c;
      }

      // for_each
      template <typename Func>
      void for_each(Func func) const {
        for (const auto& item : _data) {
          func(item);
        }
      }

      // distinct – optimized: use QSet (hash) instead of std::set (tree)
      Query<T> distinct() const {
        QSet<T> seen;
        QList<T> result;
        result.reserve(_data.size());

        for (const auto& item : _data) {
          if (!seen.contains(item)) {
            seen.insert(item);
            result.append(item);
          }
        }

        return Query<T>(std::move(result));
      }

      // reverse – QList copy + std::reverse
      Query<T> reverse() const {
        QList<T> result(_data);
        std::reverse(result.begin(), result.end());
        return Query<T>(std::move(result));
      }

      // sum(selector)
      template <typename Selector>
      auto sum(Selector selector) const -> typename std::decay<decltype(selector(std::declval<T>()))>::type {
        using R = typename std::decay<decltype(selector(std::declval<T>()))>::type;

        R total{};
        for (const auto& item : _data) {
          total += selector(item);
        }

        return total;
      }

      // first()
      T first() const {
        if (_data.isEmpty()) {
          throw std::runtime_error("qlinq::first() called on empty sequence");
        }
        return _data.first();
      }

      // first(predicate)
      template <typename Pred>
      T first(Pred pred) const {
        for (const auto& item : _data) {
          if (pred(item)) {
            return item;
          }
        }
        throw std::runtime_error("qlinq::first(predicate) found no match");
      }

      // firstOrDefault()
      std::optional<T> firstOrDefault() const {
        if (_data.isEmpty()) {
          return std::nullopt;
        }
        return _data.first();
      }

      // firstOrDefault(predicate)
      template <typename Pred>
      std::optional<T> firstOrDefault(Pred pred) const {
        for (const auto& item : _data) {
          if (pred(item)) {
            return item;
          }
        }
        return std::nullopt;
      }

      // aggregate
      template <typename Acc, typename Func>
      Acc aggregate(Acc seed, Func func) const {
        for (const auto& item : _data) {
          seed = func(seed, item);
        }
        return seed;
      }

    private:
      QList<T> _data;
  };

  // Free helpers.
  template <typename T>
  Query<T> from(const QList<T>& list) {
    return Query<T>::from(list);
  }

  template <typename T>
  Query<T> from(QList<T>&& list) {
    return Query<T>::from(std::move(list));
  }

} // namespace qlinq
