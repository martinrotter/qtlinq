#include <qtlinq/qtlinq.h>

#include <QList>
#include <QString>
#include <QtTest>

using namespace qlinq;

// Helper types for tests
struct Person {
    QString name;
    int age;

    bool operator==(const Person& other) const {
      return name == other.name && age == other.age;
    }
};

struct Animal {
    virtual ~Animal() = default;
};

struct Dog : Animal {};
struct Cat : Animal {};

class TestQtLinq : public QObject {
    Q_OBJECT

  private slots:
    void basics_size_count_empty();
    void where_and_select();
    void selectMany_flatten();
    void take_and_skip();
    void orderBy_and_orderByDescending();
    void any_and_all();
    void min_max_basic();
    void min_max_selector();
    void minBy_maxBy();
    void count_value();
    void for_each_accumulate();
    void distinct_basic();
    void reverse_basic();
    void sum_selector();
    void first_basic();
    void first_predicate();
    void first_throws_on_empty();
    void first_predicate_throws_when_not_found();
    void firstOrDefault_basic();
    void firstOrDefault_predicate();
    void aggregate_basic();
    void ofType_pointer_polymorphic();
    void ofType_value_types();
    void range_based_for_iteration();
    void const_query_iteration();
};

// ---- Tests implementations ----

void TestQtLinq::basics_size_count_empty() {
  QList<int> empty;
  auto qEmpty = from(empty);

  QVERIFY(qEmpty.isEmpty());
  QCOMPARE(qEmpty.size(), 0);
  QCOMPARE(qEmpty.count(), 0);

  QList<int> xs{1, 2, 3};
  auto q = from(xs);
  QVERIFY(!q.isEmpty());
  QCOMPARE(q.size(), 3);
  QCOMPARE(q.count(), 3);
}

void TestQtLinq::where_and_select() {
  QList<int> xs{1, 2, 3, 4, 5, 6};
  auto q = from(xs)
             .where([](int v) {
               return v % 2 == 0;
             }) // 2,4,6
             .select([](int v) {
               return v * 10;
             }); // 20,40,60

  auto result = q.toList();
  QList<int> expected{20, 40, 60};
  QCOMPARE(result, expected);
}

void TestQtLinq::selectMany_flatten() {
  QList<QList<int>> xs{{1, 2}, {}, {3}, {4, 5}};

  auto flat = from(xs)
                .selectMany([](const QList<int>& l) {
                  return l;
                })
                .toList();

  QList<int> expected{1, 2, 3, 4, 5};
  QCOMPARE(flat, expected);
}

void TestQtLinq::take_and_skip() {
  QList<int> xs{1, 2, 3, 4, 5};

  auto taken = from(xs).take(3).toList();
  QList<int> expectedTaken{1, 2, 3};
  QCOMPARE(taken, expectedTaken);

  auto takenTooMany = from(xs).take(10).toList();
  QCOMPARE(takenTooMany, xs);

  auto takenZero = from(xs).take(0).toList();
  QCOMPARE(takenZero.size(), 0);

  auto skipped = from(xs).skip(2).toList(); // 3,4,5
  QList<int> expectedSkipped{3, 4, 5};
  QCOMPARE(skipped, expectedSkipped);

  auto skippedTooMany = from(xs).skip(10).toList();
  QCOMPARE(skippedTooMany.size(), 0);

  auto skippedZero = from(xs).skip(0).toList();
  QCOMPARE(skippedZero, xs);
}

void TestQtLinq::orderBy_and_orderByDescending() {
  QList<Person> people{{"Alice", 30}, {"Bob", 20}, {"Carol", 25}, {"Dave", 40}};

  auto byAgeAsc = from(people)
                    .orderBy([](const Person& p) {
                      return p.age;
                    })
                    .toList();

  QCOMPARE(byAgeAsc[0].name, QString("Bob"));
  QCOMPARE(byAgeAsc[1].name, QString("Carol"));
  QCOMPARE(byAgeAsc[2].name, QString("Alice"));
  QCOMPARE(byAgeAsc[3].name, QString("Dave"));

  auto byAgeDesc = from(people)
                     .orderByDescending([](const Person& p) {
                       return p.age;
                     })
                     .toList();

  QCOMPARE(byAgeDesc[0].name, QString("Dave"));
  QCOMPARE(byAgeDesc[1].name, QString("Alice"));
  QCOMPARE(byAgeDesc[2].name, QString("Carol"));
  QCOMPARE(byAgeDesc[3].name, QString("Bob"));
}

void TestQtLinq::any_and_all() {
  QList<int> xs{1, 2, 3, 4};

  auto q = from(xs);
  QVERIFY(q.any([](int v) {
    return v % 2 == 0;
  })); // has even
  QVERIFY(!q.any([](int v) {
    return v > 10;
  })); // none >10

  QVERIFY(q.all([](int v) {
    return v > 0;
  })); // all > 0
  QVERIFY(!q.all([](int v) {
    return v % 2 == 0;
  })); // not all even
}

void TestQtLinq::min_max_basic() {
  QList<int> xs{5, 1, 9, 3};

  auto q = from(xs);
  auto minV = q.min();
  auto maxV = q.max();

  QVERIFY(minV.has_value());
  QVERIFY(maxV.has_value());
  QCOMPARE(minV.value(), 1);
  QCOMPARE(maxV.value(), 9);

  QList<int> empty;
  auto qEmpty = from(empty);
  QVERIFY(!qEmpty.min().has_value());
  QVERIFY(!qEmpty.max().has_value());
}

void TestQtLinq::min_max_selector() {
  QList<Person> people{{"Alice", 30}, {"Bob", 20}, {"Carol", 25}, {"Dave", 40}};

  auto q = from(people);

  auto minAge = q.min([](const Person& p) {
    return p.age;
  });
  auto maxAge = q.max([](const Person& p) {
    return p.age;
  });

  QVERIFY(minAge.has_value());
  QVERIFY(maxAge.has_value());
  QCOMPARE(minAge.value(), 20);
  QCOMPARE(maxAge.value(), 40);

  QList<Person> emptyPeople;
  auto qEmpty = from(emptyPeople);
  QVERIFY(!qEmpty
             .min([](const Person& p) {
               return p.age;
             })
             .has_value());
  QVERIFY(!qEmpty
             .max([](const Person& p) {
               return p.age;
             })
             .has_value());
}

void TestQtLinq::minBy_maxBy() {
  QList<Person> people{{"Alice", 30}, {"Bob", 20}, {"Carol", 25}, {"Dave", 40}};

  auto q = from(people);

  auto youngest = q.minBy([](const Person& p) {
    return p.age;
  });
  auto oldest = q.maxBy([](const Person& p) {
    return p.age;
  });

  QVERIFY(youngest.has_value());
  QVERIFY(oldest.has_value());
  QCOMPARE(youngest->name, QString("Bob"));
  QCOMPARE(oldest->name, QString("Dave"));

  QList<Person> empty;
  auto qEmpty = from(empty);
  QVERIFY(!qEmpty
             .minBy([](const Person& p) {
               return p.age;
             })
             .has_value());
  QVERIFY(!qEmpty
             .maxBy([](const Person& p) {
               return p.age;
             })
             .has_value());
}

void TestQtLinq::count_value() {
  QList<int> xs{1, 2, 2, 3, 2};
  auto q = from(xs);

  QCOMPARE(q.count(2), 3);
  QCOMPARE(q.count(5), 0);
}

void TestQtLinq::for_each_accumulate() {
  QList<int> xs{1, 2, 3, 4};
  auto q = from(xs);

  int sum = 0;
  q.for_each([&](int v) {
    sum += v;
  });

  QCOMPARE(sum, 10);
}

void TestQtLinq::distinct_basic() {
  QList<int> xs{1, 2, 2, 3, 1, 4, 4};
  auto result = from(xs).distinct().toList();

  QList<int> expected{1, 2, 3, 4};
  QCOMPARE(result, expected);
}

void TestQtLinq::reverse_basic() {
  QList<int> xs{1, 2, 3, 4};
  auto result = from(xs).reverse().toList();

  QList<int> expected{4, 3, 2, 1};
  QCOMPARE(result, expected);
}

void TestQtLinq::sum_selector() {
  QList<Person> people{{"Alice", 30}, {"Bob", 20}, {"Carol", 25}};

  auto q = from(people);
  int ageSum = q.sum([](const Person& p) {
    return p.age;
  });
  QCOMPARE(ageSum, 30 + 20 + 25);

  QList<Person> empty;
  auto qEmpty = from(empty);
  int emptySum = qEmpty.sum([](const Person& p) {
    return p.age;
  });
  QCOMPARE(emptySum, 0); // default-initialized int
}

void TestQtLinq::first_basic() {
  QList<int> xs{10, 20, 30};
  auto q = from(xs);

  int first = q.first();
  QCOMPARE(first, 10);
}

void TestQtLinq::first_predicate() {
  QList<int> xs{1, 3, 4, 6};
  auto q = from(xs);

  int firstEven = q.first([](int v) {
    return v % 2 == 0;
  });
  QCOMPARE(firstEven, 4);
}

void TestQtLinq::first_throws_on_empty() {
  QList<int> xs;
  auto q = from(xs);

  bool threw = false;
  try {
    (void)q.first();
  }
  catch (const std::runtime_error&) {
    threw = true;
  }
  QVERIFY(threw);
}

void TestQtLinq::first_predicate_throws_when_not_found() {
  QList<int> xs{1, 3, 5};
  auto q = from(xs);

  bool threw = false;
  try {
    (void)q.first([](int v) {
      return v % 2 == 0;
    });
  }
  catch (const std::runtime_error&) {
    threw = true;
  }
  QVERIFY(threw);
}

void TestQtLinq::firstOrDefault_basic() {
  QList<int> xs{10, 20, 30};
  auto q = from(xs);

  auto v = q.firstOrDefault();
  QVERIFY(v.has_value());
  QCOMPARE(v.value(), 10);

  QList<int> empty;
  auto qEmpty = from(empty);
  auto none = qEmpty.firstOrDefault();
  QVERIFY(!none.has_value());
}

void TestQtLinq::firstOrDefault_predicate() {
  QList<int> xs{1, 3, 4, 6};
  auto q = from(xs);

  auto firstEven = q.firstOrDefault([](int v) {
    return v % 2 == 0;
  });
  QVERIFY(firstEven.has_value());
  QCOMPARE(firstEven.value(), 4);

  auto none = q.firstOrDefault([](int v) {
    return v > 100;
  });
  QVERIFY(!none.has_value());
}

void TestQtLinq::aggregate_basic() {
  QList<int> xs{1, 2, 3, 4};
  auto q = from(xs);

  int sum = q.aggregate(0, [](int acc, int v) {
    return acc + v;
  });
  QCOMPARE(sum, 10);

  QList<QString> strs{"a", "b", "c"};
  auto qStr = from(strs);
  QString cat = qStr.aggregate(QString{}, [](const QString& acc, const QString& s) {
    return acc + s;
  });
  QCOMPARE(cat, QString("abc"));
}

void TestQtLinq::ofType_pointer_polymorphic() {
  QList<Animal*> animals;
  animals.append(new Dog());
  animals.append(new Cat());
  animals.append(new Dog());

  auto q = from(animals);

  auto dogs = q.ofType<Dog*>().toList();
  auto cats = q.ofType<Cat*>().toList();

  QCOMPARE(dogs.size(), 2);
  QCOMPARE(cats.size(), 1);
  QVERIFY(dynamic_cast<Dog*>(dogs[0]) != nullptr);
  QVERIFY(dynamic_cast<Dog*>(dogs[1]) != nullptr);
  QVERIFY(dynamic_cast<Cat*>(cats[0]) != nullptr);

  // Cleanup to avoid leaks
  for (Animal* a : animals) {
    delete a;
  }
}

void TestQtLinq::ofType_value_types() {
  QList<int> xs{1, 2, 3};
  auto q = from(xs);

  // Same type: should keep all
  auto sameType = q.ofType<int>().toList();
  QCOMPARE(sameType, xs);

  // Different type: should filter all out
  auto noMatch = q.ofType<double>().toList();
  QCOMPARE(noMatch.size(), 0);
}

void TestQtLinq::range_based_for_iteration() {
  QList<int> xs{1, 2, 3, 4};
  int sum = 0;

  for (int v : from(xs).where([](int x) {
         return x % 2 == 0;
       })) {
    sum += v; // 2 + 4 = 6
  }

  QCOMPARE(sum, 6);
}

void TestQtLinq::const_query_iteration() {
  QList<int> xs{5, 10, 15};
  const auto q = from(xs);

  int sum = 0;
  for (const int& v : q) {
    sum += v;
  }

  QCOMPARE(sum, 30);
}

QTEST_APPLESS_MAIN(TestQtLinq)
#include "tst_qtlinq.moc"
