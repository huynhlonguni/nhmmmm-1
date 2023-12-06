
class RSA {
public:
	static bool genPrivateKeyFromPublicKey(const BigInt &p, const BigInt &q, const BigInt &e, BigInt &d) {
		BigInt phi = (p - 1) * (q - 1);

		if (BigInt::gcd(e, phi) != 1) return 0;

		d = BigInt::inverseMod(e, phi);

		if (d == 0) {
			// cout << "Met" << endl;
			return 0;
		}

		return 1;
	}
};