/*
 * SerialBridge - control the Switch from a PC over Serial.
 *
 * PC <-> USB-to-UART port, Switch <-> USB/OTG port.
 * Commands are line-based and case-insensitive.
 */

#include <SwitchController.h>
#include <ctype.h>

SwitchController ctrl;

static const uint32_t SERIAL_BAUD = 115200;
static const size_t LINE_BUF = 96;
static char lineBuf[LINE_BUF];

static void toUpperInPlace(char *s) {
    while (*s) {
        *s = (char)toupper((unsigned char)*s);
        ++s;
    }
}

static bool readLine(char *out, size_t maxLen) {
    static size_t idx = 0;
    while (Serial.available() > 0) {
        char c = (char)Serial.read();
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            out[idx] = '\0';
            idx = 0;
            return true;
        }
        if (idx < maxLen - 1) {
            out[idx++] = c;
        }
    }
    return false;
}

static uint16_t buttonFromToken(const char *tok) {
    if (!tok) {
        return 0;
    }
    if (strcmp(tok, "A") == 0) return BTN_A;
    if (strcmp(tok, "B") == 0) return BTN_B;
    if (strcmp(tok, "X") == 0) return BTN_X;
    if (strcmp(tok, "Y") == 0) return BTN_Y;
    if (strcmp(tok, "L") == 0) return BTN_L;
    if (strcmp(tok, "R") == 0) return BTN_R;
    if (strcmp(tok, "ZL") == 0) return BTN_ZL;
    if (strcmp(tok, "ZR") == 0) return BTN_ZR;
    if (strcmp(tok, "PLUS") == 0) return BTN_PLUS;
    if (strcmp(tok, "MINUS") == 0) return BTN_MINUS;
    if (strcmp(tok, "HOME") == 0) return BTN_HOME;
    if (strcmp(tok, "CAPTURE") == 0) return BTN_CAPTURE;
    if (strcmp(tok, "LCLICK") == 0) return BTN_LCLICK;
    if (strcmp(tok, "RCLICK") == 0) return BTN_RCLICK;
    return 0;
}

static bool hatFromToken(const char *tok, uint8_t *out) {
    if (!tok || !out) {
        return false;
    }
    if (strcmp(tok, "UP") == 0) { *out = HAT_UP; return true; }
    if (strcmp(tok, "UP_RIGHT") == 0) { *out = HAT_UP_RIGHT; return true; }
    if (strcmp(tok, "RIGHT") == 0) { *out = HAT_RIGHT; return true; }
    if (strcmp(tok, "DOWN_RIGHT") == 0) { *out = HAT_DOWN_RIGHT; return true; }
    if (strcmp(tok, "DOWN") == 0) { *out = HAT_DOWN; return true; }
    if (strcmp(tok, "DOWN_LEFT") == 0) { *out = HAT_DOWN_LEFT; return true; }
    if (strcmp(tok, "LEFT") == 0) { *out = HAT_LEFT; return true; }
    if (strcmp(tok, "UP_LEFT") == 0) { *out = HAT_UP_LEFT; return true; }
    if (strcmp(tok, "NEUTRAL") == 0) { *out = HAT_NEUTRAL; return true; }
    return false;
}

static void printHelp() {
    Serial.println(F("Commands:"));
    Serial.println(F("  HELP"));
    Serial.println(F("  NEUTRAL"));
    Serial.println(F("  TAP <BTN> [ms]"));
    Serial.println(F("  PRESS <BTN>"));
    Serial.println(F("  RELEASE <BTN>"));
    Serial.println(F("  HAT <UP|UP_RIGHT|RIGHT|DOWN_RIGHT|DOWN|DOWN_LEFT|LEFT|UP_LEFT|NEUTRAL>"));
    Serial.println(F("  LS <x> <y>   (0-255)"));
    Serial.println(F("  RS <x> <y>   (0-255)"));
    Serial.println(F("  <BTN>        (shorthand for TAP <BTN> 50)"));
    Serial.println(F("Buttons: A B X Y L R ZL ZR PLUS MINUS HOME CAPTURE LCLICK RCLICK"));
}

static void handleCommand(char *line) {
    char *s = line;
    while (*s == ' ' || *s == '\t') {
        ++s;
    }
    if (*s == '\0') {
        return;
    }

    toUpperInPlace(s);
    char *cmd = strtok(s, " \t");
    if (!cmd) {
        return;
    }

    if (strcmp(cmd, "HELP") == 0) {
        printHelp();
        return;
    }

    if (strcmp(cmd, "NEUTRAL") == 0) {
        ctrl.neutral();
        return;
    }

    if (strcmp(cmd, "TAP") == 0) {
        char *btnTok = strtok(NULL, " \t");
        uint16_t mask = buttonFromToken(btnTok);
        if (!mask) {
            Serial.println(F("ERR: TAP needs a valid button"));
            return;
        }
        char *msTok = strtok(NULL, " \t");
        uint32_t ms = msTok ? (uint32_t)atoi(msTok) : 50;
        ctrl.tap(mask, ms);
        return;
    }

    if (strcmp(cmd, "PRESS") == 0) {
        char *btnTok = strtok(NULL, " \t");
        uint16_t mask = buttonFromToken(btnTok);
        if (!mask) {
            Serial.println(F("ERR: PRESS needs a valid button"));
            return;
        }
        ctrl.press(mask);
        return;
    }

    if (strcmp(cmd, "RELEASE") == 0) {
        char *btnTok = strtok(NULL, " \t");
        uint16_t mask = buttonFromToken(btnTok);
        if (!mask) {
            Serial.println(F("ERR: RELEASE needs a valid button"));
            return;
        }
        ctrl.release(mask);
        return;
    }

    if (strcmp(cmd, "HAT") == 0) {
        char *hatTok = strtok(NULL, " \t");
        uint8_t hat = HAT_NEUTRAL;
        if (!hatFromToken(hatTok, &hat)) {
            Serial.println(F("ERR: HAT needs a valid value"));
            return;
        }
        ctrl.setHat(hat);
        return;
    }

    if (strcmp(cmd, "LS") == 0 || strcmp(cmd, "RS") == 0) {
        char *xTok = strtok(NULL, " \t");
        char *yTok = strtok(NULL, " \t");
        if (!xTok || !yTok) {
            Serial.println(F("ERR: LS/RS needs x and y"));
            return;
        }
        int x = atoi(xTok);
        int y = atoi(yTok);
        x = constrain(x, 0, 255);
        y = constrain(y, 0, 255);
        if (strcmp(cmd, "LS") == 0) {
            ctrl.setLeftStick((uint8_t)x, (uint8_t)y);
        } else {
            ctrl.setRightStick((uint8_t)x, (uint8_t)y);
        }
        return;
    }

    uint16_t mask = buttonFromToken(cmd);
    if (mask) {
        ctrl.tap(mask, 50);
        return;
    }

    Serial.println(F("ERR: unknown command"));
}

void setup() {
    Serial.begin(SERIAL_BAUD);

    USB.VID(0x0F0D);
    USB.PID(0x0092);
    USB.manufacturerName("HORI CO.,LTD.");
    USB.productName("POKKEN CONTROLLER");

    ctrl.begin();
    USB.begin();

    while (!ctrl.ready()) {
        delay(10);
    }
    delay(200);

    printHelp();
}

void loop() {
    if (readLine(lineBuf, LINE_BUF)) {
        handleCommand(lineBuf);
    }
}
