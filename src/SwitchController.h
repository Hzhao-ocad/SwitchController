/*
 * SwitchController — Nintendo Switch HORIPAD emulator for ESP32-S3 (Arduino).
 *
 * Makes an ESP32-S3 appear as a Pokken Tournament Pro Pad (VID 0x0F0D / PID 0x0092)
 * to the Nintendo Switch over USB-OTG. No auth handshake needed.
 *
 * Board settings (Arduino IDE):
 *   Board:               "ESP32S3 Dev Module"
 *   USB CDC On Boot:     "Disabled"
 *   USB Mode:            "USB-OTG (TinyUSB)"
 *
 * Usage:
 *   #include <SwitchController.h>
 *
 *   SwitchController ctrl;
 *
 *   void setup() {
 *       USB.VID(0x0F0D);
 *       USB.PID(0x0092);
 *       USB.manufacturerName("HORI CO.,LTD.");
 *       USB.productName("POKKEN CONTROLLER");
 *
 *       ctrl.begin();
 *       USB.begin();
 *
 *       while (!ctrl.ready()) delay(10);
 *       delay(200);
 *   }
 *
 *   void loop() {
 *       ctrl.tap(BTN_A, 50);
 *       delay(1000);
 *   }
 */

#ifndef SWITCH_CONTROLLER_H
#define SWITCH_CONTROLLER_H

#include "USBHID.h"

// ---- Buttons (bitmask; OR together to combine) ----
#define BTN_Y        (1u << 0)
#define BTN_B        (1u << 1)
#define BTN_A        (1u << 2)
#define BTN_X        (1u << 3)
#define BTN_L        (1u << 4)
#define BTN_R        (1u << 5)
#define BTN_ZL       (1u << 6)
#define BTN_ZR       (1u << 7)
#define BTN_MINUS    (1u << 8)
#define BTN_PLUS     (1u << 9)
#define BTN_LCLICK   (1u << 10)
#define BTN_RCLICK   (1u << 11)
#define BTN_HOME     (1u << 12)
#define BTN_CAPTURE  (1u << 13)

// ---- D-pad hat values ----
#define HAT_UP         0
#define HAT_UP_RIGHT   1
#define HAT_RIGHT      2
#define HAT_DOWN_RIGHT 3
#define HAT_DOWN       4
#define HAT_DOWN_LEFT  5
#define HAT_LEFT       6
#define HAT_UP_LEFT    7
#define HAT_NEUTRAL    8

// ---- Stick axis constants ----
#define STICK_MIN     0x00
#define STICK_NEUTRAL 0x80
#define STICK_MAX     0xFF

// ---- 8-byte HID input report ----
typedef struct __attribute__((packed)) {
    uint16_t buttons;
    uint8_t  hat;
    uint8_t  lx, ly;
    uint8_t  rx, ry;
    uint8_t  vendor;
} switch_report_t;

// ---- Controller class ----
class SwitchController : public USBHIDDevice {
public:
    SwitchController();

    // Lifecycle
    void begin();
    bool ready();

    // Send the current state to the host. Returns false if not ready.
    // Called automatically by all mutators — you rarely need to call this.
    bool send();

    // Buttons
    void press(uint16_t mask);         // OR bits into the current button state
    void release(uint16_t mask);       // clear bits from the current button state
    void setButtons(uint16_t mask);    // overwrite the current button state
    void tap(uint16_t mask, uint32_t duration_ms);  // press, delay, release (blocking)

    // D-pad
    void setHat(uint8_t hat);

    // Analog sticks (0..255, 0x80 = center, Y inverted per Switch convention)
    void setLeftStick(uint8_t x, uint8_t y);
    void setRightStick(uint8_t x, uint8_t y);

    // Reset everything to neutral and send
    void neutral();

    // Raw report access (for scripted playback)
    void setReport(const switch_report_t *r);
    void getReport(switch_report_t *out);

    // USBHIDDevice override
    uint16_t _onGetDescriptor(uint8_t *buffer) override;

private:
    USBHID hid;
    switch_report_t report;
};

#endif // SWITCH_CONTROLLER_H
