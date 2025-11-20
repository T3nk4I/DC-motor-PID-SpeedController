#include <Arduino.h>
#include <util/atomic.h>

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






void setup() {
  BTS7960.init();
}

void loop() {

}

