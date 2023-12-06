#include "BigInt.h"
#include <time.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <iostream>
using namespace std;

/* Assumes srand() has been called with an appropriate seed at some point
   Code assumes C99 is available; minor tweaks needed for older compilers.
 */
int gen_random_int() {
    const int BITS_PER_RAND = (int)(log2(RAND_MAX/2 + 1) + 1.0); /* Or log(RAND_MAX + 1) / log(2) with older language standards */
    int ret = 0;
    for (int i = 0; i < sizeof(int) * CHAR_BIT; i += BITS_PER_RAND) {
        ret <<= BITS_PER_RAND;
        ret |= rand();
    }
    return ret;
}

void test(long long a, long long b) {
	BigInt BigA(a);
	BigInt BigB(b);

	if (BigA + BigB != a + b) {
		cout << "Failed +: " << a << ", " << b << endl;
		cout << "Expected: " << a + b << endl;
		cout << "Outputed: " << BigA + BigB << endl;
		cout << endl;
	}

	if (BigA - BigB != a - b) {
		cout << "Failed -: " << a << ", " << b << endl;
		cout << "Expected: " << a - b << endl;
		cout << "Outputed: " << BigA - BigB << endl;
		cout << endl;
	}

	if (BigA * BigB != a * b) {
		cout << "Failed *: " << a << ", " << b << endl;
		cout << "Expected: " << a * b << endl;
		cout << "Outputed: " << BigA * BigB << endl;
		cout << endl;
	}

	if (b == 0)
		return;

	BigInt q = BigA / BigB;
	BigInt r = BigA % BigB;

	if (q * BigB + r != BigA) {
		cout << "Failed /, %: " << a << ", " << b << endl;
		cout << "Q: " << q << endl;
		cout << "R: " << r << endl;
		cout << endl;
	}

	if (a == b && BigA != BigB) {
		cout << "Failed ==: " << a << ", " << b << endl;
		cout << endl;
	}

	if (a < b && BigA > BigB) {
		cout << "Failed <>: " << a << ", " << b << endl;
		cout << endl;
	}

	// BigInt prime(2147483647);

	// BigInt inv = BigInt::inverseMod(BigA, prime);

	// if (BigInt::gcd(BigA, prime) == 1) {
	// 	if (inv * BigA % prime != 1) {
	// 		cout << "Failed inverse prime: " << a << endl;
	// 		cout << endl;
	// 	}
	// }
}

int main() {
	int n = 5000;
	srand(time(NULL));

	// for (int i = 0; i < n; i++) {
	// 	long long a = gen_random_int();
	// 	long long b = gen_random_int();

	// 	if (i % 500 == 0) {
	// 	}

	// 	test(a, b);
	// }

	for (int i = -1000; i <= 1000; i++) {
		for (int j = -1000; j <= 1000; j++) {
			test(i, j);
		}
	}

}