/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL6ZjcnKgDN"
#define BLYNK_TEMPLATE_NAME         "Shelter"
#define BLYNK_AUTH_TOKEN            "Le7YkzassFelCQJkcwLKVT261E_pbyWV"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <HTTPClient.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "LACERTILIA";
char pass[] = "LACERTILIA";

BlynkTimer timer;
Servo servo1;
Servo servo2;

//pin
const int Raindrop = 35;
const int Moisture = A0;
const int Pinservo1 = 19;//kanan
const int Pinservo2 = 21;//kiri
//
int hujan,ket;
int st;
int nilai_soil;
int persen_soil;
//
int batas_basah;
int batas_kering;
int batas = 10;
int lastrain = -1;
int lastsoil = -1;

enum State{
  Shelter_OPEN,
  Shelter_CLOSE
};

State CurrentState = Shelter_OPEN;
//kalibrasi widget kelembaban tanah
BLYNK_WRITE(V7){
  batas_kering = param.asInt();
  Blynk.virtualWrite(V7,batas_kering);
}
BLYNK_WRITE(V8){
  batas_basah = param.asInt();
  Blynk.virtualWrite(V8,batas_basah);
}
//Widget
void widget_soil(){
  Bacasensor();
  if(WiFi.status() == WL_CONNECTED && Blynk.connected()){
    update_blynk_moisture();
    update_blynk_hujan();
    widget_LCD1();
    widget_LCD2();
    update_state();
  }
}
void update_blynk_moisture(){
  Blynk.virtualWrite(V0,persen_soil);
  Blynk.virtualWrite(V1,nilai_soil);
  Blynk.virtualWrite(V2,ket);
}
void update_blynk_hujan(){
  Blynk.virtualWrite(V3,hujan);
}
void update_state(){
  Blynk.virtualWrite(V6,st);
}
void widget_LCD1(){
  if(hujan == 1){
    Blynk.virtualWrite(V4,"Tidak Hujan");
  }
  else{
    Blynk.virtualWrite(V4,"Hujan");
  }
}
void widget_LCD2(){
  if(st == 0){
    Blynk.virtualWrite(V5,"Atap TERBUKA");
  }
  else{
    Blynk.virtualWrite(V5,"Atap TERTUTUP");
  }
}
BLYNK_CONNECTED() { 
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V7);
  Blynk.syncVirtual(V8);
  Blynk.syncAll();
}
void reconnectBlynk() {
  if (WiFi.status() == WL_CONNECTED && !Blynk.connected()) {
    Serial.println("Reconnecting to Blynk...");
    if (Blynk.connect()) {
      Serial.println("Reconnected to Blynk!");
    } else {
      Serial.println("Failed to reconnect to Blynk.");
    }
  }
}
void setup()
{
  // Debug console
  Serial.begin(115200);
  pinMode(LED_BUILTIN,OUTPUT);
  LEDWifi();

  Blynk.begin(BLYNK_AUTH_TOKEN,ssid,pass);

  servo1.attach(Pinservo1,500,2500);
  servo2.attach(Pinservo2,500,2500);

  pinMode(Raindrop,INPUT);
  pinMode(Moisture,INPUT);

  timer.setInterval(1500L,widget_soil);
  timer.setInterval(3000L,reconnectBlynk);
  timer.setInterval(10000L,KeepAlive);
}

void loop()
{
  LEDWifi();
  if(WiFi.status() == WL_CONNECTED && Blynk.connected()){
    Blynk.run();
  }
  timer.run();
}

void Bacasensor(){
  hujan = digitalRead(Raindrop);
  nilai_soil = analogRead(Moisture);
  persen_soil = map(nilai_soil,batas_basah,batas_kering,100,0);
  persen_soil = constrain(persen_soil, 0, 100);
  
  Serial.print("hujan : ");
  Serial.println(hujan);
  Serial.print("kelembaban : ");
  Serial.print(nilai_soil);
  Serial.print(" / ");
  Serial.print(persen_soil);
  Serial.println("%");
  
  if(persen_soil <=60){
    Serial.println("Kelembaban Rendah");
    ket = 0;
  }
  else if(persen_soil > 60 && persen_soil <=80){
    Serial.println("Kelembaban Sedang");
    ket = 1;
  }
  else if(persen_soil > 80){
    Serial.println("Kelembaban Tinggi");
    ket = 2;
  }

  if(hujan != lastrain || abs(persen_soil - lastsoil) > 4){
    action();
    lastrain = hujan;
    lastsoil = persen_soil;
  }
}

//Action
void action(){
  switch(CurrentState){
    case Shelter_OPEN:
      if(hujan == 0 && persen_soil > 60){
        st = 1;
        Serial.println("Atap Tertutup");
        tutup();
        CurrentState = Shelter_CLOSE;
      }
      else if(hujan == 1 && persen_soil <= 60){
        st = 1;
        Serial.println("Atap Tertutup");
        tutup();
        CurrentState = Shelter_CLOSE;
      }
    break;
    case Shelter_CLOSE:
      if(hujan == 0 && persen_soil <= 60){
        st = 0;
        Serial.println("Atap Terbuka");
        buka();
        CurrentState = Shelter_OPEN;
      }
      else if(hujan == 1 && persen_soil > 60){
        st = 0;
        Serial.println("Atap Terbuka");
        buka();
        CurrentState = Shelter_OPEN;
      }
    break;
  }
}
void buka(){
  servo1.write(110);
  servo2.write(110);
}
void tutup(){
  servo1.write(180);
  servo2.write(180);
 
}
void KeepAlive(){
  Blynk.virtualWrite(V9, "keep-alive"); // Kirim data dummy
}
void LEDWifi(){
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }
}

