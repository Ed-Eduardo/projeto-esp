#include <WiFi.h>        // Biblioteca de WiFi
#include <WiFiUdp.h>     // Biblioteca de UDP via WiFi
#include <SNMP_Agent.h>  // Biblioteca agente SNMP
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // Biblioteca Display
#include <SimpleDHT.h>          // Biblioteca sensor DHT22
#include <OneWire.h>            // Biblioteca complementar DallasTemperature
#include <DallasTemperature.h>  // Biblioteca sensot DS18B20
#include <NTPClient.h>          // Biblioteca Cliente NTP horario via rede
#include <ESP32Ping.h>
#define DHTPIN 4        // Difinir porta constante sensor DHT22
#define DS18B20PIN 26   // Difinir porta constante sensor DS18B20
#define PINO_BUZZER 27  // Difinir porta do buzzer
int cont_snmp;          // Variavel controle de tempo para realizar rotinas de leitura snmp, ler sensores e escriter em display
int cont = 0;
// Declaração de variaveis para controles
float valor_temp_ext_atual, valor_umidade_ext_atual, valor_temp_int_atual = 0;
char* desenvolvedor = "By Ryuuma";
char* temp_ext_atual = "0";
char* umidade_ext_atual = "0";
char* temp_int_atual = "0";
const char* ssid = "sensores";
const char* password = "3]3L_d7TPf.h";
IPAddress ip_remoto(8, 8, 8, 8);
SimpleDHT22 dht;
OneWire oneWire(DS18B20PIN);
DallasTemperature ds(&oneWire);
WiFiUDP udp;
WiFiUDP udp_snmp;
SNMPAgent snmp = SNMPAgent("public", "private");
LiquidCrystal_I2C lcd(0x27, 16, 2);
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);
String hora;
const float temp_max = 2;
const float temp_min = 8;

void atualizarSnmp() {
  snprintf(temp_ext_atual, 25, "%.2f", valor_temp_ext_atual);
  snprintf(umidade_ext_atual, 25, "%.2f", valor_umidade_ext_atual);
  snprintf(temp_int_atual, 25, "%.2f", valor_temp_int_atual);
  snmp.addReadWriteStringHandler(".1.3.6.1.2.1.10.1", &temp_ext_atual, 25, true);
  snmp.addReadWriteStringHandler(".1.3.6.1.2.1.10.2", &umidade_ext_atual, 25, true);
  snmp.addReadWriteStringHandler(".1.3.6.1.2.1.10.3", &temp_int_atual, 25, true);
  snmp.sortHandlers();
}
void imprimirLcd() {
  char str[16];
  lcd.clear();
  lcd.setCursor(0, 0);
  snprintf(str, 16, "TI:%.2fC %s", valor_temp_int_atual, hora);
  lcd.print(str);
  lcd.setCursor(0, 1);
  snprintf(str, 16, "TE:%.2fC", valor_temp_ext_atual);
  lcd.print(str);
  delay(3000);
  lcd.clear();
  lcd.setCursor(0, 0);
  snprintf(str, 16, "UMIDADE:%.2f", valor_umidade_ext_atual);
  lcd.print(str);
  delay(2000);
  //lcd.print("TEMP:" + String(valor_temp_int_atual) + "C");
}
void atualizaHora() {
  hora = ntp.getFormattedTime();
}
void iniciaSensor() {
  float t, u, tg;
  int status = dht.read2(DHTPIN, &t, &u, NULL);
  ds.requestTemperatures();
  tg = ds.getTempCByIndex(0);
  if (status == SimpleDHTErrSuccess) {
    valor_temp_ext_atual = t;
    valor_temp_int_atual = tg;
    valor_umidade_ext_atual = u;
  }
}
void lerSensor() {
  float t, u, tg;
  int status = dht.read2(DHTPIN, &t, &u, NULL);
  ds.requestTemperatures();
  tg = ds.getTempCByIndex(0);
  if (status == SimpleDHTErrSuccess) {
    valor_temp_ext_atual = t - 4;
    valor_temp_int_atual = tg;
    valor_umidade_ext_atual = u;
  }
  if ((valor_temp_int_atual <= temp_min) || (valor_temp_int_atual >= temp_min)) {
    digitalWrite(PINO_BUZZER, HIGH);  // Ligar o buzzer
  } else {
    digitalWrite(PINO_BUZZER, LOW);  // Desligar o buzzer
  }
}
void validaConexao() {

  if (Ping.ping(WiFi.gatewayIP())) {
    Serial.println(WiFi.localIP());
  } else {
    WiFi.begin(ssid, password);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Conectando....");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }
}
void setup() {
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
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
  cont_snmp = 0;
  ds.begin();
  snmp.setUDP(&udp_snmp);
  snmp.begin();
  snmp.addReadOnlyStaticStringHandler(".1.3.6.1.2.1.1.1", desenvolvedor);
  snmp.sortHandlers();
  temp_ext_atual = (char*)malloc(25 * sizeof(char));
  umidade_ext_atual = (char*)malloc(25 * sizeof(char));
  temp_int_atual = (char*)malloc(25 * sizeof(char));
  ntp.begin();
  ntp.forceUpdate();
  iniciaSensor();
}
void loop() {
  snmp.loop();
  atualizaHora();
  lerSensor();
  imprimirLcd();
  atualizarSnmp();
  delay(1000);
  validaConexao();
}