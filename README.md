# Sistem Inteligent de Irigație cu Monitorizare Web și Afișaj OLED (ProiectSM)

Sistem IoT complet automatizat pentru monitorizarea umidității solului și gestionarea irigațiilor în timp real, bazat pe microcontrolerul **Raspberry Pi Pico 2 W**. Dispozitivul oferă un control hibrid: local (prin ecran OLED și butoane fizice) și la distanță (prin server web HTTP integrat accesibil din rețeaua locală).

---

## Caracteristici principale
* **Automatizare inteligentă:** Activare pompă pe bază de prag critic cu timp de siguranță (**15s Cooldown**) pentru prevenirea inundării plantei.
* **Server Web HTTP Integrat:** Interfață web interactivă (HTML/CSS/JS) cu design Dark Mode și actualizare asincronă (**AJAX / Fetch API**) la fiecare secundă.
* **Interfață Fizică Locală:** Ecran OLED SSD1306 cu fonturi grafice personalizate, 4 pagini de stare și butoane pentru navigare meniu / udare manuală instantă.
* **Sincronizare de Timp Asincronă:** Preluarea orei direct din browserul clientului în momentul accesării paginii, eliminând necesitatea unui modul hardware RTC extern.

---

## Structură Hardware & Pinout

| Componentă | Pin Pico 2 W | Tip Semnal / Funcție |
| :--- | :--- | :--- |
| **Senzor Umiditate Sol** | GPIO 26 / ADC0 | Intrare Analogică (Măsurare directă) |
| **Ecran OLED (SDA)** | GPIO 16 | I2C0 Data (Interfață locală) |
| **Ecran OLED (SCL)** | GPIO 17 | I2C0 Clock (Interfață locală) |
| **Buton Meniu (Pagini)** | GPIO 14 | Intrare Digitală (Pull-Up Intern) |
| **Buton Udare Manuală** | GPIO 15 | Intrare Digitală (Pull-Up Intern) |
| **Modul Releu (Pompă)** | GPIO 18 | Ieșire Digitală (Actuator Putere) |
| **LED Placă Integrat** | Cyw43 LED | Semnalizare Stare Pompă (ON/OFF) |

---

## Arhitectură Software
Sistemul este complet reactiv și rulează *non-blocking*. Controlul timpilor de funcționare și ferestrele de cooldown se realizează prin structuri `absolute_time_t` și verificări de tip `time_reached`, asigurând stabilitatea stivei de rețea fără întreruperi blocate în execuție.

* **Stivă Rețea:** lwIP (Lightweight IP) configurată în mod polling (`pico_cyw43_arch_lwip_poll`).
* **Endpoint-uri HTTP implementate:**
  * `GET /` -> Servește pagina principală HTML/CSS/JS stocată în bufferul intern.
  * `GET /status` -> Returnează un obiect JSON cu starea senzorilor, limita auto și starea pompei.
  * `GET /setlimit?value=X` -> Modifică dinamic procentul critic de umiditate direct din slider.
  * `GET /water?time=X` -> Declanșează pompa web timp de 3 secunde și înregistrează timestamp-ul.
  * `GET /time?now=X` -> Sincronizează în background string-ul de timp trimis de browser.

---

## Descrierea și Funcționarea Interfeței Web

Sistemul integrează un server web HTTP capabil să deservească o pagină interactivă, optimizată pentru ecranele dispozitivelor mobile și desktop (Responsive Design). Aceasta este stocată direct în memoria Flash a microcontrolerului sub forma unui buffer de caractere (`char html[]`) și trimisă clientului la accesarea adresei IP a plăcii.

### Elemente Vizuale și Control
Interfața include:
1. **Indicatorul Istoric ("Ultima udare"):** Afișează data și ora exactă la care pompa a fost activată ultima dată.
2. **Monitorizare în Timp Real ("Nivel umiditate"):** Un bloc dinamic ce afișează procentul curent de apă din sol, actualizat automat secundă de secundă.
3. **Configurare Dinamică ("Udare automata sub"):** Un element de tip glisor (`input type="range"`) care îi permite utilizatorului să modifice pragul critic de umiditate între 0% și 100%.
4. **Buton Manual ("UDA ACUM"):** Un buton tactil de mari dimensiuni care pornește forțat pompa de apă timp de 3 secunde. În timpul activării, textul butonului se schimbă în `UDA...` și devine inactiv pentru a preveni dubla apăsare.
5. **Status Pompă:** Un indicator colorat (*Verde* pentru `ON`, *Roșu* pentru `OFF`) care reflectă starea electrică instantanee a releului.

### Mecanismul de Sincronizare și AJAX (Fetch API)
Pentru a asigura o experiență fluidă și un trafic de rețea redus, interfața web folosește scripturi **JavaScript asincrone** care rulează în fundalul browserului:

* **Rutina `updateStatus()` (Frecvență: 1Hz):** La fiecare secundă, browserul trimite o cerere asincronă de tip `fetch('/status')`. Serverul de pe Pico 2 W răspunde instant cu un pachet de date în format **JSON**. Scriptul JavaScript extrage valorile și modifică selectiv elementele din pagină (DOM) fără a reîncărca întreaga structură HTML.
* **Virtual RTC și Sincronizarea `syncTime()`:** Deoarece Raspberry Pi Pico 2 W nu dispune de un modul hardware pentru menținerea orei reale (RTC) în caz de blackout, sistemul folosește o soluție software. La încărcarea paginii și apoi la fiecare secundă, browserul trimite ora sa locală către placa de dezvoltare prin apelul `fetch('/time?now=...')`. Stringul decodificat de funcția `decode_url_time()` este stocat în memoria RAM a plăcii și este folosit ca timestamp oficial pentru marcarea momentului în care se face udarea (atât cea manuală, cât și cea automată).
* **Modificarea Pragului (`setLimit()`):** În momentul în care utilizatorul mișcă glisorul pe ecran, evenimentul `oninput` declanșează instant un apel de tip `fetch('/setlimit?value=' + v)`. Microcontrolerul interceptează valoarea, o convertește prin funcția `atoi()` și actualizează variabila globală `auto_water_percent`, modificând comportamentul automatizării în timp real.

---

## Diagrame și Modelare Sistem

### 1. Diagrama de Activități (Activity Diagram)
Descrie interacțiunile actorilor externi (Utilizatorul Web și Solul ca mediu fizic) cu modulele logice ale sistemului de irigație.

<img src="images/diagrama-activitati.png" width="800" alt="Diagrama de activități" />

### 2. Diagrama de Cazuri de Utilizare (Use-Case Diagram)
Definește rolurile și permisiunile actorilor (Utilizatorul local, Clientul Web și Mediul controlat) în raport cu funcționalitățile software expuse de sistem.

<img src="images/diagrama-use-case.png" width="600" alt="Diagrama use-case" />

### 3. Diagrama de Funcționare și Secvență (Sequence Diagram)
Ilustrează interacțiunea asincronă bazată pe protocoale de tip Request-Response între browser și stiva lwIP a microcontrolerului, simultan cu actualizarea stării releului.

<img src="images/diagrama-secventa.png" width="800" alt="Diagrama de secvență" />

### 4. Diagrama de Stări a Pompei (State Machine Diagram)
Explică ciclul de viață al actuatorului hardware (pompa) și evenimentele de sistem care determină tranzițiile între modurile Repaus, Web, Manual și Automat.

<img src="images/diagrama-stari-pompa.png" width="100%" alt="Diagrama de stări a pompei" />

---

## Ghid de Compilare și Rulare

### 1. Configurare proiect
* Mediu de dezvoltare **VS Code** cu extensia oficială *Raspberry Pi Pico* configurată.
* **Pico SDK v2.2.0** instalat și selectat.
* Compilatorul GCC ARM Embedded Toolchain validat pentru arhitectura `pico2_w`.

### 2. Configurare Wi-Fi
Înainte de compilare, deschide fișierul sursă principal și configurează credențialele rețelei tale locale în secțiunea de configurare Wi-Fi:
```c
// =====================================================
// ================= WIFI CONFIG =======================
// =====================================================
#define WIFI_SSID "Numele_Retelei_Tale"
#define WIFI_PASS "Parola_Retelei_Tale"
```

---

## 📸 Capturi și Stări de Funcționare ale Proiectului

Această secțiune descrie comportamentul vizual real al sistemului pe hardware.

**Afișaj OLED (Ecran inițial de Boot):**

<img src="images/oled-boot.jpg" width="300" alt="OLED - ecran de boot" />

**Faza imediat după conectarea la Wi-Fi:**

<img src="images/oled-wifi-conectat.jpg" width="300" alt="OLED - conectat la Wi-Fi" />

După conectarea cu succes, utilizatorul poate naviga ciclic prin cele 4 pagini de stare apăsând butonul fizic **MENU (GPIO 14)**.

#### Pagina 1: Status General (Ecranul Principal)
Afișează numele proiectului, adresa IP alocată (esențială pentru ca utilizatorul să știe ce să tasteze în browser) și starea curentă a releului.

<img src="images/oled-pagina1-status.jpg" width="300" alt="OLED - pagina 1, status general" />

#### Pagina 2: Status Date Udare
Afișează ultima udare și starea pompei.

<img src="images/oled-pagina2-udare.jpg" width="300" alt="OLED - pagina 2, date udare" />

#### Pagina 3: Status Procent Umiditate
Afișează procentul curent de umiditate citit din sol.

<img src="images/oled-pagina3-umiditate.jpg" width="300" alt="OLED - pagina 3, umiditate" />

#### Pagina 4: Status Procent Umiditate Auto
Afișează pragul de umiditate sub care pompa pornește automat.

<img src="images/oled-pagina4-prag-auto.jpg" width="300" alt="OLED - pagina 4, prag udare automată" />
