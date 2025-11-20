#include <Arduino.h>
#include <util/atomic.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define R_Enable PD2
#define L_Enable PD3
#define R_PWM PD5
#define L_PWM PD6
#define R_IS PC0
#define L_IS PC1

class Motor {
  public:
    void init() {
      DDRD |= (1<< R_Enable) | (1<< L_Enable) | (1<< R_PWM) | (1<< L_PWM);
    }

    void move(byte A, char dir) {
      if (dir == 'F') {
        analogWrite(R_PWM, A);
        PORTD |= (1<<R_Enable);
        PORTD &= ~(1<<L_Enable);
      }
      else if (dir == 'B') {
        analogWrite(L_PWM, A);
        PORTD |= (1<<L_Enable);
        PORTD &= ~(1<<R_Enable);
      }
    }
};

Motor BTS7960;

// Global Variables
int Kp ;
int Ki ;

int PWM;
float ang;
float err;
float current;

int getCurrent(int pin) {
  return analogRead(pin)/1024 * 8.5;
}

void setup() {
  BTS7960.init();
  Serial.begin(9600);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }
  display.clearDisplay();
  display.setCursor(0,0);
}

void oledWrite(String msg){
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(0,0);
  display.setTextColor(WHITE);
  display.println(msg);
  display.display();
}


void loop() {

}

