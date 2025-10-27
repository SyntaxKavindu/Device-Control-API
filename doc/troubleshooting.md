Troubleshooting — ESP32 Device Control Async API
=================================================

Quick checks
------------
- Serial logs: Connect to the ESP32 serial at 115200 baud. The device prints WiFi connection attempts, IP addresses, and mode on boot.
- WiFi status: If in AP mode, ensure your client can see SSID `ESP32_Device_Manager`.

Common issues
-------------
1) AP not visible
- Ensure `wifiMode` is AP (check Serial logs at boot: "Loaded wifiMode: AP").
- Ensure no other code changes modified `ESP32_SSID` or `ESP32_PASSWORD`.

2) Cannot save WiFi credentials
- `/api/wifi/setup` requires a Bearer token. Use `/api/connect` with the setup password to obtain the token.
- Ensure JSON body is valid, e.g. { "ssid": "MyNetwork", "password": "MyPass" }

3) Device doesn't connect after switching to STA
- Confirm you saved credentials beforehand with `/api/wifi/setup`.
- Ensure gateway is reachable; device will attempt DHCP first. If gateway is `192.168.1.1` the device will assign itself `192.168.1.200` (per code). If your router blocks static IPs or uses a different subnet, adjust logic accordingly.
- Check Serial output — it prints DHCP IP, Gateway and subsequent IP attempts.

4) IP conflict or unreachable static IP
- The firmware attempts to pick a stable IP derived from MAC. If another device already uses the same static IP, you may lose connectivity. In that case:
  - Connect to the router and change the reserved IP for the device by MAC.
  - Or change firmware to pick a different static IP or fall back to DHCP.

5) Token/authentication errors
- Token is checked by `checkAuthorization()` helper. If `Token missing` or `Invalid token` responses are seen, verify Authorization header format: `Authorization: Bearer <token>`.

Debugging tips
--------------
- Add Serial.print() logs for the values of SSID/password loaded from Preferences (mask password when printing in public logs).
- Print the `WiFi.gatewayIP()`, `WiFi.localIP()` and `WiFi.subnetMask()` values to verify the network's addressing.
- If you change IP allocation logic, test on a separate network to avoid conflicts.

Next steps to improve robustness
-------------------------------
- Implement ARP probing to detect duplicate IPs before applying static IP
- Add DNS or mDNS so the device can be found by name instead of IP
- Encrypt stored credentials or use the secure element/storage if available
- Replace the single fixed token with a per-device generated token or an OAuth-like flow
