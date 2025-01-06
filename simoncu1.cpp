#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <Adafruit_BME280.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include "DHT.h"
#include <BH1750.h> // SENSOR BH1750 INTENSITAS CAHAYA
#include <MQ135.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

BH1750 light_sensor(0x23);

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ 18, /* data=*/ 23, /* CS=*/ 5, /* reset=*/ 4); // ESP32

#define DHTPIN 17       // Pin digital 17
#define DHTTYPE DHT22
const int hujan = 32;  // Sensor Hujan
#define led1 15        // Pin LED hijau
#define led2 16        // Pin LED kuning
#define led3 19        // Pin LED Merah
#define PIN_MQ135 35    // MQ135
#define BUTTON_PIN1 12    // Pin push button next (gunakan pin GPI13 pada ESP32)
#define BUTTON_PIN2 14    // Pin push button  back (gunakan pin GPI12 pada ESP32)
#define BUTTON_PIN3 27    // Pin push button mode cuaca(gunakan pin GPI14 pada ESP32)
#define BUTTON_PIN4 26    // Pin push button mode otomatis/manual (gunakan pin GPI27 pada ESP32)
#define BUTTON_PIN5 25    // Pin push button mode otomatis/manual (gunakan pin GPI27 pada ESP32)
#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

// Wifi network station credentials----------------------------------------------
#define WIFI_SSID "Infinix ZERO 5G"
#define WIFI_PASSWORD "12345678"

// Telegram BOT Token (Get from Botfather)----------------------------------------
#define BOT_TOKEN "7864138171:AAESJ5RLL_lmkwzzc4vPTKEGNiXmvUBKfDc"
#define CHAT_ID "7813126295"
WiFiClientSecure secured_client;

UniversalTelegramBot bot(BOT_TOKEN, secured_client);
const unsigned long BOT_MTBS = 50; // mean time between scan messages
unsigned long bot_lasttime; // last time messages' scan has been done
unsigned long delayTime; //BMP280

unsigned long previousMillis = 0;  // Variabel untuk melacak waktu
const long interval = 5000;        // Interval 5 detik
bool isAutomated = false;          // Status perintah otomatis

MQ135 mq135_sensor(PIN_MQ135);  // parameter sensor gas

char t_string[5];
char h_string[5];
char lux_string[6];
char status_string[12]; // Buffer untuk status cuaca
char tekanan_string[5];//tekanan udara
char kecepatan_kilometer_per_jam_string[5];
float t_float;
float h_float;
float lux;
float tekanan;
float tekanan_atm;
float MDPL;



DHT dht(DHTPIN, DHTTYPE);

int currentPage = 0; // Variabel untuk menyimpan halaman saat ini
int menuPage = 0; // Variabel untuk halaman menu

// parameters Anemometer
/*volatile*/ float rpmcount; // hitung signals
volatile unsigned long last_micros;
unsigned long timeold;
unsigned long timemeasure = 1.00; // detik
int timetoSleep = 1;               // menit
unsigned long sleepTime = 15;      // menit
unsigned long timeNow;
int countThing = 0;
int GPIO_pulse = 2;               // ESP32 = D14
float rpm, rotasi_per_detik;       // rotasi/detik
float kecepatan_kilometer_per_jam; // kilometer/jam
float kecepatan_meter_per_detik;   //meter/detik
float calibration_value = 5.0;     // Nilai ini diperoleh dari perbandingan dengan sensor anemometer pabrikan
volatile boolean flag = false;

void ICACHE_RAM_ATTR rpm_anemometer()
{
  flag = true;
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
  u8g2.begin();
  drawIntro(); // Menampilkan intro
  Serial.println("LCD GRAPHIC MEMULAI");
  dht.begin();             // Sensor DHT22 memulai
  Serial.println("Sensor suhu dan kelembapan, DHT22 bekerja");
  u8g2.setColorIndex(1);
  light_sensor.begin();    // Sensor BH1750 intensitas cahaya
  Serial.println("Sensor Intensitas Cahaya, BH1750 bekerja");
  Serial.println(F("BME280 test"));
  bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();

  pinMode(hujan, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(BUTTON_PIN1, INPUT_PULLUP); // Inisialisasi push button
  pinMode(BUTTON_PIN2, INPUT_PULLUP); // Inisialisasi push button
  pinMode(BUTTON_PIN3, INPUT_PULLUP); // Inisialisasi push button
  pinMode(BUTTON_PIN4, INPUT_PULLUP); // Inisialisasi push button
  pinMode(BUTTON_PIN5, INPUT_PULLUP); // Inisialisasi push button
  pinMode(GPIO_pulse, INPUT_PULLUP); // inisiasi pin anemometer
  digitalWrite(GPIO_pulse, LOW);
  // inisaiasi anemometer
  detachInterrupt(digitalPinToInterrupt(GPIO_pulse));                         // memulai Interrupt pada nol
  attachInterrupt(digitalPinToInterrupt(GPIO_pulse), rpm_anemometer, RISING); //Inisialisasi pin interupt
  rpmcount = 0;
  rpm = 0;
  timeold = 0;
  timeNow = 0;
  //menghubungkan kke wifi--------------------------------------------------------------------------------
  // attempt to connect to Wifi network:
  // Coba menghubungkan ke WiFi tetapi tidak memblokir loop jika gagal
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  unsigned long wifi_start_time = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - wifi_start_time < 5000) { // Maksimal 10 detik untuk mencoba koneksi
    delay(500);
    Serial.print(".");

  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("\nWiFi connected. IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi not connected. Continuing without WiFi.");
  }
}

void loop() {
  sendAutomatedWeatherUpdate();
  // Membaca status push button (jika ditekan)
  if (digitalRead(BUTTON_PIN2) == LOW) {
    currentPage = (currentPage + 1) % 5; // Ganti halaman (loop 0-1)
    delay(100); // Debouncing delay
  }
  if (digitalRead(BUTTON_PIN1) == LOW) {
    // Navigasi mundur (decrement halaman)
    currentPage = (currentPage + 4) % 5;  // Ganti halaman mundur (loop 0-4)
    delay(100);  // Debouncing delay
  }
  // Tambahkan ini untuk mengakses halaman menu
  if (digitalRead(BUTTON_PIN3) == LOW) {
    currentPage = 0; // Halaman Utama
    delay(100); // Debouncing delay
  }
  // Jika BUTTON_PIN4 ditekan, tampilkan prediksi cuaca
  if (digitalRead(BUTTON_PIN4) == LOW) {
    digitalWrite(led2, HIGH);
    currentPage = 3; // Halaman 4
    delay(100); // Debouncing
  }
  if (digitalRead(BUTTON_PIN5) == LOW) {
    Serial.println("Tombol Reset Ditekan. Reset perangkat...");
    digitalWrite(led3, HIGH);
    delay(300); // Debounce
    ESP.restart();  // Mereset perangkat ESP32
  }
  // Sensor BH1750 (intensitas cahaya)
  lux = light_sensor.readLightLevel();
  Serial.print("Intensitas Cahaya Matahari: ");
  Serial.print(lux);
  Serial.println(" Lx");

  // Sensor Hujan
  int rainValue = analogRead(hujan);
  float voltage = rainValue * (3.3 / 4095.0); // Konversi ke tegangan (untuk ESP32)
  if (voltage < 1.0) {
    Serial.println("Kondisi: Hujan.");
    digitalWrite(led3, LOW);
    digitalWrite(led2, HIGH);
    digitalWrite(led1, HIGH);
    strcpy(status_string, "Hujan");
  } else if (voltage < 2.5) {
    Serial.println("Kondisi: Gerimis.");
    digitalWrite(led2, LOW);
    digitalWrite(led1, HIGH);
    digitalWrite(led3, HIGH);
    strcpy(status_string, "Gerimis");
  } else {
    Serial.println("Kondisi: Cerah.");
    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);
    digitalWrite(led3, HIGH);
    strcpy(status_string, "Cerah");
  }
  Serial.print("Kondisi Cuaca: ");
  Serial.println(status_string);

  // SENSOR ANEMOMETER
  // Cek apakah flag bernilai true, yang menandakan bahwa kondisi tertentu telah terpenuhi
  if (flag == true)
  {
    // Cek apakah waktu sejak deteksi terakhir sudah lebih dari atau sama dengan 5000 mikrodetik (5 milidetik)
    if (long(micros() - last_micros) >= 5000)
    {
      // Jika ya, maka tambahkan satu ke penghitung rpm (rpmcount)
      rpmcount++;
      // Perbarui waktu terakhir ketika magnet terdeteksi dengan waktu saat ini
      last_micros = micros();
    }
    // Setelah memproses, setel flag kembali ke false untuk menandakan bahwa kondisi telah ditangani
    flag = false; // reset flag
  }
  if ((millis() - timeold) >= timemeasure * 10000)
  {
    countThing++;
    detachInterrupt(digitalPinToInterrupt(GPIO_pulse));                                                                     // Menonaktifkan interrupt saat menghitung
    rotasi_per_detik = float(rpmcount) / float(timemeasure);                                                                // rotasi per detik
    //kecepatan_meter_per_detik = rotasi_per_detik; // rotasi/detik sebelum dikalibrasi untuk dijadikan meter per detik
    kecepatan_meter_per_detik = ((-0.0181 * (rotasi_per_detik * rotasi_per_detik)) + (1.3859 * rotasi_per_detik) + 1.4055); // meter/detik sesudah dikalibrasi dan sudah dijadikan meter per detik
    if (kecepatan_meter_per_detik <= 1.5)
    { // Minimum pembacaan sensor kecepatan angin adalah 1.5 meter/detik
      kecepatan_meter_per_detik = 0.0;
    }
    kecepatan_kilometer_per_jam = kecepatan_meter_per_detik * 3.6; // kilometer/jam
    Serial.print("rotasi_per_detik=");
    Serial.print(rotasi_per_detik);
    Serial.print("   kecepatan_meter_per_detik="); // Minimal kecepatan angin yang dapat dibaca sensor adalah 4 meter/detik dan maksimum 30 meter/detik.
    Serial.print(kecepatan_meter_per_detik);
    Serial.print("   kecepatan_kilometer_per_jam=");
    Serial.print(kecepatan_kilometer_per_jam);
    Serial.println("   ");
    if (countThing == 1) // kirim data per 10 detik sekali
    {
      Serial.println("Mengirim data ke server");
      countThing = 0;
    }
    timeold = millis();
    rpmcount = 0;
    attachInterrupt(digitalPinToInterrupt(GPIO_pulse), rpm_anemometer, RISING); // enable interrupt
  }

  // Jika `menuPage` aktif, tampilkan halaman menu
  u8g2.firstPage();
  do {
    if (currentPage == 0) {
      drawPage1(); // Halaman 1: Suhu, Kelembapan, Cahaya, Status Hujan
    } else if (currentPage == 1) {
      drawPage2(); // Halaman 2: Sensor MQ135
    } else if (currentPage == 2) {
      drawPage3(); // Halaman 3: Tambahkan halaman 3
    } else if (currentPage == 3) {
      drawPage4(); // Halaman 4: Tambahkan halaman 4
    } else if (currentPage == 4) {
      drawPage5(); // Halaman 5: Tambahkan halaman 5
    }
  } while (u8g2.nextPage());

  delay(1000);
  //PESAN TEKS KE TELEGRAM---------------------------------------------
  // Jika WiFi terhubung, lakukan komunikasi Telegram
  if (WiFi.status() == WL_CONNECTED) {
    if (millis() - bot_lasttime > BOT_MTBS) {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      while (numNewMessages) {
        Serial.println("got response");
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
      bot_lasttime = millis();
    }
  }
  printValues();
  delay(delayTime);

}

void drawPage1() {
  h_float = dht.readHumidity();
  t_float = dht.readTemperature();
  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawFrame(0, 0, 40, 31);         // upper frame
  u8g2.drawFrame(0, 33, 40, 31);        // lower frame
  u8g2.drawFrame(42, 0, 40, 31);
  u8g2.drawFrame(42, 33, 40, 31);
  u8g2.drawFrame(84, 0, 40, 31);
  u8g2.drawFrame(84, 33, 40, 31);

  // KONVERSI DARI strings BAGIAN DHT22
  // SUHU
  u8g2.drawStr( 2, 27, "Celsius");
  u8g2.setFont(u8g2_font_ncenB10_tr);
  dtostrf(t_float, 3, 1, t_string);
  u8g2.drawStr(5, 15, t_string);

  // KELEMBAPAN
  u8g2.setFont(u8g2_font_4x6_mf);
  u8g2.drawStr( 2, 60, "Kelembapan ");
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(34, 48, "%");
  u8g2.setFont(u8g2_font_ncenB10_tr);
  dtostrf(h_float, 3, 1, h_string);
  u8g2.drawStr(5, 48, h_string);

  // INTENSITAS CAHAYA
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr( 47, 27, " Lux");
  dtostrf(lux, 5, 1, lux_string);
  u8g2.drawStr(47, 15, lux_string);

  // STATUS HUJAN
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(47, 48, "CUACA ");
  u8g2.drawStr(47, 60, status_string); // Menampilkan status cuaca
  //kecepatan Angin
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(86, 27, "KM/J "); //kecepatan_kilometer_per_jam_string
  dtostrf(kecepatan_kilometer_per_jam, 5, 1, kecepatan_kilometer_per_jam_string);
  u8g2.drawStr(86, 12, kecepatan_kilometer_per_jam_string);
  // TOMBOL tekanan udara
  tekanan = bme.readPressure() / 100.0F; // Tekanan dalam hPa
  tekanan_atm = tekanan / 1013.25;
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(86, 55, " atm ");
  //u8g2.drawStr(86, 60, "  Udara");
  u8g2.setFont(u8g2_font_ncenB10_tr);
  dtostrf(tekanan_atm, 3, 1, tekanan_string);
  u8g2.drawStr(86, 48, tekanan_string);
}

void drawPage2() {
  // Pembacaan sensor MQ135
  float rzero = mq135_sensor.getRZero();
  float correctedRZero = mq135_sensor.getCorrectedRZero(t_float, h_float);
  float resistance = mq135_sensor.getResistance();
  //float adjustedPPM = ppm / 10.0;
  float ppm = mq135_sensor.getPPM();
  float correctedPPM = mq135_sensor.getCorrectedPPM(t_float, h_float);
  //float gasConcentration = 0.0;
  //float gasConcentrationPPM = 0.0;  // Konsentrasi gas dalam PPM (parts per million)
  String statusUdara;
  // Menentukan status udara berdasarkan nilai PPM
  if (ppm <= 400) {
    statusUdara = "Kadar CO2 Rendah";
  } else if (ppm > 400 && ppm <= 5000) {
    statusUdara = "Kadar CO2 Sedang";
  } else {
    statusUdara = "Kadar CO2 Berbahaya";
  }

  u8g2.clearBuffer();
  u8g2.setFontMode(1);
  u8g2.drawFrame(0, 0, 120, 31);         // upper frame
  u8g2.drawFrame(0, 33, 120, 31);        // lower frame
  u8g2.setFont(u8g2_font_6x10_mf);

  // Tampilkan data dari sensor MQ135
  u8g2.drawStr(0, 10, "KADAR CO2:");
  u8g2.drawStr(0, 20, "data sensor MQ135");
  // Tampilkan status udara
  u8g2.setCursor(0, 30);
  u8g2.print("PPM: ");
  u8g2.print(ppm, 2);

  u8g2.setCursor(0, 50);
  u8g2.print("   ");
  u8g2.print(statusUdara);

  u8g2.sendBuffer();
}
/*
  // Tampilkan hasil pembacaan dalam beberapa baris
  u8g2.drawStr(0, 20, "RZero:");
  u8g2.setCursor(50, 20);
  u8g2.print(rzero, 2);

  u8g2.drawStr(0, 30, "Corrected RZero:");
  u8g2.setCursor(90, 30);
  u8g2.print(correctedRZero, 2);

  u8g2.drawStr(0, 40, "Resistance:");
  u8g2.setCursor(70, 40);
  u8g2.print(resistance, 2);

  u8g2.drawStr(0, 50, "PPM:");
  u8g2.setCursor(50, 50);
  u8g2.print(ppm, 2);

  u8g2.drawStr(0, 60, "Corrected PPM:");
  u8g2.setCursor(90, 60);
  u8g2.print(correctedPPM, 2);
  }*/

void drawPage3() {
  // Implementasi halaman 3
  float rzero = mq135_sensor.getRZero();
  float correctedRZero = mq135_sensor.getCorrectedRZero(t_float, h_float);
  float resistance = mq135_sensor.getResistance();
  //float adjustedPPM = ppm / 10.0;
  float ppm = mq135_sensor.getPPM();
  float correctedPPM = mq135_sensor.getCorrectedPPM(t_float, h_float);
  // Tampilkan data dari sensor MQ135
  const float a = 1.0;  // Adjust based on datasheet or calibration
  const float b = 3.6;  // Adjust based on datasheet or calibration
  const float r0 = correctedRZero;  // R0 value from sensor

  // Menghitung PPM CO (Formula berdasarkan datasheet dan kalibrasi sensor)
  float ppm_CO = a * pow((resistance / r0), b);

  // Menentukan status udara berdasarkan nilai PPM CO
  String statusUdara;
  if (ppm_CO <= 9) {
    statusUdara = "Kadar CO Rendah";
  } else if (ppm_CO > 9 && ppm_CO <= 35) {
    statusUdara = "Kadar CO Sedang";
  } else {
    statusUdara = "Kadar CO Berbahaya";
  }

  // Menampilkan hasil pada display secara real-time
  u8g2.clearBuffer();  // Membersihkan buffer
  u8g2.setFontMode(1);  // Menyalakan mode font
  u8g2.drawFrame(0, 0, 128, 31);  // Gambar bingkai atas
  u8g2.drawFrame(0, 33, 128, 31); // Gambar bingkai bawah

  // Tampilkan data dari sensor MQ135
  u8g2.setFont(u8g2_font_6x10_mf);  // Menetapkan font untuk teks
  u8g2.drawStr(0, 10, "KADAR CO:");
  u8g2.setFont(u8g2_font_4x6_tr);
  u8g2.drawStr(0, 20, "data sensor MQ135");
  u8g2.setCursor(0, 30);
  u8g2.setFont(u8g2_font_6x10_mf);  // Menetapkan font untuk teks
  u8g2.print("PPM CO: ");
  u8g2.print(ppm_CO, 2);  // Menampilkan nilai PPM CO

  u8g2.setCursor(0, 50);
  u8g2.print("   ");
  u8g2.print(statusUdara);  // Tampilkan status udara berdasarkan nilai PPM CO

  u8g2.sendBuffer();  // Kirim buffer ke layar

  // Delay agar pembaruan dapat terlihat
  delay(1000);  // Pembaruan setiap 1 detik
}

void drawPage4() {
  // Membersihkan buffer sebelum menggambar
  u8g2.clearBuffer();

  // Menampilkan bingkai pada display
  u8g2.drawFrame(0, 0, 128, 11);  // Bingkai atas (judul)
  u8g2.drawFrame(0, 13, 128, 50); // Bingkai bawah (konten)

  // Menampilkan judul prediksi cuaca
  u8g2.setFont(u8g2_font_6x10_mf); // Mengatur font kecil
  u8g2.drawStr(25, 10, "PREDIKSI CUACA");

  // Validasi data sensor sebelum digunakan
  u8g2.setFont(u8g2_font_4x6_mf); // Mengatur font kecil
  if (h_float < 0 || h_float > 100 || lux < 0) {
    // Jika data sensor tidak valid, tampilkan peringatan
    u8g2.drawStr(10, 35, "Error: Sensor Invalid");
  } else {
    // Logika prediksi cuaca berdasarkan data sensor yang valid
    if (h_float > 80 && lux < 500) {
      u8g2.drawStr(10, 50, "Hujan Deras");
    } else if (h_float > 60 && lux >= 500 && lux < 2000) {
      u8g2.drawStr(10, 50, "Hujan Ringan");
    } else if (lux >= 2000 && h_float <= 60) {
      u8g2.drawStr(10, 50, "Cerah");
    } else {
      u8g2.drawStr(10, 50, "Berawan");
    }
  }

  // Kirim buffer untuk ditampilkan di layar
  u8g2.sendBuffer();
}

void drawPage5() {
  // Implementasi halaman 5
  // Menampilkan hasil pada display secara real-time
  u8g2.clearBuffer();  // Membersihkan buffer
  u8g2.setFontMode(1);  // Menyalakan mode font
  u8g2.drawFrame(0, 0, 128, 31);  // Gambar bingkai atas
  u8g2.drawFrame(0, 33, 128, 31); // Gambar bingkai bawah
  // Tampilkan data dari sensor MQ135
  u8g2.setFont(u8g2_font_6x10_mf);  // Menetapkan font untuk teks
  // Menampilkan judul halaman
  u8g2.setFont(u8g2_font_6x10_mf); // Mengatur font kecil u8g2_font_10x20_tf
  u8g2.drawStr(10, 10, "KETINGGIAN TANAH (m)");

  // Mengambil data tekanan atmosfer dari sensor BMP280
  float tekanan = bme.readPressure(); // Membaca tekanan dalam Pascal
  float ketinggian = bme.readAltitude(1013.25); // 1013.25 adalah tekanan di permukaan laut (hPa)

  // Menampilkan data tekanan (opsional)
  u8g2.drawStr(10, 42, "Tekanan (hPa):");
  u8g2.setFont(u8g2_font_10x20_tf);
  u8g2.setCursor(10, 60);
  u8g2.print(tekanan / 100.0); // Konversi Pascal ke hPa

  // Menampilkan data ketinggian
  u8g2.setFont(u8g2_font_10x20_tf);
  u8g2.setCursor(10, 30);
  u8g2.print(ketinggian, 2); // Menampilkan ketinggian dengan 2 desimal

  // Mengirim buffer ke display
  u8g2.sendBuffer();
  u8g2.setFont(u8g2_font_6x10_mf);
}

void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID )
    {
      bot.sendMessage(chat_id, "PENGGUNA TIDAK DIKENAL", "");
    }
    else
    {
      String text = bot.messages[i].text;
      String from_name = bot.messages[i].from_name;
      if (from_name == "")
        from_name = "Guest";
      if (text == "/suhu")
      { String msg = "Temperature suhu adalah ";
        msg += msg.concat(t_float);
        msg += "C";
        bot.sendMessage(chat_id, msg, "");
      }
      /*if (text == "/tempF")
        {
          String msg = "Temperature is ";
          msg += msg.concat(temperatureF);
          msg += "F";
          bot.sendMessage(chat_id,msg, "");
        }*/
      if (text == "/lembab")
      {
        String msg = "kelembapan udara adalah ";
        msg += msg.concat(h_float);
        msg += "%";
        bot.sendMessage(chat_id, msg, "");
      }
      if (text == "/cahaya")
      {
        String msg = "Intensitas Cahaya adalah ";
        msg += msg.concat(lux);
        msg += "lux";
        bot.sendMessage(chat_id, msg, "");
      }
      if (text == "/pressure")
      {
        String msg = "Tekanan udara sekarang adalah ";
        msg += msg.concat(tekanan_atm);
        msg += "Atmosfer";
        bot.sendMessage(chat_id, msg, "");
      }
      if (text == "/angin")
      {
        String msg = "Kecepatan angin yang terbaca adalah ";
        msg += msg.concat(kecepatan_kilometer_per_jam);
        msg += "  KM/Jam";
        bot.sendMessage(chat_id, msg, "");
      }
      if (text == "/status")
      {
        String msg = "Status Cuaca Sekarang Adalah";
        msg += msg.concat(status_string);
        msg += "  ";
        bot.sendMessage(chat_id, msg, "");
      }
      if (text == "/otomatis")
      {
        isAutomated = true;  // Mengaktifkan pengiriman otomatis
        bot.sendMessage(chat_id, "Monitoring cuaca realtime diaktifkan. Data akan dikirim setiap 5 detik.", "");
      }
      if (text == "/manual")
      {
        isAutomated = false;  // Menonaktifkan pengiriman otomatis
        bot.sendMessage(chat_id, "Monitoring cuaca manual diaktifkan. Pengiriman data berhenti.", "");
      }
      if (text == "/start")
      {
        String welcome = "SELAMAT DATANG DI SISTEM MONITORING CUACA BERKELANJUTAN       tekan tombol atau tulis perintah dibawah ini untuk menjalankan perintah.\n";
        welcome += "/otomatis : Menampilkan Monitoring cuaca realtime \n";
        welcome += "/manual : Menampilkan Monitoring cuaca manual \n";
        welcome += "/suhu : suhu dalam celcius \n";
        //welcome += "/tempF : Temperature in faranthit\n";
        welcome += "/lembab : Kelembapan Udara adalah: \n";
        welcome += "/cahaya : Menampilkan Intensitas Cahaya  :\n";
        welcome += "/pressure : menampilkan tekanan udara :\n";
        welcome += "/angin : Menampilkan Kecepatan Angin:\n";
        welcome += "/status : Menampilkan Kondisi Cuaca Sekarang :\n";
        welcome += "/Info : Informasi Lebih Lanjut :\n";
        welcome += "/help : Tombol Bantuan :\n";
        bot.sendMessage(chat_id, welcome, "Markdown");
      }
      // Perintah Info
      if (text == "/Info")
      {
        String info = "Informasi tentang alat:\n";
        info += "Nama Alat: Sistem Monitoring Cuaca Berkelanjutan\n";
        info += "Pembuat:\n";
        info += "- Wisnu Kusuma Sandjaya (3100210012)\n";
        info += "- Yahya Harista Ghani (3100210014)\n";
        info += "- Nur Rahman (3112230003)";
        info += "/sensor : Sensor-sensor yang digunakan pada alat:\n";
        info += "/start : Kembali ke menu utama:\n";
        bot.sendMessage(chat_id, info, "Markdown");
      }
      if (text == "/sensor")
      {
        String sensor = "Informasi tentang sensor-sensor yang digunakan pada alat:\n";
        sensor += "Nama Alat: Sistem Monitoring Cuaca Berkelanjutan\n";
        sensor += "Pembuat:\n";
        sensor += "- DHT22  : Sensor Suhu dan Kelembapan Udara\n";
        sensor += "- BMP280 : Sensor tekanan dan ketinggian tanah \n";
        sensor += "- BH1750 : Sensor Intensitas Cahaya \n";
        sensor += "- Raindrop Sensor : Sensor Hujan \n";
        sensor += "- Anemometer : Sensor Kecepatan Angin \n";
        sensor += "- MQ135 : Sensor Gas \n";
        sensor += "/start : Kembali ke menu utama:\n";
        bot.sendMessage(chat_id, sensor, "Markdown");
      }
    }
  }
}
void drawIntro() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(20, 20, "Weather");
  u8g2.drawStr(20, 40, "Monitoring");
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(30, 60, "KELOMPOK 3");
  u8g2.sendBuffer();
  delay(2000); // Menampilkan intro selama 2 detik
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(40, 20, "Disusun Oleh");
  u8g2.drawStr(5, 30, "Wisnu Kusuma Sandjaya");
  u8g2.drawStr(5, 40, "Yahya Harista Ghani");
  u8g2.drawStr(5, 50, "Nur Rahman");
  u8g2.setFont(u8g2_font_5x7_tr);
  u8g2.drawStr(5, 57, "- PROYEK KETEKNIKAN");
  u8g2.sendBuffer();
  delay(2000); // Menampilkan intro selama 2 detik
}
void drawMenuPage() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_mf);
  u8g2.drawStr(30, 20, "Weather Menu");
  u8g2.drawStr(10, 40, "1. Real-time Monitoring");
  u8g2.drawStr(10, 50, "2. Manual Mode");
  u8g2.drawStr(10, 60, "3. Back");
}
void printValues() {
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.println();
}
void sendAutomatedWeatherUpdate() {
  if (isAutomated) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;  // Update waktu terakhir
      // Kirimkan data monitoring cuaca secara otomatis setiap 5 detik
      String automatedMsg = "Data Monitoring Cuaca:\n";
      automatedMsg += "Temperature: " + String(t_float) + "C\n";
      automatedMsg += "Humidity: " + String(h_float) + "%\n";
      automatedMsg += "Light Intensity: " + String(lux) + " lux\n";
      automatedMsg += "Pressure: " + String(tekanan_atm) + " atm\n";
      automatedMsg += "Wind Speed: " + String(kecepatan_kilometer_per_jam) + " km/h\n";
      automatedMsg += "Weather Status: " + String(status_string) + "\n";
      bot.sendMessage(CHAT_ID, automatedMsg, "");
    }
  }
}
/*void prediksiCuaca() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB10_tr);
  u8g2.drawStr(0, 15, "Prediksi Cuaca");
  u8g2.setFont(u8g2_font_5x7_tr);

  if (h_float > 70 && lux < 500) {
    u8g2.drawStr(0, 35, "Hujan Lebat");
  } else if (h_float > 60 && lux >= 500 && lux < 2000) {
    u8g2.drawStr(0, 35, "Hujan Ringan");
  } else if (lux > 2000) {
    u8g2.drawStr(0, 35, "Cerah");
  } else {
    u8g2.drawStr(0, 35, "Berawan");
  }
  u8g2.sendBuffer();
  }*/