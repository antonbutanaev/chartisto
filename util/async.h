#pragma once

#include <thread>
#include <mutex>
#include <future>
#include <deque>
#include <vector>
#include <util/funcIterator.h>

namespace util {

class Async {
public:
	Async() {
		const auto nThreads = std::thread::hardware_concurrency();
		for (size_t i = 0; i < nThreads; ++i)
			threads_.push_back(std::thread([&]{
				for (;;) {
					std::unique_lock l(mutex_);
					condVar_.wait(l, [&]{return stopped_ || !tasks_.empty();});
					if (stopped_)
						break;
					auto task = std::move(tasks_.front());
					tasks_.pop_front();
					l.unlock();
					task();
				}
			}));
	}

	~Async() {
		std::unique_lock l(mutex_);
		stopped_ = true;
		l.unlock();
		condVar_.notify_all();
		for (auto &thread: threads_)
			thread.join();
	}

	template<class Func> auto exec(Func &&f) {
		auto promise = std::make_shared<std::promise<decltype(f())>>();
		auto future = promise->get_future();
		std::unique_lock l(mutex_);
		tasks_.emplace_back([promise = std::move(promise), f = std::forward<Func>(f)] {
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

	template<class FuncIterator, class CreateTask>
	auto execTasks(FuncIterator funcIterator, CreateTask createTask) {
		using Result = decltype(createTask(*funcIterator())());
		std::vector<std::future<Result>> futures;
		iterateFunc(funcIterator, [&] (const auto &params) {
			futures.push_back(exec(createTask(params)));
		});
		std::vector<Result> results;
		results.reserve(futures.size());
		for (auto &future: futures)
			results.push_back(future.get());
		return results;
	}

private:
	bool stopped_{false};
	std::mutex mutex_;
	std::condition_variable condVar_;
	std::vector<std::thread> threads_;
	std::deque<std::function<void()>> tasks_;
};

}
