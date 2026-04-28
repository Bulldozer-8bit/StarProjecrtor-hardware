#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include "time.h"
#include "stars_data.h"

// --- 用户配置 ---
const char* ssid     = "MiMesh_NEWNEW";
const char* password = "newplusnew";

const float TIME_ACCELERATION = 400.0; // 速度倍率
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft); 

const float LATITUDE = 40.0; 
const float F_SCALE = 180.0;
const float MAG_LIMIT = 4.0; // 恢复到 5.0 等星，增加细节

float base_LST = 163.5; 
float offset_LST = 0;   

struct Meteor {
    float x, y;
    float vx, vy;
    int life;
    bool active = false;
};
Meteor meteor;

typedef struct {
    uint16_t ra_raw;
    uint16_t dec_raw;
    int16_t  mag_raw;
} __attribute__((packed)) Star;

void triggerMeteor() {
    if (!meteor.active && random(0, 500) < 2) { 
        meteor.active = true;
        meteor.x = random(20, 220);
        meteor.y = random(20, 220);
        float angle = random(0, 360) * DEG_TO_RAD;
        float speed = random(5, 15);
        meteor.vx = cos(angle) * speed;
        meteor.vy = sin(angle) * speed;
        meteor.life = 12; 
    }
}

void drawStarfield() {
    img.fillSprite(TFT_BLACK); 
    
    float phi1 = LATITUDE * DEG_TO_RAD;
    float lambda0 = (base_LST + offset_LST) * DEG_TO_RAD;

    int total_stars = stars_bin_len / sizeof(Star);
    Star* stars = (Star*)stars_bin;

    for (int i = 0; i < total_stars; i++) {
        float ra = stars[i].ra_raw * 360.0 / 65535.0;
        float dec = (stars[i].dec_raw * 180.0 / 65535.0) - 90.0;
        float vmag = stars[i].mag_raw / 100.0;
        if (vmag > MAG_LIMIT) continue;

        float lambda_s = ra * DEG_TO_RAD;
        float phi_s = dec * DEG_TO_RAD;
        float cos_c = (sin(phi1) * sin(phi_s) + cos(phi1) * cos(phi_s) * cos(lambda_s - lambda0));
        
        if (cos_c <= 0.1) continue;

        float k = F_SCALE / cos_c;
        float x_offset = -k * cos(phi_s) * sin(lambda_s - lambda0);
        float y_offset = k * (cos(phi1) * sin(phi_s) - sin(phi1) * cos(phi_s) * cos(lambda_s - lambda0));

        int sx = 120 + (int)x_offset;
        int sy = 120 - (int)y_offset;

        if ((sx-120)*(sx-120) + (sy-120)*(sy-120) < 118*118) {
            
            // --- 层次感亮度算法 ---
            // 1. 使用指数公式计算亮度：亮度 I = I0 * 10^(-0.4 * mag)
            // 这里我们根据星等映射到 0-255 的灰度
            float norm_mag = (MAG_LIMIT - vmag) / (MAG_LIMIT + 1.5);
            int brightness = pow(norm_mag, 1.5) * 255; // 1.5次方增加对比度
            brightness = constrain(brightness, 20, 255); // 基础可见亮度
            
            uint16_t color = tft.color565(brightness, brightness, brightness);

            // 2. 根据亮度决定绘制半径
            if (vmag < 1.2) {
                // 极亮的星（如天狼星、织女星）
                img.fillCircle(sx, sy, 2, color);
                img.drawPixel(sx, sy, TFT_WHITE); // 中心加白
            } else if (vmag < 3.0) {
                // 较亮的星座主星
                img.fillCircle(sx, sy, 1, color);
            } else {
                // 背景微弱星
                img.drawPixel(sx, sy, color);
            }
        }
    }

    // 绘制带尾迹的流星
    if (meteor.active) {
        for(int j=0; j<3; j++) { // 简单的残影效果
            int tail_b = 255 - (j * 80);
            img.drawLine(meteor.x - meteor.vx*j*0.3, meteor.y - meteor.vy*j*0.3, 
                         meteor.x + meteor.vx - meteor.vx*j*0.3, meteor.y + meteor.vy - meteor.vy*j*0.3, 
                         tft.color565(tail_b, tail_b, tail_b));
        }
        meteor.x += meteor.vx;
        meteor.y += meteor.vy;
        meteor.life--;
        if (meteor.life <= 0) meteor.active = false;
    }

    img.pushSprite(0, 0); 
}

void setup() {
    tft.init();
    tft.setRotation(0);
    img.createSprite(240, 240);
    // 强制关闭屏幕反色，确保背景纯黑
    tft.invertDisplay(true);

    WiFi.begin(ssid, password);
    unsigned long start_wifi = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start_wifi < 8000) { delay(500); }
    
    if(WiFi.status() == WL_CONNECTED) {
        configTime(28800, 0, "pool.ntp.org");
        struct tm timeinfo;
        if(getLocalTime(&timeinfo)){
            float hrs = timeinfo.tm_hour + timeinfo.tm_min/60.0 + timeinfo.tm_sec/3600.0;
            base_LST = fmod(163.5 + (hrs - 20.0) * 15.041, 360.0);
        }
    }
}

void loop() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 60) { // 约 25 FPS
        float deltaTime = (millis() - lastUpdate) / 1000.0;
        offset_LST += 0.00417 * deltaTime * TIME_ACCELERATION;
        triggerMeteor();
        drawStarfield();
        lastUpdate = millis();
    }
}