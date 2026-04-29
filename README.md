# SwitchController

Nintendo Switch HORIPAD emulator for ESP32-S3 over USB-OTG. It spoofs a Pokken Tournament Pro Pad (VID 0x0F0D / PID 0x0092) so the Switch treats the ESP32-S3 as a wired USB controller.

## Hardware and ports
- ESP32-S3 dev boards usually expose two USB ports: a native USB/OTG port (often labeled "USB" or "OTG") and a USB-to-UART port for programming/Serial.
- Plug the USB/OTG port into the Switch (or the Switch dock). The USB-to-UART/programming port will not work for the Switch.
- For the SerialBridge example, connect the PC to the USB-to-UART port and the Switch to the USB/OTG port.

## Arduino setup
- Board: "ESP32S3 Dev Module"
- USB Mode: "USB-OTG (TinyUSB)"
- USB CDC On Boot: "Disabled"
- Switch: System Settings -> Controllers and Sensors -> Pro Controller Wired Communication = On

## Basic usage
```cpp
#include <SwitchController.h>

SwitchController ctrl;

void setup() {
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
}

void loop() {
  ctrl.tap(BTN_A, 50);
  delay(1000);
}
```

## Examples
- DpadButton: presses `BTN_A` every 4 seconds.
- SerialBridge: PC sends Serial commands; ESP32-S3 mirrors them to the Switch.

## SerialBridge commands
Commands are case-insensitive and line-based (end with newline).

```
HELP
NEUTRAL
TAP A 50
PRESS A
RELEASE A
HAT UP
HAT DOWN_RIGHT
LS 0 255
RS 128 128
```

Button tokens: A, B, X, Y, L, R, ZL, ZR, PLUS, MINUS, HOME, CAPTURE, LCLICK, RCLICK

Hat tokens: UP, UP_RIGHT, RIGHT, DOWN_RIGHT, DOWN, DOWN_LEFT, LEFT, UP_LEFT, NEUTRAL
