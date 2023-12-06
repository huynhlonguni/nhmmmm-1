#include <iostream>
#include <vector>
#include <exception>
#include <string>
#include <sstream>
#include <random>

#ifdef PARALLEL_PRIME_CHECK
#include <thread>
#include <atomic>
#endif

using namespace std;

#define IS_POSITIVE(n) ((n).sign == 0) //can be Zero

#define IS_NEGATIVE(n) ((n).sign == 1) //can be Zero

class BigInt {
private:
	vector<char> bits;
	bool sign = 0;

private:
	void clean() {
		while (!bits.empty() && bits.back() == 0) {
			bits.pop_back();
		}
		if (bits.size() == 0) sign = 0;
	}

	void parseBinaryString(const string &bs) {
		bits.resize(bs.length());
		for (int i = 0; i < bs.length(); i++) {
			bits[i] = bs[i] - '0';
		}
	}

	void parseHexString(const string &hs) {
		const bool tbl[16][4] = {
			{0,0,0,0}, //0
			{0,0,0,1}, //1
			{0,0,1,0}, //2
			{0,0,1,1}, //3
			{0,1,0,0}, //4
			{0,1,0,1}, //5
			{0,1,1,0}, //6
			{0,1,1,1}, //7
			{1,0,0,0}, //8
			{1,0,0,1}, //9
			{1,0,1,0}, //A
			{1,0,1,1}, //B
			{1,1,0,0}, //C
			{1,1,0,1}, //D
			{1,1,1,0}, //E
			{1,1,1,1}, //F
		};

		bits.resize(hs.length() * 4);
		for (int i = 0; i < hs.length(); i++) {
			int idx = 0;
			char c = hs[i];

			if (c >= '0' && c <= '9') idx = c - '0';
			else if (c >= 'A' && c <= 'F') idx = c - 'A' + 10;

			bits[i * 4 + 0] = tbl[idx][3];
			bits[i * 4 + 1] = tbl[idx][2];
			bits[i * 4 + 2] = tbl[idx][1];
			bits[i * 4 + 3] = tbl[idx][0];
		}
	}

	void parseLongLong(long long n) {
		if (n < 0) {
			sign = 1;
			n = -n;
		}
		while (n) {
			bits.push_back(n & 1);
			n >>= 1;
		}
	}

	static bool sameSign(const BigInt &a, const BigInt &b) {
		return (a.sign == b.sign);// || (isZero(a) && isZero(b));
	}

	static bool isEqual(const BigInt &a, const BigInt &b) {
		if (isZero(a) && isZero(b)) return 1;

		return (a.sign == b.sign) && (a.bits == b.bits);
	}

	static int compare(BigInt a, BigInt b) {
		a.clean();
		b.clean();

		if (isZero(a) && isZero(b)) return 0;
		if (a.sign != b.sign) return IS_POSITIVE(a) ? 1 : -1;

		int larger = 1;
		if (a.sign) larger = -1;

		if (a.bits.size() > b.bits.size()) return larger;
		else if (a.bits.size() < b.bits.size()) return -larger;

		for (int i = a.bits.size() - 1; i >= 0; i--) {
			if (a.bits[i] > b.bits[i]) return larger;
			else if (a.bits[i] < b.bits[i]) return -larger;
		}

		return 0;
	}

	static BigInt addUnsigned(const BigInt &a, const BigInt &b) {
		BigInt res;
		int carry = 0;

		int m = max(a.bits.size(), b.bits.size());
		res.bits.resize(m + 1);

		for (int i = 0; i < m; i++) {
			int tb = a[i];
			int ob = b[i];
			int toSum = tb + ob + carry;
			res.bits[i] = toSum % 2;
			carry = toSum > 1;
		}
		if (carry) res.bits[m] = 1;
		else res.clean();
		return res;
	}

	static BigInt subUnsinged(const BigInt &large, const BigInt &small) {
		BigInt res;
		int carry = 0;

		res.bits.resize(large.bits.size());

		for (int i = 0; i < large.bits.size(); i++) {
			int lg = large.bits[i]; //no check OOB
			int sm = small[i]; //check for OOB
			int toSub = lg - carry - sm;
			carry = toSub < 0;

			res.bits[i] = toSub == 1 || toSub == -1;
		}
		res.clean();
		return res;
	}

	static void divideUnsigned(const BigInt &a, BigInt b, BigInt &q, BigInt &r) {
		int msb = 0;
		if ((msb = isPowerOfTwo(b)) != -1) {
			q = a >> msb;
			r = a;
			r.bits.resize(msb);
			r.clean();
			return;
		}

		if (a < b) {
			q = 0;
			r = a;
			return;
		}

		if (a == b) {
			q = 1;
			r = 0;
			return;
		}

		q = 0;
		q.bits.resize(a.bits.size());
		r = 0;
		for (int i = a.bits.size() - 1; i >= 0; i--) {
			r <<= 1;
			r.bits[0] = a.bits[i];
			r.clean();
			if (r >= b) {
				r = subUnsinged(r, b);
				q.bits[i] = 1;
			}
		}
		q.clean();
		r.clean();
	}

	static void divide(BigInt a, BigInt b, BigInt &q, BigInt &r) {
		if (isZero(b)) {
			throw logic_error("division by zero");
			// q = a;
			// r = 0;
			// return;
		}
		if (isZero(a)) {
			q = r = 0;
			return;
		}

		if (IS_NEGATIVE(b)) {
			divide(a, -b, q, r);
			q = -q;
			return;
		}

		if (IS_NEGATIVE(a)) {
			divide(-a, b, q, r);
			q = -q;
			if (isZero(r)) {
				return;
			}
			q = q - 1;
			r = b - r;
			return;
		}
		divideUnsigned(a, b, q, r);
	}

	static bool millerRabinWitness(const BigInt &n, const BigInt &n1, const BigInt &d, const int &s, const BigInt &base) {
		BigInt x = powMod(base, d, n);
		BigInt y;
		for (int j = 0; j < s; j++) {
			y = mulMod(x, x, n);
			if (y == 1 && x != 1 && x != n1)
				return 0;
			x = y;
		}
		if (y != 1)
			return 0;

		return 1;
	}

	static bool millerRabin(const BigInt &n, int times = 7) {
		//https://en.wikipedia.org/wiki/Miller%E2%80%93Rabin_primality_test#Testing_against_small_sets_of_bases
		const int deterministic_32[] = {
			2, 7, 61
		};

		const int deterministic_64[] = {
			2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37
		};

		bool less_than_32 = n.bits.size() <= 32;
		bool less_than_64 = n.bits.size() <= 64;

		if (less_than_32) times = sizeof(deterministic_32) / sizeof(int);
		else if (less_than_64) times = sizeof(deterministic_64) / sizeof(int);

		BigInt n1 = n - 1;
		BigInt n2 = n - 2;
		BigInt d = n1;
		int s = 0;

		while (d % 2 == 0) {
			d >>= 1;
			s++;
		}

#ifdef PARALLEL_PRIME_CHECK
		vector<thread> threads;
		atomic<bool> prime(1);
#endif

		for (int i = 0; i < times; i++) {
			BigInt a;
			if (less_than_32) {
				a = deterministic_32[i];
				if (a == n) continue; //This is for sure a prime, but early return messes with threads and it's cheap to skip it 
			}
			else if (less_than_64) {
				a = deterministic_64[i];
				if (a == n) continue; //Same as above
			}
			else
				a = rand(2, n2);

#ifdef PARALLEL_PRIME_CHECK
			threads.emplace_back([n, n1, d, s, a, &prime]() {
				bool res = millerRabinWitness(n, n1, d, s, a);

				if (!res) //One test failed
					prime = 0;
			});
#else
			if (!millerRabinWitness(n, n1, d, s, a)) return 0;
#endif
		}

#ifdef PARALLEL_PRIME_CHECK
		//Wait for threads to finish
		for (thread &t: threads) {
			t.join();
		}

		return prime;
#else
		return 1;
#endif
	}

public:
	BigInt() {}

	BigInt(const BigInt&) = default;

	BigInt(const string &s, int base = 16) {
		switch (base) {
			case 16:
				parseHexString(s);
				break;
			case 10:
				break;
			case 2:
				parseBinaryString(s);
				break;
		}
		clean();
	}

	BigInt(const long long n) {
		parseLongLong(n);
	}

	//Asignment
	BigInt& operator=(const BigInt &other) = default;

	~BigInt() = default;

	//Math
	BigInt operator+() const {
		return *this;
	}

	BigInt operator-() const {
		BigInt res = *this;

		if (isZero(res)) res.sign = 0;
		else res.sign = !res.sign;

		return res;
	}

	BigInt operator+(const BigInt &other) const {
		if (!sameSign(*this, other)) {
			if (IS_NEGATIVE(other)) return *this - (-other);
			else return other - (-*this);
		}

		BigInt res = addUnsigned(*this, other);
		res.sign = this->sign;

		return res;
	}

	BigInt operator-(const BigInt &other) const {
		if (!sameSign(*this, other)) {
			return *this + (-other);
		}

		BigInt lhs = abs(*this);
		BigInt rhs = abs(other);

		const bool bigger = lhs > rhs;
		BigInt &large = bigger ? lhs : rhs;
		BigInt &small = bigger ? rhs : lhs;

		BigInt res = subUnsinged(large, small);
		res.sign = !bigger ^ (this->sign);

		return res;
	}

	BigInt operator*(const BigInt &other) const {
		if (isZero(*this) || isZero(other))
			return BigInt(0);

		BigInt res;

		int msb = 0;
		if ((msb = isPowerOfTwo(*this)) != -1) {
			res = other << msb;
			res.sign = (this->sign != other.sign);
			return res;
		}

		if ((msb = isPowerOfTwo(other)) != -1) {
			res = *this << msb;
			res.sign = (this->sign != other.sign);
			return res;
		}

		BigInt rhs = abs(other);

		for (int i = 0; i < this->bits.size(); i++) {
			if (this->bits[i])
				res = res + rhs;
			rhs <<= 1;
		}

		res.sign = (this->sign != other.sign);

		return res;
	}

	BigInt operator/(const BigInt &other) const {
		BigInt q, r;
		divide(*this, other, q, r);
		return q;
	}

	BigInt operator%(const BigInt &other) const {
		if (other == 2) { //Even odd check
			return (*this)[0];
		}

		BigInt q, r;
		divide(*this, other, q, r);
		return r;
	}

	static BigInt abs(BigInt a) {
		a.sign = 0;
		return a;
	}

	static BigInt addMod(const BigInt &a, const BigInt &b, const BigInt &n) {
		BigInt tmp = addUnsigned(a, b);
		if (tmp < n) return tmp;
		return subUnsinged(tmp, n); //should be tmp % n, but assume tmp wont be too large
	}

	static BigInt mulMod(BigInt a, const BigInt &b, const BigInt &n) {
		BigInt P;

		if (b.bits[0])
			P = a;

		for (int i = 1; i < b.bits.size(); i++) {
			// a = addMod(a << 1, 0, n); // 2 * a % n
			a <<= 1;
			if (a >= n) a = subUnsinged(a, n);

			if (b.bits[i])
				P = addMod(P, a, n);
		}
		return P;
	}

	static BigInt powMod(const BigInt &a, const BigInt &b, const BigInt &n) {
		BigInt y = 1;

		for (int i = b.bits.size() - 1; i >= 0; i--) {
			y = mulMod(y, y, n); // y ^ 2 % n
			if (b.bits[i])
				y = mulMod(y, a, n);
		}
		return y;
	}

	static BigInt inverseMod(BigInt a, BigInt n) {
		if (n == 1)
			return 0;

		BigInt n0 = n;
		BigInt x = 1;
		BigInt y = 0;

		while (a > 1) {
			BigInt q, r;
			divideUnsigned(a, n, q, r);

			a = n;
			n = r;

			BigInt tmp = y;
			y = x - q * y;
			x = tmp;
		}

		if (x < 0)
			x = x + n0;

		return x;
	}

	static BigInt gcd(BigInt a, BigInt b) {
		if (isZero(a)) return b;
		if (isZero(b)) return a;

		int k = 1;

		int aMsb = firstSetBit(a);
		if (aMsb < 0) aMsb = 0;
		int bMsb = firstSetBit(b);
		if (bMsb < 0) bMsb = 0;
		const int shareMsb = min(aMsb, bMsb);

		a >>= aMsb;
		b >>= bMsb;

		while (a != b) {
			if (a < b)
				swap(a, b);

			a = a - b;
			while (a % 2 == 0) a >>= 1;
		}

		return a << shareMsb;
	}

	//Comparisions
	bool operator<(const BigInt &other) const {
		return compare(*this, other) == -1;
	}

	bool operator>(const BigInt &other) const {
		return compare(*this, other) == 1;
	}

	bool operator<=(const BigInt &other) const {
		return !(*this > other);
	}

	bool operator>=(const BigInt &other) const {
		return !(*this < other);
	}

	bool operator==(const BigInt &other) const {
		return isEqual(*this, other);
	}

	bool operator!=(const BigInt &other) const {
		return !(*this == other);
	}

	//Bit Shift
	BigInt operator<<(int pos) const {
		BigInt res = *this;
		while (pos--)
			res.bits.insert(res.bits.begin(), 0);
		return res;
	}

	BigInt operator>>(int pos) const {
		BigInt res = *this;
		while (pos--) {
			if (res.bits.size() == 0) break;
			res.bits.erase(res.bits.begin());
		}
		return res;
	}

	BigInt& operator<<=(int pos) {
		while (pos--)
			bits.insert(bits.begin(), 0);
		return *this;
	}

	BigInt& operator>>=(int pos) {
		while (pos--) {
			if (bits.size() == 0) break;
			bits.erase(bits.begin());
		}
		return *this;
	}

	bool operator[](int index) const {
		if (index < bits.size()) return bits[index];
		return 0;
	}

	//Identity
	static bool isZero(BigInt n) {
		n.clean();
		return n.bits.size() == 0;
	}

	static int firstSetBit(const BigInt &n) {
		for (int i = 0; i < n.bits.size(); i++) {
			if (n.bits[i])
				return i;
		}
		return -1;
	}

	//return -1 if not, else return MSB index
	static int isPowerOfTwo(BigInt n) {
		int cnt = 0;
		int msb = 0;
		for (int i = 0; i < n.bits.size(); i++) {
			if (n.bits[i]) msb = i;
			cnt += n.bits[i];
		}
		if (cnt != 1) return -1;
		else return msb;
	}

	static bool isPrime(const BigInt &n) {
		if (n < 2) return 0;
		if (n == 2) return 1;
		if (n % 2 == 0) return 0;
		return millerRabin(n);
	}

	//Utils
	static BigInt rand(int size, bool enforce_size = 0) {
		static bool init = 0;
		static random_device rd;
		static mt19937 eng;
		static uniform_int_distribution<int> dist(0,1);

		if (!init) {
			eng.seed(rd()); //seed once
			init = 1;
		}

		BigInt res;
		res.bits.resize(size);

		if (enforce_size) size--;

		for (int i = 0; i < size; i++)
			res.bits[i] = dist(eng);

		if (enforce_size)
			res.bits[size] = 1; //Set MSB

		res.clean();
		return res;
	}

	static BigInt rand(const BigInt &low, const BigInt &high) {
		BigInt tmp = rand(high.bits.size());
		return tmp % (high - low) + low;
	}

	//IO
	friend ostream& operator<<(ostream &os, const BigInt &n) {
		const int maxBit = sizeof(long long) * 8 - 1; //63 bit
		if (n.bits.size() <= maxBit)
			os << n.toLongLong();
		else
			os << n.toDecString();
		return os;
	}

	long long toLongLong() const {
		long long res = 0;
		for (int i = min((size_t)63, bits.size()) - 1; i >= 0; i--)
			res = (res << 1) | bits[i];
		if (sign) res *= -1;
		return res;
	}

	string toDecString() const {
		stringstream builder;

		BigInt n = abs(*this);
		while (n != 0) {
			BigInt q, r;
			divideUnsigned(n, 10, q, r);

			builder << r.toLongLong();
			n = q;
		}

		if (sign) builder << "-";

		string reversed = builder.str();
		string res(reversed.rbegin(), reversed.rend());

		return res;
	}

	string toHexString(bool displaySign = 0) const {
		if (isZero(*this)) return "0";

		const char tbl[16] = {
			'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
		};

		stringstream builder;

		if (displaySign && sign) builder << "-";

		for (int i = 0; i < bits.size(); i += 4) {
			char c = 0;
			c |= (*this)[i + 0] << 0;
			c |= (*this)[i + 1] << 1;
			c |= (*this)[i + 2] << 2;
			c |= (*this)[i + 3] << 3;
			builder << tbl[c];
		}

		return builder.str();
	}
};