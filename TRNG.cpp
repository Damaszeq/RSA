#include <iostream>
#include <cstdint>
#include <fstream>

#ifdef _MSC_VER 
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

uint64_t get_raw_jitter_v0() {
    uint64_t t1 = __rdtsc(); 
    //Read Time-Stamp Counter - liczba cykli od uruchomienia procesora
    for (volatile int i = 0; i < 300; ++i) { 
        if (i % 7 == 0) { } 
    } 
    uint64_t t2 = __rdtsc();
    return t2 - t1;
}

// Dodanie prostego mechanizmu mieszania (XOR) do zwiększenia entropii i trudności przewidywania
static uint64_t g_entropy_pool = 0;
uint64_t get_raw_jitter_v1() {
    int dynamic_limit = 150 + (g_entropy_pool & 0xFF); //150-405 iteracji, zależnie od aktualnej entropii
    uint64_t t1 = __rdtsc();
    for (volatile int i = 0; i < dynamic_limit; ++i) {
        if (i % 7 == 0) { g_entropy_pool ^= i; } 
    }
    uint64_t t2 = __rdtsc();
    uint64_t delta = t2 - t1;
    g_entropy_pool ^= delta;
    return delta;
}

// Dodanie prostego mechanizmu mieszania (XOR) do zwiększenia entropii i trudności przewidywania
static uint8_t g_entropy_pool2 = 0;
uint64_t get_raw_jitter_v2() {
    int dynamic_limit = 100 + (g_entropy_pool2 & 0x0F); //100-115 iteracji, zależnie od aktualnej entropii (mniejszy zakres dla szybszego generowania)
    uint64_t t1 = __rdtsc();
    for (volatile int i = 0; i < dynamic_limit; ++i) {
        if (i % 7 == 0) { g_entropy_pool2 ^= i; } 
    }
    uint64_t t2 = __rdtsc();
    uint64_t delta = t2 - t1;
    g_entropy_pool2 ^= delta;
    return delta;
}


// Ekstrakcja entropii: zbiera 32 próbki jittera i miesza je operacją XOR
uint32_t get_raw_bit() {
    uint32_t combined_bit = 0;
    for(int i = 0; i < 32; ++i) {
        uint64_t d = get_raw_jitter_v1();
        combined_bit ^= (uint32_t)((d ^ (d >> 1) ^ (d >> 4)) & 1);
    }
    return combined_bit;
}
// Korektor von Neumanna
uint32_t get_secure_bit() {
    while (true) {
        uint32_t b1 = get_raw_bit();
        uint32_t b2 = get_raw_bit();
        if (b1 != b2) return b1;
    }
}

// Generuje pełną 32-bitową liczbę z bezpiecznych bitów 
uint32_t generate_trng_32() {
    uint32_t result = 0;
    for (int i = 0; i < 32; ++i) {
        result = (result << 1) | get_secure_bit();
    }
    return result;
}
// Generuje 64-bitową liczbę z bezpiecznych bitów (opcjonalnie, jeśli potrzebna jest większa entropia)
uint64_t generate_trng_64() {
    uint64_t result = 0;
    for (int i = 0; i < 64; ++i) {
        result = (result << 1) | get_secure_bit();
    }
    return result;
}

// Testy statystyczne: rozkład bitów i brak powtórzeń sekwencyjnych
void run_quick_check(int samples = 1000) {
    int ones = 0;
    int repeats = 0;
    uint32_t last = 0;

    std::cout << "--- Test TRNG (" << samples << " probek) ---";

    for (int i = 0; i < samples; ++i) {
        uint32_t val = generate_trng_32();
        for(int b=0; b<32; ++b) {
            if ((val >> b) & 1) ones++;
        }
        if (val == last && i > 0) repeats++;
        last = val;

        if (i > 0 && i % (samples / 10) == 0) 
            std::cout << "Postep: " << (i * 100 / samples) << "%" << std::endl; 
    }

    double ratio = static_cast<double>(ones) / (samples * 32.0);
    std::cout << "\nStosunek 1/0: " << ratio << " (Idealnie: 0.5)" << std::endl;
    //std::cout << "Powtorzenia: " << repeats << std::endl;

    if (ratio > 0.49 && ratio < 0.51 && repeats == 0) {
        std::cout << "WYNIK: Sukces - wysoka jakosc entropii." << std::endl;
    } else {
        std::cout << "WYNIK: Slaba jakosc lub bias." << std::endl;
    }
}

void generate_histogram() {
    const int num_samples = 250000; // 250k * 4 bajty = 1M próbek
    uint32_t counts[256] = {0};

    std::cout << "Generowanie danych do histogramu (1M bajtow)..." << std::endl;

    for (int i = 0; i < num_samples; ++i) {
        uint32_t val = generate_trng_32();
        counts[(val >> 24) & 0xFF]++;
        counts[(val >> 16) & 0xFF]++;
        counts[(val >> 8) & 0xFF]++;
        counts[val & 0xFF]++;
    }

    std::ofstream csv("histogram.csv");
    csv << "Wartosc,Czestosc\n";
    for (int i = 0; i < 256; ++i) {
        csv << i << "," << counts[i] << "\n";
    }
    csv.close();
    std::cout << "Dane zapisano do histogram.csv" << std::endl;
}

int main() {
    int choice = 0;
    
    while (true) {
        std::cout << "\n==========================================" << std::endl;
        std::cout << "              CPU JITTER TRNG" << std::endl;
        std::cout << "==========================================" << std::endl;
        std::cout << "1. TEST statystyczny (podaj liczbe probek)" << std::endl;
        std::cout << "2. GENERUJ PLIK (wybierz format: .bin / .txt)" << std::endl;
        std::cout << "3. POJEDYNCZA liczba 32-bitowa" << std::endl;
        std::cout << "4. SERIA liczb 32-bitowych (podaj ile)" << std::endl;
        std::cout << "5. GENERUJ HISTOGRAM" << std::endl;
        std::cout << "6. ZAKONCZYC program" << std::endl;
        std::cout << "------------------------------------------" << std::endl;
        std::cout << "Twoj wybor: ";
        
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(1000, '\n');
            continue;
        }

        switch (choice) {
            case 1: {
                int samples;
                std::cout << "Podaj liczbe probek do testu: ";
                std::cin >> samples;
                if (samples > 0) run_quick_check(samples);
                else std::cout << "Liczba musi byc dodatnia!" << std::endl;
                break;
            }

            case 2: {
                int format, bitSize;
                int count = 100000;
                
                std::cout << "Podaj liczbe probek do testu: ";
                std::cin >> count;
                
                std::cout << "Wybierz typ danych: (1) 32-bit | (2) 64-bit: ";
                std::cin >> bitSize;

                std::cout << "Wybierz format zapisu: (1) BINARNY [.bin] | (2) TEKSTOWY [.txt]: ";
                std::cin >> format;
                
                std::string ext = (format == 2) ? ".txt" : ".bin";
                std::string filename = "random_data" + std::to_string(bitSize == 2 ? 64 : 32) + ext;
                
                std::ofstream outFile;
                if (format == 2) outFile.open(filename);
                else outFile.open(filename, std::ios::binary);
                
                if (!outFile) {
                    std::cerr << "Blad pliku!" << std::endl;
                } else {
                    std::cout << "Generowanie danych " << (bitSize == 2 ? "64" : "32") << "-bitowych..." << std::endl;
                    
                    for (int i = 0; i < count; ++i) {
                        // Pasek postępu
                        if (count >= 10 && i % (count / 10) == 0 && i > 0) 
                            std::cout << "Postep: " << (i * 100 / count) << "%" << std::endl; 

                        if (bitSize == 2) {
                            // LOGIKA 64-BIT
                            uint64_t val = generate_trng_64();
                            if (format == 2) outFile << val << "\n";
                            else outFile.write(reinterpret_cast<const char*>(&val), sizeof(val));
                        } else {
                            // LOGIKA 32-BIT
                            uint32_t val = generate_trng_32();
                            if (format == 2) outFile << val << "\n";
                            else outFile.write(reinterpret_cast<const char*>(&val), sizeof(val));
                        }
                    }
                    outFile.close();
                    std::cout << "Gotowe! Zapisano do: " << filename << std::endl;
                }
                break;
            }

            case 3:
                std::cout << "\nLosowa liczba: " << generate_trng_32() << std::endl;
                break;

            case 4: {
                int count;
                std::cout << "Ile liczb wylosowac?: ";
                std::cin >> count;
                for (int i = 0; i < count; ++i) {
                    std::cout << "[" << i+1 << "] " << generate_trng_32() << std::endl;
                }
                break;
            }

            case 5: {
                generate_histogram();
                break;
            }

            case 6:
                return 0;

            case 7: { // Ukryta opcja do generowania danych z surowego jittera bez postprocessingu
                int count = 1000000;
                std::string filename = "random_data_jitter_v0.bin";
                std::ofstream outFile;
                outFile.open(filename, std::ios::binary);
                if (!outFile) {
                    std::cerr << "Blad pliku!" << std::endl;
                } else {
                    std::cout << "Generowanie..." << std::endl;
                    for (int i = 0; i < count; ++i) {
                        uint32_t val = get_raw_jitter_v0();
                        outFile.write(reinterpret_cast<const char*>(&val), sizeof(val));
                    }
                    outFile.close();
                    std::cout << "Zapisano do: " << filename << std::endl;
                }
                break;
            } // Ukryta opcja do generowania danych z surowego jittera z prostym mechanizmem mieszania

                case 8: {
                int count = 1000000;
                std::string filename = "random_data_jitter_v1.bin";
                std::ofstream outFile;
                outFile.open(filename, std::ios::binary);
                if (!outFile) {
                    std::cerr << "Blad pliku!" << std::endl;
                } else {
                    std::cout << "Generowanie..." << std::endl;
                    for (int i = 0; i < count; ++i) {
                        uint32_t val = get_raw_jitter_v1();
                        outFile.write(reinterpret_cast<const char*>(&val), sizeof(val));
                    }
                    outFile.close();
                    std::cout << "Zapisano do: " << filename << std::endl;
                }
                break;
            }

             case 9: { // Ukryta opcja do generowania danych z surowego jittera bez postprocessingu
                int count = 1000000;
                std::string filename = "random_data_jitter_v2.bin";
                std::ofstream outFile;
                outFile.open(filename, std::ios::binary);
                if (!outFile) {
                    std::cerr << "Blad pliku!" << std::endl;
                } else {
                    std::cout << "Generowanie..." << std::endl;
                    for (int i = 0; i < count; ++i) {
                        uint32_t val = get_raw_jitter_v2();
                        outFile.write(reinterpret_cast<const char*>(&val), sizeof(val));
                    }
                    outFile.close();
                    std::cout << "Zapisano do: " << filename << std::endl;
                }
                break;
             }

            default:
                std::cout << "Niepoprawny wybor." << std::endl;
                break;
        }
    }
    return 0;
}



/* rezultat - 8-bitowy histogram * -> entropia 8bit 
10M MIST 800-22C -> postprocessing -> histogram 8bit -> ent 8bit
*/

//rekord min 9841


