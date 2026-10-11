#include <M5Cardputer.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <SPI.h>
#include <SD.h>
#include <time.h>
#include <string>
#include <esp_wifi.h>
#include "core/CommandParser.h"

#ifndef HELLEBORE_VERSION
#define HELLEBORE_VERSION "0.1.0"
#endif

// M5Stack Cardputer Adv microSD bus. Keep configurable for board revisions.
#ifndef HELLEBORE_SD_CS
#define HELLEBORE_SD_CS 12
#define HELLEBORE_SD_SCK 40
#define HELLEBORE_SD_MISO 39
#define HELLEBORE_SD_MOSI 14
#endif

namespace {
enum class Screen : uint8_t { Dashboard, Wifi, Ble, Diagnostics, Reports, AttackLab, Terminal, Companion };
Screen screen = Screen::Dashboard;
int selection = 0;
int scroll = 0;
String terminalInput;
String terminalOutput = "HELLEBORE ready. Type help.";
String statusLine = "Ready";
bool dirty = true;
bool sdReady = false;
bool bleReady = false;
bool wifiScanDone = false;
bool bleScanDone = false;
bool passiveAuditDone = false;
bool passiveAuditRunning = false;
volatile uint32_t frameBeacons = 0, frameProbeRequests = 0, frameProbeResponses = 0;
volatile uint32_t frameDeauth = 0, frameDisassoc = 0, frameAuth = 0, frameData = 0, frameOtherMgmt = 0;
volatile uint8_t observedChannel = 0;
int wifiCount = 0;
int bleCount = 0;
uint32_t bootMillis = 0;
uint32_t lastScanMillis = 0;
String lastReport = "No report saved yet";
String networkCheckSummary = "Not run";
uint32_t companionXP = 0;
String companionMood = "CURIOUS";
String companionThought = "Let us map the airwaves.";
constexpr uint16_t BG = 0x0812, PANEL = 0x10A2, PANEL_ACTIVE = 0x253F;
constexpr uint16_t CYAN = 0x07FF, WHITE = 0xFFFF, MUTED = 0x7BEF, GOLD = 0xFDE0, RED = 0xF986, GREEN = 0x87F0;

const char* menuItems[] = {"Wi-Fi discovery", "BLE discovery", "Diagnostics", "Reports / microSD", "Wireless security lab", "Terminal"};
constexpr int MENU_COUNT = sizeof(menuItems) / sizeof(menuItems[0]);

void header(const char* title) {
    M5Cardputer.Display.fillScreen(BG);
    M5Cardputer.Display.fillRect(0, 0, 240, 20, 0x18E3);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(GOLD, 0x18E3);
    M5Cardputer.Display.setCursor(6, 6);
    M5Cardputer.Display.print("HELLEBORE");
    M5Cardputer.Display.setTextColor(WHITE, 0x18E3);
    M5Cardputer.Display.setCursor(139, 6);
    M5Cardputer.Display.print(title);
}

void footer(const char* text) {
    M5Cardputer.Display.fillRect(0, 121, 240, 14, 0x10A2);
    M5Cardputer.Display.setTextColor(MUTED, 0x10A2);
    M5Cardputer.Display.setCursor(5, 124);
    M5Cardputer.Display.print(text);
}

void textLine(int y, const String& text, uint16_t color = WHITE) {
    M5Cardputer.Display.setTextColor(color, BG);
    M5Cardputer.Display.setCursor(7, y);
    M5Cardputer.Display.print(text.substring(0, 38));
}

void drawDashboard() {
    header("DASHBOARD");
    M5Cardputer.Display.setTextColor(WHITE, BG);
    M5Cardputer.Display.setCursor(7, 25);
    M5Cardputer.Display.print("WIRELESS FIELD TOOLKIT");
    for (int i = 0; i < MENU_COUNT; ++i) {
        int x = 6 + (i % 2) * 117;
        int y = 40 + (i / 2) * 25;
        bool active = i == selection;
        uint16_t fill = active ? PANEL_ACTIVE : PANEL;
        M5Cardputer.Display.fillRoundRect(x, y, 111, 21, 3, fill);
        M5Cardputer.Display.drawRoundRect(x, y, 111, 21, 3, active ? CYAN : 0x4208);
        M5Cardputer.Display.setTextColor(active ? WHITE : 0xC618, fill);
        M5Cardputer.Display.setCursor(x + 5, y + 7);
        M5Cardputer.Display.printf("%d %s", i + 1, menuItems[i]);
    }
    footer("1-6 open  P companion  W/S move");
}

void drawWifi() {
    header("WI-FI");
    textLine(24, "2.4 GHz access point discovery", CYAN);
    textLine(36, wifiScanDone ? "Scan complete" : "Press R to scan", wifiScanDone ? GREEN : GOLD);
    if (!wifiScanDone) {
        textLine(51, "Press R to discover nearby APs");
    } else if (wifiCount == 0) {
        textLine(51, "No networks found (or scan failed)", GOLD);
    } else {
        int total = WiFi.scanComplete();
        int maxRows = 4;
        int start = scroll;
        for (int i = 0; i < maxRows && start + i < total; ++i) {
            int idx = start + i;
            String ssid = WiFi.SSID(idx);
            if (ssid.length() == 0) ssid = "<hidden SSID>";
            String row = ssid.substring(0, 13) + " " + String(WiFi.RSSI(idx)) + "dBm";
            row += " ch" + String(WiFi.channel(idx));
            wifi_auth_mode_t auth = WiFi.encryptionType(idx);
            row += (auth == WIFI_AUTH_OPEN) ? " OPEN" : " SEC";
            textLine(51 + i * 14, row, auth == WIFI_AUTH_OPEN ? RED : WHITE);
        }
        if (total > maxRows) textLine(109, "W/S scroll results", MUTED);
    }
    footer("R scan  W/S scroll  E export  Q back");
}

void runWifiScan() {
    statusLine = "Scanning Wi-Fi...";
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(false, false);
    int found = WiFi.scanNetworks(false, true, false, 300);
    wifiCount = found > 0 ? found : 0;
    wifiScanDone = true;
    scroll = 0;
    lastScanMillis = millis();
    statusLine = "Wi-Fi scan finished";
    dirty = true;
}

void drawBle() {
    header("BLE");
    textLine(24, "Bluetooth Low Energy discovery", CYAN);
    textLine(37, bleScanDone ? "Scan complete" : "Press R to scan", bleScanDone ? GREEN : GOLD);
    if (!bleReady) {
        textLine(52, "BLE stack unavailable", RED);
    } else if (!bleScanDone) {
        textLine(52, "Scans nearby advertisements");
        textLine(66, "Names may not be broadcast", MUTED);
    } else if (bleCount == 0) {
        textLine(52, "No advertisements found", GOLD);
    } else {
        BLEScan* scanner = BLEDevice::getScan();
        BLEScanResults results = scanner->getResults();
        int total = results.getCount();
        for (int i = 0; i < 4 && scroll + i < total; ++i) {
            BLEAdvertisedDevice device = results.getDevice(scroll + i);
            String name = device.haveName() ? String(device.getName().c_str()) : String("(unnamed)");
            String row = name.substring(0, 13) + " " + String(device.getRSSI()) + "dBm";
            textLine(52 + i * 14, row);
        }
        if (total > 4) textLine(109, "W/S scroll results", MUTED);
    }
    footer("R scan  W/S scroll  E export  ESC back");
}

void runBleScan() {
    if (!bleReady) {
        BLEDevice::init("HELLEBORE");
        bleReady = true;
    }
    statusLine = "Scanning BLE for 4s...";
    BLEScan* scanner = BLEDevice::getScan();
    scanner->setActiveScan(true);
    scanner->setInterval(100);
    scanner->setWindow(80);
    BLEScanResults results = scanner->start(4, false);
    bleCount = results.getCount();
    bleScanDone = true;
    scroll = 0;
    statusLine = "BLE scan finished";
    dirty = true;
}

bool mountSD() {
    if (sdReady) return true;
    SPI.begin(HELLEBORE_SD_SCK, HELLEBORE_SD_MISO, HELLEBORE_SD_MOSI, HELLEBORE_SD_CS);
    sdReady = SD.begin(HELLEBORE_SD_CS, SPI, 10000000);
    return sdReady;
}

String makeReport() {
    String report = "HELLEBORE REPORT\n";
    report += "Version: " HELLEBORE_VERSION "\n";
    report += "Companion_XP: " + String(companionXP) + "\n";
    report += "Companion_Level: " + String(companionLevel()) + "\n";
    report += "Uptime_ms: " + String(millis() - bootMillis) + "\n";
    report += "Free_heap: " + String(ESP.getFreeHeap()) + "\n";
    report += "Network_check: " + networkCheckSummary + "\n";
    if (WiFi.status() == WL_CONNECTED) {
        report += "Connected_SSID: " + WiFi.SSID() + "\n";
        report += "Local_IP: " + WiFi.localIP().toString() + "\n";
        report += "Gateway_IP: " + WiFi.gatewayIP().toString() + "\n";
        report += "Subnet: " + WiFi.subnetMask().toString() + "\n";
        report += "WiFi_RSSI: " + String(WiFi.RSSI()) + "\n";
    }
    report += "WiFi_scan_complete: " + String(wifiScanDone ? "yes" : "no") + "\n";
    report += "WiFi_AP_count: " + String(wifiCount) + "\n";
    report += "BLE_scan_complete: " + String(bleScanDone ? "yes" : "no") + "\n";
    report += "BLE_device_count: " + String(bleCount) + "\n";
    report += "Passive_audit_complete: " + String(passiveAuditDone ? "yes" : "no") + "\n";
    report += "Passive_audit_beacons: " + String((uint32_t)frameBeacons) + "\n";
    report += "Passive_audit_probe_requests: " + String((uint32_t)frameProbeRequests) + "\n";
    report += "Passive_audit_probe_responses: " + String((uint32_t)frameProbeResponses) + "\n";
    report += "Passive_audit_deauth_frames: " + String((uint32_t)frameDeauth) + "\n";
    report += "Passive_audit_disassoc_frames: " + String((uint32_t)frameDisassoc) + "\n";
    report += "Passive_audit_auth_frames: " + String((uint32_t)frameAuth) + "\n";
    report += "Passive_audit_data_frames: " + String((uint32_t)frameData) + "\n";
    report += "Passive_audit_other_mgmt: " + String((uint32_t)frameOtherMgmt) + "\n";
    report += "Note: passive frame counts are observations, not proof of an attack.\n";
    if (wifiScanDone) {
        int total = WiFi.scanComplete();
        for (int i = 0; i < total; ++i) {
            String ssid = WiFi.SSID(i);
            ssid.replace(",", "_");
            report += "AP," + ssid + "," + WiFi.BSSIDstr(i) + "," + String(WiFi.RSSI(i)) + "," + String(WiFi.channel(i));
            report += WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? ",OPEN\n" : ",SECURED\n";
        }
    }
    return report;
}

bool saveReport() {
    if (!mountSD()) {
        lastReport = "microSD unavailable";
        statusLine = "Insert/verify microSD";
        return false;
    }
    String path = "/HELLEBORE.TXT";
    if (SD.exists(path)) {
        path = "/HELLEBORE-" + String(millis()) + ".TXT";
    }
    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        lastReport = "SD write failed";
        statusLine = "SD write failed";
        return false;
    }
    String report = makeReport();
    size_t written = file.print(report);
    file.close();
    if (written != report.length()) {
        lastReport = "Incomplete report write";
        statusLine = "Report incomplete";
        return false;
    }
    lastReport = "Saved " + path;
    statusLine = "Report saved to SD";
    return true;
}

bool runActiveNetworkChecks() {
    if (WiFi.status() != WL_CONNECTED) {
        networkCheckSummary = "Not connected to Wi-Fi";
        statusLine = "Connect to an authorized Wi-Fi network first";
        terminalOutput = "NETCHECK: not connected. Join your own/test network using a trusted setup first.";
        dirty = true;
        return false;
    }
    IPAddress resolved;
    bool dnsOk = WiFi.hostByName("example.com", resolved) == 1;
    IPAddress gateway = WiFi.gatewayIP();
    bool gatewayHttp = false;
    bool gatewayHttps = false;
    WiFiClient client;
    client.setTimeout(1200);
    if (gateway != IPAddress(0, 0, 0, 0)) {
        gatewayHttp = client.connect(gateway, 80);
        if (gatewayHttp) client.stop();
        gatewayHttps = client.connect(gateway, 443);
        if (gatewayHttps) client.stop();
    }
    networkCheckSummary = String("DNS ") + (dnsOk ? "OK" : "FAIL") +
        " | GW:80 " + (gatewayHttp ? "open/responding" : "no response") +
        " | GW:443 " + (gatewayHttps ? "open/responding" : "no response");
    terminalOutput = "NETCHECK " + WiFi.localIP().toString() + " gw " + gateway.toString() +
        " DNS:" + (dnsOk ? "OK" : "FAIL") +
        " 80:" + (gatewayHttp ? "YES" : "NO") + " 443:" + (gatewayHttps ? "YES" : "NO");
    statusLine = "Active network checks complete";
    dirty = true;
    return dnsOk;
}

void drawDiagnostics() {
    header("DIAGNOSTICS");
    textLine(26, "Firmware: " HELLEBORE_VERSION, CYAN);
    textLine(40, "Uptime: " + String((millis() - bootMillis) / 1000) + "s");
    textLine(54, "Free heap: " + String(ESP.getFreeHeap()) + " bytes");
    textLine(68, "Wi-Fi: " + String(WiFi.getMode() == WIFI_MODE_NULL ? "idle" : "ready"));
    textLine(82, "BLE: " + String(bleReady ? "initialized" : "not initialized"));
    textLine(96, "microSD: " + String(sdReady ? "mounted" : "not mounted"));
    textLine(109, "R: network checks (connected Wi-Fi)", CYAN);
    footer("R network check  ESC back");
}

void drawReports() {
    header("REPORTS");
    textLine(28, "Report destination: microSD", CYAN);
    textLine(44, "Last: " + lastReport, lastReport.startsWith("Saved") ? GREEN : GOLD);
    textLine(62, "Reports include device health,", WHITE);
    textLine(75, "scan counts and available AP data.", WHITE);
    textLine(91, "BLE details stay summarized in v0.1.", MUTED);
    footer("E save report  R test SD  ESC back");
}

void passiveWifiCallback(void* buffer, wifi_promiscuous_pkt_type_t packetType) {
    if (!buffer) return;
    const wifi_promiscuous_pkt_t* packet = (const wifi_promiscuous_pkt_t*)buffer;
    if (!packet->payload) return;
    const uint8_t* frame = packet->payload;
    uint16_t fc = (uint16_t)frame[0] | ((uint16_t)frame[1] << 8);
    uint8_t type = (fc >> 2) & 0x03;
    uint8_t subtype = (fc >> 4) & 0x0F;
    observedChannel = packet->rx_ctrl.channel;
    if (type == 0) {
        if (subtype == 8) ++frameBeacons;
        else if (subtype == 4) ++frameProbeRequests;
        else if (subtype == 5) ++frameProbeResponses;
        else if (subtype == 12) ++frameDeauth;
        else if (subtype == 10) ++frameDisassoc;
        else if (subtype == 11) ++frameAuth;
        else ++frameOtherMgmt;
    } else if (type == 2) {
        ++frameData;
    }
}

void runPassiveWifiAudit() {
    statusLine = "Starting passive Wi-Fi audit...";
    passiveAuditRunning = true;
    passiveAuditDone = false;
    frameBeacons = frameProbeRequests = frameProbeResponses = 0;
    frameDeauth = frameDisassoc = frameAuth = frameData = frameOtherMgmt = 0;
    observedChannel = 0;
    WiFi.mode(WIFI_STA);
    WiFi.disconnect(false, false);
    esp_err_t cbResult = esp_wifi_set_promiscuous_rx_cb(&passiveWifiCallback);
    esp_err_t onResult = cbResult == ESP_OK ? esp_wifi_set_promiscuous(true) : cbResult;
    if (onResult != ESP_OK) {
        passiveAuditRunning = false;
        statusLine = "Passive monitor unavailable";
        terminalOutput = "Passive monitor failed (ESP error " + String((int)onResult) + ")";
        dirty = true;
        return;
    }
    // Listen only; this routine never transmits crafted frames or disrupts clients.
    const uint8_t channels[] = {1, 6, 11};
    for (uint8_t channel : channels) {
        esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
        uint32_t started = millis();
        while (millis() - started < 1800) {
            M5Cardputer.update();
            delay(10);
        }
    }
    esp_wifi_set_promiscuous(false);
    WiFi.mode(WIFI_MODE_NULL);
    passiveAuditRunning = false;
    passiveAuditDone = true;
    statusLine = "Passive audit complete (ch 1/6/11)";
    terminalOutput = "Passive Wi-Fi audit complete; press E to save evidence.";
    dirty = true;
}

void drawAttackLab() {
    header("WIRELESS AUDIT");
    textLine(25, "PASSIVE 2.4 GHz FRAME AUDIT", CYAN);
    textLine(38, passiveAuditRunning ? "Listening..." : (passiveAuditDone ? "Last audit: complete" : "No audit run yet"), passiveAuditDone ? GREEN : GOLD);
    textLine(51, "Beacons: " + String((uint32_t)frameBeacons));
    textLine(63, "Probe req/resp: " + String((uint32_t)frameProbeRequests) + "/" + String((uint32_t)frameProbeResponses));
    textLine(75, "Deauth/disassoc: " + String((uint32_t)frameDeauth) + "/" + String((uint32_t)frameDisassoc));
    textLine(87, "Auth/data: " + String((uint32_t)frameAuth) + "/" + String((uint32_t)frameData));
    textLine(101, "Listen-only; counts aren't verdicts.", MUTED);
    footer("R run audit  E export  ESC back");
}
uint8_t companionLevel() { return (uint8_t)(1 + (companionXP / 5 > 19 ? 19 : companionXP / 5)); }

void drawCompanion() {
    header("HELLE");
    M5Cardputer.Display.drawRoundRect(7, 26, 226, 72, 8, CYAN);
    M5Cardputer.Display.setTextColor(GOLD, BG);
    M5Cardputer.Display.setCursor(15, 33);
    M5Cardputer.Display.printf("HELLEBORE COMPANION  LV.%u", companionLevel());
    uint16_t face = companionMood == "PROUD" ? GOLD : CYAN;
    M5Cardputer.Display.fillRoundRect(18, 48, 42, 35, 7, face);
    M5Cardputer.Display.fillCircle(29, 60, 3, BG);
    M5Cardputer.Display.fillCircle(48, 60, 3, BG);
    M5Cardputer.Display.drawLine(29, 72, 48, 72, BG);
    M5Cardputer.Display.setTextColor(WHITE, BG);
    M5Cardputer.Display.setCursor(70, 51);
    M5Cardputer.Display.print("MOOD: " + companionMood);
    M5Cardputer.Display.setCursor(70, 65);
    M5Cardputer.Display.printf("XP: %lu  UP: %lus", (unsigned long)companionXP, (unsigned long)((millis() - bootMillis) / 1000));
    textLine(104, companionThought, MUTED);
    footer("R interact  E save report  ESC back");
}

void interactCompanion() {
    ++companionXP;
    if (passiveAuditDone || wifiScanDone || bleScanDone) {
        companionMood = "PROUD";
        companionThought = "Evidence beats guesswork. Nice work.";
    } else if (WiFi.status() == WL_CONNECTED) {
        companionMood = "CURIOUS";
        companionThought = "Link found. Keep your scope authorized.";
    } else {
        companionMood = "CURIOUS";
        companionThought = "Run a scan, then save what you learn.";
    }
    dirty = true;
}

void drawTerminal() {
    header("TERMINAL");
    textLine(30, terminalOutput, CYAN);
    M5Cardputer.Display.setTextColor(WHITE, BG);
    M5Cardputer.Display.setCursor(7, 91);
    M5Cardputer.Display.print("> ");
    M5Cardputer.Display.print(terminalInput.substring(0, 28));
    footer("ENTER run  Q dashboard");
}

void runCommand() {
    switch (hellebore::parseCommand(std::string(terminalInput.c_str()))) {
        case hellebore::Command::Help: terminalOutput = "help about status scanwifi scanble netcheck audit report"; break;
        case hellebore::Command::About: terminalOutput = "HELLEBORE v" HELLEBORE_VERSION " Cardputer Adv"; break;
        case hellebore::Command::Status: terminalOutput = "heap " + String(ESP.getFreeHeap()) + " | " + statusLine; break;
        case hellebore::Command::Clear: terminalOutput = ""; break;
        case hellebore::Command::Unknown: {
            String cmd = terminalInput;
            cmd.trim();
            cmd.toLowerCase();
            if (cmd == "scanwifi") { runWifiScan(); terminalOutput = "Wi-Fi scan complete"; }
            else if (cmd == "scanble") { runBleScan(); terminalOutput = "BLE scan complete"; }
            else if (cmd == "audit") { runPassiveWifiAudit(); }
            else if (cmd == "netcheck") { runActiveNetworkChecks(); }
            else if (cmd == "report") { terminalOutput = saveReport() ? "Report saved" : "Report failed; check SD"; }
            else terminalOutput = "Unknown command. Type help.";
            break;
        }
    }
    terminalInput = "";
    dirty = true;
}

void openSelection() {
    switch (selection) {
        case 0: screen = Screen::Wifi; break;
        case 1: screen = Screen::Ble; break;
        case 2: screen = Screen::Diagnostics; break;
        case 3: screen = Screen::Reports; break;
        case 4: screen = Screen::AttackLab; break;
        case 5: screen = Screen::Terminal; break;
        default: screen = Screen::Dashboard; break;
    }
    scroll = 0;
    dirty = true;
}

void back() { screen = Screen::Dashboard; dirty = true; }

void key(char c) {
    if (screen == Screen::Terminal) {
        if (c == '\n' || c == '\r') runCommand();
        else if (c == '\b' || c == 0x7F) {
            if (terminalInput.length()) terminalInput.remove(terminalInput.length() - 1);
            dirty = true;
        } else if (c >= 32 && c <= 126 && terminalInput.length() < 48) {
            terminalInput += c; dirty = true;
        }
        return;
    }
    if (screen == Screen::Companion) {
        if (c == 'r' || c == 'R') interactCompanion();
        else if (c == 'e' || c == 'E') { saveReport(); ++companionXP; companionMood = "PROUD"; companionThought = "Report secured. Evidence matters."; dirty = true; }
        return;
    }
    if (screen == Screen::Dashboard) {
        if (c >= '1' && c <= '6') { selection = c - '1'; openSelection(); }
        else if (c == 'w' || c == 'W') { selection = (selection + MENU_COUNT - 1) % MENU_COUNT; dirty = true; }
        else if (c == 's' || c == 'S') { selection = (selection + 1) % MENU_COUNT; dirty = true; }
        else if (c == 'p' || c == 'P') { screen = Screen::Companion; dirty = true; }
        else if (c == '\n' || c == '\r') openSelection();
        return;
    }
    if (c == 0x1B || c == '\b' || c == 0x7F) { back(); return; }
    if (c == 'w' || c == 'W') { if (scroll > 0) --scroll; dirty = true; return; }
    if (c == 's' || c == 'S') { ++scroll; dirty = true; return; }
    if (c == 'r' || c == 'R') {
        if (screen == Screen::Wifi) runWifiScan();
        else if (screen == Screen::Ble) runBleScan();
        else if (screen == Screen::Diagnostics) {
            runActiveNetworkChecks();
        } else if (screen == Screen::Reports) {
            sdReady = mountSD(); statusLine = sdReady ? "microSD OK" : "microSD unavailable"; dirty = true;
        } else if (screen == Screen::AttackLab) {
            runPassiveWifiAudit();
        }
        return;
    }
    if (c == 'e' || c == 'E') {
        if (screen == Screen::Wifi || screen == Screen::Reports || screen == Screen::AttackLab) { saveReport(); dirty = true; }
        return;
    }

}

void render() {
    if (!dirty) return;
    switch (screen) {
        case Screen::Dashboard: drawDashboard(); break;
        case Screen::Wifi: drawWifi(); break;
        case Screen::Ble: drawBle(); break;
        case Screen::Diagnostics: drawDiagnostics(); break;
        case Screen::Reports: drawReports(); break;
        case Screen::AttackLab: drawAttackLab(); break;
        case Screen::Terminal: drawTerminal(); break;
        case Screen::Companion: drawCompanion(); break;
    }
    dirty = false;
}
} // namespace

void setup() {
    auto config = M5.config();
    M5Cardputer.begin(config, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setBrightness(100);
    M5Cardputer.Display.setTextSize(1);
    bootMillis = millis();
    WiFi.mode(WIFI_MODE_NULL);
    M5Cardputer.Display.fillScreen(BG);
    M5Cardputer.Display.setTextColor(GOLD, BG);
    M5Cardputer.Display.setTextSize(2);
    M5Cardputer.Display.setCursor(24, 43);
    M5Cardputer.Display.print("HELLEBORE");
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setTextColor(CYAN, BG);
    M5Cardputer.Display.setCursor(36, 72);
    M5Cardputer.Display.print("FIELD COMPANION ONLINE");
    delay(650);
    render();
}

void loop() {
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        auto state = M5Cardputer.Keyboard.keysState();
        for (char c : state.word) key(c);
        if (state.enter) key('\n');
        if (state.del) key('\b');
    }
    render();
    delay(5);
}
