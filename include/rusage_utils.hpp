#ifndef __RUSAGE_UTILS_HPP__
#define __RUSAGE_UTILS_HPP__

#include <cstdint>
#include <mutex>
#include <sys/resource.h>
#include <unistd.h>

namespace benchmark
{
int64_t get_involuntary_context_switches(int who = RUSAGE_THREAD) noexcept
{
  struct rusage rusage
  {
  };
  getrusage(who, &rusage);
  return static_cast<int64_t>(rusage.ru_nivcsw);
}

class ContextSwitchesCounter
{
public:
  explicit ContextSwitchesCounter(int who = RUSAGE_THREAD) : who_{ who }
  {
  }

  void init() noexcept
  {
    involuntary_context_switches_previous_ = get_involuntary_context_switches(who_);
  }

  [[nodiscard]] int64_t get() noexcept
  {
    std::call_once(once_, [this] { init(); });
    int64_t current = get_involuntary_context_switches(who_);
    return current - std::exchange(involuntary_context_switches_previous_, current);
  }

private:
  int64_t involuntary_context_switches_previous_;
  const int who_;
  std::once_flag once_;
};

struct PageFaults
{
  std::int64_t minor{ 0 };
  std::int64_t major{ 0 };
};

std::size_t get_process_memory(int who = RUSAGE_SELF) noexcept
{
  struct rusage rusage
  {
  };
  getrusage(who, &rusage);
  return static_cast<std::size_t>(rusage.ru_maxrss);
}

void print_process_memory(const std::string& log_text) noexcept
{
  printf("%s %ld MB\n", log_text.c_str(), get_process_memory() / 1024);
}

PageFaults get_total_page_faults(int who = RUSAGE_SELF) noexcept
{
  struct rusage rusage
  {
  };
  getrusage(who, &rusage);
  PageFaults new_page_faults{ rusage.ru_minflt, rusage.ru_majflt };
  return new_page_faults;
}

void print_page_faults(const std::string& log_text) noexcept
{
  auto pf = get_total_page_faults();
  printf("%s [Minor: %ld, Major: %ld]\n", log_text.c_str(), pf.minor, pf.major);
}

class PageFaultCounter
{
public:
  explicit PageFaultCounter(int who = RUSAGE_SELF) : who_{ who }
  {
  }

  void reset() noexcept
  {
    page_faults_previous_ = get_total_page_faults(who_);
  }

  [[nodiscard]] PageFaults get_page_faults_difference() noexcept
  {
    PageFaults current = get_total_page_faults(who_);
    PageFaults diff;
    diff.major = current.major - page_faults_previous_.major;
    diff.minor = current.minor - page_faults_previous_.minor;
    page_faults_previous_ = current;
    return diff;
  }

private:
  PageFaults page_faults_previous_;
  const int who_;
};

}  // namespace benchmark

#endif  // __RUSAGE_UTILS_HPP__
