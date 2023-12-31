#include <chrono>
#include <filesystem>
#include <memory>

#include "benchmark/benchmark.h"
#include "db_interface.h"
#include "workloads/create_workload.h"
#include "ycsbr/benchmark.h"
#include "ycsbr/impl/executor.h"
#include "ycsbr/impl/flag.h"
#include "ycsbr/request.h"
#include "ycsbr/run_options.h"
#include "ycsbr/session.h"
#include "ycsbr/trace.h"
#include "ycsbr/trace_workload.h"

namespace {

using namespace ycsbr;

class SimpleWorkload {
 public:
  SimpleWorkload(size_t num_requests) : num_requests_(num_requests) {}
  class Producer {
   public:
    Producer(size_t num_requests) : num_requests_(num_requests), index_(0) {}
    void Prepare() {}
    bool HasNext() const { return index_ < num_requests_; }
    Request Next() {
      ++index_;
      return Request();
    }

   private:
    size_t num_requests_;
    size_t index_;
  };

  std::vector<Producer> GetProducers(const size_t num_producers) const {
    std::vector<Producer> producers;
    for (size_t i = 0; i < num_producers; ++i) {
      producers.emplace_back(num_requests_);
    }
    return producers;
  }

 private:
  size_t num_requests_;
};

template <WorkloadType Type>
void BM_SimpleInterfaceOverhead(benchmark::State& state) {
  const std::filesystem::path trace_file = CreateWorkloadFile<Type>();
  Trace::Options options;
  options.value_size = 16;
  const Trace trace = Trace::LoadFromFile(trace_file.string(), options);

  BenchmarkOptions<NoOpInterface> boptions;
  boptions.latency_sample_period = state.range(0);
  for (auto _ : state) {
    const auto result = ReplayTrace<NoOpInterface>(trace, nullptr, boptions);
    state.SetIterationTime(
        result.RunTime<std::chrono::duration<double>>().count());
  }

  const size_t requests_processed = kTraceSize * state.iterations();
  state.SetItemsProcessed(requests_processed);
  state.counters["PerRequestLatency"] =
      benchmark::Counter(requests_processed, benchmark::Counter::kIsRate |
                                                 benchmark::Counter::kInvert);
  std::filesystem::remove(trace_file);
}

template <WorkloadType Type>
void BM_ExecutorLoopOverhead(benchmark::State& state) {
  const std::filesystem::path trace_file = CreateWorkloadFile<Type>();
  Trace::Options options;
  options.value_size = 16;
  const Trace trace = Trace::LoadFromFile(trace_file.string(), options);
  const TraceWorkload workload = TraceWorkload(&trace);
  const std::vector<TraceWorkload::Producer> producers =
      workload.GetProducers(1);

  RunOptions roptions;
  roptions.latency_sample_period = state.range(0);
  NoOpInterface db;
  impl::Flag can_start;
  impl::Executor<NoOpInterface, TraceWorkload::Producer> executor(
      &db, producers.at(0), 0, &can_start, roptions);
  for (auto _ : state) {
    executor.BM_WorkloadLoop();
  }
  const size_t requests_processed = kTraceSize * state.iterations();
  state.SetItemsProcessed(requests_processed);
  state.counters["PerRequestLatency"] =
      benchmark::Counter(requests_processed, benchmark::Counter::kIsRate |
                                                 benchmark::Counter::kInvert);
  std::filesystem::remove(trace_file);
}

template <WorkloadType Type>
void BM_SessionTraceReplayOverhead(benchmark::State& state) {
  const std::filesystem::path trace_file = CreateWorkloadFile<Type>();
  Trace::Options options;
  options.value_size = 16;
  const Trace trace = Trace::LoadFromFile(trace_file.string(), options);

  Session<NoOpInterface> session(1);
  RunOptions roptions;
  roptions.latency_sample_period = state.range(0);
  for (auto _ : state) {
    const auto result = session.ReplayTrace(trace, roptions);
    state.SetIterationTime(
        result.RunTime<std::chrono::duration<double>>().count());
  }

  const size_t requests_processed = kTraceSize * state.iterations();
  state.SetItemsProcessed(requests_processed);
  state.counters["PerRequestLatency"] =
      benchmark::Counter(requests_processed, benchmark::Counter::kIsRate |
                                                 benchmark::Counter::kInvert);
  std::filesystem::remove(trace_file);
}

void BM_SessionWorkloadOverhead(benchmark::State& state) {
  const size_t latency_sample_period = state.range(0);
  const size_t workload_length = state.range(1);

  Session<NoOpInterface> session(1);
  RunOptions roptions;
  roptions.latency_sample_period = latency_sample_period;
  for (auto _ : state) {
    const auto result = session.RunWorkload<SimpleWorkload>(
        SimpleWorkload(workload_length), roptions);
    state.SetIterationTime(
        result.RunTime<std::chrono::duration<double>>().count());
  }

  const size_t requests_processed = workload_length * state.iterations();
  state.SetItemsProcessed(requests_processed);
  state.counters["PerRequestLatency"] =
      benchmark::Counter(requests_processed, benchmark::Counter::kIsRate |
                                                 benchmark::Counter::kInvert);
}

BENCHMARK_TEMPLATE(BM_SimpleInterfaceOverhead, WorkloadType::kRunA)
    ->Arg(1)
    ->Arg(5)
    ->Arg(10)
    ->Arg(20)
    ->Arg(30)
    ->UseManualTime();

BENCHMARK_TEMPLATE(BM_ExecutorLoopOverhead, WorkloadType::kLoadA)
    ->Arg(1)
    ->Arg(20)
    ->Arg(30)
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_ExecutorLoopOverhead, WorkloadType::kRunA)
    ->Arg(1)
    ->Arg(20)
    ->Arg(30)
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_ExecutorLoopOverhead, WorkloadType::kRunE)
    ->Arg(1)
    ->Arg(20)
    ->Arg(30)
    ->UseRealTime();

BENCHMARK_TEMPLATE(BM_SessionTraceReplayOverhead, WorkloadType::kRunA)
    ->Arg(1)
    ->Arg(5)
    ->Arg(10)
    ->Arg(20)
    ->Arg(30)
    ->UseManualTime();

BENCHMARK(BM_SessionWorkloadOverhead)
    ->Args({500, 1000})  // (latency_sample_period, workload_length)
    ->Args({1000, 10000})
    ->Args({10000, 100000})
    ->Args({10000, 1000000})
    ->UseManualTime();

}  // namespace
