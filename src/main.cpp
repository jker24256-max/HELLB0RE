#include <M5Cardputer.h>
#include <string>
#include "core/CommandParser.h"

#ifndef HELLEBORE_VERSION
#define HELLEBORE_VERSION "0.1.0"
#endif

namespace {
enum class Screen { Dashboard, Module, Terminal };
Screen currentScreen = Screen::Dashboard;
int selectedModule = 0;
String terminalInput;
String terminalOutput = "HELLEBORE terminal ready. Type help.";
bool dirty = true;

const char* modules[] = {
    "Wi-Fi Lab",
    "BLE Lab",
    "Diagnostics",
    "Reports"
};

void drawHeader(const char* subtitle) {
    M5Cardputer.Display.fillScreen(0x0812);
    M5Cardputer.Display.fillRect(0, 0, 240, 22, 0x18E3);
    M5Cardputer.Display.setTextColor(0xFDE0, 0x18E3);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.setCursor(7, 7);
    M5Cardputer.Display.print("HELLEBORE");
    M5Cardputer.Display.setTextColor(0xBDF7, 0x18E3);
    M5Cardputer.Display.setCursor(145, 7);
    M5Cardputer.Display.print(subtitle);
}

void drawDashboard() {
    drawHeader("v" HELLEBORE_VERSION);
    M5Cardputer.Display.setTextColor(0xFFFF, 0x0812);
    M5Cardputer.Display.setCursor(8, 29);
    M5Cardputer.Display.print("WIRELESS SECURITY TOOLKIT");
    for (int i = 0; i < 4; ++i) {
        const int x = 8 + (i % 2) * 116;
        const int y = 46 + (i / 2) * 34;
        const bool active = i == selectedModule;
        const uint16_t bg = active ? 0x253F : 0x10A2;
        M5Cardputer.Display.fillRoundRect(x, y, 108, 28, 4, bg);
        M5Cardputer.Display.drawRoundRect(x, y, 108, 28, 4, active ? 0x07FF : 0x4208);
        M5Cardputer.Display.setTextColor(active ? 0xFFFF : 0xC618, bg);
        M5Cardputer.Display.setCursor(x + 8, y + 10);
        M5Cardputer.Display.printf("%d. %s", i + 1, modules[i]);
    }
    M5Cardputer.Display.setTextColor(0x7BEF, 0x0812);
    M5Cardputer.Display.setCursor(8, 119);
    M5Cardputer.Display.print("1-4 select  ENTER open  T terminal");
}

void drawModule() {
    drawHeader("MODULE");
    M5Cardputer.Display.setTextColor(0xFFFF, 0x0812);
    M5Cardputer.Display.setCursor(8, 36);
    M5Cardputer.Display.printf("%s", modules[selectedModule]);
    M5Cardputer.Display.setTextColor(0xFFE0, 0x0812);
    M5Cardputer.Display.setCursor(8, 55);
    M5Cardputer.Display.print("Not implemented yet");
    M5Cardputer.Display.setTextColor(0xBDF7, 0x0812);
    M5Cardputer.Display.setCursor(8, 72);
    M5Cardputer.Display.print("This module will appear here");
    M5Cardputer.Display.setCursor(8, 85);
    M5Cardputer.Display.print("after its tests are in place.");
    M5Cardputer.Display.setCursor(8, 119);
    M5Cardputer.Display.print("ESC / BACKSPACE: dashboard");
}

void drawTerminal() {
    drawHeader("TERMINAL");
    M5Cardputer.Display.setTextColor(0x07FF, 0x0812);
    M5Cardputer.Display.setCursor(8, 32);
    M5Cardputer.Display.print(terminalOutput.substring(0, 90));
    M5Cardputer.Display.setTextColor(0xFFFF, 0x0812);
    M5Cardputer.Display.setCursor(8, 91);
    M5Cardputer.Display.print(">");
    M5Cardputer.Display.print(terminalInput.substring(0, 27));
    M5Cardputer.Display.setTextColor(0x7BEF, 0x0812);
    M5Cardputer.Display.setCursor(8, 119);
    M5Cardputer.Display.print("ENTER run  ESC return");
}

void render() {
    if (!dirty) return;
    switch (currentScreen) {
        case Screen::Dashboard: drawDashboard(); break;
        case Screen::Module: drawModule(); break;
        case Screen::Terminal: drawTerminal(); break;
    }
    dirty = false;
}

void runCommand() {
    const std::string commandText(terminalInput.c_str());
    switch (hellebore::parseCommand(commandText)) {
        case hellebore::Command::Help:
            terminalOutput = "help about status clear";
            break;
        case hellebore::Command::About:
            terminalOutput = "HELLEBORE v" HELLEBORE_VERSION " | Cardputer Adv";
            break;
        case hellebore::Command::Status:
            terminalOutput = "UI: OK | input: OK | modules: staged";
            break;
        case hellebore::Command::Clear:
            terminalOutput = "Terminal cleared.";
            break;
        case hellebore::Command::Unknown:
            terminalOutput = "Unknown command. Type help.";
            break;
    }
    terminalInput = "";
    dirty = true;
}

void handleCharacter(char c) {
    if (currentScreen == Screen::Terminal) {
        if (c == '\n' || c == '\r') {
            runCommand();
        } else if (c == '\b' || c == 0x7F) {
            if (terminalInput.length()) terminalInput.remove(terminalInput.length() - 1);
            dirty = true;
        } else if (c >= 32 && c <= 126 && terminalInput.length() < 48) {
            terminalInput += c;
            dirty = true;
        }
        return;
    }

    if (c == 't' || c == 'T') {
        currentScreen = Screen::Terminal;
        dirty = true;
    } else if (c >= '1' && c <= '4') {
        selectedModule = c - '1';
        dirty = true;
    } else if (c == '\n' || c == '\r') {
        currentScreen = Screen::Module;
        dirty = true;
    } else if (c == 0x1B || c == '\b' || c == 0x7F) {
        currentScreen = Screen::Dashboard;
        dirty = true;
    }
}
} // namespace

void setup() {
    auto config = M5.config();
    M5Cardputer.begin(config, true);
    M5Cardputer.Display.setRotation(1);
    M5Cardputer.Display.setBrightness(100);
    M5Cardputer.Display.setTextSize(1);
    M5Cardputer.Display.fillScreen(0x0812);
    M5Cardputer.Display.setTextColor(0x07FF, 0x0812);
    M5Cardputer.Display.setCursor(8, 48);
    M5Cardputer.Display.print("HELLEBORE");
    M5Cardputer.Display.setTextColor(0xFFFF, 0x0812);
    M5Cardputer.Display.setCursor(8, 66);
    M5Cardputer.Display.print("Starting dashboard...");
    delay(500);
    dirty = true;
}

void loop() {
    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
        auto state = M5Cardputer.Keyboard.keysState();
        for (char c : state.word) handleCharacter(c);
        if (state.enter) handleCharacter('\n');
        if (state.del) handleCharacter('\b');
        if (state.esc) handleCharacter(0x1B);
    }
    render();
    delay(5);
}
