#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AS5600.h>
#include <Wire.h>

// --- Configuration ---
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define R_Enable 2
#define L_Enable 3
#define R_PWM 10
#define L_PWM 9

#define PREV PD6
#define SELECT PD7

// --- PI Constants (Tune these!) ---
float Kp = 0.3;   // Proportional gain
float Ki = 1.5;   // Integral gain

// --- Variables ---
float targetRPM = 0;
float currentRPM = 0;
float integral = 0;
float outputPWM = 0;
unsigned long lastTime = 0;

unsigned long lastDisplayUpdate = 0;
const unsigned long displayInterval = 300; // Update OLED every 300ms

bool lastPrevState = HIGH;
bool lastSelectState = HIGH;

// --- Classes ---
class Motor {
   public:
    void init() {
        pinMode(R_Enable, OUTPUT);
        pinMode(L_Enable, OUTPUT);
        pinMode(R_PWM, OUTPUT);
        pinMode(L_PWM, OUTPUT);
        digitalWrite(R_Enable, HIGH); // Enable the driver
        digitalWrite(L_Enable, HIGH);
    }

    void drive(float pwmValue) {
        // Logic: PWM > 0 Forward, PWM < 0 Backward
        if (pwmValue >= 0) {
            int val = (int)constrain(pwmValue, 0, 255);
            analogWrite(R_PWM, val);
            analogWrite(L_PWM, 0);
        } else {
            int val = (int)constrain(abs(pwmValue), 0, 255);
            analogWrite(L_PWM, val);
            analogWrite(R_PWM, 0);
        }
    }
};

Motor BTS7960;
AS5600 encoder;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    BTS7960.init();

    pinMode(PREV, INPUT_PULLUP);
    pinMode(SELECT, INPUT_PULLUP);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) while (true);
    encoder.begin();
    
    display.clearDisplay();
    display.setTextColor(WHITE);
    lastTime = millis();
}

void readButtons() {
    bool currentPrev = digitalRead(PREV);
    bool currentSelect = digitalRead(SELECT);

    if (currentPrev == LOW && lastPrevState == HIGH) {
        targetRPM -= 50; // Decrease target speed
    }
    lastPrevState = currentPrev;

    if (currentSelect == LOW && lastSelectState == HIGH) {
        targetRPM += 50; // Increase target speed
    }
    lastSelectState = currentSelect;

    targetRPM = constrain(targetRPM, 0, 2000); // Set your motor's max RPM
}

void updateOLED() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Target RPM: ");
    display.println((int)targetRPM);

    display.setCursor(0, 20);
    display.print("Actual RPM:");
    display.setTextSize(2);
    display.setCursor(0, 35);
    display.print((int)currentRPM);

    display.setTextSize(1);
    display.setCursor(80, 50);
    display.print("PWM:");
    display.print((int)outputPWM);
    
    display.display();
}

void loop() {
    // 1. Read Target (Buttons)
    readButtons();

    // 2. Compute PI every loop iteration
    unsigned long currentTime = millis();
    float dt = (currentTime - lastTime) / 1000.0; // Delta time in seconds
    
    if (dt > 0) {
        // Get actual speed
        if (encoder.detectMagnet()) {
            currentRPM = encoder.getAngularSpeed(AS5600_MODE_RPM);
        } else {
            currentRPM = 0;
        }

        // Calculate Error
        float error = targetRPM - currentRPM;

        // Calculate Integral with Anti-Windup
        integral += error * dt;
        integral = constrain(integral, -150, 150); // Prevent "runaway" power

        // Calculate PI Output
        outputPWM = (Kp * error) + (Ki * integral);
        
        // Command Motor
        BTS7960.drive(outputPWM);

        lastTime = currentTime;
    }

    // 3. Update Display (Non-blocking)
    if (millis() - lastDisplayUpdate >= displayInterval) {
        updateOLED();
        lastDisplayUpdate = millis();
    }
}

