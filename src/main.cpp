#include <Arduino.h>
#include <stdint.h>
#include "stars_data.h" // 包含你生成的星表数组

typedef struct {
    uint16_t ra_raw;
    uint16_t dec_raw;
    int16_t  mag_raw;
} __attribute__((packed)) Star;

void setup() {
    Serial.begin(115200);
    // 等待串口
    unsigned long timeout = millis();
    while (!Serial && (millis() - timeout < 5000));

    Serial.println("\n--- 开始解析真实星表数据 ---");

    // stars_bin 和 stars_bin_len 是 xxd 自动生成的变量名
    int total_stars = stars_bin_len / sizeof(Star);
    Serial.printf("检测到星点总数: %d\n", total_stars);

    Star* stars = (Star*)stars_bin;

    for (int i = 0; i < total_stars; i++) {
        // 还原物理量
        float ra = stars[i].ra_raw * 360.0 / 65535.0;
        float dec = (stars[i].dec_raw * 180.0 / 65535.0) - 90.0;
        float mag = stars[i].mag_raw / 100.0;

        // 每隔 100 颗打印一颗，防止串口刷屏太快
        if (i % 100 == 0) {
            Serial.printf("索引[%d] -> RA: %.2f, Dec: %.2f, Mag: %.2f\n", i, ra, dec, mag);
        }
    }
    Serial.println("--- 解析完成 ---");
}

void loop() {
    // 保持空运行
}