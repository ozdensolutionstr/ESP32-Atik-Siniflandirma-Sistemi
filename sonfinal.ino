/*
 * ESP32 A1 Kartı ile Atık Sınıflandırma Uygulaması
 * 
 * Bu uygulama, ESP32 A1 kartı kullanarak atık türlerini sınıflandırır.
 * Kullanıcılar telefonlarından fotoğraf yükleyebilir ve Google Gemini API ile analiz edilir.
 * 
 * Desteklenen atık türleri:
 * - Kağıt
 * - Cam
 * - Metal
 * - Plastik
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <base64.h>

// Wi-Fi ayarları
const char* ssid = "WIFI_SSID_BURAYA";
const char* password = "WIFI_SIFRE_BURAYA";

// Gemini API ayarları
const char* geminiApiKey = "API_ANAHTARINIZ_BURAYA";
const char* geminiHost = "generativelanguage.googleapis.com";
const int geminiPort = 443;

WebServer server(80);
WiFiClientSecure client;

// Görüntü analizi sonucu
String analizSonucu = "";
String atikTuru = "";

void setup() {
  Serial.begin(115200);
  
  // SPIFFS başlatma - daha ayrıntılı hata mesajları ekle
  Serial.println("SPIFFS başlatılıyor...");
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS başlatma hatası, yeniden başlatılıyor...");
    delay(3000);
    ESP.restart(); // Cihazı yeniden başlat
    return;
  }
  
  // SPIFFS dosyalarını listele - doğrulama
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  Serial.println("SPIFFS dosyaları:");
  bool logoFound = false;
  
  while(file) {
    Serial.print("  ");
    Serial.print(file.name());
    Serial.print(" - ");
    Serial.println(file.size());
    
    if (String(file.name()) == "/logo.png") {
      logoFound = true;
    }
    
    file = root.openNextFile();
  }
  
  if (!logoFound) {
    Serial.println("logo.png dosyası bulunamadı!");
  } else {
    Serial.println("logo.png dosyası bulundu!");
  }
  
  // Wi-Fi bağlantısı
  WiFi.begin(ssid, password);
  
  int baglantiDenemesi = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    baglantiDenemesi++;
    if (baglantiDenemesi > 20) { // 10 saniye sonra timeout
      delay(3000);
      ESP.restart(); // Cihazı yeniden başlat
      return;
    }
  }
  
  // SSL doğrulamasını geçersiz kıl (yalnızca geliştirme için)
  client.setInsecure();
  
  // Web sunucu rotaları
  server.on("/", HTTP_GET, handleRoot);
  server.on("/style.css", HTTP_GET, handleCss);
  server.on("/script.js", HTTP_GET, handleJs);
  server.on("/proxy", HTTP_POST, handleProxy);
  server.on("/save-result", HTTP_POST, handleSaveResult);
  server.on("/logo.png", HTTP_GET, handleLogo);
  server.on("/Pglw6P01.svg", HTTP_GET, handleSvg);
  
  // Sunucuyu başlat
  server.begin();
}

void loop() {
  server.handleClient();
}

// Logo dosyasını servis et
void handleLogo() {
  if (!SPIFFS.exists("/logo.png")) {
    Serial.println("Logo dosyası bulunamadı!");
    server.send(404, "text/plain", "Logo bulunamadı!");
    return;
  }

  File file = SPIFFS.open("/logo.png", "r");
  if (!file) {
    Serial.println("Logo dosyası açılamadı!");
    server.send(404, "text/plain", "Logo bulunamadı!");
    return;
  }

  server.sendHeader("Content-Type", "image/png");
  server.sendHeader("Content-Disposition", "inline; filename=logo.png");
  server.streamFile(file, "image/png");
  file.close();
  Serial.println("Logo başarıyla gönderildi");
}

// SVG dosyasını servis et
void handleSvg() {
  if (!SPIFFS.exists("/Pglw6P01.svg")) {
    Serial.println("SVG dosyası bulunamadı!");
    server.send(404, "text/plain", "SVG bulunamadı!");
    return;
  }

  File file = SPIFFS.open("/Pglw6P01.svg", "r");
  if (!file) {
    Serial.println("SVG dosyası açılamadı!");
    server.send(404, "text/plain", "SVG bulunamadı!");
    return;
  }

  server.sendHeader("Content-Type", "image/svg+xml");
  server.streamFile(file, "image/svg+xml");
  file.close();
  Serial.println("SVG başarıyla gönderildi");
}

// Ana sayfa
void handleRoot() {
  String html = "<!DOCTYPE html>\n";
  html += "<html lang='tr'>\n";
  html += "<head>\n";
  html += "  <meta charset='UTF-8'>\n";
  html += "  <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n";
  html += "  <title>Atık Sınıflandırma</title>\n";
  html += "  <link rel='stylesheet' href='style.css'>\n";
  html += "</head>\n";
  html += "<body>\n";
  html += "  <div class='sidebar' id='sidebar'>\n";
  html += "    <div class='sidebar-header'>\n";
  html += "      <h2>Kullanıcılar</h2>\n";
  html += "      <button class='close-sidebar' id='close-sidebar'>×</button>\n";
  html += "    </div>\n";
  html += "    <div class='user-list' id='user-list'>\n";
  html += "      <!-- Kullanıcılar JS ile eklenecek -->\n";
  html += "    </div>\n";
  html += "  </div>\n";
  html += "  <div class='overlay' id='sidebar-overlay'></div>\n";
  
  html += "  <div class='container'>\n";
  html += "    <div class='header'>\n";
  html += "      <button class='menu-button' id='open-sidebar'>☰</button>\n";
  html += "      <div class='logo-container'>\n";
  html += "        <img src='/logo.png' alt='BrainTech Logo' class='logo'>\n";
  html += "      </div>\n";
  html += "      <h1>TAK. Atık Sınıflandırma Sistemi</h1>\n";
  html += "    </div>\n";
  
  html += "    <div class='user-info'>\n";
  html += "      <div class='active-user' id='active-user'>Kullanıcı 1</div>\n";
  html += "      <div class='user-points'><span id='user-points'>0</span> Puan</div>\n";
  html += "    </div>\n";
  
  html += "    <div class='upload-container'>\n";
  html += "      <div class='file-input-container'>\n";
  html += "        <label for='image-upload' class='file-input-label'>Fotoğraf Çek</label>\n";
  html += "        <input type='file' id='image-upload' accept='image/*' class='file-input'>\n";
  html += "        <div id='file-name' class='file-name'>Dosya seçilmedi</div>\n";
  html += "      </div>\n";
  html += "      <div class='preview-container hidden' id='preview-container'>\n";
  html += "        <img id='preview-image' src='' alt='Önizleme'>\n";
  html += "      </div>\n";
  html += "    </div>\n";
  html += "    <div id='loading-container' class='loading-container hidden'>\n";
  html += "      <div class='loader'></div>\n";
  html += "      <div class='loading-text'>Analiz ediliyor...</div>\n";
  html += "    </div>\n";
  html += "    <div id='result-container' class='result-container hidden'>\n";
  html += "      <h2>Analiz Sonucu</h2>\n";
  html += "      <div id='waste-type' class='waste-type'></div>\n";
  html += "      <div class='earned-points'>\n";
  html += "        <span id='earned-points'>0</span> puan kazandınız!\n";
  html += "      </div>\n";
  html += "    </div>\n";
  
  html += "    <div class='waste-info'>\n";
  html += "      <h3>Atık Puanları</h3>\n";
  html += "      <ul>\n";
  html += "        <li><span class='waste-name'>PLASTİK:</span> <span class='waste-points'>10 puan</span></li>\n";
  html += "        <li><span class='waste-name'>KAĞIT:</span> <span class='waste-points'>15 puan</span></li>\n";
  html += "        <li><span class='waste-name'>METAL:</span> <span class='waste-points'>20 puan</span></li>\n";
  html += "        <li><span class='waste-name'>CAM:</span> <span class='waste-points'>8 puan</span></li>\n";
  html += "      </ul>\n";
  html += "    </div>\n";
  
  // İletişim bilgisi footer'ı ekleyelim
  html += "    <div class='footer'>\n";
  html += "      <p>İletişim: <a href='mailto:ibrahimunalofficial@gmail.com'>ibrahimunalofficial@gmail.com</a></p>\n";
  html += "    </div>\n";
  
  html += "  </div>\n";
  html += "  <script src='script.js'></script>\n";
  html += "</body>\n";
  html += "</html>\n";
  
  server.send(200, "text/html", html);
}

// CSS
void handleCss() {
  String css = "* {\n";
  css += "  box-sizing: border-box;\n";
  css += "  margin: 0;\n";
  css += "  padding: 0;\n";
  css += "}\n\n";
  css += "body {\n";
  css += "  font-family: Arial, sans-serif;\n";
  css += "  background-image: url('/Pglw6P01.svg');\n";
  css += "  background-repeat: no-repeat;\n";
  css += "  background-attachment: fixed;\n";
  css += "  background-size: cover;\n";
  css += "  background-position: center;\n";
  css += "  background-color: #2c3e50;\n";
  css += "  color: #333;\n";
  css += "  position: relative;\n";
  css += "  overflow-x: hidden;\n";
  css += "}\n\n";
  
  // Logo stilleri
  css += "/* Logo stilleri */\n";
  css += ".logo-container {\n";
  css += "  display: flex;\n";
  css += "  align-items: center;\n";
  css += "  margin-right: 10px;\n";
  css += "}\n\n";
  css += ".logo {\n";
  css += "  width: 40px;\n";
  css += "  height: 40px;\n";
  css += "  border-radius: 50%;\n";
  css += "  object-fit: cover;\n";
  css += "}\n\n";
  
  // Header düzenlemeleri
  css += ".header {\n";
  css += "  display: flex;\n";
  css += "  align-items: center;\n";
  css += "  margin-bottom: 15px;\n";
  css += "  background-color: #fff;\n";
  css += "  padding: 10px 15px;\n";
  css += "  border-radius: 8px;\n";
  css += "  box-shadow: 0 2px 4px rgba(0,0,0,0.1);\n";
  css += "}\n\n";
  css += ".header h1 {\n";
  css += "  margin-left: 10px;\n";
  css += "  font-size: 1.5rem;\n";
  css += "  color: #2c3e50;\n";
  css += "  flex: 1;\n";
  css += "}\n\n";
  
  // Çekmeceli menü
  css += "/* Çekmeceli Menü */\n";
  css += ".sidebar {\n";
  css += "  position: fixed;\n";
  css += "  top: 0;\n";
  css += "  left: -280px;\n";
  css += "  width: 270px;\n";
  css += "  height: 100%;\n";
  css += "  background-color: #fff;\n";
  css += "  box-shadow: 2px 0 8px rgba(0,0,0,0.1);\n";
  css += "  z-index: 1000;\n";
  css += "  transition: left 0.3s ease;\n";
  css += "  padding: 20px 0;\n";
  css += "}\n\n";
  css += ".sidebar.open {\n";
  css += "  left: 0;\n";
  css += "}\n\n";
  css += ".sidebar-header {\n";
  css += "  display: flex;\n";
  css += "  justify-content: space-between;\n";
  css += "  align-items: center;\n";
  css += "  padding: 0 20px 15px;\n";
  css += "  border-bottom: 1px solid #eee;\n";
  css += "}\n\n";
  css += ".sidebar-header h2 {\n";
  css += "  font-size: 1.2rem;\n";
  css += "  color: #2c3e50;\n";
  css += "}\n\n";
  css += ".close-sidebar {\n";
  css += "  background: none;\n";
  css += "  border: none;\n";
  css += "  font-size: 1.5rem;\n";
  css += "  cursor: pointer;\n";
  css += "  color: #999;\n";
  css += "}\n\n";
  css += ".overlay {\n";
  css += "  position: fixed;\n";
  css += "  top: 0;\n";
  css += "  left: 0;\n";
  css += "  width: 100%;\n";
  css += "  height: 100%;\n";
  css += "  background-color: rgba(0,0,0,0.5);\n";
  css += "  z-index: 999;\n";
  css += "  display: none;\n";
  css += "}\n\n";
  css += ".user-list {\n";
  css += "  padding: 15px;\n";
  css += "}\n\n";
  css += ".user-card {\n";
  css += "  background-color: #f8f9fa;\n";
  css += "  border-radius: 8px;\n";
  css += "  padding: 15px;\n";
  css += "  margin-bottom: 10px;\n";
  css += "  cursor: pointer;\n";
  css += "  transition: all 0.3s ease;\n";
  css += "  border-left: 3px solid transparent;\n";
  css += "}\n\n";
  css += ".user-card:hover {\n";
  css += "  background-color: #e9ecef;\n";
  css += "  transform: translateX(5px);\n";
  css += "}\n\n";
  css += ".user-card.active {\n";
  css += "  border-left-color: #3498db;\n";
  css += "  background-color: #e3f2fd;\n";
  css += "}\n\n";
  css += ".user-card h3 {\n";
  css += "  font-size: 1rem;\n";
  css += "  margin-bottom: 5px;\n";
  css += "}\n\n";
  css += ".user-card p {\n";
  css += "  font-size: 0.85rem;\n";
  css += "  color: #6c757d;\n";
  css += "}\n\n";
  css += ".menu-button {\n";
  css += "  background: none;\n";
  css += "  border: none;\n";
  css += "  font-size: 1.5rem;\n";
  css += "  cursor: pointer;\n";
  css += "  color: #333;\n";
  css += "}\n\n";
  
  // Ana içerik
  css += ".container {\n";
  css += "  max-width: 800px;\n";
  css += "  margin: 0 auto;\n";
  css += "  padding: 20px;\n";
  css += "  background-color: rgba(255, 255, 255, 0.85);\n";
  css += "  border-radius: 10px;\n";
  css += "  box-shadow: 0 0 10px rgba(0, 0, 0, 0.2);\n";
  css += "}\n\n";
  
  // Footer stilini ekle
  css += ".footer {\n";
  css += "  margin-top: 30px;\n";
  css += "  padding: 15px 0;\n";
  css += "  text-align: center;\n";
  css += "  border-top: 1px solid #ddd;\n";
  css += "  color: #666;\n";
  css += "  font-size: 0.9rem;\n";
  css += "}\n\n";
  css += ".footer a {\n";
  css += "  color: #3498db;\n";
  css += "  text-decoration: none;\n";
  css += "}\n\n";
  css += ".footer a:hover {\n";
  css += "  text-decoration: underline;\n";
  css += "}\n\n";
  
  css += ".user-info {\n";
  css += "  display: flex;\n";
  css += "  justify-content: space-between;\n";
  css += "  align-items: center;\n";
  css += "  background-color: #fff;\n";
  css += "  padding: 12px 15px;\n";
  css += "  border-radius: 8px;\n";
  css += "  margin-bottom: 20px;\n";
  css += "  box-shadow: 0 2px 4px rgba(0,0,0,0.1);\n";
  css += "}\n\n";
  css += ".active-user {\n";
  css += "  font-weight: bold;\n";
  css += "  color: #2c3e50;\n";
  css += "}\n\n";
  css += ".user-points {\n";
  css += "  background-color: #3498db;\n";
  css += "  color: white;\n";
  css += "  padding: 5px 10px;\n";
  css += "  border-radius: 20px;\n";
  css += "  font-size: 0.9rem;\n";
  css += "}\n\n";
  css += "h1 {\n";
  css += "  text-align: center;\n";
  css += "  margin-bottom: 20px;\n";
  css += "  color: #2c3e50;\n";
  css += "}\n\n";
  css += ".upload-container {\n";
  css += "  width: 100%;\n";
  css += "  background-color: #fff;\n";
  css += "  border-radius: 10px;\n";
  css += "  overflow: hidden;\n";
  css += "  margin-bottom: 20px;\n";
  css += "  border: 1px solid #ddd;\n";
  css += "  padding: 20px;\n";
  css += "  box-shadow: 0 2px 4px rgba(0,0,0,0.1);\n";
  css += "}\n\n";
  css += ".file-input-container {\n";
  css += "  display: flex;\n";
  css += "  flex-direction: column;\n";
  css += "  align-items: center;\n";
  css += "  margin-bottom: 20px;\n";
  css += "}\n\n";
  css += ".file-input-label {\n";
  css += "  padding: 12px 24px;\n";
  css += "  background-color: #3498db;\n";
  css += "  color: #fff;\n";
  css += "  border-radius: 5px;\n";
  css += "  cursor: pointer;\n";
  css += "  font-size: 16px;\n";
  css += "  transition: background-color 0.3s;\n";
  css += "  text-align: center;\n";
  css += "  margin-bottom: 10px;\n";
  css += "}\n\n";
  css += ".file-input-label:hover {\n";
  css += "  background-color: #2980b9;\n";
  css += "}\n\n";
  css += ".file-input {\n";
  css += "  display: none;\n";
  css += "}\n\n";
  css += ".file-name {\n";
  css += "  font-size: 14px;\n";
  css += "  color: #7f8c8d;\n";
  css += "  text-align: center;\n";
  css += "}\n\n";
  css += ".preview-container {\n";
  css += "  margin: 20px 0;\n";
  css += "  text-align: center;\n";
  css += "}\n\n";
  css += "#preview-image {\n";
  css += "  max-width: 100%;\n";
  css += "  max-height: 300px;\n";
  css += "  border-radius: 5px;\n";
  css += "  border: 1px solid #ddd;\n";
  css += "}\n\n";
  css += ".buttons {\n";
  css += "  display: flex;\n";
  css += "  justify-content: center;\n";
  css += "  gap: 20px;\n";
  css += "  margin-bottom: 20px;\n";
  css += "}\n\n";
  css += ".btn {\n";
  css += "  padding: 12px 24px;\n";
  css += "  background-color: #3498db;\n";
  css += "  color: #fff;\n";
  css += "  border: none;\n";
  css += "  border-radius: 5px;\n";
  css += "  cursor: pointer;\n";
  css += "  font-size: 16px;\n";
  css += "  transition: background-color 0.3s;\n";
  css += "}\n\n";
  css += ".btn:hover {\n";
  css += "  background-color: #2980b9;\n";
  css += "}\n\n";
  css += ".btn:disabled {\n";
  css += "  background-color: #bdc3c7;\n";
  css += "  cursor: not-allowed;\n";
  css += "}\n\n";
  css += ".loading-container {\n";
  css += "  display: flex;\n";
  css += "  flex-direction: column;\n";
  css += "  align-items: center;\n";
  css += "  justify-content: center;\n";
  css += "  margin: 20px 0;\n";
  css += "}\n\n";
  css += ".loader {\n";
  css += "  border: 5px solid #f3f3f3;\n";
  css += "  border-top: 5px solid #3498db;\n";
  css += "  border-radius: 50%;\n";
  css += "  width: 50px;\n";
  css += "  height: 50px;\n";
  css += "  animation: spin 2s linear infinite;\n";
  css += "  margin-bottom: 10px;\n";
  css += "}\n\n";
  css += "@keyframes spin {\n";
  css += "  0% { transform: rotate(0deg); }\n";
  css += "  100% { transform: rotate(360deg); }\n";
  css += "}\n\n";
  css += ".loading-text {\n";
  css += "  font-size: 16px;\n";
  css += "  color: #2c3e50;\n";
  css += "}\n\n";
  css += ".result-container {\n";
  css += "  background-color: #fff;\n";
  css += "  border-radius: 10px;\n";
  css += "  padding: 20px;\n";
  css += "  margin-top: 20px;\n";
  css += "  border: 1px solid #ddd;\n";
  css += "  box-shadow: 0 2px 4px rgba(0,0,0,0.1);\n";
  css += "}\n\n";
  css += ".result-container h2 {\n";
  css += "  text-align: center;\n";
  css += "  margin-bottom: 15px;\n";
  css += "  color: #2c3e50;\n";
  css += "}\n\n";
  css += ".result {\n";
  css += "  font-size: 16px;\n";
  css += "  line-height: 1.6;\n";
  css += "  margin-bottom: 15px;\n";
  css += "}\n\n";
  css += ".waste-type {\n";
  css += "  font-size: 24px;\n";
  css += "  font-weight: bold;\n";
  css += "  text-align: center;\n";
  css += "  padding: 15px;\n";
  css += "  border-radius: 5px;\n";
  css += "  margin: 10px 0;\n";
  css += "  text-transform: uppercase;\n";
  css += "  background-color: #e8f4fd;\n";
  css += "}\n\n";
  css += ".earned-points {\n";
  css += "  text-align: center;\n";
  css += "  font-size: 1.1rem;\n";
  css += "  color: #27ae60;\n";
  css += "  font-weight: bold;\n";
  css += "  margin-top: 10px;\n";
  css += "}\n\n";
  css += ".waste-info {\n";
  css += "  background-color: #fff;\n";
  css += "  border-radius: 10px;\n";
  css += "  padding: 20px;\n";
  css += "  margin-top: 20px;\n";
  css += "  border: 1px solid #ddd;\n";
  css += "  box-shadow: 0 2px 4px rgba(0,0,0,0.1);\n";
  css += "}\n\n";
  css += ".waste-info h3 {\n";
  css += "  color: #2c3e50;\n";
  css += "  margin-bottom: 15px;\n";
  css += "  text-align: center;\n";
  css += "}\n\n";
  css += ".waste-info ul {\n";
  css += "  list-style: none;\n";
  css += "}\n\n";
  css += ".waste-info li {\n";
  css += "  display: flex;\n";
  css += "  justify-content: space-between;\n";
  css += "  padding: 8px 0;\n";
  css += "  border-bottom: 1px solid #eee;\n";
  css += "}\n\n";
  css += ".waste-info li:last-child {\n";
  css += "  border-bottom: none;\n";
  css += "}\n\n";
  css += ".waste-name {\n";
  css += "  font-weight: bold;\n";
  css += "}\n\n";
  css += ".hidden {\n";
  css += "  display: none;\n";
  css += "}\n\n";
  css += "@media (max-width: 600px) {\n";
  css += "  .container {\n";
  css += "    padding: 10px;\n";
  css += "  }\n";
  css += "  .header h1 {\n";
  css += "    font-size: 1.3rem;\n";
  css += "  }\n";
  css += "  .user-info {\n";
  css += "    padding: 10px;\n";
  css += "  }\n";
  css += "  .active-user {\n";
  css += "    font-size: 0.9rem;\n";
  css += "  }\n";
  css += "  .user-points {\n";
  css += "    font-size: 0.8rem;\n";
  css += "  }\n";
  css += "}\n";
  
  server.send(200, "text/css", css);
}

// JavaScript
void handleJs() {
  String js = "document.addEventListener('DOMContentLoaded', () => {\n";
  js += "  // DOM Elementleri\n";
  js += "  const fileInput = document.getElementById('image-upload');\n";
  js += "  const fileName = document.getElementById('file-name');\n";
  js += "  const previewContainer = document.getElementById('preview-container');\n";
  js += "  const previewImage = document.getElementById('preview-image');\n";
  js += "  const loadingContainer = document.getElementById('loading-container');\n";
  js += "  const resultContainer = document.getElementById('result-container');\n";
  js += "  const wasteTypeElement = document.getElementById('waste-type');\n";
  js += "  const earnedPointsElement = document.getElementById('earned-points');\n";
  js += "  const userPointsElement = document.getElementById('user-points');\n";
  js += "  const activeUserElement = document.getElementById('active-user');\n";
  js += "  const userListElement = document.getElementById('user-list');\n";
  js += "  const sidebar = document.getElementById('sidebar');\n";
  js += "  const sidebarOverlay = document.getElementById('sidebar-overlay');\n";
  js += "  const openSidebarBtn = document.getElementById('open-sidebar');\n";
  js += "  const closeSidebarBtn = document.getElementById('close-sidebar');\n\n";
  
  // Yeni kodlar: Önbelleği temizleme (isteğe bağlı)
  js += "  // Kullanıcı verilerini sıfırla (ilk yüklemede çalışır)\n";
  js += "  const resetData = new URLSearchParams(window.location.search).get('reset');\n";
  js += "  if (resetData === 'true') {\n";
  js += "    localStorage.removeItem('recycleUsers');\n";
  js += "    localStorage.removeItem('activeUserId');\n";
  js += "    console.log('Kullanıcı verileri sıfırlandı!');\n";
  js += "  }\n\n";
  
  // Gemini API bilgilerini ekle
  js += "  const geminiApiKey = '" + String(geminiApiKey) + "';\n";
  js += "  const geminiHost = '" + String(geminiHost) + "';\n";
  js += "  const maxImageSize = 300; // Maksimum görüntü boyutu (piksel)\n\n";
  
  // Atık puan sistemini tanımla
  js += "  // Atık puan sistemi\n";
  js += "  const wastePoints = {\n";
  js += "    'plastik': 10,\n";
  js += "    'kağıt': 15,\n";
  js += "    'metal': 20,\n";
  js += "    'cam': 8\n";
  js += "  };\n\n";
  
  // Kullanıcı verileri
  js += "  // Kullanıcı verileri\n";
  js += "  let users = [\n";
  js += "    { id: 1, name: 'Dilara', points: 0 },\n";
  js += "    { id: 2, name: 'Elvin', points: 0 },\n";
  js += "    { id: 3, name: 'Esma', points: 0 },\n";
  js += "    { id: 4, name: 'Sümeyra', points: 0 },\n";
  js += "    { id: 5, name: 'Sare', points: 0 }\n";
  js += "  ];\n\n";
  
  js += "  // Aktif kullanıcı\n";
  js += "  let activeUser = users[0];\n\n";
  
  // Local Storage'dan verileri yükle
  js += "  // Local storage'dan kullanıcı verilerini yükle\n";
  js += "  function loadUserData() {\n";
  js += "    const savedUsers = localStorage.getItem('recycleUsers');\n";
  js += "    if (savedUsers) {\n";
  js += "      users = JSON.parse(savedUsers);\n";
  js += "    } else {\n";
  js += "      // Kayıtlı veri yoksa yeni kullanıcı verilerini kaydet\n";
  js += "      saveUserData();\n";
  js += "    }\n";
  js += "    \n";
  js += "    const savedActiveUserId = localStorage.getItem('activeUserId');\n";
  js += "    if (savedActiveUserId) {\n";
  js += "      const foundUser = users.find(user => user.id === parseInt(savedActiveUserId));\n";
  js += "      if (foundUser) {\n";
  js += "        activeUser = foundUser;\n";
  js += "      }\n";
  js += "    }\n";
  js += "    \n";
  js += "    renderUserList();\n";
  js += "    updateActiveUserDisplay();\n";
  js += "  }\n\n";
  
  // Kullanıcı verilerini kaydet
  js += "  // Kullanıcı verilerini local storage'a kaydet\n";
  js += "  function saveUserData() {\n";
  js += "    localStorage.setItem('recycleUsers', JSON.stringify(users));\n";
  js += "    localStorage.setItem('activeUserId', activeUser.id.toString());\n";
  js += "  }\n\n";
  
  // Kullanıcı listesini oluştur
  js += "  // Kullanıcı listesini oluştur\n";
  js += "  function renderUserList() {\n";
  js += "    userListElement.innerHTML = '';\n";
  js += "    \n";
  js += "    users.forEach(user => {\n";
  js += "      const userCard = document.createElement('div');\n";
  js += "      userCard.className = `user-card ${user.id === activeUser.id ? 'active' : ''}`;\n";
  js += "      userCard.innerHTML = `\n";
  js += "        <h3>${user.name}</h3>\n";
  js += "        <p>${user.points} puan</p>\n";
  js += "      `;\n";
  js += "      \n";
  js += "      userCard.addEventListener('click', () => {\n";
  js += "        activeUser = user;\n";
  js += "        updateActiveUserDisplay();\n";
  js += "        renderUserList(); // Aktif durum güncellemesi için yeniden render\n";
  js += "        saveUserData();\n";
  js += "        closeSidebar();\n";
  js += "      });\n";
  js += "      \n";
  js += "      userListElement.appendChild(userCard);\n";
  js += "    });\n";
  js += "  }\n\n";
  
  // Aktif kullanıcı bilgilerini göster
  js += "  // Aktif kullanıcı bilgilerini güncelle\n";
  js += "  function updateActiveUserDisplay() {\n";
  js += "    activeUserElement.textContent = activeUser.name;\n";
  js += "    userPointsElement.textContent = activeUser.points;\n";
  js += "  }\n\n";
  
  // Çekmeceli menü kontrolleri
  js += "  // Çekmeceli menü kontrolleri\n";
  js += "  function openSidebar() {\n";
  js += "    sidebar.classList.add('open');\n";
  js += "    sidebarOverlay.style.display = 'block';\n";
  js += "  }\n\n";
  
  js += "  function closeSidebar() {\n";
  js += "    sidebar.classList.remove('open');\n";
  js += "    sidebarOverlay.style.display = 'none';\n";
  js += "  }\n\n";
  
  js += "  openSidebarBtn.addEventListener('click', openSidebar);\n";
  js += "  closeSidebarBtn.addEventListener('click', closeSidebar);\n";
  js += "  sidebarOverlay.addEventListener('click', closeSidebar);\n\n";
  
  // Uygulama başlatma
  js += "  // Uygulama başlatma\n";
  js += "  loadUserData();\n\n";
  
  // Dosya seçme işlevi
  js += "  // Dosya seçme işlevi\n";
  js += "  fileInput.addEventListener('change', async (e) => {\n";
  js += "    const file = e.target.files[0];\n";
  js += "    if (file) {\n";
  js += "      fileName.textContent = file.name;\n";
  js += "      \n";
  js += "      // Görüntü önizleme\n";
  js += "      const reader = new FileReader();\n";
  js += "      reader.onload = function(event) {\n";
  js += "        previewImage.src = event.target.result;\n";
  js += "        previewContainer.classList.remove('hidden');\n";
  js += "      };\n";
  js += "      reader.readAsDataURL(file);\n";
  
  js += "      // Otomatik analiz işlemini başlat\n";
  js += "      setTimeout(async () => {\n";
  js += "        try {\n";
  js += "          // UI güncelle\n";
  js += "          loadingContainer.classList.remove('hidden');\n";
  js += "          resultContainer.classList.add('hidden');\n\n";
  
  js += "          // Görüntüyü sıkıştır ve boyutunu küçült\n";
  js += "          const resizedImage = await resizeAndCompressImage(file, maxImageSize);\n";
  js += "          console.log(`Orijinal boyut: ~${Math.round(file.size/1024)}KB, Sıkıştırılmış: ~${Math.round(resizedImage.size/1024)}KB`);\n\n";
  
  js += "          // Dosyayı Base64'e dönüştür\n";
  js += "          const base64Image = await fileToBase64(resizedImage);\n";
  js += "          const imageData = base64Image.split(',')[1]; // Base64 veri kısmını al\n\n";
  
  js += "          // Gemini API isteği hazırla\n";
  js += "          const requestData = {\n";
  js += "            contents: [{\n";
  js += "              parts: [\n";
  js += "                { text: 'Bu görüntüdeki atık türünü analiz et. Lütfen görüntüdeki nesnenin kağıt, cam, metal veya plastik atık kategorilerinden hangisine ait olduğunu belirle. Cevabını şu formatta ver: \\'Analiz sonucu:[detaylı açıklama]. Atık türü:[kağıt/cam/metal/plastik]\\''  },\n";
  js += "                { inlineData: { mimeType: 'image/jpeg', data: imageData } }\n";
  js += "              ]\n";
  js += "            }]\n";
  js += "          };\n\n";
  
  js += "          // İstek boyutunu log et\n";
  js += "          const requestSize = JSON.stringify(requestData).length;\n";
  js += "          console.log(`İstek boyutu: ~${Math.round(requestSize/1024)}KB`);\n\n";
  
  js += "          // Zaman aşımı kontrolü ile API isteği\n";
  js += "          const timeoutPromise = new Promise((_, reject) =>\n";
  js += "            setTimeout(() => reject(new Error('API yanıt zaman aşımı - 30 saniye')), 30000)\n";
  js += "          );\n\n";
  
  js += "          // Doğrudan Gemini API'ye istek gönder\n";
  js += "          const geminiUrl = `https://${geminiHost}/v1beta/models/gemini-2.0-flash:generateContent?key=${geminiApiKey}`;\n";
  js += "          console.log('Gemini API isteği gönderiliyor...');\n\n";
  
  js += "          const fetchPromise = fetch(geminiUrl, {\n";
  js += "            method: 'POST',\n";
  js += "            headers: { 'Content-Type': 'application/json' },\n";
  js += "            body: JSON.stringify(requestData)\n";
  js += "          });\n\n";
  
  js += "          // İstek veya zaman aşımından hangisi önce gerçekleşirse\n";
  js += "          const response = await Promise.race([fetchPromise, timeoutPromise]);\n\n";
  
  js += "          if (!response.ok) {\n";
  js += "            throw new Error(`API isteği başarısız: ${response.status}`);\n";
  js += "          }\n\n";
  
  js += "          // Gemini yanıtı\n";
  js += "          const geminiData = await response.json();\n";
  js += "          console.log('Gemini API yanıtı alındı');\n\n";
  
  js += "          // Sonucu işle\n";
  js += "          const processedResult = processResult(geminiData);\n";
  js += "          \n";
  js += "          // Kazanılan puanı hesapla\n";
  js += "          let earnedPoints = 0;\n";
  js += "          if (processedResult.wasteType in wastePoints) {\n";
  js += "            earnedPoints = wastePoints[processedResult.wasteType];\n";
  js += "          } else {\n";
  js += "            earnedPoints = wastePoints['diğer'];\n";
  js += "          }\n";
  js += "          \n";
  js += "          // Kullanıcıya puanları ekle\n";
  js += "          activeUser.points += earnedPoints;\n";
  js += "          \n";
  js += "          // Kullanıcı verilerini kaydet\n";
  js += "          saveUserData();\n";
  js += "          updateActiveUserDisplay();\n\n";

  js += "          // Sonucu ESP32'ye gönder\n";
  js += "          console.log('Analiz sonucu ESP32 sunucusuna gönderiliyor...');\n";
  js += "          const saveResponse = await fetch('/save-result', {\n";
  js += "            method: 'POST',\n";
  js += "            headers: { 'Content-Type': 'application/json' },\n";
  js += "            body: JSON.stringify({\n";
  js += "              result: processedResult.resultText,\n";
  js += "              wasteType: processedResult.wasteType,\n";
  js += "              userId: activeUser.id,\n";
  js += "              earnedPoints: earnedPoints,\n";
  js += "              totalPoints: activeUser.points\n";
  js += "            })\n";
  js += "          });\n\n";
  
  js += "          if (!saveResponse.ok) {\n";
  js += "            console.warn('Sonuç ESP32 sunucusuna kaydedilemedi, ancak analiz sonucu gösteriliyor');\n";
  js += "          }\n\n";
  
  js += "          // Sonucu göster\n";
  js += "          wasteTypeElement.textContent = processedResult.wasteType;\n";
  js += "          earnedPointsElement.textContent = earnedPoints;\n";
  js += "          resultContainer.classList.remove('hidden');\n";
  js += "          \n";
  js += "        } catch (error) {\n";
  js += "          console.error('Hata:', error);\n";
  js += "          wasteTypeElement.textContent = 'Belirlenemedi';\n";
  js += "          earnedPointsElement.textContent = '0';\n";
  js += "          resultContainer.classList.remove('hidden');\n";
  js += "        } finally {\n";
  js += "          loadingContainer.classList.add('hidden');\n";
  js += "          fileInput.disabled = false;\n";
  js += "        }\n";
  js += "      }, 1000); // Önizlemenin yüklenmesi için kısa bir gecikme\n";
  js += "    } else {\n";
  js += "      fileName.textContent = 'Dosya seçilmedi';\n";
  js += "      previewContainer.classList.add('hidden');\n";
  js += "    }\n";
  js += "  });\n\n";
  
  // Görüntü sıkıştırma ve boyut küçültme
  js += "  // Görüntüyü sıkıştır ve boyutunu küçült\n";
  js += "  function resizeAndCompressImage(file, maxSize) {\n";
  js += "    return new Promise((resolve, reject) => {\n";
  js += "      const reader = new FileReader();\n";
  js += "      reader.onload = function(event) {\n";
  js += "        const img = new Image();\n";
  js += "        img.onload = function() {\n";
  js += "          // Boyut oranını hesapla\n";
  js += "          let width = img.width;\n";
  js += "          let height = img.height;\n";
  js += "          let ratio = 1;\n\n";
  
  js += "          if (width > height && width > maxSize) {\n";
  js += "            ratio = maxSize / width;\n";
  js += "          } else if (height > maxSize) {\n";
  js += "            ratio = maxSize / height;\n";
  js += "          }\n\n";
  
  js += "          // Yeni boyutları hesapla\n";
  js += "          width = Math.floor(width * ratio);\n";
  js += "          height = Math.floor(height * ratio);\n\n";
  
  js += "          // Canvas oluştur ve görüntüyü çiz\n";
  js += "          const canvas = document.createElement('canvas');\n";
  js += "          canvas.width = width;\n";
  js += "          canvas.height = height;\n";
  js += "          const ctx = canvas.getContext('2d');\n";
  js += "          ctx.drawImage(img, 0, 0, width, height);\n\n";
  
  js += "          // JPEG olarak sıkıştır (0.7 kalite - iyi sıkıştırma)\n";
  js += "          canvas.toBlob(function(blob) {\n";
  js += "            resolve(new File([blob], file.name, { type: 'image/jpeg' }));\n";
  js += "          }, 'image/jpeg', 0.7);\n";
  js += "        };\n";
  js += "        img.onerror = reject;\n";
  js += "        img.src = event.target.result;\n";
  js += "      };\n";
  js += "      reader.onerror = reject;\n";
  js += "      reader.readAsDataURL(file);\n";
  js += "    });\n";
  js += "  }\n\n";
  
  // Dosyayı Base64'e dönüştürme fonksiyonu
  js += "  function fileToBase64(file) {\n";
  js += "    return new Promise((resolve, reject) => {\n";
  js += "      const reader = new FileReader();\n";
  js += "      reader.onload = () => resolve(reader.result);\n";
  js += "      reader.onerror = reject;\n";
  js += "      reader.readAsDataURL(file);\n";
  js += "    });\n";
  js += "  }\n\n";
  
  // API sonucunu işleme fonksiyonu
  js += "  function processResult(data) {\n";
  js += "    let wasteType = '';\n\n";
  
  js += "    // Gemini API yanıtını işle\n";
  js += "    if (data.candidates && data.candidates.length > 0 &&\n";
  js += "        data.candidates[0].content && data.candidates[0].content.parts &&\n";
  js += "        data.candidates[0].content.parts.length > 0) {\n\n";
  
  js += "      const resultText = data.candidates[0].content.parts[0].text;\n\n";
  
  js += "      // Atık türünü çıkar\n";
  js += "      if (resultText.toLowerCase().includes('plastik')) {\n";
  js += "        wasteType = 'plastik';\n";
  js += "      } else if (resultText.toLowerCase().includes('kağıt')) {\n";
  js += "        wasteType = 'kağıt';\n";
  js += "      } else if (resultText.toLowerCase().includes('cam')) {\n";
  js += "        wasteType = 'cam';\n";
  js += "      } else if (resultText.toLowerCase().includes('metal')) {\n";
  js += "        wasteType = 'metal';\n";
  js += "      }\n";
  js += "    }\n\n";
  
  js += "    // Sonuç değerlerini döndür\n";
  js += "    return { resultText: '', wasteType: wasteType };\n";
  js += "  }\n";
  js += "});\n";
  
  server.send(200, "application/javascript", js);
}

// API proxy
void handleProxy() {
  if (server.hasArg("plain") == false) {
    server.send(400, "application/json", "{\"error\":\"İstek gövdesi boş\"}");
    return;
  }
  
  String requestBody = server.arg("plain");
  
  // Metin içeriğini bulmak için basit bir arama
  int textPos = requestBody.indexOf("\"text\":");
  if (textPos > 0) {
    // Metin içeriğini yaklaşık olarak çıkar (JSON ayrıştırma olmadan basit bir kesme)
    int textStart = requestBody.indexOf("\"", textPos + 7) + 1;
    int textEnd = requestBody.indexOf("\"", textStart);
    if (textStart > 0 && textEnd > textStart) {
      String promptText = requestBody.substring(textStart, textEnd);
    }
  }
  
  // Gemini API'ye bağlan
  if (!client.connect(geminiHost, geminiPort)) {
    server.send(500, "application/json", "{\"error\":\"API bağlantı hatası\"}");
    return;
  }
  
  // Zaman aşımı ayarı (45 saniye)
  client.setTimeout(45000);
  
  // API model parametresi güncellendi
  String url = "/v1beta/models/gemini-2.0-flash:generateContent?key=";
  url += geminiApiKey;
  
  // İstek gönderilme zamanını kaydet
  unsigned long requestStartTime = millis();
  
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + geminiHost + "\r\n" +
               "Content-Type: application/json\r\n" +
               "Content-Length: " + requestBody.length() + "\r\n" +
               "Connection: close\r\n\r\n" +
               requestBody);
               
  // HTTP yanıt başlığını oku
  String statusLine = "";
  String headers = "";
  
  // Zaman aşımı kontrolü (45 saniye)
  unsigned long timeout = 45000; // 45 saniye
  unsigned long startTime = millis();
  
  while (client.connected() && (millis() - startTime < timeout)) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      if (line.startsWith("HTTP/")) {
        statusLine = line;
      }
      headers += line + "\n";
      
      if (line == "\r") {
        break;
      }
    }
    delay(10); // Küçük bir gecikme ekleyerek ESP32'nin diğer işleri yapmasına izin ver
  }
  
  // Zaman aşımı kontrolü
  if (millis() - startTime >= timeout) {
    client.stop();
    server.send(504, "application/json", "{\"error\":\"API yanıt zaman aşımı\"}");
    return;
  }
  
  // HTTP yanıt gövdesini oku
  String response = "";
  startTime = millis(); // Zaman aşımı için süreyi sıfırla
  
  while (client.connected() && (millis() - startTime < timeout)) {
    if (client.available()) {
      char c = client.read();
      response += c;
    } else if (response.length() > 0) {
      // Veri akışı sona erdi
      break;
    }
    delay(10); // Küçük bir gecikme ekleyerek ESP32'nin diğer işleri yapmasına izin ver
  }
  
  // İkinci zaman aşımı kontrolü
  if (millis() - startTime >= timeout && client.connected()) {
    client.stop();
    server.send(504, "application/json", "{\"error\":\"API yanıt içeriği okuma zaman aşımı\"}");
    return;
  }
  
  // Bağlantıyı kapat
  client.stop();
  
  // Toplam işlem süresini hesapla ve göster
  unsigned long totalTime = millis() - requestStartTime;
  
  // HTTP durum kodunu kontrol et
  if (!statusLine.startsWith("HTTP/1.1 200")) {
    server.send(500, "application/json", "{\"error\":\"API yanıt hatası: " + statusLine + "\"}");
    return;
  }
  
  // Basit metin araması
  int textStartPos = response.indexOf("\"text\":");
  if (textStartPos > 0) {
    int contentStart = response.indexOf("\"", textStartPos + 7) + 1;
    int contentEnd = response.indexOf("\"", contentStart);
    if (contentStart > 0 && contentEnd > contentStart) {
      String resultText = response.substring(contentStart, contentEnd);
      
      // Atık türünü çıkar
      int wasteTypeIndex = resultText.indexOf("Atık türü:");
      if (wasteTypeIndex != -1 && wasteTypeIndex + 15 < resultText.length()) {
        // Basit bir yaklaşım - JSON ayrıştırma olmadan
        String wasteType = resultText.substring(wasteTypeIndex + 10);
        int endOfWaste = wasteType.indexOf('.');
        if (endOfWaste > 0) {
          wasteType = wasteType.substring(0, endOfWaste);
        }
        wasteType.trim();
      }
    }
  }
  
  // API yanıtını istemciye ilet
  server.send(200, "application/json", response);
}

// Analiz sonucunu kaydetme
void handleSaveResult() {
  if (server.hasArg("plain") == false) {
    server.send(400, "application/json", "{\"error\":\"İstek gövdesi boş\"}");
    return;
  }
  
  String requestBody = server.arg("plain");
  
  // Basit şekilde ayrıştır, bellek sorunlarından kaçınmak için
  int resultPos = requestBody.indexOf("\"result\":");
  int wasteTypePos = requestBody.indexOf("\"wasteType\":");
  int userIdPos = requestBody.indexOf("\"userId\":");
  int earnedPointsPos = requestBody.indexOf("\"earnedPoints\":");
  int totalPointsPos = requestBody.indexOf("\"totalPoints\":");
  
  if (resultPos > 0 && wasteTypePos > 0) {
    // Analiz sonucunu çıkar
    int resultStart = requestBody.indexOf("\"", resultPos + 9) + 1;
    int resultEnd = requestBody.indexOf("\"", resultStart);
    if (resultStart > 0 && resultEnd > resultStart) {
      analizSonucu = requestBody.substring(resultStart, resultEnd);
    }
    
    // Atık türünü çıkar
    int wasteTypeStart = requestBody.indexOf("\"", wasteTypePos + 12) + 1;
    int wasteTypeEnd = requestBody.indexOf("\"", wasteTypeStart);
    if (wasteTypeStart > 0 && wasteTypeEnd > wasteTypeStart) {
      atikTuru = requestBody.substring(wasteTypeStart, wasteTypeEnd);
      Serial.println(atikTuru); // Sadece atık türünü yazdır
    }
    
    // Kullanıcı bilgilerini çıkar
    if (userIdPos > 0) {
      int userIdStart = userIdPos + 9; // "userId": sonrası
      int userIdEnd = requestBody.indexOf(",", userIdStart);
      if (userIdEnd > userIdStart) {
        String userId = requestBody.substring(userIdStart, userIdEnd);
        userId.trim();
      }
    }
    
    // Kazanılan puanları çıkar
    if (earnedPointsPos > 0) {
      int earnedPointsStart = earnedPointsPos + 15; // "earnedPoints": sonrası
      int earnedPointsEnd = requestBody.indexOf(",", earnedPointsStart);
      if (earnedPointsEnd > earnedPointsStart) {
        String earnedPoints = requestBody.substring(earnedPointsStart, earnedPointsEnd);
        earnedPoints.trim();
      }
    }
    
    // Toplam puanları çıkar
    if (totalPointsPos > 0) {
      int totalPointsStart = totalPointsPos + 14; // "totalPoints": sonrası
      int totalPointsEnd = requestBody.indexOf("}", totalPointsStart);
      if (totalPointsEnd > totalPointsStart) {
        String totalPoints = requestBody.substring(totalPointsStart, totalPointsEnd);
        totalPoints.trim();
      }
    }
    
    server.send(200, "application/json", "{\"success\":true}");
  } else {
    server.send(400, "application/json", "{\"error\":\"Geçersiz sonuç formatı\"}");
  }
}
