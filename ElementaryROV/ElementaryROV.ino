#include <SoftwareSerial.h>
#include "src/Smoothed/Smoothed.h"
#include "src/MSP/MSP.h"

MSP msp;
SoftwareSerial msp_uart(9, 10);

#define MSP_SET_MOTOR 214
#define SERIAL_DEBUG

#define UP 2
#define LEFT 5
#define DOWN 4
#define RIGHT 3
#define L 6
#define R 7
#define X A1
#define Y A0

#define THRESHOLD 25

#define MOTORS_MIN 1100
#define MOTORS_MAX 1900                                                 

#define SMOOTH_COUNT 7

Smoothed<float> axis_l;
Smoothed<float> axis_r;
Smoothed<float> axis_v;

int8_t axis_treshold(int8_t value) {
    if (abs(value) < THRESHOLD) {
        return 0;
    }
    return map(abs(value), THRESHOLD, 100, 0, 100) * (value < 0 ? -1 : 1);
}

int8_t get_up() {
    return digitalRead(UP);
}

int8_t get_down() {
    return digitalRead(DOWN);
}

int8_t get_left() {
    return digitalRead(LEFT);
}

int8_t get_right() {
    return digitalRead(RIGHT);
}

int get_X() {
    int value = axis_treshold(map(analogRead(X), 0, 1023, -100, 100));
    return clamp(value);
}

int get_Y() {
    int value = axis_treshold(map(analogRead(Y), 0, 1023, -100, 100));
    return clamp(value);
}

int8_t get_L() {
    return digitalRead(L);
}

int8_t get_R() {
    return digitalRead(R);
}

int8_t clamp(int v) {
    int8_t divider = get_divider();
    v = constrain(v, (-100 / divider), (100 / divider));
    return v;
}

int8_t get_left_th() {
    return clamp(get_Y() + get_X());
}

int8_t get_right_th() {
    return clamp(get_Y() - get_X());
}

int8_t get_vert_th() {
    int8_t pwr = 0;

    if (get_up()) {
        pwr = -100;
    }

    if (get_down()) {
        pwr = 100;
    }

    return clamp(pwr);
}

int8_t get_divider() {
    float divider = 2;

    // faster
    if (get_right()) {
        divider = 1;
    }

    // slower
    if (get_left()) {
        divider = 4;
    }

    return divider;
}

void setup() {
    Serial.begin(115200);
    Serial.println("Begin...");

    axis_l.begin(SMOOTHED_AVERAGE, SMOOTH_COUNT);
    axis_r.begin(SMOOTHED_AVERAGE, SMOOTH_COUNT);
    axis_v.begin(SMOOTHED_AVERAGE, SMOOTH_COUNT);

    msp_uart.begin(9600);
    msp.begin(msp_uart);

    pinMode(UP, INPUT);
    pinMode(LEFT, INPUT);
    pinMode(RIGHT, INPUT);
    pinMode(DOWN, INPUT);
    pinMode(L, INPUT);
    pinMode(R, INPUT);
    pinMode(X, INPUT);
    pinMode(Y, INPUT);

    pinMode(LED_BUILTIN, OUTPUT);

    for (int i = 0; i < 8; i++) {
        Serial.print("Init ");
        set_motors(0,0,0,0);
        delay(500);
        digitalWrite(LED_BUILTIN, i % 2 == 0 ? HIGH : LOW);
    }
}

inline uint16_t power_to_useconds(int8_t val) {
    return map(val, -100, 100, MOTORS_MIN, MOTORS_MAX);
}

void set_motors(int8_t m1, int8_t m2, int8_t m3, int8_t m4) {
#ifdef SERIAL_DEBUG
    Serial.print("Motors: \t");
    Serial.print(m1); Serial.print("\t");
    Serial.print(m2); Serial.print("\t");
    Serial.print(m3); Serial.print("\t");
    Serial.print(m4); Serial.print("\t");
    Serial.println();

    Serial.print("PWM: \t\t");
    Serial.print(power_to_useconds(m1)); Serial.print("\t");
    Serial.print(power_to_useconds(m2)); Serial.print("\t");
    Serial.print(power_to_useconds(m3)); Serial.print("\t");
    Serial.print(power_to_useconds(m4)); Serial.print("\t");
    Serial.println();

    Serial.print("Axis:\t\t");
    Serial.print(get_X()); Serial.print("\t");
    Serial.print(get_Y()); Serial.print("\t");
    Serial.println();

    Serial.println();
    Serial.println();
#endif

    uint16_t data[8] = {
        power_to_useconds(m1),
        power_to_useconds(m2),
        power_to_useconds(m3),
        power_to_useconds(m4),
        power_to_useconds(0),
        power_to_useconds(0),
        power_to_useconds(0),
        power_to_useconds(0)
    };

    msp.send(MSP_SET_MOTOR, data, 16);
}

void loop() {
    axis_r.add(get_right_th());
    axis_l.add(get_left_th());
    axis_v.add(get_vert_th());

    set_motors(
        static_cast<int8_t>(axis_r.get()),
        static_cast<int8_t>(axis_v.get()),
        static_cast<int8_t>(axis_l.get()),
        static_cast<int8_t>(axis_v.get())
    );

    delay(5);
}
