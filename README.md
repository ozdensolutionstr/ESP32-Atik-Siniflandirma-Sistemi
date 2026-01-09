# ESP32 Atık Sınıflandırma Sistemi (TAK)

## 📋 Proje Hakkında

Bu proje, ESP32 A1 kartı kullanarak atık türlerini otomatik olarak sınıflandıran akıllı bir geri dönüşüm sistemidir. Sistem, Google Gemini AI API'sini kullanarak görüntü analizi yapar ve kullanıcılara puan sistemi ile geri dönüşüm teşviki sağlar.

**Geliştirici:** Özden Solutions  
**İletişim:** ozdensolutions@icloud.com

## ✨ Özellikler

### 🎯 Ana Özellikler
- **AI Destekli Görüntü Analizi:** Google Gemini 2.0 Flash modeli ile gerçek zamanlı atık sınıflandırma
- **Çoklu Kullanıcı Desteği:** 5 farklı kullanıcı için ayrı puan takibi
- **Puan Sistemi:** Atık türüne göre farklı puan değerleri
- **Web Tabanlı Arayüz:** Modern, responsive ve kullanıcı dostu web arayüzü
- **Otomatik Görüntü İşleme:** Görüntü sıkıştırma ve optimizasyon
- **Yerel Veri Saklama:** LocalStorage ile kullanıcı verilerinin kalıcı saklanması

### 🗂️ Desteklenen Atık Türleri
- **Plastik** - 10 puan
- **Kağıt** - 15 puan
- **Metal** - 20 puan
- **Cam** - 8 puan

## 🏗️ Sistem Mimarisi

### Donanım Bileşenleri
- **ESP32 A1 Kartı:** Ana işlemci ve web sunucusu
- **Wi-Fi Modülü:** İnternet bağlantısı için
- **SPIFFS:** Logo ve arka plan görselleri için dosya sistemi
- **NEMA 17 Step Motor:** Atık sınıflandırma mekanizması için hareket kontrolü
- **CW422C Step Motor Sürücü:** NEMA 17 motor kontrolü için sürücü modülü
- **2x Servo Motor:** Ek hareket kontrolü ve mekanizma yönetimi için

### Yazılım Bileşenleri
- **Arduino Framework:** ESP32 programlama
- **WebServer:** HTTP sunucu (Port 80)
- **WiFiClientSecure:** HTTPS bağlantıları için
- **Google Gemini API:** Görüntü analizi için AI servisi

### Sistem Akışı
```
1. Kullanıcı fotoğraf yükler
   ↓
2. Görüntü sıkıştırılır ve optimize edilir (max 300px)
   ↓
3. Base64 formatına dönüştürülür
   ↓
4. Google Gemini API'ye gönderilir
   ↓
5. AI analiz sonucu döner
   ↓
6. Atık türü belirlenir ve puan hesaplanır
   ↓
7. Motor kontrolü aktif edilir (NEMA 17 + Servo motorlar)
   ↓
8. Atık uygun bölmeye yönlendirilir
   ↓
9. Kullanıcı puanı güncellenir
   ↓
10. Sonuç kullanıcıya gösterilir
```

## 📦 Gereksinimler

### Donanım
- ESP32 A1 geliştirme kartı
- USB kablosu (programlama için)
- Wi-Fi erişimi olan ağ
- NEMA 17 Step Motor
- CW422C Step Motor Sürücü modülü
- 2x Servo Motor
- Motor kontrolü için güç kaynağı
- Bağlantı kabloları ve breadboard (isteğe bağlı)

### Yazılım
- Arduino IDE (v1.8.13 veya üzeri)
- ESP32 Board Support Package
- Aşağıdaki Arduino kütüphaneleri:
  - `WiFi` (ESP32 ile birlikte gelir)
  - `WebServer` (ESP32 ile birlikte gelir)
  - `SPIFFS` (ESP32 ile birlikte gelir)
  - `WiFiClientSecure` (ESP32 ile birlikte gelir)
  - `ArduinoJson` (v6.x)
  - `base64` (ESP32 ile birlikte gelir)
  - `Servo` (ESP32 ile birlikte gelir - servo motor kontrolü için)
  - `AccelStepper` (isteğe bağlı - NEMA 17 step motor kontrolü için gelişmiş özellikler)

### API Gereksinimleri
- Google Gemini API anahtarı
- İnternet bağlantısı (HTTPS için)

## 🚀 Kurulum

### 1. Arduino IDE Kurulumu

1. Arduino IDE'yi indirin ve kurun: https://www.arduino.cc/en/software
2. ESP32 board desteğini ekleyin:
   - File → Preferences → Additional Board Manager URLs
   - Şu URL'yi ekleyin: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools → Board → Boards Manager → "esp32" ara → "esp32 by Espressif Systems" kurun

### 2. Kütüphaneleri Kurun

Arduino IDE'de:
- Sketch → Include Library → Manage Libraries
- Aşağıdaki kütüphaneleri arayın ve kurun:
  - `ArduinoJson` (by Benoit Blanchon)

### 3. SPIFFS Dosyalarını Yükleyin

SPIFFS dosya sistemine aşağıdaki dosyaları yüklemeniz gerekmektedir:

1. **SPIFFS Data Upload Tool kurulumu:**
   - Tools → Manage Libraries → "ESP32 Sketch Data Upload" ara ve kur

2. **Dosyaları hazırlayın:**
   - Proje klasöründe `data` klasörü oluşturun
   - `data/logo.png` - Logo görseli (40x40px önerilir)
   - `data/Pglw6P01.svg` - Arka plan SVG görseli

3. **Dosyaları yükleyin:**
   - Tools → ESP32 Sketch Data Upload

### 4. Kod Yapılandırması

`sonfinal.ino` dosyasını açın ve aşağıdaki ayarları yapın:

```cpp
// Wi-Fi ayarları
const char* ssid = "WIFI_AG_ADINIZ";        // Wi-Fi ağ adınız
const char* password = "WIFI_SIFRENIZ";     // Wi-Fi şifreniz

// Gemini API ayarları
const char* geminiApiKey = "API_ANAHTARINIZ"; // Google Gemini API anahtarınız
```

### 5. Kodu Yükleyin

1. ESP32 kartınızı bilgisayara bağlayın
2. Tools → Board → ESP32 Arduino → "ESP32 Dev Module" seçin
3. Tools → Port → Doğru COM portunu seçin
4. Upload butonuna tıklayın

## 📱 Kullanım

### İlk Başlatma

1. ESP32 kartınızı USB ile bilgisayara bağlayın
2. Serial Monitor'ü açın (115200 baud rate)
3. Kart başladığında Serial Monitor'de IP adresini göreceksiniz
4. Tarayıcınızda bu IP adresine gidin (örn: `http://192.168.1.100`)

### Web Arayüzü Kullanımı

1. **Kullanıcı Seçimi:**
   - Sol üstteki menü butonuna (☰) tıklayın
   - Kullanıcı listesinden aktif kullanıcıyı seçin
   - Varsayılan kullanıcılar: Dilara, Elvin, Esma, Sümeyra, Sare

2. **Fotoğraf Yükleme:**
   - "Fotoğraf Çek" butonuna tıklayın
   - Telefonunuzdan veya bilgisayarınızdan bir atık fotoğrafı seçin
   - Sistem otomatik olarak analiz edecektir

3. **Sonuç Görüntüleme:**
   - Analiz tamamlandığında atık türü gösterilir
   - Kazanılan puan otomatik olarak kullanıcı hesabına eklenir
   - Toplam puan güncellenir

### Puan Sistemi

| Atık Türü | Puan |
|-----------|------|
| Plastik   | 10   |
| Kağıt     | 15   |
| Metal     | 20   |
| Cam       | 8    |

## 🔧 Teknik Detaylar

### API Endpoints

- `GET /` - Ana sayfa (HTML)
- `GET /style.css` - CSS stilleri
- `GET /script.js` - JavaScript kodu
- `GET /logo.png` - Logo görseli
- `GET /Pglw6P01.svg` - Arka plan görseli
- `POST /proxy` - Gemini API proxy (kullanılmıyor, doğrudan çağrı yapılıyor)
- `POST /save-result` - Analiz sonucunu kaydetme

### Görüntü İşleme

- **Maksimum boyut:** 300px (en veya boy)
- **Format:** JPEG
- **Kalite:** 0.7 (70%)
- **Encoding:** Base64

### Veri Saklama

- **LocalStorage:** Tarayıcıda kullanıcı verileri saklanır
- **Anahtarlar:**
  - `recycleUsers`: Kullanıcı listesi ve puanları
  - `activeUserId`: Aktif kullanıcı ID'si

### Zaman Aşımı Ayarları

- **Wi-Fi bağlantısı:** 10 saniye
- **Gemini API isteği:** 30 saniye (client-side)
- **ESP32 proxy:** 45 saniye (server-side)

### Motor Kontrolü

- **NEMA 17 Step Motor:**
  - CW422C sürücü modülü ile kontrol edilir
  - Step motor kontrolü için dijital pinler kullanılır
  - Mikrostep ayarları sürücü üzerinden yapılabilir
  
- **Servo Motorlar (2x):**
  - ESP32'nin PWM pinleri üzerinden kontrol edilir
  - Standart servo kütüphanesi kullanılır
  - 0-180 derece açı kontrolü sağlanır

- **Bağlantı Notları:**
  - Motorlar için ayrı güç kaynağı kullanılması önerilir
  - ESP32 ve motor sürücüleri arasında ortak toprak (GND) bağlantısı yapılmalıdır
  - Yüksek akım çeken motorlar için uygun güç kaynağı seçilmelidir

## 🐛 Sorun Giderme

### Wi-Fi Bağlantı Sorunları

**Sorun:** ESP32 Wi-Fi'ye bağlanamıyor
- **Çözüm:** 
  - SSID ve şifrenin doğru olduğundan emin olun
  - Wi-Fi sinyal gücünü kontrol edin
  - Serial Monitor'de hata mesajlarını kontrol edin

### SPIFFS Dosya Sorunları

**Sorun:** Logo veya SVG görselleri görünmüyor
- **Çözüm:**
  - SPIFFS Data Upload işlemini tekrar yapın
  - Dosya isimlerinin doğru olduğundan emin olun (`/logo.png`, `/Pglw6P01.svg`)
  - Serial Monitor'de dosya listesini kontrol edin

### API Bağlantı Sorunları

**Sorun:** Gemini API yanıt vermiyor
- **Çözüm:**
  - API anahtarının geçerli olduğundan emin olun
  - İnternet bağlantısını kontrol edin
  - Tarayıcı konsolunda (F12) hata mesajlarını kontrol edin
  - API kotasını kontrol edin

### Görüntü Analiz Sorunları

**Sorun:** Atık türü belirlenemiyor
- **Çözüm:**
  - Fotoğrafın net ve iyi aydınlatılmış olduğundan emin olun
  - Atık nesnenin fotoğrafın merkezinde olduğundan emin olun
  - Desteklenen atık türlerinden birini fotoğrafladığınızdan emin olun

### Motor Kontrol Sorunları

**Sorun:** NEMA 17 step motor çalışmıyor
- **Çözüm:**
  - CW422C sürücü modülünün doğru bağlandığından emin olun
  - Motor için yeterli güç kaynağının bağlı olduğundan emin olun (genellikle 12V)
  - Step motor pin bağlantılarını kontrol edin (STEP, DIR, ENABLE)
  - Sürücü modülündeki mikrostep ayarlarını kontrol edin
  - Motor sargılarının doğru bağlandığından emin olun (A+, A-, B+, B-)

**Sorun:** Servo motorlar çalışmıyor
- **Çözüm:**
  - Servo motorların PWM pinlerine doğru bağlandığından emin olun
  - Servo motorlar için ayrı güç kaynağı kullanılıyorsa, GND bağlantısını kontrol edin
  - Servo motorların güç gereksinimlerini kontrol edin (genellikle 5V)
  - Pin numaralarının kodda doğru tanımlandığından emin olun
  - Servo kütüphanesinin doğru yüklendiğinden emin olun

**Sorun:** Motorlar titriyor veya düzgün çalışmıyor
- **Çözüm:**
  - Güç kaynağının yeterli akım sağladığından emin olun
  - Bağlantı kablolarının gevşek olmadığından emin olun
  - Motor sürücülerinin aşırı ısınmadığını kontrol edin
  - Ortak toprak (GND) bağlantısının yapıldığından emin olun

## 📊 Sistem Gereksinimleri

### ESP32 Kaynak Kullanımı
- **RAM:** ~50KB (dinamik)
- **Flash:** ~1.5MB (program + SPIFFS)
- **CPU:** %10-30 (idle), %50-80 (aktif analiz)

### Ağ Gereksinimleri
- **Bant genişliği:** ~50-200KB/analiz (görüntü boyutuna bağlı)
- **Gecikme:** <100ms (yerel ağ), 1-3s (API yanıtı)

### Motor Güç Gereksinimleri
- **NEMA 17 Step Motor:**
  - **Gerilim:** 12V DC (önerilen)
  - **Akım:** 1.5-2A (yüke bağlı)
  - **Güç:** ~18-24W
  
- **Servo Motorlar (2x):**
  - **Gerilim:** 5V DC (standart servo)
  - **Akım:** 0.5-1A (yüke bağlı, her servo için)
  - **Güç:** ~2.5-5W (her servo için)
  
- **Toplam Güç Gereksinimi:**
  - **Maksimum:** ~30-35W (tüm motorlar aktifken)
  - **Önerilen Güç Kaynağı:** 12V/3A veya daha yüksek kapasiteli kaynak
  - **Not:** ESP32 ve motorlar için ayrı güç kaynakları kullanılması önerilir

## 🔒 Güvenlik Notları

⚠️ **ÖNEMLİ:** Bu proje geliştirme amaçlıdır. Üretim ortamında kullanmadan önce:

1. **API Anahtarı Güvenliği:**
   - API anahtarını kod içinde saklamayın
   - Ortam değişkenleri veya güvenli konfigürasyon dosyaları kullanın
   - API anahtarını GitHub'a yüklemeyin

2. **Wi-Fi Güvenliği:**
   - Güçlü Wi-Fi şifreleri kullanın
   - WPA2/WPA3 şifreleme kullanın

3. **HTTPS:**
   - Üretim ortamında SSL sertifikası kullanın
   - `client.setInsecure()` metodunu kaldırın

## 📝 Lisans

Bu proje eğitim amaçlı geliştirilmiştir.

## 👥 Geliştirici

**İbrahim Ünal**
- E-posta: ibrahimunalofficial@gmail.com

## 🔄 Güncellemeler

### Versiyon Bilgisi
- **Mevcut Versiyon:** 1.0
- **Son Güncelleme:** 2024

### Gelecek Özellikler
- [ ] Veritabanı entegrasyonu
- [ ] Kullanıcı kayıt sistemi
- [ ] İstatistik ve raporlama
- [ ] Mobil uygulama desteği
- [ ] Çoklu dil desteği

## 📞 Destek

Sorularınız veya destek ihtiyacınız için:
- **E-posta:** ozdensolutions@icloud.com
- **Serial Monitor:** Hata ayıklama için Serial Monitor'ü kullanın (115200 baud)

## 🙏 Teşekkürler

- Google Gemini AI ekibine görüntü analizi desteği için
- ESP32 topluluğuna harika dokümantasyon için

---

**Not:** Bu sistem eğitim ve araştırma amaçlıdır. Ticari kullanım için lisans ve güvenlik kontrolleri yapılmalıdır.

