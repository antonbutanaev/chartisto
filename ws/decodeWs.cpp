#include <iostream>

using namespace std;

int main() {
	for (;;) {
		auto finRsvCode = cin.get();
		if (!cin)
			return 0;
		cout << "finRsvCode " << finRsvCode << endl;

		auto len = cin.get();
		if (!cin)
			return 1;
		bool hasMask = len & 128;
		len &= ~128;
		cout << "hasMask " << hasMask << endl;
		cout << "len " << len << endl;

		int numBytes = len < 126? 0 : len == 126? 2 : 8;

		cout << "numBytes " << numBytes << endl;

		auto dataLen = len;
		if (numBytes) {
			dataLen = 0;
			for (int i = 0; i < numBytes; ++i) {
				const auto b = cin.get();
				if (!cin)
					return 1;
				dataLen <<= 8;
				dataLen |= b & 0xff;
			}
		}

		cout << "dataLen " << dataLen << endl;

		char mask[4] = {0};
		if (hasMask)
			cin.read(&mask[0], 4);

		for(int i = 0; i < dataLen; ++i) {
			const auto c = cin.get();
			cout << char(c ^ mask[i % 4]);
		}
		cout << endl;
	}
}
