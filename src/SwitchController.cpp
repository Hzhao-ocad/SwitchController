#include "SwitchController.h"
#include "USBHID.h"
#include <cstring>

// ---- HORIPAD HID report descriptor (86 bytes) ----
// Pokken Tournament Pro Pad descriptor — the Switch accepts this without
// the crypto handshake required by a real Pro Controller.
static const uint8_t horipad_desc[] = {
    0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,        // USAGE (Game Pad)
    0xa1, 0x01,        // COLLECTION (Application)
    0x15, 0x00,        //   LOGICAL_MINIMUM (0)
    0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
    0x35, 0x00,        //   PHYSICAL_MINIMUM (0)
    0x45, 0x01,        //   PHYSICAL_MAXIMUM (1)
    0x75, 0x01,        //   REPORT_SIZE (1)
    0x95, 0x10,        //   REPORT_COUNT (16)
    0x05, 0x09,        //   USAGE_PAGE (Button)
    0x19, 0x01,        //   USAGE_MINIMUM (Button 1)
    0x29, 0x10,        //   USAGE_MAXIMUM (Button 16)
    0x81, 0x02,        //   INPUT (Data,Var,Abs)
    0x05, 0x01,        //   USAGE_PAGE (Generic Desktop)
    0x25, 0x07,        //   LOGICAL_MAXIMUM (7)
    0x46, 0x3b, 0x01,  //   PHYSICAL_MAXIMUM (315)
    0x75, 0x04,        //   REPORT_SIZE (4)
    0x95, 0x01,        //   REPORT_COUNT (1)
    0x65, 0x14,        //   UNIT (English Rotation, Degrees)
    0x09, 0x39,        //   USAGE (Hat Switch)
    0x81, 0x42,        //   INPUT (Data,Var,Abs,Null state)
    0x65, 0x00,        //   UNIT (None)
    0x95, 0x01,        //   REPORT_COUNT (1)
    0x81, 0x01,        //   INPUT (Const,Arr,Abs) -- 4-bit pad
    0x26, 0xff, 0x00,  //   LOGICAL_MAXIMUM (255)
    0x46, 0xff, 0x00,  //   PHYSICAL_MAXIMUM (255)
    0x09, 0x30,        //   USAGE (X)
    0x09, 0x31,        //   USAGE (Y)
    0x09, 0x32,        //   USAGE (Z)
    0x09, 0x35,        //   USAGE (Rz)
    0x75, 0x08,        //   REPORT_SIZE (8)
    0x95, 0x04,        //   REPORT_COUNT (4)
    0x81, 0x02,        //   INPUT (Data,Var,Abs)
    0x06, 0x00, 0xff,  //   USAGE_PAGE (Vendor Defined 0xFF00)
    0x09, 0x20,        //   USAGE (0x20)
    0x95, 0x01,        //   REPORT_COUNT (1)
    0x81, 0x02,        //   INPUT (Data,Var,Abs)
    0x0a, 0x21, 0x26,  //   USAGE (0x2621)
    0x95, 0x08,        //   REPORT_COUNT (8)
    0x91, 0x02,        //   OUTPUT (Data,Var,Abs)
    0xc0               // END_COLLECTION
};

// ---- Constructor ----
SwitchController::SwitchController() : hid() {
    static bool registered = false;
    if (!registered) {
        registered = true;
        USBHID::addDevice(this, sizeof(horipad_desc));
    }
    memset(&report, 0, sizeof(report));
    report.hat = HAT_NEUTRAL;
    report.lx = report.ly = report.rx = report.ry = STICK_NEUTRAL;
}

// ---- Lifecycle ----
void SwitchController::begin() {
    hid.begin();
}

bool SwitchController::ready() {
    return hid.ready();
}

// ---- Send ----
bool SwitchController::send() {
    return hid.SendReport(0, &report, sizeof(report));
}

// ---- Buttons ----
void SwitchController::press(uint16_t mask) {
    report.buttons |= mask;
    send();
}

void SwitchController::release(uint16_t mask) {
    report.buttons &= ~mask;
    send();
}

void SwitchController::setButtons(uint16_t mask) {
    report.buttons = mask;
    send();
}

void SwitchController::tap(uint16_t mask, uint32_t duration_ms) {
    press(mask);
    if (duration_ms) delay(duration_ms);
    release(mask);
}

// ---- D-pad ----
void SwitchController::setHat(uint8_t hat) {
    report.hat = hat;
    send();
}

// ---- Sticks ----
void SwitchController::setLeftStick(uint8_t x, uint8_t y) {
    report.lx = x;
    report.ly = y;
    send();
}

void SwitchController::setRightStick(uint8_t x, uint8_t y) {
    report.rx = x;
    report.ry = y;
    send();
}

// ---- Bulk ----
void SwitchController::neutral() {
    report.buttons = 0;
    report.hat     = HAT_NEUTRAL;
    report.lx = report.ly = report.rx = report.ry = STICK_NEUTRAL;
    report.vendor  = 0;
    send();
}

void SwitchController::setReport(const switch_report_t *r) {
    memcpy(&report, r, sizeof(report));
    send();
}

void SwitchController::getReport(switch_report_t *out) {
    memcpy(out, &report, sizeof(report));
}

// ---- USBHIDDevice override ----
uint16_t SwitchController::_onGetDescriptor(uint8_t *buffer) {
    memcpy(buffer, horipad_desc, sizeof(horipad_desc));
    return sizeof(horipad_desc);
}
