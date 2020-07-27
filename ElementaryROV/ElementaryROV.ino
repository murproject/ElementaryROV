
#define UP 2
#define LEFT 5
#define DOWN 4
#define RIGHT 3
#define L 6
#define R 7
#define X A1
#define Y A0



int8_t axis_trashold(int8_t value) {
    if (abs(value) < 10) {
        return 0;
    }
    return value;
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
    return axis_trashold(map(analogRead(X), 0, 1024, -100, 100));
}

int get_Y() {
    return axis_trashold(map(analogRead(Y), 0, 1024, -100, 100));
}
int8_t get_L() {

    return digitalRead(L);
}

int8_t get_R() {

    return digitalRead(R);
}


int8_t clamp(int v) {
    v = constrain(v, -100, 100);
    return v;
}

int8_t get_left_th() {
    return clamp(get_Y() + get_X());
}

int8_t get_right_th() {
    return clamp(get_Y() - get_X());
}

int8_t get_vert_th_1() {
    int8_t pwr = 0;
    float del = 2;
    if (get_up()) {
        pwr = 100;
    }
    if (get_down()) {
        pwr = -100;
    }
    if (get_left()) {
        del = 1;
    }
    if (get_right()) {
        del = 3;
    }
    return pwr / del;
}
int8_t get_vert_th_2() {
    int8_t pwr = 0;
    float del = 2;
    if (get_up()) {
        pwr = 100;
    }
    if (get_down()) {
        pwr = -100;
    }
    if (get_left()) {
        del = 1;
    }
    if (get_right()) {
        del = 3;
    }
    return pwr / del;
}

int8_t get_add() {
    int8_t pwr = 0;
    float del = 2;
    if (get_L()) {
        pwr = 100;
    }
    if (get_R()) {
        pwr = -100;
    }
    if (get_left()) {
        del = 1;
    }
    if (get_right()) {
        del = 3;
    }
    return pwr / del;
}

void check() {
    get_X();
    get_Y();
    get_left();
    get_right();
    get_up();
    get_down();
    get_L();
    get_R();
}
#include <SoftwareSerial.h>

SoftwareSerial owi(9, 10);
void setup() {
    Serial.begin(115200);
    owi.begin(1200);
    pinMode(UP, INPUT);
    pinMode(LEFT, INPUT);
    pinMode(RIGHT, INPUT);
    pinMode(DOWN, INPUT);
    pinMode(L, INPUT);
    pinMode(R, INPUT);
    pinMode(X, INPUT);
    pinMode(Y, INPUT);
}

unsigned char Crc8(uint8_t *pcBlock, unsigned int len) {
    unsigned char crc = 0xFF;
    unsigned int i;

    while (len--)
    {
        crc ^= *pcBlock++;

        for (i = 0; i < 8; i++)
            crc = crc & 0x80 ? (crc << 1) ^ 0x31 : crc << 1;
    }

    return crc;
}

void loop() {

    uint8_t data[] = {
        0xAA,
        0xEE,
        get_left_th(),
        get_vert_th_1(),
        get_right_th(),
        get_vert_th_2(),
        get_add(),
        0,
        0xEF
    };
    data[7] = Crc8(data + 2, 5);
    owi.write(data, 9);
   
    for (int i = 0; i < 9; i++) {
        Serial.print((int)data[i]);
        Serial.print(' ');

    }
    Serial.println();
    delay(50);
}
