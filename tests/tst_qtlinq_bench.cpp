// tests/tst_qtlinq_bench.cpp

#include <qtlinq/qtlinq.h>

#include <QElapsedTimer>
#include <QList>
#include <QString>
#include <QVector>
#include <QtTest>

using namespace qlinq;

struct Person {
    QString name;
    int age;
};

class BenchQtLinq : public QObject {
    Q_OBJECT

  private:
    QList<int> ints;          // huge list of ints
    QList<QList<int>> nested; // for selectMany
    QList<Person> people;     // for min/max/minBy/etc.

  private slots:
    void initTestCase();

    // Benchmark tests
    void bench_where();
    void bench_select();
    void bench_selectMany();
    void bench_take();
    void bench_skip();
    void bench_orderBy();
    void bench_orderByDescending();
    void bench_any_all();
    void bench_min_max();
    void bench_minBy_maxBy();
    void bench_count();
    void bench_distinct();
    void bench_reverse();
    void bench_sum();
    void bench_first();
    void bench_first_predicate();
    void bench_aggregate();
    void bench_for_range_loop();

    void cleanupTestCase();
};

void BenchQtLinq::initTestCase() {
  constexpr int N = 2'000'000; // 2 million ints
  constexpr int CHUNK = 10;

  // Prepare huge list of ints
  ints.reserve(N);
  for (int i = 0; i < N; ++i) {
    ints.append((i * 31) % 10007); // deterministic pseudo-random
  }

  // Prepare nested lists for selectMany
  nested.reserve(N / CHUNK);
  for (int i = 0; i < N; i += CHUNK) {
    QList<int> chunk;
    chunk.reserve(CHUNK);
    for (int j = 0; j < CHUNK && i + j < N; ++j) {
      chunk.append(ints[i + j]);
    }
    nested.append(std::move(chunk));
  }

  // Prepare list of people
  people.reserve(N / 10);
  for (int i = 0; i < N / 10; ++i) {
    people.append(Person{QStringLiteral("Person_%1").arg(i), 18 + (i % 50)});
  }
}

// ------------------- Benchmarks -----------------------

void BenchQtLinq::bench_where() {
  auto q = from(ints);
  QBENCHMARK {
    auto out = q.where([](int x) {
                  return (x & 1) == 0;
                })
                 .toList();
    Q_UNUSED(out);
  }
}

void BenchQtLinq::bench_select() {
  auto q = from(ints);
  QBENCHMARK {
    auto out = q.select([](int x) {
                  return x * 2;
                })
                 .toList();
    Q_UNUSED(out);
  }
}

void BenchQtLinq::bench_selectMany() {
  auto q = from(nested);
  QBENCHMARK {
    auto out = q.selectMany([](const QList<int>& l) {
                  return l;
                })
                 .toList();
    Q_UNUSED(out);
  }
}

void BenchQtLinq::bench_take() {
  auto q = from(ints);
  QBENCHMARK {
    auto out = q.take(500000).toList();
    Q_UNUSED(out);
  }
}

void BenchQtLinq::bench_skip() {
  auto q = from(ints);
  QBENCHMARK {
    auto out = q.skip(500000).toList();
    Q_UNUSED(out);
  }
}

void BenchQtLinq::bench_orderBy() {
  auto q = from(ints);
  QBENCHMARK {
    auto out = q.orderBy([](int x) {
                  return x;
                })
                 .toList();
    Q_UNUSED(out);
  }
}

void BenchQtLinq::bench_orderByDescending() {
  auto q = from(ints);
  QBENCHMARK {
    auto out = q.orderByDescending([](int x) {
                  return x;
                })
                 .toList();
    Q_UNUSED(out);
  }
}

void BenchQtLinq::bench_any_all() {
  auto q = from(ints);
  QBENCHMARK {
    bool r1 = q.any([](int x) {
      return x == 42;
    });
    bool r2 = q.all([](int x) {
      return x >= 0;
    });
    Q_UNUSED(r1);
    Q_UNUSED(r2);
  }
}

void BenchQtLinq::bench_min_max() {
  auto q = from(ints);
  QBENCHMARK {
    auto mn = q.min();
    auto mx = q.max();
    Q_UNUSED(mn);
    Q_UNUSED(mx);
  }
}

void BenchQtLinq::bench_minBy_maxBy() {
  auto q = from(people);
  QBENCHMARK {
    auto mn = q.minBy([](const Person& p) {
      return p.age;
    });
    auto mx = q.maxBy([](const Person& p) {
      return p.age;
    });
    Q_UNUSED(mn);
    Q_UNUSED(mx);
  }
}

void BenchQtLinq::bench_count() {
  auto q = from(ints);
  QBENCHMARK {
    auto c = q.count(123);
    Q_UNUSED(c);
  }
}

void BenchQtLinq::bench_distinct() {
  auto q = from(ints);
  QBENCHMARK {
    auto d = q.distinct().toList();
    Q_UNUSED(d);
  }
}

void BenchQtLinq::bench_reverse() {
  auto q = from(ints);
  QBENCHMARK {
    auto r = q.reverse().toList();
    Q_UNUSED(r);
  }
}

void BenchQtLinq::bench_sum() {
  auto q = from(people);
  QBENCHMARK {
    auto s = q.sum([](const Person& p) {
      return p.age;
    });
    Q_UNUSED(s);
  }
}

void BenchQtLinq::bench_first() {
  auto q = from(ints);
  QBENCHMARK {
    int v = q.first();
    Q_UNUSED(v);
  }
}

void BenchQtLinq::bench_first_predicate() {
  auto q = from(ints);
  QBENCHMARK {
    auto v = q.first([](int x) {
      return x > 5000;
    });
    Q_UNUSED(v);
  }
}

void BenchQtLinq::bench_aggregate() {
  auto q = from(ints);
  QBENCHMARK {
    auto sum = q.aggregate(0LL, [](long long acc, int x) {
      return acc + x;
    });
    Q_UNUSED(sum);
  }
}

void BenchQtLinq::bench_for_range_loop() {
  auto q = from(ints);
  QBENCHMARK {
    long long sum = 0;
    for (int v : q) {
      sum += v;
    }
    Q_UNUSED(sum);
  }
}

void BenchQtLinq::cleanupTestCase() {
  // nothing to free
}

QTEST_APPLESS_MAIN(BenchQtLinq)
#include "tst_qtlinq_bench.moc"
