#include <fstream>
#include "../BigInt.h"
#include "../RSA.h"

int main(int argc, char const *argv[]) {
	if (argc < 3) {
		cout << argv[0] << " <input file> <output file>" << endl;
		exit(1);
	}

	ifstream inp(argv[1]);
	string line;

	inp >> line;
	BigInt N(line);

	inp >> line;
	BigInt k(line);

	inp >> line;
	BigInt x(line);

	BigInt y = BigInt::powMod(x, k, N);

	ofstream out(argv[2]);
	out << y.toHexString() << endl;

	cout << y.toHexString();

	return 0;
}