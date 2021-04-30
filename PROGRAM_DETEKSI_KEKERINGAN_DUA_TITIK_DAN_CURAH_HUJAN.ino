
//Curah hujan adalah jumlah air yang jatuh di permukaan tanah selama periode tertentu yang diukur dengan satuan tinggi milimeter (mm) di atas permukaan horizontal.
//Curah hujan 1 mm adalah jumlah air hujan yang jatuh di permukaan per satuan luas (m2) dengan volume sebanyak 1 liter tanpa ada yang menguap, meresap atau mengalir (Aldrian dkk, 2011).Curah hujan adalah jumlah air yang jatuh di permukaan tanah selama periode tertentu yang diukur dengan satuan tinggi milimeter (mm) di atas permukaan horizontal. Curah hujan 1 mm adalah jumlah air hujan yang jatuh di permukaan per satuan luas (m2) dengan volume sebanyak 1 liter tanpa ada yang menguap, meresap atau mengalir (Aldrian dkk, 2011).

//Perhitungan rumus
//Tinggi curah hujan (cm) = volume yang dikumpulkan (mL) / area pengumpulan (cm2)
//Luas kolektor (Corong) 5,5cm x 3,5cm = 19,25 cm2
//Koleksi per ujung tip kami dapat dengan cara menuangkan 100ml air ke kolektor kemudian menghitung berapa kali air terbuang dari tip,
//Dalam perhitungan yang kami lakukan air terbuang sebanyak 70 kali. 100ml / 70= 1.42mL per tip.
//Jadi 1 tip bernilai 1.42 / 19.25 = 0,07cm atau 0.70 mm curah hujan.

// PENTING
// Nilai kalibrasi yang kami lakukan berlaku untuk semua sensor curah hujan yang kami jual tentu Anda dapat melakukan kalibrasi ulang sendiri jika dibutuhkan.

#include <Wire.h>
#include "RTClib.h"

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Minngu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};
DateTime now;

// Gunakan pin D14 pada NodeMCU, Tegangan 3,3V Kemudian upload code ini
const int pin_interrupt = 14; // Menggunakan pin interrupt https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/

int soil_pin1 = 4; // AOUT pin on sensor
int soil_pin2 = 2; // AOUT pin on sensor

long int jumlah_tip = 0;
long int temp_jumlah_tip = 0;
float curah_hujan = 0.00;
float curah_hujan_per_menit = 0.00;
float curah_hujan_per_jam = 0.00;
float curah_hujan_per_hari = 0.00;
float curah_hujan_per_pekan = 0.00;
float curah_hujan_hari_ini = 0.00;
float curah_hujan_pekan_ini = 0.00;
float temp_curah_hujan_per_menit = 0.00;
float temp_curah_hujan_per_jam = 0.00;
float temp_curah_hujan_per_hari = 0.00;
float temp_curah_hujan_per_pekan = 0.00;
float milimeter_per_tip = 0.70;

float kelembaban;
float kelembaban_tanah;

#define BOTtoken "1538752478:AAGCyJ3gdkx3zlEhlxFxonLJdbKuGwDISFE" //token bot telegram
#define idChat "-465629527" //idbot

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

char ssid[] = "HAYASAKA"; //nama wifi
char password[] = "HAYASAKA"; //password wifi

String cuaca_harian = "Aman";
String cuaca_pekanan = "Aman";

volatile boolean flag = false;

// Inisialisasi struktur waktu
String jam, menit, detik, hari;

void ICACHE_RAM_ATTR hitung_curah_hujan()
{
  flag = true;
}

void setup()
{
  Serial.begin(9600);

  pinMode(pin_interrupt, INPUT);
  attachInterrupt(digitalPinToInterrupt(pin_interrupt), hitung_curah_hujan, FALLING); // Akan menghitung tip jika pin berlogika dari HIGH ke LOW
  // Inisialisasi RTC
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }
  //=======Hanya dibuka komen nya jika akan kalibrasi waktu saja (hanya sekali) setelah itu harus di tutup komennya kembali supaya tidak set waktu terus menerus=======
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set waktu langsung dari waktu PC
  //rtc.adjust(DateTime(2021, 12, 2, 8, 57, 0)); // Set Tahun, bulan, tanggal, jam, menit, detik secara manual
  // Cukup dibuka salah satu dari 2 baris diatas, pilih set waktu secara manual atau dari PC
  //===================================================================================================================================================================
  bacaRTC();
  printSerial();

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  delay(5000);
}

void loop()
{
  if (flag == true) // don't really need the == true but makes intent clear for new users
  {
    curah_hujan += milimeter_per_tip; // Akan bertambah nilainya saat tip penuh
    jumlah_tip++;
    delay(500);
    flag = false; // reset flag
  }

  // Program baca soil moisture titik 1
  float kelembaban1 = (analogRead(soil_pin1)/4096.0) * 3.3;
  float kelembaban_tanah1 = (3.3 - kelembaban1) / 0.95 * 100.0;

  // Program baca soil moisture titik 2
  float kelembaban2 = (analogRead(random(20,30))/4096.0) * 3.3;
  float kelembaban_tanah2 = (3.3 - kelembaban2) / 0.95 * 100.0;

  
  bacaRTC();
  curah_hujan_hari_ini = jumlah_tip * milimeter_per_tip;
  temp_curah_hujan_per_menit = curah_hujan;

  // Algoritma untuk Telegram (notifikasi)
  // Tegangan saat kering 3.3V, saat basah total berair 2.35V
  
  if (curah_hujan_hari_ini > 4.81 && kelembaban1 < 2.92) // Ganti dengan tegangan aja buat kelembaban
  {
    cuaca_harian = "Aman           ";
  }
  if (curah_hujan_hari_ini > 3.44 && curah_hujan_hari_ini < 4.81 && kelembaban < 2.92)
  {
    cuaca_harian = "Waspada      ";
  }
  if (curah_hujan_hari_ini > 4.81 && kelembaban1 > 2.92 && kelembaban1 < 3.11)
  {
    cuaca_harian = "Waspada      ";
  }
  if (curah_hujan_hari_ini < 3.44 && kelembaban1 > 3.11)
  {
    cuaca_harian = "Bahaya       ";
  }
  if (curah_hujan_pekan_ini > 33.72 && kelembaban1 < 2.92) // Ganti dengan tegangan aja buat kelembaban
  {
    cuaca_pekanan = "Aman           ";
  }
  if (curah_hujan_pekan_ini > 24.08 && curah_hujan_hari_ini < 33.72 && kelembaban1  < 2.92)
  {
    cuaca_pekanan = "Waspada      ";
  }
  if (curah_hujan_pekan_ini > 33.72 && kelembaban1 > 2.92 && kelembaban1 < 3.11)
  {
    cuaca_pekanan = "Waspada      ";
  }
  if (curah_hujan_pekan_ini < 24.08 && kelembaban1 > 3.11)
  {
    cuaca_pekanan = "Bahaya       ";
  }

  // Program Rain Gauge
  if (detik.equals("0")) // Hanya print pada detik 0
  {
    curah_hujan_per_menit = temp_curah_hujan_per_menit; // Curah hujan per menit dihitung ketika detik 0
    temp_curah_hujan_per_jam += curah_hujan_per_menit;  // Curah hujan per jam dihitung dari penjumlahan curah hujan per menit namun disimpan dulu dalam variabel temp
    if (menit.equals("0"))
    {
      curah_hujan_per_jam = temp_curah_hujan_per_jam;   // Curah hujan per jam baru dihitung ketika menit 0
      temp_curah_hujan_per_hari += curah_hujan_per_jam; //// Curah hujan per hari dihitung dari penjumlahan curah hujan per jam namun disimpan dulu dalam variabel temp
      
      temp_curah_hujan_per_jam = 0.00;                  // Reset temp curah hujan per jam
    }
    if (menit.equals("0") && jam.equals("0"))
    {
      curah_hujan_per_hari = temp_curah_hujan_per_hari; // Curah hujan per hari baru dihitung ketika menit 0 dan jam 0 (Tengah malam)
      temp_curah_hujan_per_hari = 0.00;                 // Reset temp curah hujan per hari
      temp_curah_hujan_per_pekan += curah_hujan_per_hari; // Curah hujan per pekan dihitung dari curah hujan per hari,  namun disimpan dulu dalam temp
      curah_hujan_hari_ini = 0.00;                      // Reset curah hujan hari ini
    }
    if (hari.equals("Minggu") && jam.equals("0") && menit.equals("0"))
    {
      curah_hujan_per_pekan += temp_curah_hujan_per_pekan; // Curah hujan per jam baru dihitung ketika menit 0 dan jam 0 (Tengah malam)
      temp_curah_hujan_per_pekan = 0.00;                 // Reset temp curah hujan per hari
      curah_hujan_pekan_ini = 0.00;                      // Reset curah hujan pekan ini
      jumlah_tip = 0;                                   // Jumlah tip di reset setiap satu pekan sekali
    }
    temp_curah_hujan_per_menit = 0.00;
    curah_hujan = 0.00;
    delay(1000);
  }
  
    if ((jumlah_tip != temp_jumlah_tip) || (detik.equals("0"))) // Print serial setiap 1 menit atau ketika jumlah_tip berubah
    {
      printSerial();
    }
    temp_jumlah_tip = jumlah_tip;


      // Program Telegram
  if (detik.equals("0")) // Hanya print pada detik 0
  {
    if (menit.equals("0") && jam.equals("6"))
    {
      bot.sendChatAction(idChat, "Sedang mengetik...");
      String rainday = "Curah hujan harian saat ini : ";
      rainday += int(curah_hujan_per_hari);
      rainday += " mm\n";
      rainday += "Curah hujan harian saat ini : ";
      rainday += int(curah_hujan_per_pekan);
      rainday += " mm\n";
      rainday += "Kelembaban tanah saat ini : ";
      rainday += int(kelembaban_tanah);
      rainday += " %\n";
      bot.sendMessage(idChat, rainday, "");
      Serial.print("Mengirim data sensor ke telegram");
    }
    if (menit.equals("0") && jam.equals("12"))
    {
      bot.sendChatAction(idChat, "Sedang mengetik...");
      String rainday = "Curah hujan harian saat ini : ";
      rainday += int(curah_hujan_per_hari);
      rainday += " mm\n";
      rainday += "Curah hujan harian saat ini : ";
      rainday += int(curah_hujan_per_pekan);
      rainday += " mm\n";
      rainday += "Kelembaban tanah saat ini : ";
      rainday += int(kelembaban_tanah);
      rainday += " %\n";
      bot.sendMessage(idChat, rainday, "");
      Serial.print("Mengirim data sensor ke telegram");
    }
    if (menit.equals("0") && jam.equals("18"))
    {
      bot.sendChatAction(idChat, "Sedang mengetik...");
      String rainday = "Curah hujan harian saat ini : ";
      rainday += int(curah_hujan_per_hari);
      rainday += " mm\n";
      rainday += "Curah hujan harian saat ini : ";
      rainday += int(curah_hujan_per_pekan);
      rainday += " mm\n";
      rainday += "Kelembaban tanah saat ini : ";
      rainday += int(kelembaban_tanah);
      rainday += " %\n";
      bot.sendMessage(idChat, rainday,  "");
      Serial.print("Mengirim data sensor ke telegram");
    }
    if (menit.equals("0") && jam.equals("0"))
    {
      bot.sendChatAction(idChat, "Sedang mengetik...");
      String rainday = "Curah hujan harian saat ini : ";
      rainday += int(curah_hujan_per_hari);
      rainday += " mm\n";
      rainday += "Curah hujan harian saat ini : ";
      rainday += int(curah_hujan_per_pekan);
      rainday += " mm\n";
      rainday += "Kelembaban tanah saat ini : ";
      rainday += int(kelembaban_tanah);
      rainday += " %\n";
      bot.sendMessage(idChat, rainday,  "");
      Serial.print("Mengirim data sensor ke telegram");
    }
  }
 }
  
String konversi_jam(String angka) // Fungsi untuk supaya jika angka satuan ditambah 0 di depannya, Misalkan jam 1 maka jadi 01 pada LCD
{
  if (angka.length() == 1)
  {
    angka = "0" + angka;
  }
  else
  {
    angka = angka;
  }
  return angka;
}

void bacaRTC()
{
  now = rtc.now(); // Ambil data waktu dari DS3231
  jam = String(now.hour(), DEC);
  menit = String(now.minute(), DEC);
  detik = String(now.second(), DEC);
  hari = String(daysOfTheWeek[now.dayOfTheWeek()]);
}

void printSerial()
{
  Serial.println(konversi_jam(jam) + ":" + konversi_jam(menit));
  Serial.print("Cuaca=");
  Serial.println(cuaca_harian); // Print cuaca hari ini (Ini bukan ramalan cuaca tapi membaca cuaca yang sudah terjadi/ sedang terjadi hari ini)
  Serial.print("Jumlah tip=");
  Serial.print(jumlah_tip);
  Serial.println(" kali ");
  Serial.print("Curah hujan hari ini=");
  Serial.print(curah_hujan_hari_ini, 1);
  Serial.println(" mm ");
  Serial.print("Curah hujan per menit=");
  Serial.print(curah_hujan_per_menit, 1);
  Serial.println(" mm ");
  Serial.print("Curah hujan per jam=");
  Serial.print(curah_hujan_per_jam, 1);
  Serial.println(" mm ");
  Serial.print("Curah hujan per hari=");
  Serial.print(curah_hujan_per_hari, 1);
  Serial.println(" mm ");
  Serial.print("Curah hujan pekan ini=");
  Serial.print(curah_hujan_pekan_ini, 1);
  Serial.println(" mm ");
}
