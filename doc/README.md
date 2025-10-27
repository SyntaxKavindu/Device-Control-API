ESP32 Device Control Async API
================================

This documentation describes the ESP32 Device Control Async API project.
The firmware exposes an HTTP JSON API backed by an asynchronous web server (`ESPAsyncWebServer`) and stores settings using the ESP32 `Preferences` API.

Contents
- doc/README.md (this file)
- doc/API.md — API endpoints, payloads and responses
- doc/architecture.md — system architecture, components and data flows
- doc/setup.md — how to build, flash and set up the device
- doc/troubleshooting.md — common issues, logs and debugging tips

High level summary
- WiFi modes: AP and STA
  - AP default network: 192.168.10.1 (device acts as hotspot)
  - STA behavior: if router gateway is 192.168.1.1, the device assigns static IP 192.168.1.200; otherwise a MAC-derived static host is used
- Persistent storage: `Preferences` stores SSID/password and the selected WiFi mode
- Authentication: a simple token-based Bearer authentication is implemented for protected endpoints
- Web server: `ESPAsyncWebServer` handles endpoints concurrently and supports request body handlers

Read the API reference next: `doc/API.md`.
