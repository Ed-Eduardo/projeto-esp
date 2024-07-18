#include <WiFi.h>        // Biblioteca de WiFi
#include <WiFiUdp.h>     // Biblioteca de UDP via WiFi
#include <SNMP_Agent.h>  // Biblioteca agente SNMP
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // Biblioteca Display
#include "DHT.h"          // Biblioteca sensor DHT22
#include <OneWire.h>            // Biblioteca complementar DallasTemperature
#include <DallasTemperature.h>  // Biblioteca sensot DS18B20
#include <NTPClient.h>          // Biblioteca Cliente NTP horario via rede
#include <ESP32Ping.h>
#define DHTPIN 4        // Difinir porta constante sensor DHT22
#define DHTTYPE DHT22
#define DS18B20PIN 26   // Difinir porta constante sensor DS18B20
#define PINO_BUZZER 27  // Difinir porta do buzzer

const char* ssid = "sensores";
const char* password = "3]3L_d7TPf.h";
IPAddress ip_remoto(8, 8, 8, 8);
//SimpleDHT22 dht;
OneWire oneWire(DS18B20PIN);
DallasTemperature ds(&oneWire);
WiFiUDP udp;
WiFiUDP udp_snmp;
SNMPAgent snmp = SNMPAgent("public", "private");
LiquidCrystal_I2C lcd(0x27, 16, 2);
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);
DHT dht(DHTPIN, DHTTYPE);

void iniciaWifi (){
  WiFi.begin(ssid, password);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectando....");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Conectado!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString());
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void lerSensor(float *temperatura_interna, float *temperatura_externa, float *umidade){
  ds.requestTemperatures();
  *temperatura_interna = ds.getTempCByIndex(0);
  *temperatura_externa = dht.readTemperature();
  *umidade = dht.readHumidity();
}

void imprimirLcd(float temperatura_interna, float temperatura_externa, float umidade){
  char str[16];
  lcd.clear();
  lcd.setCursor(0, 0);
  snprintf(str, 16, "TI:%.2fC", temperatura_interna);
  lcd.print(str);
  lcd.setCursor(0, 1);
  snprintf(str, 16, "TE:%.2fC", temperatura_externa);
  lcd.print(str);
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  snprintf(str, 16, "UMIDADE:%.2f", umidade);
  lcd.print(str);
  delay(4000);
}

void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  iniciaWifi();
  ds.begin();
  dht.begin();
  ntp.begin();
  ntp.forceUpdate();
  delay(1000);
}

void loop() {
  float temperatura_interna, temperatura_externa, umidade = 0;
  lerSensor(&temperatura_interna, &temperatura_externa, &umidade);
  imprimirLcd(temperatura_interna, temperatura_externa, umidade); 
}
