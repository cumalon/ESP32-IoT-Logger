#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <Wire.h>

#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>

#include "DHT.h"

// ---------- LCD ----------
hd44780_I2Cexp lcd;

// ---------- DHT22 ----------
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ---------- Config ----------
Preferences prefs;
WebServer server(80);

String ssid;
String password;
String thingSpeakApiKey;

unsigned long intervalDHT = 60000;
unsigned long intervalThingSpeak = 60000;
unsigned long intervalIP = 15000;

unsigned long tempsAnteriorDHT = 0;
unsigned long tempsAnteriorThingSpeak = 0;
unsigned long tempsAnteriorIP = 0;

float temperatura = NAN;
float humitat = NAN;

const char* thingSpeakServer = "http://api.thingspeak.com/update";

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);

  int status = lcd.begin(16, 2);
  if (status) {
    while (true);
  }

  lcd.backlight();
  dht.begin();

  carregarConfiguracio();

  if (ssid == "") {
    iniciarModeConfiguracio();
  }

  connectarWiFi();

  if (WiFi.status() != WL_CONNECTED) {
    iniciarModeConfiguracio();
  }

  llegirDHT();
  mostrarDHT();
  enviarThingSpeak();

  tempsAnteriorDHT = millis();
  tempsAnteriorThingSpeak = millis();
  tempsAnteriorIP = millis();
}

// ---------- Loop ----------
void loop() {
  unsigned long ara = millis();

  if (ara - tempsAnteriorIP >= intervalIP) {
    tempsAnteriorIP = ara;
    mostrarIP();
    delay(3000);
    mostrarDHT();
  }

  if (ara - tempsAnteriorDHT >= intervalDHT) {
    tempsAnteriorDHT = ara;
    llegirDHT();
    mostrarDHT();
  }

  if (ara - tempsAnteriorThingSpeak >= intervalThingSpeak) {
    tempsAnteriorThingSpeak = ara;
    enviarThingSpeak();
  }
}

// ---------- Configuració ----------
void carregarConfiguracio() {
  prefs.begin("config", true);

  ssid = prefs.getString("ssid", "");
  password = prefs.getString("pass", "");
  thingSpeakApiKey = prefs.getString("api", "");

  intervalDHT = prefs.getULong("intDHT", 60000);
  intervalThingSpeak = prefs.getULong("intTS", 60000);

  prefs.end();
}

void guardarConfiguracio() {
  prefs.begin("config", false);

  prefs.putString("ssid", server.arg("ssid"));
  prefs.putString("pass", server.arg("password"));
  prefs.putString("api", server.arg("api"));

  prefs.putULong("intDHT", server.arg("intDHT").toInt() * 1000UL);
  prefs.putULong("intTS", server.arg("intTS").toInt() * 1000UL);

  prefs.end();
}

// ---------- Mode configuració ----------
void iniciarModeConfiguracio() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32_DHT_Config", "12345678");

  IPAddress ip = WiFi.softAPIP();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Mode config");
  lcd.setCursor(0, 1);
  lcd.print(ip);

  server.on("/", paginaConfiguracio);
  server.on("/save", HTTP_POST, desarConfiguracio);
  server.begin();

  while (true) {
    server.handleClient();
    delay(10);
  }
}

void paginaConfiguracio() {
  String html = "";

  html += "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<title>Configuracio ESP32</title>";
  html += "</head><body>";
  html += "<h2>Configuracio ESP32 DHT22</h2>";

  html += "<form method='POST' action='/save'>";

  html += "SSID WiFi:<br>";
  html += "<input name='ssid' value='" + ssid + "'><br><br>";

  html += "Contrasenya WiFi:<br>";
  html += "<input name='password' type='password' value='" + password + "'><br><br>";

  html += "ThingSpeak Write API Key:<br>";
  html += "<input name='api' value='" + thingSpeakApiKey + "'><br><br>";

  html += "Interval lectura DHT, en segons:<br>";
  html += "<input name='intDHT' type='number' value='60'><br><br>";

  html += "Interval enviament ThingSpeak, en segons:<br>";
  html += "<input name='intTS' type='number' value='60'><br><br>";

  html += "<button type='submit'>Desar i reiniciar</button>";
  html += "</form>";

  html += "<p>Connecta't a la WiFi <b>ESP32_DHT_Config</b> i obre <b>192.168.4.1</b></p>";

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

// ---------- WiFi normal ----------
void connectarWiFi() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connectant WiFi");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long inici = millis();
  const unsigned long timeout = 20000;

  while (WiFi.status() != WL_CONNECTED && millis() - inici < timeout) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();

  if (WiFi.status() == WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("WiFi OK");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(3000);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Error WiFi");
    lcd.setCursor(0, 1);
    lcd.print("Mode config");
    delay(3000);
  }
}

// ---------- DHT ----------
void llegirDHT() {
  temperatura = dht.readTemperature();
  humitat = dht.readHumidity();

  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.print(" C   Humitat: ");
  Serial.print(humitat);
  Serial.println(" %");
}

void mostrarDHT() {
  lcd.clear();

  if (isnan(temperatura) || isnan(humitat)) {
    lcd.setCursor(0, 0);
    lcd.print("Error DHT22");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperatura, 1);
    lcd.print((char)223);
    lcd.print("C");

    lcd.setCursor(0, 1);
    lcd.print("Hum: ");
    lcd.print(humitat, 1);
    lcd.print("%");
  }
}

// ---------- IP ----------
void mostrarIP() {
  lcd.clear();

  if (WiFi.status() == WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("IP WiFi:");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
  } else {
    lcd.setCursor(0, 0);
    lcd.print("WiFi perduda");
    lcd.setCursor(0, 1);
    lcd.print("Reiniciant...");
    delay(2000);
    ESP.restart();
  }
}

// ---------- ThingSpeak ----------
void enviarThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (isnan(temperatura) || isnan(humitat)) return;
  if (thingSpeakApiKey == "") return;

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
