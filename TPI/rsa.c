// TODO copy task 2.2
/* BigNum.c */
#include <stdio.h>
#include <openssl/bn.h>

#define NBITS 256

void printBN(char *msg, BIGNUM * a)
{
    /* Use BN_bn2hex(a) for hex string
     * Use BN_bn2dec(a) for decimal string */
    char * number_str = BN_bn2hex(a);
    printf("%s %s\n", msg, number_str);
    OPENSSL_free(number_str);
}

int main()
{
    BN_CTX *ctx = BN_CTX_new();

    BIGNUM *p = BN_new();
    BIGNUM *q = BN_new();
    BIGNUM *d = BN_new();
    BIGNUM *n = BN_new();
    BIGNUM *e = BN_new();

    BIGNUM *m = BN_new();
    BIGNUM *enc = BN_new();
    BIGNUM *dec = BN_new();
    BIGNUM *phi = BN_new();

    // init a, b, n
    BN_generate_prime_ex(p, NBITS, 1, NULL, NULL, NULL);
    BN_generate_prime_ex(q, NBITS, 1, NULL, NULL, NULL);

    // n = p * q
    BN_mul(n, p, q, ctx);

    // phi = (p-1) * (q-1)
    BIGNUM *p1 = BN_new();
    BIGNUM *q1 = BN_new();
    BN_sub(p1,p, BN_value_one());
    BN_sub(q1,q, BN_value_one());
    BN_mul(phi, p1, q1, ctx);

    // e = 65537 TODO check if valid
    //BN_dec2bn(&e, "65537");
    BN_hex2bn(&e, "010001");

    // d * e % phi = 1 (d = e^-1 % phi)?
    BN_mod_inverse(d, e, phi, ctx);

    BN_hex2bn(&m, "82");
    //BN_hex2bn(&one, "1");
    //BN_hex2bn(&n, "D346C54547F674360BA9E86ADDE8265B0341036320A6FDFB2EF5665E536B88982CCE582A3F0373B6EBA6CD99D98E6FA99338E075D52F473D96E36338E7445625");
    printBN(" d made = ", d);
    BN_hex2bn(&d, "710463501AF1408BD52E506677DBFCD3F295C51534F8054ADD58ADA656A6674AC850397F41B87804738AC81ED5C5D03A8ECC426154C393C504322CA8C89A7BA1");
    BN_hex2bn(&n, "B177CEA9ACAFD727D4C83DC28435E872224F8A7EA9C432764695BB64C48E4A99C7F89BCA639A00CB7A6339540399BC5D8B56C72B45DED7AAA1B514C60859AC39");
    printBN(" d found = ", d);

    printBN(" m  = ", m);

    // Encrypt
    // c = m**e mod n
    BN_mod_exp(enc, m, e, n, ctx);
    printBN("enc = ", enc);
    BN_hex2bn(&enc, "091F38DD814A1730BE69D77FEE777580D870A9FCEFC209F5A65DD0B13D64DA842B96F9125C11B2E1100A2C687FA3A94D7E16DE11B7E1F67EE5072D0D222A367E");

    // Decrypt
    BN_mod_exp(dec, enc, d, n, ctx);
    printBN("dec = ", dec);

    return 0;
}