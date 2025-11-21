#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <Wire.h>
#include <util/atomic.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define R_Enable PD2
#define L_Enable PD3
#define R_PWM PD5
#define L_PWM PD6
#define R_IS PC0
#define L_IS PC1

#define BTN PB0
#define CLK PD7
#define DT PD4

class Motor {
   public:
    void init() {
        DDRD |= (1 << R_Enable) | (1 << L_Enable) | (1 << R_PWM) | (1 << L_PWM);
    }

    void move(byte A, char dir) {
        if (dir == 'F') {
            analogWrite(R_PWM, A);
            PORTD |= (1 << R_Enable);
            PORTD &= ~(1 << L_Enable);
        } else if (dir == 'B') {
            analogWrite(L_PWM, A);
            PORTD |= (1 << L_Enable);
            PORTD &= ~(1 << R_Enable);
        }
    }
};

Motor BTS7960;

// Global Variables
int Kp;
int Ki;

int PWM;
float ang;
float err;
float current;

int counter = 0;
int direction = 0;
int CLK_state, prev_CLK_state;

int getCurrent(int pin) {
    return analogRead(pin) / 1024 * 8.5;
}

void setup() {
    Serial.begin(9600);
    BTS7960.init();
    DDRD &= ~(1 << CLK) | (1 << DT);
    DDRB &= ~(1 << BTN);

    Serial.begin(9600);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        while (true);
    }
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(WHITE);
    prev_CLK_state = (PIND & (1 << CLK)) >> CLK;
    display.println("starting...");
}

void oledWrite(String msg) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1.5);
    display.println("Seleccione RPM");
    display.setTextSize(3);
    display.setCursor(0, 20);
    display.println(msg);
    display.display();
}

void loop() {
    CLK_state = (PIND & (1 << CLK)) >> CLK;
    if (CLK_state != prev_CLK_state) {  // step has occurred
        if (((PIND & (1 << DT)) >> DT) != CLK_state) {
            counter--;
            direction = 1;
        } else {
            counter++;
            direction = 0;
        }
    }
    Serial.println(counter);
    oledWrite(String(counter));
    prev_CLK_state = CLK_state;
}