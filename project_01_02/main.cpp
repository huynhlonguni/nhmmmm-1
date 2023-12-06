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
	BigInt p(line);

	inp >> line;
	BigInt q(line);

	inp >> line;
	BigInt e(line);

	BigInt d;

	bool res = RSA::genPrivateKeyFromPublicKey(p, q, e, d);

	ofstream out(argv[2]);

	if (res) {
		cout << d.toHexString();
		out << d.toHexString() << endl;
	}
	else {
		cout << -1;
		out << -1 << endl;
	}

	return 0;
}