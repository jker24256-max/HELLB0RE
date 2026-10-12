#include <Arduino.h>
#include <M5Cardputer.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>

namespace {
constexpr char VERSION[] = "0.1.0";
constexpr uint16_t BG = 0x0841;
constexpr uint16_t PANEL = 0x10A2;
constexpr uint16_t CYAN = 0x07FF;
constexpr uint16_t GOLD = 0xFDE0;
constexpr uint16_t WHITE = 0xFFFF;
constexpr uint16_t MUTED = 0x7BEF;
constexpr uint16_t GREEN = 0x07E0;
constexpr uint16_t RED = 0xF800;

enum class Mood : uint8_t { Calm, Curious, Happy, Sleepy };
Mood mood = Mood::Calm;
uint32_t bootAt = 0;
uint32_t lastFrame = 0;
uint32_t lastBlink = 0;
uint32_t lastInteract = 0;
uint32_t focusLeft = 25UL * 60UL;
uint32_t focusTick = 0;
uint16_t interactions = 0;
bool blink = false;
bool focusRunning = false;
bool sdAvailable = false;
bool showTools = false;
bool focusScreen = false;
String thought = "HELLEBORE IS READY.";

void centerText(int y, const String& text, uint16_t color, uint8_t size=1) {
  M5Cardputer.Display.setTextSize(size);
  M5Cardputer.Display.setTextColor(color, BG);
  int16_t x = (240 - (int16_t)text.length() * 6 * size) / 2;
  if (x < 2) x = 2;
  M5Cardputer.Display.setCursor(x, y);
  M5Cardputer.Display.print(text);
  M5Cardputer.Display.setTextSize(1);
}

void saveState() {
  if (!sdAvailable) return;
  File f = SD.open("/HELLEBORE.DAT", FILE_WRITE);
  if (!f) return;
  f.printf("%u,%u\n", (unsigned)interactions, (unsigned)mood);
  f.close();
}

void loadState() {
  sdAvailable = SD.begin(12, SPI, 25000000);
  if (!sdAvailable) return;
  File f = SD.open("/HELLEBORE.DAT", FILE_READ);
  if (!f) return;
  interactions = (uint16_t)f.parseInt();
  if (f.read() == ',') mood = (Mood)constrain(f.parseInt(), 0, 3);
  f.close();
}

void mascot(int x, int y, uint32_t now) {
  const int bob = (int)((now / 260) % 3) - 1;
  y += bob;
  const uint16_t body = mood == Mood::Happy ? GOLD : (mood == Mood::Sleepy ? MUTED : CYAN);
  // HELLEBORE's little antenna-eared companion, drawn natively for 240x135.
  M5Cardputer.Display.drawLine(x+15,y+13,x+8,y+3,body);
  M5Cardputer.Display.drawLine(x+49,y+13,x+56,y+3,body);
  M5Cardputer.Display.fillCircle(x+8,y+3,2,GOLD);
  M5Cardputer.Display.fillCircle(x+56,y+3,2,GOLD);
  M5Cardputer.Display.fillRoundRect(x,y+9,64,42,14,body);
  M5Cardputer.Display.fillRoundRect(x+10,y+47,14,5,2,body);
  M5Cardputer.Display.fillRoundRect(x+40,y+47,14,5,2,body);
  if (blink) {
    M5Cardputer.Display.drawLine(x+15,y+27,x+24,y+27,BG);
    M5Cardputer.Display.drawLine(x+40,y+27,x+49,y+27,BG);
  } else {
    M5Cardputer.Display.fillRoundRect(x+16,y+20,7,12,3,BG);
    M5Cardputer.Display.fillRoundRect(x+41,y+20,7,12,3,BG);
    M5Cardputer.Display.fillCircle(x+19,y+23,2,WHITE);
    M5Cardputer.Display.fillCircle(x+44,y+23,2,WHITE);
  }
  if (mood == Mood::Happy) {
    M5Cardputer.Display.drawLine(x+25,y+39,x+32,y+43,BG);
    M5Cardputer.Display.drawLine(x+32,y+43,x+39,y+39,BG);
  } else {
    M5Cardputer.Display.drawLine(x+26,y+40,x+38,y+40,BG);
  }
}

void drawHome(uint32_t now) {
  M5Cardputer.Display.fillScreen(BG);
  M5Cardputer.Display.fillRect(0,0,240,19,PANEL);
  M5Cardputer.Display.setTextColor(GOLD,PANEL);
  M5Cardputer.Display.setCursor(7,6);
  M5Cardputer.Display.print("HELLEBORE");
  M5Cardputer.Display.setTextColor(MUTED,PANEL);
  M5Cardputer.Display.setCursor(174,6);
  M5Cardputer.Display.print("DESK PET");
  M5Cardputer.Display.setTextColor(CYAN,BG);
  M5Cardputer.Display.setCursor(8,25);
  M5Cardputer.Display.printf("UP %02lu:%02lu  V%s", (unsigned long)((now-bootAt)/60000), (unsigned long)((now-bootAt)/1000%60), VERSION);
  M5Cardputer.Display.drawRoundRect(6,35,228,65,8,CYAN);
  // Small decorative star field.
  for (int i=0;i<9;i++) {
    int x=(i*29+17)%228+6, y=40+(i*13)%54;
    uint16_t c=((now/900+i)%4==0)?GOLD:MUTED;
    M5Cardputer.Display.drawPixel(x,y,c);
  }
  mascot(88,42,now);
  M5Cardputer.Display.fillRoundRect(8,103,224,16,4,PANEL);
  M5Cardputer.Display.setTextColor(WHITE,PANEL);
  M5Cardputer.Display.setCursor(14,108);
  M5Cardputer.Display.printf("MOOD %s  /  HELLO #%u", mood==Mood::Calm?"CALM":mood==Mood::Curious?"CURIOUS":mood==Mood::Happy?"HAPPY":"SLEEPY", interactions);
  M5Cardputer.Display.fillCircle(222,27,3,sdAvailable?GREEN:RED);
  M5Cardputer.Display.fillRect(0,122,240,13,PANEL);
  M5Cardputer.Display.setTextColor(MUTED,PANEL);
  M5Cardputer.Display.setCursor(5,125);
  M5Cardputer.Display.print("P PET  F FOCUS  M MENU  T TOOLS");
}

void drawFocus() {
  M5Cardputer.Display.fillScreen(BG);
  M5Cardputer.Display.fillRect(0,0,240,19,PANEL);
  M5Cardputer.Display.setTextColor(GOLD,PANEL);
  M5Cardputer.Display.setCursor(7,6);
  M5Cardputer.Display.print("HELLEBORE / FOCUS");
  centerText(34,"FOCUS NOOK",CYAN);
  centerText(50,String(focusLeft/60/10)+String((focusLeft/60)%10)+":"+String((focusLeft%60)/10)+String(focusLeft%10),GOLD,2);
  M5Cardputer.Display.drawRoundRect(15,79,210,9,4,CYAN);
  uint32_t done=1500UL-focusLeft;
  int w=(int)(done*204UL/1500UL);
  M5Cardputer.Display.fillRoundRect(18,82,w,3,2,CYAN);
  centerText(96,focusRunning?"YOU'RE DOING GREAT":"SMALL STEPS COUNT",WHITE);
  centerText(111,"SPACE START/PAUSE  R RESET",MUTED);
}

void drawTools() {
  M5Cardputer.Display.fillScreen(BG);
  M5Cardputer.Display.fillRect(0,0,240,19,PANEL);
  M5Cardputer.Display.setTextColor(GOLD,PANEL);
  M5Cardputer.Display.setCursor(7,6);
  M5Cardputer.Display.print("HELLEBORE / TOOLS");
  centerText(30,"A TINY DESK COMPANION",CYAN);
  centerText(48,"P  Interact with Helle",WHITE);
  centerText(63,"F  Focus timer",WHITE);
  centerText(78,"T  Return to desk",WHITE);
  centerText(93,"S  Save companion state",WHITE);
  centerText(111,sdAvailable?"MICROSD READY":"MICROSD NOT MOUNTED",sdAvailable?GREEN:RED);
}

void interact() {
  ++interactions;
  mood = (Mood)(((uint8_t)mood + 1) % 4);
  const char* lines[] = {"HELLEBORE IS READY.","OH? WHAT'S THAT?","YOU'RE DOING GREAT!","TINY REST TIME..."};
  thought = lines[(uint8_t)mood];
  lastInteract = millis();
  saveState();
}

void handleKey(char c) {
  if (showTools) {
    if (c=='t'||c=='T'||c==0x1B) showTools=false;
    else if (c=='p'||c=='P') interact();
    else if (c=='f'||c=='F') { showTools=false; focusScreen=true; focusRunning=false; focusTick=millis(); }
    else if (c=='s'||c=='S') saveState();
    return;
  }
  if (focusScreen) {
    if (c==' ') { focusRunning=!focusRunning; focusTick=millis(); }
    else if (c=='r'||c=='R') { focusLeft=1500; focusRunning=false; }
    else if (c==0x1B||c=='t'||c=='T') { focusRunning=false; focusScreen=false; showTools=false; }
    return;
  }
  if (c=='p'||c=='P') interact();
  else if (c=='f'||c=='F') { focusLeft=1500; focusRunning=false; focusScreen=true; focusTick=millis(); lastFrame=0; showTools=false; }
  else if (c=='m'||c=='M') showTools=true;
  else if (c=='t'||c=='T') showTools=true;
  else if (c=='s'||c=='S') saveState();
}

} // namespace

void setup() {
  auto cfg=M5.config();
  M5Cardputer.begin(cfg,true);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setBrightness(150);
  M5Cardputer.Display.fillScreen(BG);
  centerText(48,"HELLEBORE",GOLD,2);
  centerText(72,"INITIALIZING DESK MODE",CYAN);
  bootAt=millis();
  focusTick=bootAt;
  loadState();
  delay(350);
}

void loop() {
  M5Cardputer.update();
  const uint32_t now=millis();
  if (focusRunning && now-focusTick>=1000) {
    uint32_t elapsed=(now-focusTick)/1000;
    focusLeft=elapsed>=focusLeft?0:focusLeft-elapsed;
    focusTick+=elapsed*1000;
    if (focusLeft==0) { focusRunning=false; mood=Mood::Happy; thought="FOCUS COMPLETE. NICE WORK."; saveState(); }
  } else if (!focusRunning) focusTick=now;

  if (now-lastBlink>3600) { blink=!blink; lastBlink=now; }
  if (M5Cardputer.Keyboard.isChange() && M5Cardputer.Keyboard.isPressed()) {
    auto keys=M5Cardputer.Keyboard.keys();
    for (auto key:keys.word) handleKey((char)key);
    if (keys.enter) handleKey('\n');
    if (keys.del) handleKey(0x7F);
  }
  if (now-lastFrame>180) {
    lastFrame=now;
    if (showTools) drawTools();
    else if (focusScreen) drawFocus();
    else drawHome(now);
  }
  delay(5);
}
