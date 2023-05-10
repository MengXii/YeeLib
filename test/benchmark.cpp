#include "benchmark/benchmark.h"

#include <iostream>
#include <string>
#include <vector>

class A {
 public:
  A(int data) : a_(data), b_(data / 2), c_(data * 2), s_("hello world"){};
  ~A() = default;
  A(const A& a) {
    for (int i = 0; i < 1000; ++i)
      ;
  }
  A(const A&& a) {
  }

 private:
  int a_, b_, c_;
  std::string s_;
};
static void test1(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<A> a;
    for (int i = 1; i <= state.range(0); ++i) {
      A ai(i);
      a.push_back(ai);
    }
  }
}

static void test2(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<A> a;
    for (int i = 1; i <= state.range(0); ++i) {
      A ai(i);
      a.emplace_back(ai);
    }
  }
}
BENCHMARK(test1)->Range(100, 1000 * 1000 * 10);
BENCHMARK(test2)->Range(100, 1000 * 1000 * 10);
BENCHMARK_MAIN();
