// For license, see <project-root-folder>/LICENSE.md.

#include <qtlinq/qtlinq.h>

#include <QElapsedTimer>
#include <QList>
#include <QString>
#include <QVector>
#include <QtTest>

using namespace qlinq;

// -------------------------------------------------------------
//  Complex heavy benchmark type
// -------------------------------------------------------------
struct ComplexItem {
    QString name;
    QString category;
    int id;
    int priority;
    QVector<int> payload;
    double weight;

    bool operator<(const ComplexItem& other) const noexcept {
      if (priority != other.priority) {
        return priority < other.priority;
      }
      return id < other.id;
    }
};

struct Person {
    QString name;
    int age;
};

class BenchQtLinq : public QObject {
    Q_OBJECT

  private:
    QList<int> ints;                 // huge list of ints
    QList<QList<int>> nested;        // for selectMany
    QList<Person> people;            // for min/max/minBy/etc.
    QList<ComplexItem> complexItems; // heavy objects

  private slots:
    void initTestCase();

    // Original benchmark tests
    void bench_where();
    void bench_select();
    void bench_selectMany();
    void bench_take();
    void bench_skip();
    void bench_orderBy();
    void bench_any_all();
    void bench_min_max();
    void bench_count();
    void bench_distinct();
    void bench_first();
    void bench_first_predicate();
    void bench_for_range_loop();

    // New chained benchmarks
    void bench_chain_where_select_orderBy_take();
    void bench_chain_select_where_skip_take_sum();
    void bench_chain_where_distinct_select_aggregate();
    void bench_chain_selectMany_where_sum();
    void bench_chain_orderBy_select_reverse_take();
    void bench_chain_multiple_where();
    void bench_chain_where_minBy_maxBy();
    void bench_chain_where_select_count();

    void cleanupTestCase();
};

// -------------------------------------------------------------
//  Data Initialization
// -------------------------------------------------------------
void BenchQtLinq::initTestCase() {
  constexpr int N = 2'000'000;
  constexpr int CHUNK = 10;

  // Prepare huge list of ints
  ints.reserve(N);
  for (int i = 0; i < N; ++i) {
    ints.append((i * 31) % 10007);
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

  // Prepare complex heavy items
  complexItems.reserve(N / 20);
  for (int i = 0; i < N / 20; ++i) {
    ComplexItem c;
    c.name = QStringLiteral("Item_%1").arg(i);
    c.category = QStringLiteral("Category_%1").arg(i % 15);
    c.id = i;
    c.priority = (i * 7) % 100;
    c.weight = (i % 1000) * 0.1;

    c.payload.reserve(20);
    for (int j = 0; j < 20; ++j) {
      c.payload.append((i + j * 13) % 500);
    }

    complexItems.append(std::move(c));
  }
}

// -------------------------------------------------------------
//  ORIGINAL BENCHMARKS
// -------------------------------------------------------------

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
    auto out = q.orderBy([](int a,int b) {
                  return a<b;
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

// -------------------------------------------------------------
//  NEW CHAINED BENCHMARKS
// -------------------------------------------------------------

void BenchQtLinq::bench_chain_where_select_orderBy_take() {
  auto q = from(complexItems);
  QBENCHMARK {
    auto out = q.where([](const ComplexItem& x) {
                  return x.priority > 50;
                })
                 .select([](const ComplexItem& x) {
                   return x.weight * 1.5;
                 })
                 .orderBy([](double w) {
                   return w;
                 })
                 .take(1000)
                 .toList();
    Q_UNUSED(out);
  }
}

void BenchQtLinq::bench_chain_select_where_skip_take_sum() {
  auto q = from(complexItems);
  QBENCHMARK {
    auto total = q.select([](const ComplexItem& x) {
                    return x.priority + x.id;
                  })
                   .where([](int v) {
                     return v % 3 == 0;
                   })
                   .skip(500)
                   .take(500)
                   .sum([](int v) {
                     return v;
                   });
    Q_UNUSED(total);
  }
}

void BenchQtLinq::bench_chain_where_distinct_select_aggregate() {
  auto q = from(ints);
  QBENCHMARK {
    auto result = q.where([](int x) {
                     return x > 500;
                   })
                    .distinct()
                    .select([](int x) {
                      return x * 2;
                    })
                    .aggregate(0LL, [](long long acc, int x) {
                      return acc + x;
                    });
    Q_UNUSED(result);
  }
}

void BenchQtLinq::bench_chain_selectMany_where_sum() {
  auto q = from(nested);
  QBENCHMARK {
    auto total = q.selectMany([](const QList<int>& l) {
                    return l;
                  })
                   .where([](int x) {
                     return x % 7 == 0;
                   })
                   .sum([](int x) {
                     return x;
                   });
    Q_UNUSED(total);
  }
}

void BenchQtLinq::bench_chain_orderBy_select_reverse_take() {
  auto q = from(complexItems);
  QBENCHMARK {
    auto out = q.orderBy([](const ComplexItem& x) {
                  return x.priority;
                })
                 .select([](const ComplexItem& x) {
                   return x.id;
                 })
                 .reverse()
                 .take(2000)
                 .toList();
    Q_UNUSED(out);
  }
}

void BenchQtLinq::bench_chain_multiple_where() {
  auto q = from(complexItems);
  QBENCHMARK {
    auto out = q.where([](const ComplexItem& x) {
                  return x.id % 2 == 0;
                })
                 .where([](const ComplexItem& x) {
                   return x.priority > 30;
                 })
                 .where([](const ComplexItem& x) {
                   return x.payload.size() == 20;
                 })
                 .toList();
    Q_UNUSED(out);
  }
}

void BenchQtLinq::bench_chain_where_minBy_maxBy() {
  auto q = from(complexItems);
  QBENCHMARK {
    auto mn = q.where([](const ComplexItem& x) {
                 return x.priority > 20;
               })
                .minBy([](const ComplexItem& x) {
                  return x.weight;
                });
    auto mx = q.where([](const ComplexItem& x) {
                 return x.priority > 20;
               })
                .maxBy([](const ComplexItem& x) {
                  return x.weight;
                });
    Q_UNUSED(mn);
    Q_UNUSED(mx);
  }
}

void BenchQtLinq::bench_chain_where_select_count() {
  auto q = from(complexItems);
  QBENCHMARK {
    auto c = q.where([](const ComplexItem& x) {
                return x.priority % 4 == 0;
              })
               .select([](const ComplexItem& x) {
                 return x.id;
               })
               .count(1000);
    Q_UNUSED(c);
  }
}

// -------------------------------------------------------------

void BenchQtLinq::cleanupTestCase() {
  // nothing to free
}

QTEST_APPLESS_MAIN(BenchQtLinq)
#include "tst_qtlinq_bench.moc"
