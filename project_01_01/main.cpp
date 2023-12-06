#include <fstream>
#define PARALLEL_PRIME_CHECK
#include "../BigInt.h"

int main(int argc, char const *argv[]) {
	if (argc < 3) {
		cout << argv[0] << " <input file> <output file>" << endl;
		exit(1);
	}

	ifstream inp(argv[1]);
	string line;
	inp >> line;

	BigInt num(line);

	bool res = BigInt::isPrime(num);

	ofstream out(argv[2]);
	out << res << endl;

	cout << res;
	
	return 0;
}