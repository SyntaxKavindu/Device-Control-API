Architecture — ESP32 Device Control Async API
=============================================

Overview
--------
This firmware runs on an ESP32 and provides:
- An asynchronous HTTP server using `ESPAsyncWebServer` to expose JSON APIs
- Persistent configuration using the ESP32 `Preferences` API
- Device management in memory via `DeviceManager` and `Device` structures
- Simple token-based authentication for protected endpoints

Main components
---------------
- main.cpp
  - Initializes WiFi and the AsyncWebServer
  - Registers endpoints and request handlers
  - Contains connection logic for AP/STA modes
  - Stores and loads WiFi credentials and mode from `Preferences`

- Device.h / DeviceManager.h
  - Define the `Device` struct and `DeviceManager` container
  - DeviceManager exposes `addDevice`, `getAllDevices`, `updateDeviceStatus`

- Libraries used
  - `ESPAsyncWebServer` and `AsyncTCP` — non-blocking HTTP server
  - `ArduinoJson` — JSON (de)serialization
  - `Preferences` — key/value persistent storage for ESP32
  - `WiFi` — network control

WiFi mode lifecycle
-------------------
- On boot, `loadWiFiMode()` is called to set `wifiMode` (AP or STA).
- `connectToWiFi()` respects `wifiMode`:
  - AP: calls `setupAPMode()` which configures an AP IP (192.168.10.1) and starts softAP
  - STA: loads saved SSID/password from Preferences and attempts to connect via DHCP to discover the gateway and subnet. If gateway is 192.168.1.1, the device will set a static IP 192.168.1.200; otherwise a MAC-derived static IP is computed and applied.
- Mode change via `/api/wifi/mode` stores the requested mode and requires a manual restart to take effect.

Networking decisions
-------------------
- AP uses 192.168.10.1 to be consistent with typical mobile AP subnets (user-specified requirement).
- STA static IP selection:
  - Router gateway 192.168.1.1 -> device uses 192.168.1.200
  - Else -> device computes a MAC-derived address in a safe host range
- The firmware attempts to reduce IP conflicts but does not perform ARP probing by default.

Security
--------
- The current authentication mechanism is intentionally simple: a single `AUTH_TOKEN` and an `AUTH_PASSWORD`. Protect these values in production or replace with a robust authentication flow (JWT, OAuth2, or a per-device generated token).
- Credentials are stored in Preferences (not encrypted). For sensitive deployments, encrypt or protect the storage.

Extensibility
-------------
- You can add new endpoints by registering handlers on `server.on(...)`.
- Replace the static token with a dynamic token provider or integrate with a cloud service.
- Add ARP-based conflict detection before applying static IPs.
