/*
 * DpadButton — 最简单的 SwitchController 示例
 * 每 4 秒按一次方向键 RIGHT，保持 50ms 后松开。
 *
 * 接线: ESP32-S3 USB-OTG (GPIO 19/20) → Switch 底座 USB
 * Switch 设置: 系统设置 → 手柄与感应器 → Pro手柄的有线连接 = ON
 */

#include <SwitchController.h>

SwitchController ctrl;

void setup() {
    // 必须最先设置，必须在 USB.begin() 之前调用
    USB.VID(0x0F0D);
    USB.PID(0x0092);
    USB.manufacturerName("HORI CO.,LTD.");
    USB.productName("POKKEN CONTROLLER");

    ctrl.begin();
    USB.begin();

    // 等待 Switch 枚举完成
    while (!ctrl.ready()) {
        delay(10);
    }
    delay(200);
}

void loop() {
    // 按下 D-pad RIGHT 50ms
    ctrl.tap(BTN_A, 50);
    delay(4000);
}
