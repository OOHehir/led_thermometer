# led_thermometer

Visualizes live weather temperature on a 50-LED WS2812 strip using the OpenWeatherMap forecast API — each LED maps to 1°C across a -15°C to +35°C range.

## Key Technologies

- **MCU:** ESP32 (WiFi)
- **LED strip:** WS2812 (50 LEDs, GPIO10, RMT peripheral)
- **Data source:** OpenWeatherMap 5-day forecast API
- **Stack:** ESP-IDF (CMake), custom `light_driver` and `http_get` components
- **Protocol:** MQTT (optional remote monitoring)

## How It Works

Three temperature values are shown simultaneously — min (dim), current (bright), max (dim) — as lit positions on the strip. An animated blink sequence cycles every 60 seconds when new forecast data arrives.

## Getting Started

**Prerequisites:** ESP-IDF, OpenWeatherMap API key, 50-LED WS2812 strip on GPIO10

```bash
idf.py menuconfig   # set WiFi SSID/PSK, OWM API key, city
idf.py build flash monitor
```

---

Built by Owen O'Hehir — embedded Linux, IoT, Matter & Rust consulting at [electronicsconsult.com](https://electronicsconsult.com). Available for contract and consulting work.
