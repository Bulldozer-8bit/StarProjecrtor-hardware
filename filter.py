import struct
import math

# 配置参数
INPUT_FILE = 'hip_main.dat'
OUTPUT_FILE = 'stars.bin'
MAG_LIMIT = 6.0
LATITUDE = 40.0  # 北京海淀维度
ALT_LIMIT = 50.0 # 你的要求：仰角50度以上

def process_stars():
    star_count = 0
    with open(INPUT_FILE, 'r') as f, open(OUTPUT_FILE, 'wb') as out:
        for line in f:
            parts = line.split('|')
            if len(parts) < 14: continue
            
            try:
                # 提取核心数据
                ra_deg = float(parts[8])   # RAdeg
                dec_deg = float(parts[9])  # DEdeg
                vmag = float(parts[5])     # Vmag
                
                # 过滤亮度
                if vmag > MAG_LIMIT: continue
                
                # 空间裁剪逻辑：计算天顶距
                # 当投影仪垂直向上，天顶的赤纬 = 当地纬度 (40N)
                # 两点间的张角公式 (简化版)
                # 我们只保留离天顶 40度 以内的星星 (90-50=40)
                dist = math.degrees(math.acos(
                    math.sin(math.radians(LATITUDE)) * math.sin(math.radians(dec_deg)) +
                    math.cos(math.radians(LATITUDE)) * math.cos(math.radians(dec_deg)) *
                    math.cos(math.radians(ra_deg - ra_deg)) # 这里假设经度对齐，实际旋转由ESP32处理
                ))
                
                # 只有离天顶足够近的才保留
                # 注意：这里我们保留一个赤纬带，旋转逻辑交给ESP32实时计算
                if abs(dec_deg - LATITUDE) > (90 - ALT_LIMIT): continue

                # 映射为二进制格式
                # RA: 0-360 -> 0-65535
                ra_raw = int((ra_deg % 360) * 65535 / 360)
                # Dec: -90-90 -> 0-65535
                dec_raw = int((dec_deg + 90) * 65535 / 180)
                # Mag: float -> int16 (mag * 100)
                mag_raw = int(vmag * 100)
                
                # 写入 6 字节: H(unsigned short), H(unsigned short), h(short)
                out.write(struct.pack('<HHh', ra_raw, dec_raw, mag_raw))
                star_count += 1
                
            except (ValueError, IndexError):
                continue
                
    print(f"处理完成！共筛选出 {star_count} 颗星。")
    print(f"文件已保存为: {OUTPUT_FILE}")

if __name__ == "__main__":
    process_stars()
