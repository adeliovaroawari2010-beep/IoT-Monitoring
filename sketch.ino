
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Konfigurasi LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Wi-Fi Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT HiveMQ
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;

// Topic untuk menerima teks
const char* topicTeks = "adel/esp32/lcd/text";

WiFiClient espClient;
PubSubClient client(espClient);

// Fungsi menerima pesan MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  String pesan = "";

  for (unsigned int i = 0; i < length; i++) {
    pesan += (char)payload[i];
  }

  Serial.print("Pesan diterima: ");
  Serial.println(pesan);

  // Bersihkan LCD
  lcd.clear();

  // Tampilkan maksimal 16 karakter per baris
  lcd.setCursor(0, 0);
  lcd.print(pesan.substring(0, 16));

  lcd.setCursor(0, 1);
  if (pesan.length() > 16) {
    lcd.print(pesan.substring(16, 32));
  }
}

// Koneksi MQTT
void sambungkanMQTT() {
  while (!client.connected()) {
    Serial.print("Menghubungkan ke MQTT...");

    String clientId = "ESP32-LCD-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("Terhubung!");

      client.subscribe(topicTeks);

      Serial.println("Menunggu pesan...");
    } else {
      Serial.print("Gagal, kode: ");
      Serial.println(client.state());

      delay(3000);
    }
  }
}


void setup() {
  Serial.begin(115200);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  lcd.print("Connecting...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password, 6);

  int percobaan = 0;

  while (WiFi.status() != WL_CONNECTED && percobaan < 30) {
    delay(500);
    Serial.print(".");
    percobaan++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi TERHUBUNG!");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi GAGAL!");
    lcd.clear();
    lcd.print("WiFi Failed!");

    while (true) {
      delay(1000);
    }
  }

  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  lcd.clear();
  lcd.print("WiFi Connected");
  delay(1000);
  lcd.clear();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  if (!client.connected()) {
    sambungkanMQTT();
  }

  client.loop();
}
