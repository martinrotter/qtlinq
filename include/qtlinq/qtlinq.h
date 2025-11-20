// For license of this file, see <project-root-folder>/LICENSE.md.

#pragma once

#include <algorithm>
#include <optional>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <memory>

#include <QList>
#include <QSet>

namespace qlinq {
template<typename T>
class Query
{
public:
    using value_type = T;
    struct const_iterator;
    friend struct const_iterator;
    template<typename U>
    friend class Query;
    struct const_iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = int;
        using value_type = T;
        using pointer = value_type *;
        using reference = value_type &;
        const_iterator(const typename QList<typename QList<T>::const_iterator>::const_iterator &base)
            : _iterPointer(std::make_unique<QList<typename QList<T>::const_iterator>::const_iterator>(base))
            , _baseIterPointer(nullptr)
        { }
        const_iterator(const typename QList<T>::const_iterator &base)
            : _iterPointer(nullptr)
            , _baseIterPointer(std::make_unique<QList<T>::const_iterator>(base))
        { }
        const_iterator(const const_iterator &other)
            : _iterPointer(std::make_unique<QList<typename QList<T>::const_iterator>::const_iterator>(*other._iterPointer))
            , _baseIterPointer(std::make_unique<QList<T>::const_iterator>(*other._baseIterPointer))
        { }
        const_iterator()
            : _iterPointer(nullptr)
            , _baseIterPointer(nullptr)
        { }
        value_type operator*() const
        {
            if (_iterPointer)
                return (*_iterPointer)->operator*();
            return (*_baseIterPointer).operator*();
        }
        pointer operator->()
        {
            if (_iterPointer)
                return (*_iterPointer)->operator->();
            return (*_baseIterPointer).get().operator->();
        }
        const_iterator &operator++()
        {
            if (_iterPointer)
                _iterPointer->operator++();
            else
                _baseIterPointer->operator++();
            return *this;
        }
        const_iterator &operator++(int)
        {
            const_iterator temp(*this);
            operator++();
            return temp;
        }
        friend bool operator==(const const_iterator &a, const const_iterator &b)
        {
            if (a._iterPointer && b._iterPointer)
                return *a._iterPointer == *b._iterPointer;
            else if (a._baseIterPointer && b._baseIterPointer)
                return *a._baseIterPointer == *b._baseIterPointer;
            return false;
        };
        friend bool operator!=(const const_iterator &a, const const_iterator &b) { return !(a == b); };
        friend bool operator<(const const_iterator &a, const const_iterator &b)
        {
            if (a._iterPointer && b._iterPointer)
                return *a._iterPointer < *b._iterPointer;
            else if (a._baseIterPointer && b._baseIterPointer)
                return *a._baseIterPointer < *b._baseIterPointer;
            return false;
        };
        friend bool operator<=(const const_iterator &a, const const_iterator &b) { return !(a > b); };
        friend bool operator>(const const_iterator &a, const const_iterator &b)
        {
            if (a._iterPointer && b._iterPointer)
                return *a._iterPointer < *b._iterPointer;
            else if (a._baseIterPointer && b._baseIterPointer)
                return *a._baseIterPointer < *b._baseIterPointer;
            return false;
        };
        friend bool operator>=(const const_iterator &a, const const_iterator &b) { return !(a < b); };

    private:
        std::unique_ptr<typename QList<typename QList<T>::const_iterator>::const_iterator> _iterPointer;
        std::unique_ptr<typename QList<T>::const_iterator> _baseIterPointer;
    };

    Query()
        : _rangeMode(true)
        , _dataCopy(nullptr)
    { }
    explicit Query(const QList<T> &list)
        : _rangeMode(true)
        , _dataCopy(nullptr)
        , _dataIter()
    {
        _dataIter.reserve(2);
        _dataIter.append(list.cbegin());
        _dataIter.append(list.cend());
    }
    Query(const Query<T> &other)
        : _rangeMode(other._rangeMode)
        , _dataCopy(other._dataCopy)
        , _dataIter(other._dataIter)
    { }
    Query<T> &operator=(const Query<T> &other)
    {
        _rangeMode = other._rangeMode;
        _dataCopy = other._dataCopy;
        _dataIter = other._dataIter;
        return *this;
    }

    static Query<T> from(const QList<T> &list) { return Query<T>(list); }

    // Basic iteration.
    const_iterator begin() const { return cbegin(); }
    const_iterator end() const { return cend(); }
    const_iterator cbegin() const
    {
        if (_rangeMode)
            return const_iterator(_dataIter.first());
        return const_iterator(_dataIter.cbegin());
    }
    const_iterator cend() const
    {
        if (_rangeMode)
            return const_iterator(_dataIter.last());
        return const_iterator(_dataIter.cend());
    }

    // Materialization.
    QList<T> toList() const
    {
        if (_dataCopy)
            return *_dataCopy;
        if (_rangeMode)
            return QList<T>(_dataIter.first(), _dataIter.last());
        QList<T> result;
        for (auto &&i : std::as_const(_dataIter))
            result.append(*i);
        return result;
    }

    int size() const
    {
        if (_dataCopy)
            return _dataCopy->size();
        if (_rangeMode)
            return std::distance(_dataIter.first(), _dataIter.last());
        return _dataIter.size();
    }

    int count() const { return size(); }

    bool isEmpty() const
    {
        if (_dataCopy)
            return _dataCopy->isEmpty();
        if (_dataIter.isEmpty())
            return true;
        if (_rangeMode)
            return _dataIter.first() == _dataIter.last();
        return false;
    }

    // ofType<U> : filters elements where dynamic_cast<U>(item) succeeds.
    template<typename U>
    Query<U> ofType() const
    {
        static_assert(std::is_pointer<T>::value, "ofType<U>(): Source sequence type must be a pointer.");
        static_assert(std::is_pointer<U>::value, "ofType<U>(): Target sequence type must be a pointer.");

        using FromPointee = typename std::remove_pointer<T>::type;
        using ToPointee = typename std::remove_pointer<U>::type;

        static_assert(std::is_base_of<FromPointee, ToPointee>::value || std::is_base_of<ToPointee, FromPointee>::value,
                      "ofType<U>() requires convertible pointer types "
                      "(derived/base relationship missing).");

        Query<U> result;
        result._dataCopy = std::make_shared<QList<U>>();
        result._dataCopy->reserve(size());

        if (_rangeMode) {
            for (auto i = _dataIter.first(); i != _dataIter.last(); ++i) {
                if (auto casted = dynamic_cast<U>(*i))
                    result._dataCopy->append(casted);
            }
        } else {
            for (auto &&i : std::as_const(_dataIter))
                if (auto casted = dynamic_cast<U>(*i))
                    result._dataCopy->append(casted);
        }

        result._rangeMode = true;
        result._dataIter.append(result._dataCopy->begin());
        result._dataIter.append(result._dataCopy->end());
        return result;
    }

    // where: filter items.
    template<typename Pred>
    Query<T> where(Pred pred) const
    {
        Query<T> result(*this);
        result._dataIter.clear();
        result._dataIter.reserve(size());
        if (_rangeMode) {
            for (auto i = _dataIter.first(); i != _dataIter.last(); ++i) {
                if (pred(*i)) {
                    if (result._dataIter.isEmpty() || !result._rangeMode)
                        result._dataIter.append(i);
                    else if (result._rangeMode && result._dataIter.size() == 2) {
                        result._rangeMode = false;
                        const auto jbegin = result._dataIter.first();
                        const auto jend = result._dataIter.last();
                        result._dataIter.clear();
                        result._dataIter.reserve(std::distance(jbegin, jend));
                        for (auto j = jbegin; j != jend; ++j)
                            result._dataIter.append(j);
                        result._dataIter.append(i);
                    }
                } else {
                    if (result._dataIter.size() == 1)
                        result._dataIter.append(i);
                }
            }
            if (result._rangeMode && result._dataIter.size() == 1)
                result._dataIter.append(_dataIter.last());
        } else {
            for (auto &&i : std::as_const(_dataIter))
                if (pred(*i))
                    result._dataIter.append(i);
        }
        return result;
    }

    // select: project items.
    template<typename Func>
    auto select(Func func) const -> Query<typename std::decay<decltype(func(std::declval<T>()))>::type>
    {
        using ResultType = typename std::decay<decltype(func(std::declval<T>()))>::type;

        Query<ResultType> result;
        result._dataCopy = std::make_shared<QList<ResultType>>();
        result._dataCopy->reserve(size());
        if (_rangeMode) {
            for (auto i = _dataIter.first(); i != _dataIter.last(); ++i)
                result._dataCopy->append(func(*i));
        } else {
            for (auto &&i : std::as_const(_dataIter))
                result._dataCopy->append(func(*i));
        }
        result._rangeMode = true;
        result._dataIter.append(result._dataCopy->begin());
        result._dataIter.append(result._dataCopy->end());
        return result;
    }

    // selectMany: project each item to a sequence and flatten.
    template<typename Func>
    auto selectMany(Func func) const -> Query<typename std::decay<decltype(*func(std::declval<T>()).begin())>::type>
    {
        using InnerList = decltype(func(std::declval<T>()));
        using InnerType = typename std::decay<decltype(*std::declval<InnerList>().begin())>::type;
        Query<InnerType> result;
        result._dataCopy = std::make_shared<QList<InnerType>>();
        result._dataCopy->reserve(size());
        if (_rangeMode) {
            for (auto i = _dataIter.first(); i != _dataIter.last(); ++i) {
                InnerList inner = func(*i);
                for (const auto &x : inner) {
                    result._dataCopy->append(x);
                }
            }
        } else {
            for (auto &&i : std::as_const(_dataIter)) {
                InnerList inner = func(*i);
                for (const auto &x : inner) {
                    result._dataCopy->append(x);
                }
            }
        }
        result._rangeMode = true;
        result._dataIter.append(result._dataCopy->begin());
        result._dataIter.append(result._dataCopy->end());
        return result;
    }

    // take: first n items.
    Query<T> take(int n) const
    {
        if (n <= 0) {
            return Query<T>();
        }

        if (n >= size()) {
            return *this;
        }

        Query<T> result(*this);
        if (_rangeMode)
            result._dataIter.last() = result._dataIter.first() + n;
        else
            result._dataIter = result._dataIter.first(n);
        return result;
    }

    // skip: skip first n items.
    Query<T> skip(int n) const
    {
        if (n <= 0) {
            return *this;
        }

        if (n >= size()) {
            return Query<T>();
        }

        Query<T> result(*this);
        if (_rangeMode)
            result._dataIter.first() = result._dataIter.last() - n - 1;
        else
            result._dataIter = result._dataIter.last(n);
        return result;
    }

    template<typename Compare>
    Query<T> orderBy(Compare comp) const
    {
        Query<T> result(*this);
        if (_rangeMode) {
            if (std::is_sorted(_dataIter.first(), _dataIter.last(), comp))
                return result;
            result._rangeMode = false;
            result._dataIter.clear();
            result._dataIter.reserve(size());
            for (auto i = _dataIter.first(); i != _dataIter.last(); ++i)
                result._dataIter.append(i);
        }
        std::sort(result._dataIter.begin(), result._dataIter.end(),
                  [&](const typename QList<T>::const_iterator &a, const typename QList<T>::const_iterator &b) -> bool { return comp(*a, *b); });
        return result;
    }

    // any: does any element match?
    template<typename Pred>
    bool any(Pred pred) const
    {
        if (_rangeMode) {
            for (auto i = _dataIter.first(); i != _dataIter.last(); ++i)
                if (pred(*i))
                    return true;
        } else {
            for (auto &&i : std::as_const(_dataIter))
                if (pred(*i))
                    return true;
        }
        return false;
    }

    // all: do all elements match?
    template<typename Pred>
    bool all(Pred pred) const
    {
        if (_rangeMode) {
            for (auto i = _dataIter.first(); i != _dataIter.last(); ++i)
                if (!pred(*i))
                    return false;
        } else {
            for (auto &&i : std::as_const(_dataIter))
                if (!pred(*i))
                    return false;
        }
        return true;
    }

    // min: return smallest element (requires operator< on T).
    std::optional<T> min() const
    {
        if (isEmpty())
            return std::nullopt;

        if (_rangeMode)
            return *std::min_element(_dataIter.first(), _dataIter.last());
        else
            return **std::min_element(
                    _dataIter.cbegin(), _dataIter.cend(),
                    [](const typename QList<T>::const_iterator &a, const typename QList<T>::const_iterator &b) -> bool { return *a < *b; });
    }

    // min(selector): return the minimum selector value (not the element).
    template<typename Compare>
    std::optional<T> min(Compare comp) const
    {
        if (_rangeMode)
            return *std::min_element(_dataIter.first(), _dataIter.last(), comp);
        else
            return **std::min_element(
                    _dataIter.cbegin(), _dataIter.cend(),
                    [&](const typename QList<T>::const_iterator &a, const typename QList<T>::const_iterator &b) -> bool { return comp(*a, *b); });
    }

    std::optional<T> max() const
    {
        if (isEmpty())
            return std::nullopt;

        if (_rangeMode)
            return *std::max_element(_dataIter.first(), _dataIter.last());
        else
            return **std::max_element(
                    _dataIter.cbegin(), _dataIter.cend(),
                    [](const typename QList<T>::const_iterator &a, const typename QList<T>::const_iterator &b) -> bool { return *a < *b; });
    }

    // min(selector): return the minimum selector value (not the element).
    template<typename Compare>
    std::optional<T> max(Compare comp) const
    {
        if (_rangeMode)
            return *std::max_element(_dataIter.first(), _dataIter.last(), comp);
        else
            return **std::max_element(
                    _dataIter.cbegin(), _dataIter.cend(),
                    [&](const typename QList<T>::const_iterator &a, const typename QList<T>::const_iterator &b) -> bool { return comp(*a, *b); });
    }

    // count(value) – count occurrences of a specific value.
    int count(const T &value) const
    {
        if (_rangeMode)
            return std::count(_dataIter.first(), _dataIter.last(), value);
        else
            return std::count_if(_dataIter.cbegin(), _dataIter.cend(),
                                 [&](const typename QList<T>::const_iterator &a) -> bool { return *a == value; });
    }

    // for_each – apply a lambda to every element.
    template<typename Func>
    Query<T> for_each(Func func) const
    {
        Query<T> result;
        if (_rangeMode) {
            result._dataCopy = std::make_shared<QList<T>>(_dataIter.first(), _dataIter.last());
            for (auto i = result._dataCopy->begin(); i != result._dataCopy->end(); ++i)
                func(*i);
        } else {
            result._dataCopy = std::make_shared<QList<T>>();
            result._dataCopy->reserve(size());
            for (auto &&i : std::as_const(_dataIter)) {
                result._dataCopy->append(*i);
                func(result._dataCopy->last());
            }
        }
        result._rangeMode = true;
        result._dataIter.append(result._dataCopy->begin());
        result._dataIter.append(result._dataCopy->end());
        return result;
    }

    // distinct – remove duplicate elements (requires operator==).
    Query<T> distinct() const
    {
        Query<T> result(*this);
        result._dataIter.clear();
        result._dataIter.reserve(size());
        result._rangeMode = false;
        std::set<T> seen;
        if (_rangeMode) {
            for (auto i = _dataIter.first(); i != _dataIter.last(); ++i) {
                if (std::as_const(seen).find(*i) == seen.cend()) {
                    seen.insert(*i);
                    result._dataIter.append(i);
                }
            }
            bool allSequential = true;
            auto ExpectedIter = result._dataIter.first();
            for (int i = 1; allSequential && i < result._dataIter.size(); ++i)
                allSequential = result._dataIter.at(i) == ++ExpectedIter;
            if (allSequential) {
                auto newBegin = result._dataIter.first();
                auto newEnd = result._dataIter.last();
                result._dataIter.clear();
                result._dataIter.reserve(2);
                result._dataIter.append(newBegin);
                result._dataIter.append(newEnd);
                result._rangeMode = true;
            }
        } else {
            for (auto &&i : std::as_const(_dataIter)) {
                if (std::as_const(seen).find(*i) == seen.cend()) {
                    seen.insert(*i);
                    result._dataIter.append(i);
                }
            }
        }
        if (seen.size() == size())
            return *this;
        return result;
    }

    // first(): return first element, throw if empty.
    T first() const
    {
        if (isEmpty()) {
            throw std::runtime_error("qlinq::first() called on an empty sequence");
        }
        return *_dataIter.first();
    }

    // first(predicate): return first matching element, throw if not found.
    template<typename Pred>
    T first(Pred pred) const
    {
        if (_rangeMode) {
            for (auto i = _dataIter.first(); i != _dataIter.last(); ++i)
                if (pred(*i))
                    return *i;
        } else {
            for (auto &&i : std::as_const(_dataIter))
                if (pred(*i))
                    return *i;
        }
        throw std::runtime_error("qlinq::first(predicate) found no matching element");
    }

    // firstOrDefault: optional first element.
    std::optional<T> firstOrDefault() const
    {
        if (isEmpty())
            return std::nullopt;

        return *_dataIter.first();
    }

    // firstOrDefault: optional first element matching predicate.
    template<typename Pred>
    std::optional<T> firstOrDefault(Pred pred) const
    {
        if (_rangeMode) {
            for (auto i = _dataIter.first(); i != _dataIter.last(); ++i)
                if (pred(*i))
                    return *i;
        } else {
            for (auto &&i : std::as_const(_dataIter))
                if (pred(*i))
                    return *i;
        }
        return std::nullopt;
    }

private:
    std::shared_ptr<QList<T>> _dataCopy;
    QList<typename QList<T>::const_iterator> _dataIter;
    bool _rangeMode;
};

// Helper free function so you can write qlinq::from(list).
template<typename T>
Query<T> from(const QList<T> &list)
{
    return Query<T>::from(list);
}

} // namespace qlinq
