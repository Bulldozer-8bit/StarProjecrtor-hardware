# Cynthia's Zenith: A real-time indoor starry sky projector
Key Features
Scientific Accuracy: Real-time Gnomonic projection of stars up to magnitude 5.0.

Customable Location: Automatic synchronization with local time via NTP to reflect the exact sky above you (currently optimized for Beijing/Haidian).

Dynamic Events: Integrated meteor generation with realistic fading trails.

Hardware: Custom high-power LED backlight modification for long-distance projection.

Hardware Architecture
The system utilizes a specialized 1.28" circular TFT display. To get better visual effects, the backlight was replaced by a more powerful one:

Computation: ESP32-S3 handles the spherical trigonometry and Sprite-based rendering.

Optics: A point-source 3W/5W LED coupled with a combination of lens (Fresnel/camera lens) to project the LCD image.
