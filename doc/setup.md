Setup and Build — ESP32 Device Control Async API
=================================================

Prerequisites
-------------
- PlatformIO (recommended) or Arduino IDE configured for ESP32
- USB cable to flash the device
- ESP32 board (e.g., ESP32 DevKitC)

Build with PlatformIO
---------------------
1. Open the project folder in VS Code with the PlatformIO extension.
2. Ensure `platformio.ini` is present and configured for an ESP32 board.
3. Build: Use PlatformIO: Build (or `pio run`).
4. Upload: PlatformIO: Upload (or `pio run -t upload`).

Flashing from command line (PlatformIO)
--------------------------------------
# Build and upload
# Run in project root where platformio.ini lives

# Build
pio run

# Upload
pio run -t upload

Initial device setup flow
-------------------------
1. On first boot the device reads the stored WiFi mode; default is AP.
2. If in AP mode, the device creates an access point with SSID `ESP32_Device_Manager` using password `esp32password`.
3. Connect a phone or laptop to this AP and send credentials to `/api/wifi/setup` (POST, JSON) with a Bearer token.

Example PowerShell/curl (Windows PowerShell)
--------------------------------------------
# Request a token using setup password (replace values as needed)
$body = '{"password":"Esp32SecurePass"}'
Invoke-RestMethod -Method Post -Uri http://192.168.10.1/api/connect -Body $body -ContentType 'application/json'

# Save WiFi credentials (requires token returned by /api/connect)
$token = '<TOKEN_FROM_CONNECT>'
$body = '{"ssid":"MyNetwork","password":"MyPass"}'
Invoke-RestMethod -Method Post -Uri http://192.168.10.1/api/wifi/setup -Headers @{ Authorization = "Bearer $token" } -Body $body -ContentType 'application/json'

# Change access mode to STA (requires token)
$body = '{"mode":"STA"}'
Invoke-RestMethod -Method Post -Uri http://192.168.10.1/api/wifi/mode -Headers @{ Authorization = "Bearer $token" } -Body $body -ContentType 'application/json'

# After the device is restarted manually, it will attempt to connect to the saved WiFi network.

Notes
-----
- The device does not restart automatically after `/api/wifi/mode` by design; this gives control to the person configuring the device.
- If you want an automatic restart endpoint, it can be added and protected with the same token mechanism.
