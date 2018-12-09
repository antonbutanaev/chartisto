#include <unistd.h>
#include <iostream>
#include <fstream>

using namespace std;

int main(int ac, char **av) {
	if (ac != 2) {
		cerr << "Usage: " << av[0] << " file" << endl;
		return 1;
	}
	ofstream ofs(av[1] + string(".") + to_string(getpid()), ios::binary);
	for(;;) {
		char c;
		cin.read(&c, 1);
		if(!cin)
			break;
		cout.write(&c, 1);
		ofs.write(&c, 1);
		ofs.flush();
	}
	return 0;
}
