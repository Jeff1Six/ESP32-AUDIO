
#include "Arduino.h"
#include "WiFiMulti.h"
#include "Audio.h"
#include "SPI.h"
#include "SD.h"
#include "FS.h"
#include <HTTPClient.h>
#include "esp_task_wdt.h"
#include <Pangodream_18650_CL.h>

// Digital I/O used
#define SD_CS          5
#define SPI_MOSI      23
#define SPI_MISO      19
#define SPI_SCK       18
#define I2S_DOUT      25
#define I2S_BCLK      27
#define I2S_LRC       26

Audio audio;
Pangodream_18650_CL BL;
WiFiMulti wifiMulti;
String ssid =     "Jefferson/Jeniffer";
String password = "78341675";

// Manipular task
TaskHandle_t hTask1;

//Variaveis para controle do som
String som; //PARA NENHUMA MENSAGEM
boolean executado = true; //CONFIRMAR FIM DA EXECUCAO DE AUDIO

void setup() {
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  SPI.setFrequency(1000000);
  Serial.begin(115200);
  SD.begin(SD_CS);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid.c_str(), password.c_str());
  wifiMulti.run();
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(true);
    wifiMulti.run();
  }
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(18); // 0...21
}

void loop() {
  verificaBateria();
  reproduzAudio();
  delay(3000);
}

void reproduzAudio() {
  HTTPClient http;
  http.addHeader("Content-Type", "application/json");
  http.begin("http://192.168.0.105:8000/pessoas");
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    som = http.getString();
    if (!audio.isRunning()) {
      audio.connecttospeech(som.c_str(), "pt-BR");
    }
    while (audio.isRunning())
    {
      audio.loop();
    }
  } else {
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    audio.connecttospeech("Problema na Leitura", "pt-BR");
  }
}

void verificaBateria() {
  int valor_bateria = BL.getBatteryChargeLevel();
  HTTPClient http;
  http.addHeader("Content-Type", "application/json");
  http.begin("http://192.168.0.105:8000/bateriaStatus?bateria=" + String(valor_bateria));
  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    som = http.getString();
    if (atoi(som.c_str()) != 100) {
      if (!audio.isRunning()) {
        audio.connecttospeech(som.c_str(), "pt-BR");
      }
      while (audio.isRunning())
      {
        audio.loop();
      }
    }

  } else {
    Serial.print("HTTP Response Code: ");
    Serial.println(httpResponseCode);
    audio.connecttospeech("Problema na Checagem da Bateria", "pt-BR");
  }
}
