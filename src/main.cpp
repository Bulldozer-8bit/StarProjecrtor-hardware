#include <Arduino.h>
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);
    
    // 1. 画圆形的边框，确认是否对齐屏幕物理边缘
    tft.drawCircle(120, 120, 119, TFT_GREEN);
    
    // 2. 画一个十字架，确认物理中心
    tft.drawLine(0, 120, 240, 120, TFT_DARKGREY);
    tft.drawLine(120, 0, 120, 240, TFT_DARKGREY);
    
    tft.setTextColor(TFT_GOLD);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("ZENITH", 120, 100, 2);
}

void loop() {
    // 模拟一颗亮星在圆周运动
    static float angle = 0;
    int x = 120 + 80 * cos(angle);
    int y = 120 + 80 * sin(angle);
    
    tft.fillCircle(x, y, 3, TFT_WHITE);
    delay(20);
    tft.fillCircle(x, y, 3, TFT_BLACK); // 擦除
    angle += 0.05;
}