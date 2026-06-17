# ESP32-IoT-Logger

Firmware per a un registrador de dades ambientals IoT basat en ESP32, utilitzant un ESP32, un sensor DHT i una pantalla LCD amb interfície I2C.

## Característiques

- Compatible amb ESP32 WROOM32
- Sensor de temperatura i humitat DHT11
- Pantalla LCD 16x2 via I2C
- Portal web de configuració WiFi
- Suport per a WiFi personal (WPA/WPA2-PSK)
- Suport per a WiFi WPA2-Enterprise (PEAP)
- Configuració persistent mitjançant ESP32 Preferences
- Enviament de dades a ThingSpeak
- Interval de mostreig configurable
- Interval d'enviament configurable
- Portal temporal de configuració disponible a l'arrencada

---

## Maquinari

- ESP32 WROOM32 AZ-Delivery
- Mòdul sensor DHT11
- Pantalla LCD 16x2 amb adaptador I2C

---

## Pinout actual

| Component | Pin ESP32 |
|-----------|-----------|
| DHT11 VCC | 3V3 |
| DHT11 GND | GND |
| DHT11 DATA | GPIO15 |
| LCD VCC | 5V |
| LCD GND | GND |
| LCD SDA | GPIO21 |
| LCD SCL | GPIO22 |

---

## Mode de configuració

Si no hi ha cap configuració WiFi desada, l'ESP32 crea automàticament un punt d'accés de configuració.

### Punt d'accés

```text
SSID: ESP32_IoT_Config
Contrasenya: configesp32
URL: http://192.168.4.1
```

### Portal temporal a l'arrencada

Quan ja existeixen credencials WiFi vàlides a la memòria, l'ESP32 continua obrint el portal de configuració durant 60 segons a l'inici.

Això permet:

- Revisar la configuració actual
- Modificar la configuració WiFi
- Actualitzar la clau API de ThingSpeak
- Ajustar els intervals de mostreig

sense haver d'esborrar les credencials desades ni desconnectar la xarxa WiFi de producció.

Si no es desa cap canvi durant aquests 60 segons, el punt d'accés es tanca automàticament i l'ESP32 continua l'arrencada normal.

---

## Modes WiFi

### WiFi personal

Xarxes domèstiques o d'oficina estàndard WPA/WPA2.

Paràmetres necessaris:

- SSID
- Contrasenya

### WPA2-Enterprise

Compatible amb ESP32 Arduino Core 2.0.17.

Paràmetres necessaris:

- SSID
- Nom d'usuari
- Contrasenya

Configuració objectiu validada:

```text
SSID: gencat_ENS_EDU
Seguretat: WPA2-Enterprise
Autenticació: PEAP
Xifrat: AES
```

Actualment el firmware només suporta el flux habitual PEAP amb usuari i contrasenya i no requereix certificats.

---

## ThingSpeak

El firmware envia les mesures a ThingSpeak.

Assignació actual dels camps:

| Camp | Valor |
|---------|---------|
| field1 | Temperatura |
| field2 | Humitat |

La configuració es realitza mitjançant el portal web.

Paràmetre necessari:

- Write API Key

---

## Configuració per defecte

| Paràmetre | Valor per defecte |
|------------|------------|
| Interval de lectura del DHT | 300 s |
| Interval d'enviament a ThingSpeak | 300 s |

Els valors es poden modificar des del portal de configuració i es desen a la memòria no volàtil.

---

## Comportament de la pantalla LCD

La pantalla LCD no roman il·luminada permanentment.

Per reduir el consum energètic i millorar l'estabilitat del sistema:

- La retroil·luminació només s'activa quan cal mostrar informació.
- La pantalla s'apaga automàticament al cap d'uns segons.
- La LCD només s'actualitza quan és necessari.
- Les línies completes es reescriuen en cada actualització.

---

## Notes d'estabilitat de la LCD I2C

Durant el desenvolupament es van observar reinicis intermitents de l'ESP32, corrupció de la pantalla i lectures invàlides del DHT.

Després de nombroses proves, la configuració següent ha resultat estable:

- LCD alimentada a 5V
- DHT11 alimentat a 3,3V
- SDA connectat a GPIO21
- SCL connectat a GPIO22
- DATA del DHT11 connectat a GPIO15

Canvis addicionals al firmware que han millorat l'estabilitat:

- Retroil·luminació de la LCD activada només quan és necessari.
- Apagat automàtic de la LCD després de mostrar informació.
- Reducció de la freqüència d'actualització de la pantalla.
- Eliminació de la transmissió a ThingSpeak dins del `setup()`.
- Actualització de temperatura i humitat només als intervals configurats.
- Reescriptura completa de les línies de la LCD en lloc de sobreescriure parcialment el text existent.

---

## Resolució de problemes

### La LCD mostra caràcters estranys

Comprova:

1. L'alimentació de la LCD.
2. La connexió SDA.
3. La connexió SCL.
4. L'adreça I2C.
5. La connexió de massa compartida amb l'ESP32.

### L'ESP32 es reinicia inesperadament

Comprova:

1. El cablejat d'alimentació de la LCD.
2. La qualitat de la font USB.
3. Les connexions de massa compartides.
4. Les resistències pull-up dels mòduls I2C.
5. Possibles fuites de 5V cap als GPIO de l'ESP32.

### Les lectures del DHT es tornen invàlides

Comprova:

1. L'alimentació del sensor.
2. La qualitat del cablejat.
3. La qualitat del mòdul sensor.
4. La integritat de la massa compartida.
5. La distància entre el sensor i l'ESP32.

---

## Problemes coneguts

### Actualitzacions massa freqüents de la LCD

Evita reintroduir cicles d'actualització agressius de la pantalla.

Durant les proves, les actualitzacions contínues de la LCD es van associar amb:

- Corrupció de la pantalla
- Caràcters aleatoris
- Lectures DHT invàlides
- Inestabilitat de l'ESP32

La implementació actual minimitza deliberadament l'activitat de la LCD.

---

## Entorn validat

### Maquinari

- ESP32 WROOM32 (AZDelivery)
- Mòdul DHT11
- Pantalla HD44780 16x2 genèrica amb adaptador I2C

### Programari

- Arduino IDE
- ESP32 Arduino Core 2.0.17

---

## Llicència

Projecte personal amb finalitats educatives.
