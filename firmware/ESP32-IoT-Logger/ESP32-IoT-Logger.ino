#include <WiFi.h>
#include "esp_wpa2.h"
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <Wire.h>

#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

#include "DHT.h"

// ---------- LCD ----------
hd44780_I2Cexp lcd;

// ---------- DHT11 ----------
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// ---------- Config ----------
Preferences prefs;
WebServer server(80);

String wifiType = "personal";  // "personal" o "enterprise"
String ssid;
String password;
String enterpriseUsername;
String thingSpeakApiKey;

unsigned long intervalDHT = 300000;        // 300 s
unsigned long intervalThingSpeak = 300000; // 300 s

float temperatura = NAN;
float humitat = NAN;

const char* thingSpeakServer = "http://api.thingspeak.com/update";

// ---------- Control pantalla ----------
const unsigned long tempsPantallaEncesa = 5000; // 5 segons
bool pantallaEncesa = false;
unsigned long momentEncesaPantalla = 0;

// ---------- Temps ----------
unsigned long tempsAnteriorDHT = 0;
unsigned long tempsAnteriorThingSpeak = 0;

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Inici ESP32-IoT-Logger");
  Serial.print("Reset reason: ");
  Serial.println(esp_reset_reason());

  Wire.begin(21, 22);

  int status = lcd.begin(16, 2);
  Serial.print("LCD status: ");
  Serial.println(status);

  if (status) {
    Serial.println("Error inicialitzant LCD");
    while (true) {
      delay(1000);
    }
  }

  lcd.noBacklight();

  dht.begin();

  carregarConfiguracio();

  if (ssid == "") {
    iniciarModeConfiguracio();
  }

  connectarWiFi();

  if (WiFi.status() != WL_CONNECTED) {
    iniciarModeConfiguracio();
  }

  // Primera lectura, sense enviar a ThingSpeak encara
  llegirDHT();
  mostrarDHT();

  tempsAnteriorDHT = millis();
  tempsAnteriorThingSpeak = millis();

  Serial.println("Setup acabat");
}

// ---------- Loop ----------
void loop() {
  unsigned long ara = millis();

  gestionarApagatPantalla();

  if (ara - tempsAnteriorDHT >= intervalDHT) {
    tempsAnteriorDHT = ara;

    llegirDHT();

    if (ara - tempsAnteriorThingSpeak >= intervalThingSpeak) {
      tempsAnteriorThingSpeak = ara;
      enviarThingSpeak();
    }

    mostrarDHT();
  }
}

// ---------- Configuració ----------
void carregarConfiguracio() {
  prefs.begin("config", true);

  wifiType = prefs.getString("wifiType", "personal");
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("pass", "");
  enterpriseUsername = prefs.getString("entUser", "");
  thingSpeakApiKey = prefs.getString("api", "");

  intervalDHT = prefs.getULong("intDHT", 300000);
  intervalThingSpeak = prefs.getULong("intTS", 300000);

  prefs.end();

  if (wifiType != "enterprise") {
    wifiType = "personal";
  }

  Serial.print("Tipus WiFi: ");
  Serial.println(wifiType);

  Serial.print("SSID guardada: ");
  Serial.println(ssid);

  if (wifiType == "enterprise") {
    Serial.print("Usuari Enterprise: ");
    Serial.println(enterpriseUsername);
  }

  Serial.print("Interval DHT ms: ");
  Serial.println(intervalDHT);

  Serial.print("Interval ThingSpeak ms: ");
  Serial.println(intervalThingSpeak);
}

void guardarConfiguracio() {
  prefs.begin("config", false);

  String nouWifiType = server.arg("wifiType");

  if (nouWifiType != "enterprise") {
    nouWifiType = "personal";
  }

  prefs.putString("wifiType", nouWifiType);
  prefs.putString("ssid", server.arg("ssid"));
  prefs.putString("pass", server.arg("password"));
  prefs.putString("entUser", server.arg("enterpriseUsername"));
  prefs.putString("api", server.arg("api"));

  unsigned long nouIntervalDHT = server.arg("intDHT").toInt() * 1000UL;
  unsigned long nouIntervalTS = server.arg("intTS").toInt() * 1000UL;

  if (nouIntervalDHT < 5UL * 1000UL) nouIntervalDHT = 5UL * 1000UL;
  if (nouIntervalTS < 15UL * 1000UL) nouIntervalTS = 15UL * 1000UL;

  prefs.putULong("intDHT", nouIntervalDHT);
  prefs.putULong("intTS", nouIntervalTS);

  prefs.end();
}

// ---------- Mode configuració ----------
void iniciarModeConfiguracio() {
  Serial.println("Mode configuracio");

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32_IoT_Config", "configesp32");

  IPAddress ip = WiFi.softAPIP();

  mostrarMissatgeLCD("Mode config", "192.168.4.1");

  Serial.print("AP config IP: ");
  Serial.println(ip);

  server.on("/", paginaConfiguracio);
  server.on("/save", HTTP_POST, desarConfiguracio);
  server.begin();

  while (true) {
    server.handleClient();
    gestionarApagatPantalla();
    delay(10);
  }
}

void paginaConfiguracio() {
  String html = "";

  html += "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<title>Configuracio ESP32</title>";
  html += "</head><body>";
  html += "<h2>Configuracio ESP32 DHT11</h2>";

  html += "<form method='POST' action='/save'>";

  html += "Tipus de WiFi:<br>";
  html += "<select name='wifiType'>";

  html += "<option value='personal'";
  if (wifiType == "personal") html += " selected";
  html += ">Personal / domestica</option>";

  html += "<option value='enterprise'";
  if (wifiType == "enterprise") html += " selected";
  html += ">Enterprise WPA2-PEAP</option>";

  html += "</select><br><br>";

  html += "SSID WiFi:<br>";
  html += "<input name='ssid' value='" + ssid + "'><br><br>";

  html += "Contrasenya WiFi:<br>";
  html += "<input name='password' type='password' value='" + password + "'><br>";
  html += "<small>En WiFi personal és la contrasenya de la xarxa. En Enterprise és la contrasenya de l'usuari.</small><br><br>";

  html += "Usuari WiFi Enterprise:<br>";
  html += "<input name='enterpriseUsername' value='" + enterpriseUsername + "'><br>";
  html += "<small>Només necessari si el tipus de WiFi és Enterprise.</small><br><br>";

  html += "ThingSpeak Write API Key:<br>";
  html += "<input name='api' value='" + thingSpeakApiKey + "'><br><br>";

  html += "Interval lectura DHT, en segons:<br>";
  html += "<input name='intDHT' type='number' value='" + String(intervalDHT / 1000UL) + "'><br><br>";

  html += "Interval enviament ThingSpeak, en segons:<br>";
  html += "<input name='intTS' type='number' value='" + String(intervalThingSpeak / 1000UL) + "'><br><br>";

  html += "<button type='submit'>Desar i reiniciar</button>";
  html += "</form>";

  html += "<p>Connecta't a la WiFi <b>ESP32_IoT_Config</b> i obre <b>192.168.4.1</b></p>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

void desarConfiguracio() {
  guardarConfiguracio();

  server.send(200, "text/html",
              "<h2>Configuracio desada</h2><p>L'ESP32 es reiniciara...</p>");

  delay(2000);
  ESP.restart();
}

// ---------- WiFi ----------
void connectarWiFi() {
  if (wifiType == "enterprise") {
    connectarWiFiEnterprise();
  } else {
    connectarWiFiPersonal();
  }
}

void connectarWiFiPersonal() {
  Serial.print("Connectant WiFi personal");

  mostrarMissatgeLCD("WiFi personal", "Connectant...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(1000);

  WiFi.begin(ssid.c_str(), password.c_str());

  esperarConnexioWiFi();
}

void connectarWiFiEnterprise() {
  Serial.print("Connectant WiFi Enterprise");

  mostrarMissatgeLCD("WiFi Enterprise", "Connectant...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(1000);

  if (enterpriseUsername == "" || password == "") {
    Serial.println();
    Serial.println("Error: falten usuari o contrasenya Enterprise");
    mostrarMissatgeLCD("Error Enterprise", "Falten dades");
    delay(3000);
    return;
  }

  esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)enterpriseUsername.c_str(), enterpriseUsername.length());
  esp_wifi_sta_wpa2_ent_set_username((uint8_t*)enterpriseUsername.c_str(), enterpriseUsername.length());
  esp_wifi_sta_wpa2_ent_set_password((uint8_t*)password.c_str(), password.length());

  esp_wifi_sta_wpa2_ent_enable();

  WiFi.begin(ssid.c_str());

  esperarConnexioWiFi();
}

void esperarConnexioWiFi() {
  unsigned long inici = millis();
  const unsigned long timeout = 30000;

  while (WiFi.status() != WL_CONNECTED && millis() - inici < timeout) {
    delay(500);
    Serial.print(".");
    gestionarApagatPantalla();
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi OK. IP: ");
    Serial.println(WiFi.localIP());

    mostrarMissatgeLCD("WiFi OK", "");

    delay(3000);
    gestionarApagatPantalla();

  } else {
    Serial.println("Error WiFi");

    mostrarMissatgeLCD("Error WiFi", "Mode config");

    delay(3000);
    gestionarApagatPantalla();
  }
}

// ---------- DHT ----------
void llegirDHT() {
  float novaTemperatura = dht.readTemperature();
  float novaHumitat = dht.readHumidity();

  if (isnan(novaTemperatura) || isnan(novaHumitat)) {
    Serial.println("Lectura DHT invalida. Es conserva l'ultim valor bo.");
    return;
  }

  temperatura = novaTemperatura;
  humitat = novaHumitat;

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.print(" C   Humitat: ");
  Serial.print(humitat);
  Serial.println(" %");
}

void mostrarDHT() {
  if (isnan(temperatura) || isnan(humitat)) {
    mostrarMissatgeLCD("Error DHT11", "");
    return;
  }

  String linia1 = "Temp: " + String(temperatura, 1) + " C";
  String linia2 = "Hum:  " + String(humitat, 1) + " %";

  Serial.print("LCD linia 1: ");
  Serial.println(linia1);
  Serial.print("LCD linia 2: ");
  Serial.println(linia2);

  mostrarMissatgeLCD(linia1, linia2);
}

// ---------- LCD ----------
void mostrarMissatgeLCD(String linia1, String linia2) {
  linia1 = ajustarLiniaLCD(linia1);
  linia2 = ajustarLiniaLCD(linia2);

  lcd.backlight();
  pantallaEncesa = true;
  momentEncesaPantalla = millis();

  delay(20);

  lcd.home();
  delay(10);
  lcd.clear();
  delay(10);

  lcd.setCursor(0, 0);
  lcd.print(linia1);

  delay(20);

  lcd.setCursor(0, 1);
  lcd.print(linia2);

  delay(20);
}

String ajustarLiniaLCD(String text) {
  if (text.length() > 16) {
    text = text.substring(0, 16);
  }

  while (text.length() < 16) {
    text += " ";
  }

  return text;
}

void gestionarApagatPantalla() {
  if (pantallaEncesa && millis() - momentEncesaPantalla >= tempsPantallaEncesa) {
    lcd.noBacklight();
    pantallaEncesa = false;
  }
}

// ---------- ThingSpeak ----------
void enviarThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("No s'envia: WiFi no connectada");
    return;
  }

  if (isnan(temperatura) || isnan(humitat)) {
    Serial.println("No s'envia: dades DHT invalides");
    return;
  }

  if (thingSpeakApiKey == "") {
    Serial.println("No s'envia: API Key buida");
    return;
  }

  Serial.println("Enviant a ThingSpeak...");

  HTTPClient http;

  String url = String(thingSpeakServer);
  url += "?api_key=" + thingSpeakApiKey;
  url += "&field1=" + String(temperatura, 1);
  url += "&field2=" + String(humitat, 1);

  http.begin(url);
  int httpCode = http.GET();

  Serial.print("ThingSpeak resposta: ");
  Serial.println(httpCode);

  http.end();
}
