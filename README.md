# J-Clock

A Jewish calendar clock — an e-ink display that shows the current Hebrew date
and upcoming Jewish events, updated automatically via WiFi.

## How it works
1. The **ESP8266** connects to WiFi and fetches live data from the **Hebcal API**
2. The Hebrew date and upcoming events (Shabbat, Yom Tov, etc.) are parsed from the response
3. The data is rendered on a **2.9" e-ink display**
4. E-ink only redraws when the content changes, keeping power consumption low

## Components
| Part | Description |
|------|-------------|
| ESP8266 | WiFi-enabled microcontroller |
| 2.9" e-ink display | Low power, high contrast screen |
| Hebcal API | Provides Hebrew dates and Jewish calendar events |

## Features
- Displays the current Hebrew date
- Shows upcoming Jewish events and Zmanim
- E-ink display — readable in all lighting conditions, low power draw
- Automatically updates via WiFi — no manual input needed

## What I learned
- Making REST API calls from an embedded microcontroller
- Parsing JSON responses on a memory-constrained device
- Driving e-ink displays
- Designing a project around a real personal use case

## Built with
- C++ / Arduino IDE
- Hebcal REST API

## Photos

| Front — ESP32 dev module | Back | Finished |
|:---:|:---:|:---:|
| ![Front](https://user-images.githubusercontent.com/80205172/116128909-5f239b00-a6c1-11eb-8411-056e9508ecfb.jpg) | ![Back](https://user-images.githubusercontent.com/80205172/116128903-5d59d780-a6c1-11eb-996d-676ff286f517.jpg) | ![Finished](https://user-images.githubusercontent.com/80205172/116128890-5a5ee700-a6c1-11eb-9789-2d153f4571f8.jpg) |
