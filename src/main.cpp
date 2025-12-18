#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AS5600.h>
#include <Arduino.h>
#include <Wire.h>
#include <util/atomic.h>
#include <util/delay.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define R_Enable PD2
#define L_Enable PD3
#define R_PWM PB1
#define L_PWM PB2
#define R_IS PC0
#define L_IS PC1

#define PREV PD6
#define SELECT PD7
#define NEXT PB0

class Motor {
   public:
    void init() {
        DDRD |= (1 << R_Enable) | (1 << L_Enable);
        DDRB |= (1 << R_PWM) | (1 << L_PWM);
    }

    void move(byte A, char dir) {
        PORTD |= (1 << R_Enable) | (1 << L_Enable);
        if (dir == 'F') {
            analogWrite(R_PWM, A);
            PORTD &= ~(1 << L_PWM);

        } else if (dir == 'B') {
            analogWrite(L_PWM, A);
            PORTD &= ~(1 << R_PWM);
        }
    }
};

// Global Variables
float Kp = 0.8;
float Ki = 1.2;

float targetRPM = 0, currentRPM = 0;
float err = 0, lastError = 0;
float integral = 0;
float output = 0;

unsigned long lastTime = 0;

int counter = 0;
int PWM;

char direction;

float current;
int getCurrent(int pin) {
    return analogRead(pin) / 1024 * 8.5;
}

Motor BTS7960;
AS5600 encoder;

void setup() {
    Serial.begin(9600);
    Wire.begin();

    BTS7960.init();

    pinMode(R_Enable, OUTPUT);
    pinMode(L_Enable, OUTPUT);
    pinMode(R_PWM, OUTPUT);
    pinMode(L_PWM, OUTPUT);

    DDRD &= ~(1 << PREV) | (1 << SELECT);
    DDRB &= ~(1 << NEXT);

    Serial.begin(9600);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        while (true);
    }
    if (!encoder.begin()) {
        Serial.println(F("AS5600 allocation failed"));
        while (true);
    }
    
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(WHITE);
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
    if (!(PIND & (1 << PREV))) {
        _delay_ms(10);
        counter--;
    }
    if (!(PIND & (1 << SELECT))) {
        _delay_ms(10);
        counter++;
    }

    Serial.println(counter);
    oledWrite(String(counter));
    if (counter > 0) {
        direction = 'F';
    } else {
        direction = 'B';
    }

    unsigned long currentTime = millis();
    float dt = (currentTime - lastTime) / 1000.0;
    
    targetRPM = abs(counter);
    currentRPM = encoder.getAngularSpeed(AS5600_MODE_RPM);

    err = targetRPM - currentRPM;
    if (abs(integral) < 255) {
        integral += err * dt;
    }

    output = Kp * err + Ki * integral;

    BTS7960.move(output, direction);

    lastError = err;
    lastTime = currentTime;
}