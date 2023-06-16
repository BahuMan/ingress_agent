#ifndef INGRESS_H
#define INGRESS_H
#include <stdint.h>

#define CONTEST_MILLIS 5000

#define CAMP_NONE  0b00000000
#define CAMP_RED   0b11000000
#define CAMP_GREEN 0b00111000
#define CAMP_BLUE  0b00000111

class Ingress {
  public:
    void push(uint8_t color);
    bool isContested();
    uint8_t getCamp();
    static char* toCampName(uint8_t color);
  private:
    bool contested = true;
    uint32_t lastPushTime = 0;
    uint8_t color = CAMP_NONE;
};

#endif
