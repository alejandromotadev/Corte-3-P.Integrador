#include <DHT.h>
#include <DHT_U.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <LiquidCrystal.h>
#define bomba 33
#define sensorHum 34
#define echoPin 14
#define trigPin 12
DHT dht(4, DHT11);

//Asignamos una mac personalizada
uint8_t newMACAddress[] = {0xc8, 0x3d, 0xd4, 0x7a, 0x57, 0x85};
//Definimos pines de LCD
const int rs = 5, en = 18, d4 = 19, d5 = 21, d6 = 22, d7 = 23;  
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Variables
float t, h, humS, hS,agua;
long seg, distance;
int bombaEstado = 0;

byte customChar[8] = {
    0b01110,
    0b01010,
    0b01110,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000};

byte porcentChar[] = {
    B01100,
    B01101,
    B00010,
    B00100,
    B01000,
    B10110,
    B00110,
    B00000};

 //conexion a la red wifi de la escuela   
  const char *ssid = "Software";
  const char *password = "software22";

void setup()
{
  Serial.begin(115200);
  dht.begin();  
  lcd.begin(16, 2);
  lcd.createChar(0, customChar);
  lcd.createChar(1, porcentChar);
  lcd.clear();

  pinMode(bomba, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  digitalWrite(trigPin, LOW);
  digitalWrite(bomba, LOW);
  
// mac custom
  Serial.begin(115200);
  Serial.println();
  WiFi.mode(WIFI_STA);
  Serial.print("[OLD] ESP32 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);
  Serial.print("[NEW] ESP32 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  
//Conectar wifi 
  WiFi.begin(ssid, password);
  Serial.print("Conectando...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Conectado con éxito, mi IP es: ");
  Serial.println(WiFi.localIP());
  Serial.println(WiFi.macAddress());
}

void loop()
{
  digitalWrite(bomba,LOW);
  HTTPClient http;
    http.begin("http://192.168.89.218:3001/api/datos");
    http.addHeader("Content-Type","application/x-www-form-urlencoded");
    
    //variables de componentes
    t = dht.readTemperature();
    h = dht.readHumidity();
    humS = analogRead(sensorHum); // 4095 seco y 1554 humedad perfecta
    
    //sensor de humedad
    hS = ((4095.0 - humS) * 100.0) / 4095.0;
    
    //sensor ultrasonico
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    seg = pulseIn(echoPin, HIGH);
    distance = seg / 59;
    agua = ((14 - distance) * 100.0) / 7;

    //String que recibe el programa
    String text = "temp=" + String(t) + "&humedad="  + String(h) + "&suelo=" + String(hS) + "&agua=" + String(agua);

    int resp = http.POST(text);
    if(resp>0){
     Serial.println("Còdigo http =" + String(resp));
     if(resp==200){
        String response= http.getString();
        Serial.print("El servidor respondiò: ");
        Serial.println(response);
     }
     }else{
      Serial.print("Error al enviar post, codigo:");
      Serial.print(resp);
      
    }  
    http.end();
 
  // mostrar temperatura ambiental
  lcd.setCursor(0, 0);
  lcd.print(t);
  lcd.write(byte(0));
  lcd.print("C");
  
  // mostrar humedad ambiental
  lcd.setCursor(9, 0);
  lcd.print(h);
  lcd.write(byte(1));
  
  // mostrar nivel de agua
  if(agua>0){
    lcd.setCursor(9, 1);
    lcd.print(agua);
    lcd.write(byte(1));
    if (agua < 10)
    {
      lcd.setCursor(15, 1);
      lcd.print(" ");
    }
    if (agua == 100.0)
    {
      lcd.setCursor(15, 1);
      lcd.write(byte(1));
    }
  }
  
  // mostrar humedad de suelo
  lcd.setCursor(0, 1);
  lcd.print(hS);
  lcd.write(byte(1));
  if (hS < 10)
  {
    lcd.setCursor(6, 1);
    lcd.print(" ");
    lcd.setCursor(7, 1);
    lcd.print(" ");
  }
  lcd.setCursor(7, 1);
  lcd.print(" ");
  if (hS == 100.0)
  {
    lcd.setCursor(7, 1);
    lcd.write(byte(1));
  }
    regar(hS);
    delay(2000);
}

//funcion de regado
void regar(float hS)
{     
   if (hS < 51 && agua > 25)
  { 
    lcd.setCursor(0,2);
    lcd.print("Suelo: regando");
    digitalWrite(bomba,HIGH);
    delay(3000);
    digitalWrite(bomba,LOW);
    lcd.clear();
    String text = "temp=" + String(t) + "&humedad="  + String(h) + "&suelo=" + String(hS) + "&agua=" + String(agua);
    Serial.println(text);
    HTTPClient http;
    http.begin("http://192.168.89.218:3001/api/regado");
    http.addHeader("Content-Type","application/x-www-form-urlencoded");
    int resp = http.POST(text);
    //int resp = http.GET();
    if(resp>0){
     Serial.println("Còdigo http =" + String(resp));
     if(resp==200){
        String response= http.getString();
        Serial.print("El servidor respondiò: ");
        Serial.println(response);
     }
     }else{
      Serial.print("Error al enviar post, codigo:");
      Serial.print(resp); 
    }
    http.end();
  }
      else{
        digitalWrite(bomba,LOW);
    }
}
