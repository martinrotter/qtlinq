// For license of this file, see <project-root-folder>/LICENSE.md.

#include <qtlinq/qtlinq.h>

#include <QList>
#include <QString>
#include <QVector>
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
    void count_value();
    void for_each_accumulate();
    void distinct_basic();
    void first_basic();
    void first_predicate();
    void first_throws_on_empty();
    void first_predicate_throws_when_not_found();
    void firstOrDefault_basic();
    void firstOrDefault_predicate();
    void ofType_pointer_polymorphic();
    void ofType_same_pointer_type();
    void const_query_iteration();
    void range_based_for_iteration();
    void distinct();
    void where_range();
    void for_each();
};

// ------------------- TEST IMPLEMENTATIONS -------------------

void TestQtLinq::basics_size_count_empty() {
  QList<int> xs;
  auto q = from(xs);

  QCOMPARE(q.count(), 0);
  QVERIFY(q.isEmpty());

  QList<int> ys{1, 2, 3};
  auto q2 = from(ys);

  QCOMPARE(q2.count(), 3);
  QVERIFY(!q2.isEmpty());
}

void TestQtLinq::where_and_select() {
  QList<int> xs{1, 2, 3, 4, 5, 6};

  auto q = from(xs)
             .where([](int x) {
               return x % 2 == 0;
             })
             .select([](int x) {
               return x * 10;
             });

  auto list = q.toList();
  QCOMPARE(list, QList<int>({20, 40, 60}));
}

void TestQtLinq::where_range(){
  QList<int> xs{1, 2, 30, 40, 50, 6};
  auto q = from(xs)
             .where([](int x) {
               return x >10;
             });
  auto list = q.toList();
  QCOMPARE(list, QList<int>({30, 40, 50}));
}

void TestQtLinq::distinct(){
  QList<int> xs{1, 2, 3, 2, 1, 6};
  auto q = from(xs).distinct();
  auto list = q.toList();
  QCOMPARE(list, QList<int>({1, 2, 3, 6}));
}
void TestQtLinq::for_each(){
  QList<int> xs{1, 2, 3, 6};
  auto q = from(xs).for_each([](int& a){a*=10;});
  auto list = q.toList();
  QCOMPARE(list, QList<int>({10, 20, 30, 60}));
}

void TestQtLinq::selectMany_flatten() {
  QList<QList<int>> nested{{1, 2}, {3}, {4, 5}};

  auto flat = from(nested)
                .selectMany([](const QList<int>& l) {
                  return l;
                })
                .toList();

  QCOMPARE(flat, QList<int>({1, 2, 3, 4, 5}));
}

void TestQtLinq::take_and_skip() {
  QList<int> xs{1, 2, 3, 4, 5};

  auto taken = from(xs).take(3).toList();
  auto skipped = from(xs).skip(2).toList();

  QCOMPARE(taken, QList<int>({1, 2, 3}));
  QCOMPARE(skipped, QList<int>({3, 4, 5}));
}

void TestQtLinq::orderBy_and_orderByDescending() {
  QList<int> xs{5, 3, 1, 4, 2};

  auto asc = from(xs)
               .orderBy([](int a, int b)->bool {
                 return a<b;
               })
               .toList();
  auto desc = from(xs)
                .orderBy([](int a, int b)->bool {
                  return b < a;
                })
                .toList();

  QCOMPARE(asc, QList<int>({1, 2, 3, 4, 5}));
  QCOMPARE(desc, QList<int>({5, 4, 3, 2, 1}));
}

void TestQtLinq::any_and_all() {
  QList<int> xs{2, 4, 6};

  auto q = from(xs);

  QVERIFY(q.all([](int x) {
    return x % 2 == 0;
  }));
  QVERIFY(!q.any([](int x) {
    return x % 2 == 1;
  }));
}

void TestQtLinq::min_max_basic() {
  QList<int> xs{5, 1, 9, 3};

  auto q = from(xs);

  QCOMPARE(q.min().value(), 1);
  QCOMPARE(q.max().value(), 9);
}

void TestQtLinq::min_max_selector() {
  QList<Person> people{{"A", 30}, {"B", 20}, {"C", 40}};

  auto q = from(people);
  const auto ageCompare = [](const Person& a,const Person& b) {
    return a.age< b.age;
  };
  auto youngest = q.min(ageCompare);
  auto oldest = q.max(ageCompare);

  QCOMPARE(youngest.value().age, 20);
  QCOMPARE(oldest.value().age, 40);
}

void TestQtLinq::count_value() {
  QList<int> xs{1, 2, 3, 2, 2};

  auto q = from(xs);

  QCOMPARE(q.count(), 5);
  QCOMPARE(q.count(2), 3);
}

void TestQtLinq::for_each_accumulate() {
  QList<int> xs{1, 2, 3};

  int sum = 0;
  from(xs).for_each([&](int x) {
    sum += x;
  });

  QCOMPARE(sum, 6);
}

void TestQtLinq::distinct_basic() {
  QList<int> xs{1, 2, 2, 3, 1, 4, 4, 5};

  auto d = from(xs).distinct().toList();

  QCOMPARE(d, QList<int>({1, 2, 3, 4, 5}));
}

void TestQtLinq::first_basic() {
  QList<int> xs{10, 20, 30};

  auto v = from(xs).first();

  QCOMPARE(v, 10);
}

void TestQtLinq::first_predicate() {
  QList<int> xs{1, 3, 4, 6};

  auto v = from(xs).first([](int x) {
    return x % 2 == 0;
  });

  QCOMPARE(v, 4);
}

void TestQtLinq::first_throws_on_empty() {
  QList<int> empty;

  QVERIFY_THROWS_EXCEPTION(std::runtime_error, from(empty).first());
}

void TestQtLinq::first_predicate_throws_when_not_found() {
  QList<int> xs{1, 3, 5};

  QVERIFY_THROWS_EXCEPTION(std::runtime_error,from(xs).first([](int x) {
    return x % 2 == 0;
  }));
}

void TestQtLinq::firstOrDefault_basic() {
  QList<int> xs{5, 10};

  auto v = from(xs).firstOrDefault();
  QVERIFY(v.has_value());
  QCOMPARE(v.value(), 5);
}

void TestQtLinq::firstOrDefault_predicate() {
  QList<int> xs{1, 3, 4};

  auto v = from(xs).firstOrDefault([](int x) {
    return x % 2 == 0;
  });
  QVERIFY(v.has_value());
  QCOMPARE(v.value(), 4);
}

// -------------------- ofType Tests --------------------

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

  for (Animal* a : animals) {
    delete a;
  }
}

void TestQtLinq::ofType_same_pointer_type() {
  QList<Dog*> dogs;
  dogs.append(new Dog());
  dogs.append(new Dog());

  auto q = from(dogs);

  auto same = q.ofType<Dog*>().toList();
  QCOMPARE(same.size(), 2);

  for (Dog* d : dogs) {
    delete d;
  }
}

// -------------------- for-range tests --------------------

void TestQtLinq::range_based_for_iteration() {
  QList<int> xs{1, 2, 3, 4};

  int sum = 0;
  for (int v : from(xs).where([](int n) {
         return n % 2 == 0;
       })) {
    sum += v;
  }

  QCOMPARE(sum, 6);
}

void TestQtLinq::const_query_iteration() {
  QList<int> xs{5, 10, 15};

  const auto q = from(xs);
  int sum = 0;

  for (auto i = q.cbegin(), iEnd = q.cend();i!=iEnd;++i) {
    sum += *i;
  }

  QCOMPARE(sum, 30);
  sum = 0;
  for (const int& v : q) {
    sum += v;
  }

  QCOMPARE(sum, 30);
}

QTEST_APPLESS_MAIN(TestQtLinq)
#include "tst_qtlinq.moc"
