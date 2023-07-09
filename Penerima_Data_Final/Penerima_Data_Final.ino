#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Konfigurasi jaringan WiFi
const char* ssid = "Golden Rooftop";
const char* password = "feellikehome";

// Konfigurasi MQTT
const char* mqtt_server = "cc95b1fcb55f4ae6bf67f8c7dab6e85b.s2.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_username = "Test_Koneksi_2";
const char* mqtt_password = "Test_Koneksi_2";
const char* mqtt_topic = "rfid/data";

const int MAX_DATA = 1000;
String data[MAX_DATA];
int dataCount = 0;
unsigned long lastPrintTime = 0;

WiFiClientSecure espClient;
PubSubClient client(espClient);

void setupWiFi() {
  // Menghubungkan ke jaringan WiFi
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WIFI");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Callback function untuk menerima pesan yang dikirim ke ESP8266
  // Dapat Anda gunakan untuk melakukan tindakan sesuai dengan pesan yang diterima
}

void setup() {
  Serial.begin(115200);
  setupWiFi();

  // Menghubungkan ke broker MQTT
  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  while (!client.connected()) {
    if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      Serial.println("Terhubung ke broker MQTT");
    } else {
      Serial.print("Gagal tersambung ke broker MQTT, coba lagi dalam 5 detik...");
      delay(5000);
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    //String clientId = "ESP8266Client-";
    //clientId += String(random(0xffff), HEX);
    //clientId.c_str()
    if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Tambahkan data "Reader_1" pada posisi pertama dalam array
  data[0] = "Reader_2, ";

  // Baca data dari komunikasi serial
  if (Serial.available()) {
    String newData = Serial.readStringUntil('\n');

    // Periksa apakah nilai data baru sudah ada dalam array
    bool newDataExists = false;
    for (int i = 1; i < dataCount; i++) {
      if (data[i] == newData) {
        newDataExists = true;
        break;
      }
    }

    // Jika nilai data baru belum ada, tambahkan ke array
    if (!newDataExists) {
      if (dataCount < MAX_DATA) {
        data[dataCount] = newData;
        dataCount++;
      } else {
        Serial.println("Maksimum jumlah data tercapai.");
      }
    }
  }

  // Periksa apakah sudah satu detik sejak data terakhir dicetak
  unsigned long currentTime = millis();
  if (currentTime - lastPrintTime >= 1000) {
    // Gabungkan semua data dalam array menjadi satu data string dengan tanda newline
    String combinedData = "Reader_2,";
    for (int i = 1; i < dataCount; i++) {
      combinedData += data[i];
      combinedData += ",";
    }

    // Cetak data yang tergabung
    Serial.println("Data yang tergabung:");
    Serial.println(combinedData);

    // Kirim data melalui MQTT
    client.publish(mqtt_topic, combinedData.c_str());

    // Reset array kecuali data pertama
    for (int i = 1; i < dataCount; i++) {
      data[i] = "";
    }
    dataCount = 1;

    // Simpan waktu terakhir saat data dicetak
    lastPrintTime = currentTime;
  }
}
