#include <Arduino.h>
#include <TFT_eSPI.h>
#include <math.h>
#include "stars_data.h"

TFT_eSPI tft = TFT_eSPI();

// --- 对应 Python 脚本的配置参数 ---
const float LATITUDE = 40.0;    // 北京海淀纬度
const float CENTER_RA = 163.5;  // 此时天顶赤经
const float MAG_LIMIT = 5.0;    // 亮于 5.0 等的星
const float F_SCALE = 100.0;    // 缩放因子 (针对 240 像素屏幕微调)
const float BRIGHTNESS = 0.6;   // 软件亮度 (0.0-1.0)

// --- 星表结构 ---
typedef struct {
    uint16_t ra_raw;
    uint16_t dec_raw;
    int16_t  mag_raw;
} __attribute__((packed)) Star;

void drawStarfield() {
    tft.fillScreen(TFT_BLACK);
    
    // 预计算常数（弧度转换）
    float phi1 = LATITUDE * DEG_TO_RAD;
    float lambda0 = CENTER_RA * DEG_TO_RAD;

    int total_stars = stars_bin_len / sizeof(Star);
    Star* stars = (Star*)stars_bin;

    for (int i = 0; i < total_stars; i++) {
        // 1. 还原物理量
        float ra = stars[i].ra_raw * 360.0 / 65535.0;
        float dec = (stars[i].dec_raw * 180.0 / 65535.0) - 90.0;
        float vmag = stars[i].mag_raw / 100.0;

        if (vmag > MAG_LIMIT) continue;

        // 2. 核心算法：心射投影 (Gnomonic Projection)
        float lambda_s = ra * DEG_TO_RAD;
        float phi_s = dec * DEG_TO_RAD;

        float cos_c = (sin(phi1) * sin(phi_s) + 
                       cos(phi1) * cos(phi_s) * cos(lambda_s - lambda0));
        
        if (cos_c <= 0) continue; // 过滤掉地平线下或视角外的

        float k = F_SCALE / cos_c;

        // 3. 计算投影坐标
        // 注意：这里的 - 号是为了对齐 Python 脚本里的镜像修正（抬头看天模式）
        float x_offset = -k * cos(phi_s) * sin(lambda_s - lambda0);
        float y_offset = k * (cos(phi1) * sin(phi_s) - sin(phi1) * cos(phi_s) * cos(lambda_s - lambda0));

        int screen_x = 120 + (int)x_offset;
        int screen_y = 120 - (int)y_offset;

        // 4. 边界检查（针对 1.28 寸圆屏裁剪）
        int dx = screen_x - 120;
        int dy = screen_y - 120;
        if ((dx*dx + dy*dy) < 118*118) {
            // 5. 颜色与亮度处理
            float alpha = (MAG_LIMIT - vmag) / (MAG_LIMIT + 1.5);
            alpha = constrain(alpha, 0.2, 1.0) * BRIGHTNESS;
            uint8_t c = (uint8_t)(255 * alpha);
            uint16_t color = tft.color565(c, c, c);

            if (vmag < 2.5) {
                tft.fillCircle(screen_x, screen_y, 1, color); // 亮星
            } else {
                tft.drawPixel(screen_x, screen_y, color);     // 暗星
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    tft.init();
    tft.setRotation(0);
    
    Serial.println("正在生成和 Python 一模一样的星空...");
    drawStarfield();
    Serial.println("渲染完毕！");
}

void loop() {}