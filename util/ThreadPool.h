#pragma once

#include <thread>
#include <mutex>
#include <future>
#include <deque>
#include <vector>

namespace util {

class ThreadPool {
public:
	ThreadPool() {
		const auto nThreads = std::thread::hardware_concurrency();
		for (size_t i = 0; i < nThreads; ++i)
			threads_.push_back(std::thread([&]{
				for (;;) {
					std::unique_lock l(mutex_);
					condVar_.wait(l, [&]{return stopped_ || !tasks_.empty();});
					if (stopped_)
						break;
					auto task = std::move(tasks_.back());
					tasks_.pop_back();
					l.unlock();
					task();
				}
			}));
	}

	~ThreadPool() {
		std::unique_lock l(mutex_);
		stopped_ = true;
		l.unlock();
		condVar_.notify_all();
		for (auto &thread: threads_)
			thread.join();
	}

	template<class Func> auto exec(Func f) -> std::future<decltype(f())> {
		auto promise = std::make_shared<std::promise<decltype(f())>>();
		auto future = promise->get_future();
		std::unique_lock l(mutex_);
		tasks_.push_back([promise, f] {
			try {
				promise->set_value(f());
			} catch (...) {
				try {
					promise->set_exception(std::current_exception());
				} catch(...) {}
			}
		});
		l.unlock();
		condVar_.notify_one();
		return future;
	}

private:
	bool stopped_{false};
	std::mutex mutex_;
	std::condition_variable condVar_;
	std::vector<std::thread> threads_;
	std::deque<std::function<void()>> tasks_;
};

}
