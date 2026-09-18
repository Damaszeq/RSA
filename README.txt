Tomasz Wejner - projekt na Bezpieczeństwo Sieci Teleinformatycznych (LAB)
# System Kryptograficzny RSA-2048 / TRNG

Projekt realizuje własną implementację systemu asymetrycznego RSA z wykorzystaniem generowania losowości ze źródeł sprzętowych (TRNG Jitter Entropy) oraz obliczeń wielkoprzyrostowych na liczbach całkowitych (biblioteka GMP).

> **Anotacja dotycząca autorskości kodu:**  
> Implementacja algorytmu RSA oraz sprzętowego generatora liczb losowych (TRNG) jest w całości autorska. Kod funkcji skrótu z rodziny SHA stanowi zewnętrzny moduł pomocniczy wykorzystywany do tworzenia i weryfikacji podpisów cyfrowych.

---

## Wymagania systemowe i zależności

* **Kompilator C++:** `g++` (GCC) lub MSVC wspierający standard C++11 (lub nowszy).
* **Biblioteki:** `GMP` (GNU Multiple Precision Arithmetic Library) do obsługi operacji na bardzo dużych liczbach.
* **Wsparcie dla instrukcji procesora:** Instrukcja `RDTSC` (x86/x64) wykorzystywana do pomiaru mikro-odchyleń czasowych (Jitter Entropy).

---

## Struktura projektu

* `RSA.cpp` – Główny kod źródłowy programu (logika RSA, obróbka plików, menu użytkownika).
* `TRNG.cpp` – Autorska implementacja Sprzętowego Generatora Liczb Losowych (TRNG).
* `RSA.exe` – Gotowy, statycznie skompilowany plik wykonywalny dla systemu Windows.
* Pliki testowe różnych typów służące do weryfikacji prawidłowego działania szyfrowania, generowania podpisów i sprawdzania spójności danych.

---

> **Uwaga dotycząca rozmiaru plików dla Opcji 2 i 3:**  
> Szyfrowanie asymetryczne RSA w trybie bezpośrednim pozwala na zaszyfrowanie danych o rozmiarze nieprzekraczającym 255 bajtów. Do szyfrowania większych plików w praktyce stosuje się szyfrowanie hybrydowe (np. RSA + AES).
---

## Instrukcja kompilacji i uruchomienia

### 1. Uruchomienie gotowego pliku (Windows)
W katalogu projektu znajduje się wstępnie skompilowany plik `RSA.exe`. Ze względu na kompilację statyczną (`-static`) uruchomi się on bezpośrednio na większości systemów Windows bez konieczności instalowania zewnętrznych bibliotek `.dll`.

### 2. Kompilacja ze źródeł (Linux / MSYS2)

Kompilacja statyczna (tworzy niezależny plik wykonywalny):
```bash
g++ RSA.cpp -o RSA.exe -static -lgmp -lstdc++ -O2
