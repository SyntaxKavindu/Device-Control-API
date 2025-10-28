API Reference — ESP32 Device Control Async API
===============================================

Authentication
--------------
- The API uses a Bearer token in the `Authorization` header for protected endpoints.
- Example header: `Authorization: Bearer <token>`
- The firmware holds a single token (`AUTH_TOKEN`) and one setup password (`AUTH_PASSWORD`) used by `/api/connect` endpoint to request the token.

Common response shape
---------------------
Most responses are JSON objects containing at least:
- `message` (string) — a human-readable description
- `status` (boolean) — true for success, false for error
- `data` (object|array) — optional returned data

Endpoints
---------
1) POST /api/connect
- Purpose: exchange a setup password for a token
- Request body: { "password": "<setup-password>" }
- Response success: { "message":"Authentication successful","status":true, "data": {"token": "<AUTH_TOKEN>"} }
- Response error: 401 or 400 with `message` describing error

2) POST /api/wifi/setup
- Purpose: Save WiFi SSID and password into persistent storage
- Authentication: Requires `Authorization: Bearer <token>`
- Request body: { "ssid": "MyNetwork", "password": "MyPass" }
- Behavior: Saves credentials only; does NOT attempt to connect. Returns success message instructing user to change mode and restart.
- Success response: { "message": "WiFi credentials saved please change Access mode", "status": true }

3) POST /api/wifi/mode
- Purpose: Change access mode (AP/STA)
- Authentication: Requires `Authorization: Bearer <token>`
- Request body: { "mode": "AP" } or { "mode": "STA" }
- Behavior: Persists chosen mode to preferences. Does NOT restart automatically. Client should restart device to apply the new mode.
- Success response: { "message": "Mode changed to STA. Please restart the device manually to apply this change.", "status": true }

4) GET /api/devices
- Purpose: Retrieve registered devices and statuses
- Authentication: Requires `Authorization: Bearer <token>`
- Response data: array of device objects: { id, title, type, gpioPin, status }

5) PUT /api/control
- Purpose: Change device status (on/off)
- Authentication: Requires `Authorization: Bearer <token>`
- Request body: { "device": <id>, "status": true }
- Response: success or 404 if device not found

6) POST /api/validate
- Purpose: Validate the token (request has no body)
- Authentication: Requires `Authorization: Bearer <token>`
- Request: No JSON body is required or expected. The endpoint only checks the `Authorization` header.
- Implementation note: The firmware registers a request-only handler for this route (no body upload handler). Using a body-upload style handler for this endpoint may trigger "Handler did not handle the request" errors.
- Response: { "message": "Token valid", "status": true }

CORS
----
- The server responds to OPTIONS preflight requests and adds CORS headers to JSON responses:
  - Access-Control-Allow-Origin: *
  - Access-Control-Allow-Headers: Content-Type, Authorization
  - Access-Control-Allow-Methods: GET, POST, PUT, OPTIONS

Notes
-----
- The token is a fixed value in this firmware. Replace this with a secure mechanism for production.
- Sensitive fields are stored in `Preferences` but are not encrypted. Consider using secure storage if required.
