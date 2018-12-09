#include <chrono>
#include <mutex>
#include <thread>
#include <iostream>

using namespace std;

int main(int ac, char **av) {
	if (ac != 3) {
		cerr << "Usage: " << av[0] << " from to" << endl;
		return 1;
	}

	string in = av[1];
	string out = av[2];

	mutex m;
	bool done = false;
	auto lastWrite = chrono::steady_clock::now();
	string buf;

	auto writeBuf = [&]{
		cout << buf;
		cout.flush();
		buf.clear();
	};

	thread th = thread([&]{
		while (!done) {
			auto now = chrono::steady_clock::now();
			{
				lock_guard<mutex> l(m);
				if (now - lastWrite > chrono::milliseconds(100) && !buf.empty()) {
					lastWrite = now;
					writeBuf();
				}
			}
			cout.flush();
			this_thread::sleep_for(chrono::milliseconds(25));
		}
	});

	for(;;) {
		char c;
		cin.read(&c, 1);
		if(!cin)
			break;

		lock_guard<mutex> l(m);
		buf.push_back(c);
		if (buf.size() == in.size()) {
			if (buf == in)
				buf = out;

			cout.write(&buf.front(), 1);
			buf.erase(buf.begin());
			lastWrite = chrono::steady_clock::now();
		}
	}

	{
		lock_guard<mutex> l(m);
		done = true;
	}
	th.join();

	writeBuf();
}
