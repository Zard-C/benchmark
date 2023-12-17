#ifndef __SCHED_UTILS_HPP__
#define __SCHED_UTILS_HPP__

#include <pthread.h>
#include <sys/types.h>
#include <thread>

int set_thread_scheduling(std::thread::native_handle_type thread, int policy, int sched_priority)
{
  struct sched_param param;
  param.sched_priority = sched_priority;
  auto ret = pthread_setschedparam(thread, policy, &param);
  return ret;
}

#endif  // __SCHED_UTILS_HPP_