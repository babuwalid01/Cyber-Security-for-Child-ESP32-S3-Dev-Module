#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <RtcDS1302.h>
#include <ThreeWire.h>
#include <esp_heap_caps.h>
#include <esp_system.h>
#include <esp_partition.h>

const char* ssid = "22H2-2.4GHz"; 
const char* password = "Walid##Ariyan@88"; 

const char* www_username = "admin";
const char* www_password = "Walid@2026";

// Updated Network Coder Version Details
const char* DEV_VERSION = "esp32S3-11.2.8.2-NET-ULTRA RTC Module";
const char* MFG_DATE    = "2026-08-31";
const char* DEVELOPER   = "Walid Ul Islam";

// ==================== RTC TIME SETTINGS ====================
// DS1302 RTC Pin Setup (Diagram: DAT=GPIO5, SCLK=GPIO4, RST/CE=GPIO6)
ThreeWire myWire(5, 4, 6); // IO/DAT, SCLK, CE/RST
RtcDS1302<ThreeWire> Rtc(myWire);
bool rtcValid = false;
// ===========================================================

const uint16_t DNS_PORT = 53;
const char* upstreamDNS = "1.1.1.3"; 

WiFiUDP dnsServer;
WebServer server(80);

uint8_t* packetBuffer = NULL;

#define LED_RTC_STATUS    2   // RTC Activity LED Pin
#define FAN_PIN           13
#define BUZZER_PIN        14
#define LED_HEARTBEAT     15
#define LED_DNS           16
#define LED_BLOCK         17
#define NIGHT_PIN         18 
#define LED_WIFI_SEARCH   19 
#define LED_WIFI_STABLE   21 

unsigned long totalRequests = 0;
unsigned long allowedRequests = 0;
unsigned long blockedRequests = 0;

unsigned long lastHeartbeat = 0;
bool heartbeatState = false;

// RTC LED Timer Variables
unsigned long lastRtcBlink = 0;
bool rtcLedState = false;

unsigned long lastWifiSearchBlink = 0;
bool wifiSearchLedState = false;

unsigned long dnsLedOffTime = 0;
unsigned long blockLedOffTime = 0;
const unsigned long LED_FLASH_DURATION = 60; 

unsigned long lastWifiReconnectAttempt = 0;
const unsigned long RECONNECT_INTERVAL = 5000; 

bool fanActive = false;
unsigned long fanOffTime = 0;
const unsigned long FAN_RUN_DURATION = 1200000; 

const char* const blacklist[] PROGMEM = {
  "teredo.ipv6.microsoft.com", "teredo", "isatap", "wpad", "wpad.home",
  "apps.facebook", "apps.facebook.com", "canvas.facebook", "canvas.facebook.com",
  "instantgames", "instantgames.fb.com", "playables", "playables.youtube.com",
  "ads.youtube", "ads.youtube.com", "fb.gg", "game.facebook.com", "games.facebook.com", 
  "apps.fb.com", "fbinstantgames.com", "play.youtube.com",
  "apps.fbsbx.com", "game.fbsbx.com", "games.fbsbx.com", "instantgames.fbsbx.com", "canvas.fbsbx.com",
  "ad-maven.com", "adsco.re", "alwingulla.com", "bidis.pro",
  "clck.ru", "coinhive.com", "engine.propellerads.com", "go.onelink.me",
  "keboo.com", "onclickads.net", "pusher.com", "pushmaster-api.com",
  "realsrv.com", "servt.com", "shorte.st", "trafficjunky.net",
  "ultra-push.com", "yandex.ru/ads", "zeroredirect1.com",
  "1x-bet-bd.com", "1xbetbd.com", "mostbet-bd.com", "babu88bd.com",
  "krikya88.com", "parimatch-bd.com", "cric88.com", "jeetwin.com",
  "betvisa.com", "mcw.com", "casinomcw.com", "bj88.com",
  "nagad88.com", "velki.com", "velki365.com", "baaji365.com",
  "account.microsoft.com", "adnxs.com", "ads.pubmatic.com", "adservice.google.com", 
  "adssettings.google.com", "agar.io", "analytics.google.com", "armorgames.com", 
  "casem.media.net", "clickcease.com", "clientconfig.microsoft.com", "cloudflare-dns.com", 
  "cloudmoonapp.com", "corp.sts.microsoft.com", "crazygames.com", "cyberghostvpn.com", 
  "d.garena", "df.telemetry.microsoft.com", "diagnostics.support.microsoft.com", "diep.io", 
  "displaycatalog.mp.microsoft.com", "dns.adguard-dns.com", "dns.adguard.com", "dns.google", 
  "dns.google.com", "dns.microsoft", "dns.microsoft.com", "dns.nextdns.io", 
  "dns.quad9.net", "do.dsp.mp.microsoft.com", "doh.dns.microsoft", "doh.dns.microsoft.com", 
  "doh.pub", "doh.pub.com", "doubleclick", "easyfun.gg", 
  "ev.io", "expressvpn.com", "ff.garena", "freefire", 
  "gamenora.com", "gamessumo.com", "gaming", "garena", 
  "girlgames.app", "google-analytics.com", "googleadservices", "hit-oscar.opendns.com", 
  "hordes.io", "hotspotshield.com", "idcgames.com", "igamecj", 
  "iogames.space", "kartwars.io", "kongregate.com", "krunker.io", 
  "levelinfinite", "login.microsoftonline.com", "miniroyale.io", "msn.com", 
  "nordvpn.com", "now", "now.gg", "now.us", 
  "oca.telemetry.microsoft.com", "officeclient.microsoft.com", "one.one.one.one", "opendns.com", 
  "outbrain.com", "pagead2.googlesyndication.com", "playabl.ai", "poki.com", 
  "poxel.io", "protonvpn.com", "proximabeta", "pubg", 
  "pubg.com", "pubgmobile", "purchase.mp.microsoft.com", "quad9.net", 
  "reports.wes.df.telemetry.microsoft.com", "securepubads.g.doubleclick.net", "settings-win.data.microsoft.com", "shellshock.io", 
  "smashkarts.io", "sqm.microsoft.com", "sqm.telemetry.microsoft.com", "statsfe1.ws.microsoft.com", 
  "statsfe2.ws.microsoft.com", "storeimages.microsoft.com", "surfshark.com", "survey.watson.microsoft.com", 
  "surviv.io", "taboola.com", "telemetry.microsoft.com", "telemetry.urs.microsoft.com", 
  "tencent", "tunnelbear.com", "use-application-dns.net", "v10.events.data.microsoft.com", 
  "v20.events.data.microsoft.com", "venge.io", "voxiom.io", "watson.telemetry.microsoft.com", 
  "windscribe.com", "www.agar.io", "www.armorgames.com", "www.bing.com", 
  "www.crazygames.com", "www.diep.io", "www.doubleclick", "www.easyfun.gg", 
  "www.ev.io", "www.freefire", "www.gamenora.com", "www.gamessumo.com", 
  "www.gaming", "www.garena", "www.girlgames.app", "www.googleadservices", 
  "www.hordes.io", "www.idcgames.com", "www.igamecj", "www.iogames.space", 
  "www.kartwars.io", "www.kongregate.com", "www.krunker.io", "www.levelinfinite", 
  "www.miniroyale.io", "www.msn.com", "www.now.us", "www.poki.com", 
  "www.poxel.io", "www.proximabeta", "www.pubg", "www.pubg.com", 
  "www.pubgmobile", "www.shellshock.io", "www.smashkarts.io", "www.surviv.io", 
  "www.tencent", "www.venge.io", "www.voxiom.io", "www.y8.com", 
  "www.zombsroyale.io", "y8.com", "zombsroyale.io",
  "bangladrum.com", "bdtads.com", "eboundservices.com", "gads.prothomalo.com",
  "ads.prothomalo.com", "adserver.prothomalo.com", "ads.nayadiganta.com",
  "g.doubleclick.net", "pagead2.googlesyndication.com",
  "adservice.google.com", "googleadservices.com", "media.net", "casem.media.net",
  "taboola.com", "cdn.taboola.com", "trc.taboola.com", "outbrain.com",
  "widgets.outbrain.com", "mgid.com", "servicer.mgid.com", "popads.net",
  "popcash.net", "propellerads.com", "juicyads.com", "exoclick.com",
  "1xbet.com", "1x-bet.com", "melbet.com", "mostbet.com", "babu88.com",
  "krikya.com", "parimatch.com", "bet365.com", "betway.com", "777.com",
  "win777.com", "slots777.com", "casino777.com", "bet777.com", "mega777.com",
  "spin777.com", "jackpot777.com", "lucky777.com", "crazy777.com",
  "affiliates.1xbet.com", "affiliates.melbet.com", "promo.1xbet.com",
  "promotions.mostbet.com", "casingo.com", "betfair.com", "dafabet.com",
  "ad-delivery.net", "adform.net", "adhigh.net", "adk2x.com", "admaru.com",
  "admob.com", "adnxs.com", "adroll.com", "adsafeprotected.com", "adsrvr.org",
  "adsystem.com", "adthink.com", "adup-tech.com", "advertising.com",
  "affec.tv", "amazon-adsystem.com", "applovin.com", "appsflyer.com",
  "bidswitch.net", "criteo.com", "criteo.net", "scorecardresearch.com",
  "smartadserver.com", "rubiconproject.com", "pubmatic.com", "openx.net"
};
const int blacklistCount = sizeof(blacklist) / sizeof(blacklist[0]);

void sendInstantBlock(IPAddress clientIP, uint16_t clientPort, uint8_t* buffer, int size);
void triggerBuzzer(int count, int delayMs);
void handleThermalFan();
void handleDashboard();
void handleReboot();
void handleWifiStateLEDs();
void ensureWiFiConnection();
void handleNightMode();
void syncRTCTime();
void handleLEDTimers();
void handleRTCStatusLED();
String getFormattedTime();

void setup() {
  Serial.begin(115200);

  // Initialize RTC
  Rtc.Begin();
  if (Rtc.GetIsWriteProtected()) {
    Rtc.SetIsWriteProtected(false);
  }
  if (!Rtc.GetIsRunning()) {
    Rtc.SetIsRunning(true);
  }

  pinMode(LED_RTC_STATUS, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_HEARTBEAT, OUTPUT);
  pinMode(LED_DNS, OUTPUT);
  pinMode(LED_BLOCK, OUTPUT);
  pinMode(NIGHT_PIN, OUTPUT);
  pinMode(LED_WIFI_SEARCH, OUTPUT);
  pinMode(LED_WIFI_STABLE, OUTPUT);

  digitalWrite(LED_RTC_STATUS, LOW);
  digitalWrite(NIGHT_PIN, HIGH);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_HEARTBEAT, LOW);
  digitalWrite(LED_DNS, LOW);
  digitalWrite(LED_BLOCK, LOW);
  digitalWrite(LED_WIFI_STABLE, LOW);
  digitalWrite(LED_WIFI_SEARCH, HIGH); 

  if (psramInit()) {
    packetBuffer = (uint8_t*) heap_caps_malloc(512, MALLOC_CAP_SPIRAM);
    if (packetBuffer == NULL) {
      packetBuffer = (uint8_t*) malloc(512); 
    }
  } else {
    packetBuffer = (uint8_t*) malloc(512);
  }

  triggerBuzzer(2, 80);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  lastWifiReconnectAttempt = millis();

  server.on("/", handleDashboard);
  server.on("/reboot", handleReboot);
  server.begin();

  dnsServer.begin(DNS_PORT);
}

void loop() {
  unsigned long currentMillis = millis();

  ensureWiFiConnection();
  
  server.handleClient();
  handleWifiStateLEDs();
  handleNightMode();
  handleLEDTimers();
  handleRTCStatusLED(); // Dedicated RTC Status LED Handler

  if (currentMillis - lastHeartbeat >= 500) {
    lastHeartbeat = currentMillis;
    heartbeatState = !heartbeatState;
    digitalWrite(LED_HEARTBEAT, heartbeatState);
  }

  handleThermalFan();

  if (WiFi.status() == WL_CONNECTED) {
    int packetSize = dnsServer.parsePacket();
    if (packetSize >= 12 && packetBuffer != NULL) {
      IPAddress clientIP = dnsServer.remoteIP();
      uint16_t clientPort = dnsServer.remotePort();

      dnsServer.read(packetBuffer, 512);
      totalRequests++;

      digitalWrite(LED_DNS, HIGH);
      dnsLedOffTime = currentMillis + LED_FLASH_DURATION;

      char domain[128] = {0};
      int domainPos = 0;
      int i = 12;

      while (i < packetSize && packetBuffer[i] != 0 && domainPos < 126) {
        int len = packetBuffer[i++];
        for (int j = 0; j < len && i < packetSize && domainPos < 126; j++) {
          char c = (char)packetBuffer[i++];
          if (c >= 'A' && c <= 'Z') c += 32;
          domain[domainPos++] = c;
        }
        domain[domainPos++] = '.';
      }
      if (domainPos > 0) domain[domainPos - 1] = '\0';

      uint16_t queryType = 0;
      if (i + 2 <= packetSize) {
        queryType = (packetBuffer[i + 1] << 8) | packetBuffer[i + 2];
      }

      bool isBlocked = false;

      if (queryType == 28) { // IPv6 Query
        isBlocked = true;
      } 
      else if (strlen(domain) > 0) {
        for (int b = 0; b < blacklistCount; b++) {
          char tempBuf[100];
          strcpy_P(tempBuf, (char*)pgm_read_ptr(&(blacklist[b])));
          if (strstr(domain, tempBuf) != NULL) {
            isBlocked = true;
            break;
          }
        }
      }

      if (isBlocked) {
        blockedRequests++;
        sendInstantBlock(clientIP, clientPort, packetBuffer, packetSize);

        digitalWrite(LED_BLOCK, HIGH);
        blockLedOffTime = currentMillis + LED_FLASH_DURATION;
      } else {
        allowedRequests++;
        
        WiFiUDP directUpstream;
        if (directUpstream.beginPacket(upstreamDNS, 53)) {
          directUpstream.write(packetBuffer, packetSize);
          directUpstream.endPacket();

          unsigned long startWait = millis();
          while (millis() - startWait < 300) { 
            int replySize = directUpstream.parsePacket();
            if (replySize >= 12) {
              uint8_t replyBuffer[512];
              directUpstream.read(replyBuffer, 512);
              
              dnsServer.beginPacket(clientIP, clientPort);
              dnsServer.write(replyBuffer, replySize);
              dnsServer.endPacket();
              break;
            }
            delay(1);
          }
        }
      }
    }
  }
}

// ==================== RTC STATUS LED FUNCTION ====================
void handleRTCStatusLED() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastRtcBlink >= 500) {
    lastRtcBlink = currentMillis;

    RtcDateTime now = Rtc.GetDateTime();
    rtcValid = now.IsValid();

    if (rtcValid) {
      rtcLedState = !rtcLedState;
      digitalWrite(LED_RTC_STATUS, rtcLedState ? HIGH : LOW);
    } else {
      digitalWrite(LED_RTC_STATUS, LOW);
    }
  }
}
// =================================================================

void handleLEDTimers() {
  unsigned long currentMillis = millis();
  if (dnsLedOffTime > 0 && currentMillis >= dnsLedOffTime) {
    digitalWrite(LED_DNS, LOW);
    dnsLedOffTime = 0;
  }
  if (blockLedOffTime > 0 && currentMillis >= blockLedOffTime) {
    digitalWrite(LED_BLOCK, LOW);
    blockLedOffTime = 0;
  }
}

void ensureWiFiConnection() {
  unsigned long currentMillis = millis();
  if (WiFi.status() != WL_CONNECTED) {
    if (currentMillis - lastWifiReconnectAttempt >= RECONNECT_INTERVAL) {
      lastWifiReconnectAttempt = currentMillis;
      WiFi.begin(ssid, password);
    }
  }
}

void syncRTCTime() {
  RtcDateTime now = Rtc.GetDateTime();
  rtcValid = now.IsValid();
}

void handleNightMode() {
  RtcDateTime now = Rtc.GetDateTime();
  if (now.IsValid()) {
    int totalMinutes = (now.Hour() * 60) + now.Minute();
    if (totalMinutes >= 1 && totalMinutes <= 301) { 
      digitalWrite(NIGHT_PIN, LOW);
    } else {
      digitalWrite(NIGHT_PIN, HIGH);
    }
  } else {
    digitalWrite(NIGHT_PIN, HIGH);
  }
}

String getFormattedTime() {
  RtcDateTime now = Rtc.GetDateTime();
  if (!now.IsValid()) {
    return "RTC Error / Time Invalid";
  }
  char timeStr[60];
  int hour12 = now.Hour();
  String ampm = (hour12 >= 12) ? "PM" : "AM";
  if (hour12 > 12) hour12 -= 12;
  if (hour12 == 0) hour12 = 12;

  snprintf(timeStr, sizeof(timeStr), "%04d-%02d-%02d %02d:%02d:%02d %s (RTC)", 
           now.Year(), now.Month(), now.Day(), 
           hour12, now.Minute(), now.Second(), ampm.c_str());
  return String(timeStr);
}

void handleWifiStateLEDs() {
  unsigned long currentMillis = millis();
  if (WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED_WIFI_SEARCH, LOW);
    digitalWrite(LED_WIFI_STABLE, HIGH);
  } else {
    digitalWrite(LED_WIFI_STABLE, LOW);
    if (currentMillis - lastWifiSearchBlink >= 250) {
      lastWifiSearchBlink = currentMillis;
      wifiSearchLedState = !wifiSearchLedState;
      digitalWrite(LED_WIFI_SEARCH, wifiSearchLedState);
    }
  }
}

void handleDashboard() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }

  float tempC = temperatureRead();

  uint32_t totalRam = ESP.getHeapSize();
  uint32_t freeRam = ESP.getFreeHeap();
  float ramUsage = ((float)(totalRam - freeRam) / totalRam) * 100.0;

  uint32_t totalPsram = ESP.getPsramSize();
  uint32_t freePsram = ESP.getFreePsram();
  float psramUsage = (totalPsram > 0) ? (((float)(totalPsram - freePsram) / totalPsram) * 100.0) : 0.0;

  uint32_t totalFlash = ESP.getFlashChipSize();
  uint32_t usedFlash = ESP.getSketchSize();
  float flashUsage = ((float)usedFlash / totalFlash) * 100.0;

  float cpuUsage = (totalRequests > 0) ? 12.5 : 3.2; 

  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>DNS Firewall Dashboard</title>";
  html += "<style>";
  html += "body{background-color:#0f172a;color:#f8fafc;font-family:sans-serif;margin:0;padding:20px;}";
  html += ".card{background-color:#1e293b;border-radius:12px;padding:20px;margin-bottom:15px;box-shadow:0 4px 6px -1px rgba(0,0,0,0.5);}";
  html += "h2,h3{color:#38bdf8;margin-top:0;}";
  html += ".info-tag{background:#0284c7;color:#fff;padding:3px 8px;border-radius:4px;font-size:12px;font-weight:bold;display:inline-block;margin-bottom:6px;}";
  html += ".stat-grid{display:grid;grid-template-columns:1fr 1fr 1fr;gap:10px;}";
  html += ".stat-box{background:#334155;padding:15px;border-radius:8px;text-align:center;}";
  html += ".stat-val{font-size:22px;font-weight:bold;color:#4ade80;}";
  html += ".stat-lbl{font-size:12px;color:#94a3b8;}";
  html += ".blocked{color:#f87171;}";
  html += ".btn-reboot{background:#ef4444;color:white;border:none;padding:10px 18px;border-radius:6px;font-weight:bold;cursor:pointer;margin-top:10px;}";
  html += ".btn-reboot:hover{background:#dc2626;}";
  html += "</style>";
  html += "<script>setTimeout(function(){location.reload();}, 5000);</script>";
  html += "</head><body>";
  
  html += "<div class='card'>";
  html += "<h2>PRIVACY-FIRST DNS FIREWALL</h2>";
  
  // ১. উপরে বিল্ড ডেট, নিচে ভার্সন (একটার তলে আরেকটা)
  html += "<div><span class='info-tag'>BUILD DATE: " + String(MFG_DATE) + "</span></div>";
  html += "<div><span class='info-tag'>DEV VERSION: " + String(DEV_VERSION) + "</span></div>";
  
  html += "<p style='margin-top:10px;'>Developer: <b>" + String(DEVELOPER) + "</b></p>";
  html += "<p>Date & Time: <b>" + getFormattedTime() + "</b></p>";
  html += "<hr style='border:0;border-top:1px solid #334155;margin:15px 0;'>";
  
  html += "<p>CPU Usage: <b>" + String(cpuUsage, 1) + "%</b> | RAM Usage: <b>" + String(ramUsage, 1) + "%</b></p>";
  html += "<p>PSRAM Usage: <b>" + String(psramUsage, 1) + "%</b> | Flash Storage Used: <b>" + String(flashUsage, 1) + "%</b></p>";
  html += "<hr style='border:0;border-top:1px solid #334155;margin:15px 0;'>";

  html += "<p>Upstream DNS: <b>1.1.1.3 (Malware Protected)</b></p>";
  html += "<p>CPU Temp: <b>" + String(tempC, 1) + " &deg;C</b> | Fan: <b>" + (fanActive ? "<span style='color:#4ade80;'>ON</span>" : "<span style='color:#94a3b8;'>OFF</span>") + "</b></p>";
  
  html += "<form action='/reboot' method='POST' onsubmit='return confirm(\"Are you sure you want to reboot the device?\");'>";
  html += "<button type='submit' class='btn-reboot'>Reboot Device</button>";
  html += "</form>";
  html += "</div>";

  // ২. এক লাইনে ৩টি স্ট্যাটাস বক্স (TOTAL REQUESTS | PASSED | BLOCKED)
  html += "<div class='card'>";
  html += "<h3>Traffic Statistics</h3>";
  html += "<div class='stat-grid'>";
  html += "<div class='stat-box'><div class='stat-val'>" + String(totalRequests) + "</div><div class='stat-lbl'>TOTAL REQUESTS</div></div>";
  html += "<div class='stat-box'><div class='stat-val'>" + String(allowedRequests) + "</div><div class='stat-lbl'>PASSED</div></div>";
  html += "<div class='stat-box'><div class='stat-val blocked'>" + String(blockedRequests) + "</div><div class='stat-lbl'>BLOCKED</div></div>";
  html += "</div></div>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleReboot() {
  if (!server.authenticate(www_username, www_password)) {
    return server.requestAuthentication();
  }
  server.send(200, "text/html", "<h2>Rebooting ESP32-S3... Please wait 10 seconds and refresh page.</h2>");
  delay(1000);
  ESP.restart();
}

void handleThermalFan() {
  float tempC = temperatureRead();
  unsigned long currentMillis = millis();

  if (tempC >= 49.5) {
    fanActive = true;
    fanOffTime = currentMillis + FAN_RUN_DURATION;
    digitalWrite(FAN_PIN, HIGH);
  }

  if (fanActive && (currentMillis >= fanOffTime)) {
    if (tempC <= 45.0) {
      fanActive = false;
      digitalWrite(FAN_PIN, LOW);
    } else {
      fanOffTime = currentMillis + FAN_RUN_DURATION;
    }
  }
}

void triggerBuzzer(int count, int delayMs) {
  for (int i = 0; i < count; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(delayMs);
    digitalWrite(BUZZER_PIN, LOW);
    delay(delayMs);
  }
}

void sendInstantBlock(IPAddress clientIP, uint16_t clientPort, uint8_t* buffer, int size) {
  buffer[2] = 0x81; buffer[3] = 0x80;
  buffer[6] = 0x00; buffer[7] = 0x01;
  int p = size;
  buffer[p++] = 0xC0; buffer[p++] = 0x0C;
  buffer[p++] = 0x00; buffer[p++] = 0x01;
  buffer[p++] = 0x00; buffer[p++] = 0x01;
  buffer[p++] = 0x00; buffer[p++] = 0x00; buffer[p++] = 0x00; buffer[p++] = 0x01;
  buffer[p++] = 0x00; buffer[p++] = 0x04;
  buffer[p++] = 0; buffer[p++] = 0; buffer[p++] = 0; buffer[p++] = 0;
  dnsServer.beginPacket(clientIP, clientPort);
  dnsServer.write(buffer, p);
  dnsServer.endPacket();
}
