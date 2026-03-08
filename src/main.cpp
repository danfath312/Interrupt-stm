#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <HardwareTimer.h>

#define LED_PIN PC13
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
HardwareTimer *timer2 = nullptr;

volatile uint32_t totalTickCount = 0;
volatile uint32_t pendingTicks = 0;

bool ledOn = false;
bool oledReady = false;

void timerISR() {
    totalTickCount++;
    pendingTicks++;
}

void drawStatus(uint32_t tickSnapshot) {
    if (!oledReady) {
        return;
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("STM32 TIMER INT");
    display.print("TICK : ");
    display.println(tickSnapshot);
    display.print("LED  : ");
    display.println(ledOn ? "ON" : "OFF");
    display.display();
}

void setup() {
    Serial.begin(115200);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    Wire.setSCL(PB6);
    Wire.setSDA(PB7);
    Wire.begin();

    oledReady = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
    if (!oledReady) {
        Serial.println("OLED init gagal");
    } else {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("STM32 TIMER INT");
        display.println("TICK : 0");
        display.println("LED: OFF");
        display.display();
    }

    // Internal interrupt source: TIM2 update event every 500 ms.
    timer2 = new HardwareTimer(TIM2);
    timer2->setOverflow(500000, MICROSEC_FORMAT);
    timer2->attachInterrupt(timerISR);
    timer2->resume();

    Serial.println("STM32 internal timer interrupt (TIM2) siap");
}

void loop() {
    uint32_t localPending;
    uint32_t localTick;

    noInterrupts();
    localPending = pendingTicks;
    localTick = totalTickCount;
    pendingTicks = 0;
    interrupts();

    if (localPending == 0) {
        return;
    }

    // Process all pending timer events in case loop is delayed.
    while (localPending--) {
        ledOn = !ledOn;
        digitalWrite(LED_PIN, ledOn ? LOW : HIGH);
    }

    Serial.print("TICK: ");
    Serial.print(localTick);
    Serial.print(" | LED: ");
    Serial.println(ledOn ? "ON" : "OFF");

    drawStatus(localTick);
}
