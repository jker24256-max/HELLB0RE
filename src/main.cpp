#include <M5Cardputer.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <SPI.h>
#include <SD.h>
#include <time.h>
#include <string>
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
enum class Screen : uint8_t { Dashboard, Wifi, Ble, Diagnostics, Reports, AttackLab, Terminal };
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
int wifiCount = 0;
int bleCount = 0;
uint32_t bootMillis = 0;
uint32_t lastScanMillis = 0;
String lastReport = "No report saved yet";
constexpr uint16_t BG = 0x0812, PANEL = 0x10A2, PANEL_ACTIVE = 0x253F;
constexpr uint16_t CYAN = 0x07FF, WHITE = 0xFFFF, MUTED = 0x7BEF, GOLD = 0xFDE0, RED = 0xF986, GREEN = 0x87F0;

const char* menuItems[] = {"Wi-Fi discovery", "BLE discovery", "Diagnostics", "Reports / microSD", "Attack simulations", "Terminal"};
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
    footer("1-6 open  W/S move  ENTER open");
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
    report += "Uptime_ms: " + String(millis() - bootMillis) + "\n";
    report += "Free_heap: " + String(ESP.getFreeHeap()) + "\n";
    report += "WiFi_scan_complete: " + String(wifiScanDone ? "yes" : "no") + "\n";
    report += "WiFi_AP_count: " + String(wifiCount) + "\n";
    report += "BLE_scan_complete: " + String(bleScanDone ? "yes" : "no") + "\n";
    report += "BLE_device_count: " + String(bleCount) + "\n";
    report += "Note: advertised data is observational, not a complete security verdict.\n";
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

void drawDiagnostics() {
    header("DIAGNOSTICS");
    textLine(26, "Firmware: " HELLEBORE_VERSION, CYAN);
    textLine(40, "Uptime: " + String((millis() - bootMillis) / 1000) + "s");
    textLine(54, "Free heap: " + String(ESP.getFreeHeap()) + " bytes");
    textLine(68, "Wi-Fi: " + String(WiFi.getMode() == WIFI_MODE_NULL ? "idle" : "ready"));
    textLine(82, "BLE: " + String(bleReady ? "initialized" : "not initialized"));
    textLine(96, "microSD: " + String(sdReady ? "mounted" : "not mounted"));
    textLine(109, "Status: " + statusLine, MUTED);
    footer("R test SD  ESC back");
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

void drawAttackLab() {
    header("SIMULATOR");
    textLine(27, "CONTROLLED SECURITY EXERCISES", CYAN);
    textLine(43, "1. Rogue AP detection scenario", WHITE);
    textLine(57, "2. Open-network risk walkthrough", WHITE);
    textLine(71, "3. BLE privacy-risk walkthrough", WHITE);
    textLine(85, "4. Incident response checklist", WHITE);
    textLine(101, "Simulation only; no radio disruption.", GOLD);
    footer("1-4 choose exercise  ESC back");
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
        case hellebore::Command::Help: terminalOutput = "help about status scanwifi scanble report"; break;
        case hellebore::Command::About: terminalOutput = "HELLEBORE v" HELLEBORE_VERSION " Cardputer Adv"; break;
        case hellebore::Command::Status: terminalOutput = "heap " + String(ESP.getFreeHeap()) + " | " + statusLine; break;
        case hellebore::Command::Clear: terminalOutput = ""; break;
        case hellebore::Command::Unknown: {
            String cmd = terminalInput;
            cmd.trim();
            cmd.toLowerCase();
            if (cmd == "scanwifi") { runWifiScan(); terminalOutput = "Wi-Fi scan complete"; }
            else if (cmd == "scanble") { runBleScan(); terminalOutput = "BLE scan complete"; }
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
    if (screen == Screen::Dashboard) {
        if (c >= '1' && c <= '6') { selection = c - '1'; openSelection(); }
        else if (c == 'w' || c == 'W') { selection = (selection + MENU_COUNT - 1) % MENU_COUNT; dirty = true; }
        else if (c == 's' || c == 'S') { selection = (selection + 1) % MENU_COUNT; dirty = true; }
        else if (c == '\n' || c == '\r') openSelection();
        return;
    }
    if (c == 0x1B || c == '\b' || c == 0x7F) { back(); return; }
    if (c == 'w' || c == 'W') { if (scroll > 0) --scroll; dirty = true; return; }
    if (c == 's' || c == 'S') { ++scroll; dirty = true; return; }
    if (c == 'r' || c == 'R') {
        if (screen == Screen::Wifi) runWifiScan();
        else if (screen == Screen::Ble) runBleScan();
        else if (screen == Screen::Diagnostics || screen == Screen::Reports) {
            sdReady = mountSD(); statusLine = sdReady ? "microSD OK" : "microSD unavailable"; dirty = true;
        } else if (screen == Screen::AttackLab) {
            statusLine = "Scenario selected; simulation only"; dirty = true;
        }
        return;
    }
    if (c == 'e' || c == 'E') {
        if (screen == Screen::Wifi || screen == Screen::Reports) { saveReport(); dirty = true; }
        return;
    }
    if (screen == Screen::AttackLab && c >= '1' && c <= '4') {
        static const char* exercises[] = {
            "Rogue AP: compare observed SSID/BSSID changes.",
            "Open AP: verify sensitive traffic uses TLS.",
            "BLE privacy: observe rotating addresses.",
            "Response: record, contain, verify, document."
        };
        terminalOutput = exercises[c - '1'];
        screen = Screen::Terminal;
        dirty = true;
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
