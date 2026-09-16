#include <iostream>
#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <gmp.h>
#include <cstring>


#ifdef _MSC_VER 
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

// ============================================================================
// ALGORYTM SHA-256
// ============================================================================
class SHA256 {
private:
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t data[64];
    uint32_t datalen;

    const uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    uint32_t rotr(uint32_t x, uint32_t n) { return (x >> n) | (x << (32 - n)); }
    uint32_t choose(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (~x & z); }
    uint32_t majority(uint32_t x, uint32_t y, uint32_t z) { return (x & y) ^ (x & z) ^ (y & z); }
    uint32_t sig0(uint32_t x) { return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3); }
    uint32_t sig1(uint32_t x) { return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10); }
    uint32_t SIG0(uint32_t x) { return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22); }
    uint32_t SIG1(uint32_t x) { return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25); }

    void transform() {
        uint32_t a, b, c, d, e, f, g, h, i, j, t1, t2, m[64];

        for (i = 0, j = 0; i < 16; ++i, j += 4)
            m[i] = (data[j] << 24) | (data[j + 1] << 16) | (data[j + 2] << 8) | (data[j + 3]);
        for (; i < 64; ++i)
            m[i] = sig1(m[i - 2]) + m[i - 7] + sig0(m[i - 15]) + m[i - 16];

        a = state[0]; b = state[1]; c = state[2]; d = state[3];
        e = state[4]; f = state[5]; g = state[6]; h = state[7];

        for (i = 0; i < 64; ++i) {
            t1 = h + SIG1(e) + choose(e, f, g) + K[i] + m[i];
            t2 = SIG0(a) + majority(a, b, c);
            h = g; g = f; f = e;
            e = d + t1;
            d = c; c = b; b = a;
            a = t1 + t2;
        }

        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;
    }

public:
    SHA256() {
        state[0] = 0x6a09e667; state[1] = 0xbb67ae85; state[2] = 0x3c6ef372; state[3] = 0xa54ff53a;
        state[4] = 0x510e527f; state[5] = 0x9b05688c; state[6] = 0x1f83d9ab; state[7] = 0x5be0cd19;
        bitlen = 0;
        datalen = 0;
    }

    void update(const uint8_t *txt, size_t len) {
        for (size_t i = 0; i < len; ++i) {
            data[datalen] = txt[i];
            datalen++;
            if (datalen == 64) {
                transform();
                bitlen += 512;
                datalen = 0;
            }
        }
    }

    void final(uint8_t hash[32]) {
        uint32_t i = datalen;

        if (datalen < 56) {
            data[i++] = 0x80;
            while (i < 56) data[i++] = 0x00;
        } else {
            data[i++] = 0x80;
            while (i < 64) data[i++] = 0x00;
            transform();
            memset(data, 0, 56);
        }

        bitlen += datalen * 8;
        data[63] = static_cast<uint8_t>(bitlen);
        data[62] = static_cast<uint8_t>(bitlen >> 8);
        data[61] = static_cast<uint8_t>(bitlen >> 16);
        data[60] = static_cast<uint8_t>(bitlen >> 24);
        data[59] = static_cast<uint8_t>(bitlen >> 32);
        data[58] = static_cast<uint8_t>(bitlen >> 40);
        data[57] = static_cast<uint8_t>(bitlen >> 48);
        data[56] = static_cast<uint8_t>(bitlen >> 56);
        transform();

        for (i = 0; i < 4; ++i) {
            hash[i]      = (state[0] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 4]  = (state[1] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 8]  = (state[2] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 12] = (state[3] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 16] = (state[4] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 20] = (state[5] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 24] = (state[6] >> (24 - i * 8)) & 0x000000ff;
            hash[i + 28] = (state[7] >> (24 - i * 8)) & 0x000000ff;
        }
    }
};

// ============================================================================
// SPRZĘTOWY GENERATOR LICZB LOSOWYCH (TRNG)
// ============================================================================
static uint64_t g_entropy_pool = 0;

uint64_t get_raw_jitter_v1() {
    uint64_t dynamic_limit = 100 + (g_entropy_pool & 0xFF);
    uint64_t t1 = __rdtsc();
    for (volatile uint64_t i = 0; i < dynamic_limit; ++i) {
        if (i % 7 == 0) { g_entropy_pool ^= i; } 
    }
    uint64_t t2 = __rdtsc();
    uint64_t delta = t2 - t1;
    g_entropy_pool ^= delta;
    return delta;
}

uint32_t get_raw_bit() {
    uint32_t combined_bit = 0;
    for(int i = 0; i < 32; ++i) {
        uint64_t d = get_raw_jitter_v1();
        combined_bit ^= (uint32_t)((d ^ (d >> 1) ^ (d >> 4)) & 1);
    }
    return combined_bit;
}

uint32_t get_secure_bit() {
    while (true) {
        uint32_t b1 = get_raw_bit();
        uint32_t b2 = get_raw_bit();
        if (b1 != b2) return b1;
    }
}

uint32_t generate_trng_32() {
    uint32_t result = 0;
    for (int i = 0; i < 32; ++i) {
        result = (result << 1) | get_secure_bit();
    }
    return result;
}

uint64_t generate_trng_64() {
    uint64_t result = 0;
    for (int i = 0; i < 64; ++i) {
        result = (result << 1) | get_secure_bit();
    }
    return result;
}

void generate_large_random(mpz_t result, int bits) {
    mpz_set_ui(result, 0); 
    int iterations = bits / 64;
    for (int i = 0; i < iterations; ++i) {
        uint64_t chunk = generate_trng_64(); 
        mpz_t mpz_chunk;
        mpz_init(mpz_chunk);
        mpz_import(mpz_chunk, 1, 1, sizeof(chunk), 0, 0, &chunk);
        mpz_mul_2exp(result, result, 64);
        mpz_ior(result, result, mpz_chunk);
        mpz_clear(mpz_chunk);
    }
}

// ============================================================================
// MATEMATYKA RSA
// ============================================================================
void do_rsa(mpz_t wynik, mpz_t wiadomosc_liczba, mpz_t wykladnik, mpz_t modul) {
    if (mpz_cmp(wiadomosc_liczba, modul) >= 0) {
        std::cerr << "Blad: Dane sa za duze dla tego klucza!" << std::endl;
        return;
    }
    mpz_powm(wynik, wiadomosc_liczba, wykladnik, modul);
}

void menu_generuj_klucze() {
    mpz_t p, q, n, phi, e, d, p_minus_1, q_minus_1;
    mpz_inits(p, q, n, phi, e, d, p_minus_1, q_minus_1, NULL);

    std::cout << "Generowanie kluczy (moze to chwilke potrwac)..." << std::endl;
    do { generate_large_random(p, 1024); mpz_setbit(p, 0); } while (mpz_probab_prime_p(p, 25) == 0); 
    do { generate_large_random(q, 1024); mpz_setbit(q, 0); } while (mpz_probab_prime_p(q, 25) == 0); 

    mpz_mul(n, p, q);
    mpz_sub_ui(p_minus_1, p, 1);
    mpz_sub_ui(q_minus_1, q, 1);
    mpz_mul(phi, p_minus_1, q_minus_1);
    mpz_set_ui(e, 65537);

    if (mpz_invert(d, e, phi) == 0) {
        std::cerr << "Blad matematyczny przy generowaniu klucza prywatnego!" << std::endl;
        mpz_clears(p, q, n, phi, e, d, p_minus_1, q_minus_1, NULL);
        return;
    }

    FILE* pub_file = fopen("public_key.txt", "w");
    if (pub_file) {
        mpz_out_str(pub_file, 10, e);
        fprintf(pub_file, "\n");
        mpz_out_str(pub_file, 10, n);
        fclose(pub_file);
        std::cout << "Zapisano klucz publiczny do public_key.txt" << std::endl;
    }

    FILE* priv_file = fopen("private_key.txt", "w");
    if (priv_file) {
        mpz_out_str(priv_file, 10, d);
        fprintf(priv_file, "\n");
        mpz_out_str(priv_file, 10, n);
        fclose(priv_file);
        std::cout << "Zapisano klucz prywatny do private_key.txt" << std::endl;
    }

    mpz_clears(p, q, n, phi, e, d, p_minus_1, q_minus_1, NULL);
}

void menu_szyfruj_plik() {
    mpz_t e, n, M, C;
    mpz_inits(e, n, M, C, NULL);

    FILE* pub_file = fopen("public_key.txt", "r");
    if (!pub_file) {
        std::cerr << "Blad: Brak pliku public_key.txt! Wygeneruj klucze najpierw." << std::endl;
        mpz_clears(e, n, M, C, NULL);
        return;
    }
    mpz_inp_str(e, pub_file, 10);
    mpz_inp_str(n, pub_file, 10);
    fclose(pub_file);

    std::string src_name, dest_name;
    std::cout << "Podaj nazwe pliku do zaszyfrowania (np. dane.txt): ";
    std::cin >> src_name;
    std::cout << "Podaj nazwe pliku wynikowego (np. zaszyfrowane.txt): ";
    std::cin >> dest_name;

    std::ifstream file(src_name, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Blad: Nie mozna otworzyc pliku zrodlowego!" << std::endl;
        mpz_clears(e, n, M, C, NULL);
        return;
    }
    std::vector<unsigned char> file_bytes((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (file_bytes.size() > 240 || file_bytes.empty()) {
        std::cerr << "Blad: Plik pusty lub za duzy dla czystego RSA (max 240 bajtow)!" << std::endl;
        mpz_clears(e, n, M, C, NULL);
        return;
    }

    mpz_import(M, file_bytes.size(), 1, sizeof(file_bytes[0]), 0, 0, file_bytes.data());
    do_rsa(C, M, e, n);

    FILE* out_file = fopen(dest_name.c_str(), "w");
    if (out_file) {
        mpz_out_str(out_file, 10, C);
        fclose(out_file);
        std::cout << "Plik zostal pomyslnie zaszyfrowany." << std::endl;
    }
    mpz_clears(e, n, M, C, NULL);
}

void menu_odszyfruj_plik() {
    mpz_t d, n, C, M;
    mpz_inits(d, n, C, M, NULL);

    FILE* priv_file = fopen("private_key.txt", "r");
    if (!priv_file) {
        std::cerr << "Blad: Brak pliku private_key.txt!" << std::endl;
        mpz_clears(d, n, C, M, NULL);
        return;
    }
    mpz_inp_str(d, priv_file, 10);
    mpz_inp_str(n, priv_file, 10);
    fclose(priv_file);

    std::string src_name, dest_name;
    std::cout << "Podaj nazwe pliku zaszyfrowanego (np. zaszyfrowane.txt): ";
    std::cin >> src_name;
    std::cout << "Podaj nazwe pliku dla odzyskanego tekstu (np. odzyskane.txt): ";
    std::cin >> dest_name;

    FILE* in_file = fopen(src_name.c_str(), "r");
    if (!in_file) {
        std::cerr << "Blad: Nie mozna otworzyc pliku z szyfrogramem!" << std::endl;
        mpz_clears(d, n, C, M, NULL);
        return;
    }
    if (mpz_inp_str(C, in_file, 10) == 0) {
        std::cerr << "Blad: Niepoprawny format szyfrogramu!" << std::endl;
        fclose(in_file);
        mpz_clears(d, n, C, M, NULL);
        return;
    }
    fclose(in_file);

    do_rsa(M, C, d, n);

    size_t odzyskany_rozmiar;
    unsigned char* odzyskane_bajty = (unsigned char*)mpz_export(NULL, &odzyskany_rozmiar, 1, 1, 0, 0, M);

    std::ofstream out_file(dest_name, std::ios::binary);
    if (out_file.is_open()) {
        out_file.write(reinterpret_cast<char*>(odzyskane_bajty), static_cast<std::streamsize>(odzyskany_rozmiar));
        out_file.close();
        std::cout << "Plik pomyslnie odszyfrowany." << std::endl;
    }
    free(odzyskane_bajty);
    mpz_clears(d, n, C, M, NULL);
}

// ============================================================================
// PODPISYWANIE I WERYFIKACJA (SHA-256 + RSA)
// ============================================================================
void menu_podpisz_plik() {
    mpz_t d, n, M, S;
    mpz_inits(d, n, M, S, NULL);

    std::string priv_key_name, src_name, sig_name;
    std::cout << "Podaj nazwe pliku z kluczem prywatnym (np. private_key.txt): ";
    std::cin >> priv_key_name;

    FILE* priv_file = fopen(priv_key_name.c_str(), "r");
    if (!priv_file) {
        std::cerr << "Blad: Nie mozna otworzyc pliku z kluczem prywatnym!" << std::endl;
        mpz_clears(d, n, M, S, NULL);
        return;
    }
    mpz_inp_str(d, priv_file, 10);
    mpz_inp_str(n, priv_file, 10);
    fclose(priv_file);

    std::cout << "Podaj nazwe pliku do podpisania (DOWOLNY ROZMIAR): ";
    std::cin >> src_name;
    std::cout << "Podaj nazwe dla pliku podpisu (np. podpis.txt): ";
    std::cin >> sig_name;

    std::ifstream file(src_name, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Blad: Nie mozna otworzyc pliku!" << std::endl;
        mpz_clears(d, n, M, S, NULL);
        return;
    }

    SHA256 sha;
    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        sha.update(reinterpret_cast<const uint8_t*>(buffer), static_cast<size_t>(file.gcount()));
    }
    file.close();

    uint8_t hash_wynik[32];
    sha.final(hash_wynik);

    mpz_import(M, 32, 1, 1, 0, 0, hash_wynik);
    do_rsa(S, M, d, n);

    FILE* out_file = fopen(sig_name.c_str(), "w");
    if (out_file) {
        mpz_out_str(out_file, 10, S);
        fclose(out_file);
        std::cout << "Wygenerowano bezpieczny podpis SHA-256/RSA!" << std::endl;
    }
    mpz_clears(d, n, M, S, NULL);
}

void menu_weryfikuj_podpis() {
    mpz_t e, n, S, M_odzyskane, M_aktualne;
    mpz_inits(e, n, S, M_odzyskane, M_aktualne, NULL);

    std::string pub_key_name, src_name, sig_name;
    std::cout << "Podaj nazwe pliku z kluczem publicznym (np. public_key.txt): ";
    std::cin >> pub_key_name;

    FILE* pub_file = fopen(pub_key_name.c_str(), "r");
    if (!pub_file) {
        std::cerr << "Blad: Nie mozna otworzyc pliku z kluczem publicznym!" << std::endl;
        mpz_clears(e, n, S, M_odzyskane, M_aktualne, NULL);
        return;
    }
    mpz_inp_str(e, pub_file, 10);
    mpz_inp_str(n, pub_file, 10);
    fclose(pub_file);

    std::cout << "Podaj nazwe weryfikowanego pliku: ";
    std::cin >> src_name;
    std::cout << "Podaj nazwe pliku z podpisem (np. podpis.txt): ";
    std::cin >> sig_name;

    FILE* s_file = fopen(sig_name.c_str(), "r");
    if (!s_file) {
        std::cerr << "Blad: Brak pliku podpisu!" << std::endl;
        mpz_clears(e, n, S, M_odzyskane, M_aktualne, NULL);
        return;
    }
    mpz_inp_str(S, s_file, 10);
    fclose(s_file);

    do_rsa(M_odzyskane, S, e, n);

    std::ifstream file(src_name, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Blad: Nie mozna otworzyc sprawdzanego pliku!" << std::endl;
        mpz_clears(e, n, S, M_odzyskane, M_aktualne, NULL);
        return;
    }

    SHA256 sha;
    char buffer[4096];
    while (file.read(buffer, sizeof(buffer)) || file.gcount() > 0) {
        sha.update(reinterpret_cast<const uint8_t*>(buffer), static_cast<size_t>(file.gcount()));
    }
    file.close();

    uint8_t hash_wynik[32];
    sha.final(hash_wynik);

    mpz_import(M_aktualne, 32, 1, 1, 0, 0, hash_wynik);

    std::cout << "\n[WERYFIKACJA MATEMATYCZNA SHA-256 + RSA]" << std::endl;
    if (mpz_cmp(M_odzyskane, M_aktualne) == 0) {
        std::cout << ">>> SUKCES: INTEGRALNOSC I AUTENTYCZNOSC POTWIERDZONA <<<" << std::endl;
    } else {
        std::cout << ">>> ALARM: MODYFIKACJA DANYCH LUB FALSYFIKAT PODPISU! <<<" << std::endl;
    }

    mpz_clears(e, n, S, M_odzyskane, M_aktualne, NULL);
}

// ============================================================================
// MAIN - ZINTEGROWANE MENU
// ============================================================================
int main() {
    int wybor = 0;
    do {
        std::cout << "\n=== SYSTEM KRYPTOGRAFICZNY TRNG / RSA-2048 / SHA-256 ===" << std::endl;
        std::cout << "1. Generuj nowa pare kluczy RSA" << std::endl;
        std::cout << "2. Szyfruj maly plik (RSA)" << std::endl;
        std::cout << "3. Odszyfruj maly plik (RSA)" << std::endl;
        std::cout << "4. Podpisz plik (SHA-256 + RSA)" << std::endl;
        std::cout << "5. Weryfikuj podpis pliku (SHA-256 + RSA)" << std::endl;
        std::cout << "6. Wyjscie" << std::endl;
        std::cout << "Wybierz opcje (1-6): ";
        
        if (!(std::cin >> wybor)) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            continue;
        }
        std::cout << "----------------------------------------------------" << std::endl;

        switch (wybor) {
            case 1: menu_generuj_klucze(); break;
            case 2: menu_szyfruj_plik(); break;
            case 3: menu_odszyfruj_plik(); break;
            case 4: menu_podpisz_plik(); break;
            case 5: menu_weryfikuj_podpis(); break;
            case 6: std::cout << "Koniec programu." << std::endl; break;
            default: std::cout << "Niepoprawny wybor!" << std::endl;
        }
    } while (wybor != 6);

    return 0;
}