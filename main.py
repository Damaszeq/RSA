import matplotlib.pyplot as plt
import numpy as np
import os

# Zmień nazwę pliku tutaj
file_path = "random_data64.bin" 

if not os.path.exists(file_path):
    print(f"Błąd: Nie znaleziono pliku {file_path}.")
else:
    if "jitter" in file_path:
        raw_data = np.fromfile(file_path, dtype=np.uint32)
        raw_data = raw_data[raw_data > 0] 
        
        data = raw_data % 1024
        
        bins_count = 1024
        plot_range = (0, 1024)
        title = f"Rozkład surowego Jittera - {file_path}"
        xlabel = "Wartość(10bit)"
    else:
        data = np.fromfile(file_path, dtype=np.uint8)
        bins_count = 256
        plot_range = (0, 256)
        title = f"Histogram rozkładu bajtów (Gotowy TRNG) - {file_path}"
        xlabel = "Wartość bajtu (0-255)"

    # 2. RYSOWANIE
    plt.figure(figsize=(12, 6))
    
    plt.hist(data, bins=bins_count, range=plot_range, 
             color='royalblue', edgecolor='black', alpha=0.7)
    
    plt.title(title)
    plt.xlabel(xlabel)
    plt.ylabel("Liczba wystąpień")
    plt.grid(axis='y', alpha=0.3)
    
    # Dodatkowe info o statystykach na wykresie
    #plt.annotate(f"Średnia: {np.mean(data):.2f}\nOdchylenie std: {np.std(data):.2f}", 
                 #xy=(0.75, 0.85), xycoords='axes fraction', 
                 #bbox=dict(boxstyle="round", fc="white", alpha=0.5))

    plt.show()