#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "Device.h"
#include "DeviceManager.h"
#include <EEPROM.h>
#include <Preferences.h>

enum Mode {
    AP,
    STA
};

Mode wifiMode = AP;

// Access Point Settings
const char* ESP32_SSID = "ESP32_Device_Manager";
const char* ESP32_PASSWORD = "esp32password";

// WiFi Credentials Storage
Preferences preferences;

// Authentication
const char *AUTH_PASSWORD = "Esp32SecurePass";
const char *AUTH_TOKEN = "eyJzdWIiOiIxMjM0NTY3ODkwIiwibmFtZSI6IkpvaG4gRG9lIiwiaWF0IjoxNTE2MjM5MDIyfQ.SflKxwRJSMeKKF2QT4fwpMeJf36POk6yJV_adQssw5c";

// Function declarations
void connectToWiFi();
void saveWiFiCredentials(const char* ssid, const char* password);
bool loadWiFiCredentials(String &ssid, String &password);
void setupAPMode();
void saveWiFiMode(Mode m);
Mode loadWiFiMode();
bool isValidToken(const char *token);
void handleOptionsRequest(AsyncWebServerRequest *request);
void addCorsHeaders(AsyncWebServerResponse *response);
void handleWiFiSetup(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void handleWiFiMode(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void handleConnect(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void handleGetDevices(AsyncWebServerRequest *request);
void handleControl(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);
void handleValidate(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

// Global instances
AsyncWebServer server(80);
DeviceManager deviceManager;

void setup()
{
    // Initialize Serial for debugging
    Serial.begin(115200);

    // Load saved WiFi mode (AP/STA) from persistent storage
    wifiMode = loadWiFiMode();
    Serial.print("Loaded wifiMode: ");
    Serial.println(wifiMode == AP ? "AP" : "STA");

    // Connect to WiFi (respecting loaded wifiMode)
    connectToWiFi();

    // Initialize sample devices
    deviceManager.addDevice(1, "Main Room Light", "LED", 2, false);
    deviceManager.addDevice(2, "Bedroom Fan", "FAN", 4, true);
    deviceManager.addDevice(3, "Garden Light", "LED", 5, false);
    deviceManager.addDevice(4, "Kitchen Heater", "HEATER", 18, true);
    deviceManager.addDevice(5, "Garage Door", "DOOR", 19, false);
    deviceManager.addDevice(6, "Bathroom Exhaust", "FAN", 21, true);

    // Setup regular endpoints
    server.on("/api/wifi/setup", HTTP_OPTIONS, handleOptionsRequest);
    server.on("/api/wifi/setup", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleWiFiSetup);

    server.on("/api/wifi/mode", HTTP_OPTIONS, handleOptionsRequest);
    server.on("/api/wifi/mode", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleWiFiMode);

    server.on("/api/connect", HTTP_OPTIONS, handleOptionsRequest);
    server.on("/api/connect", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleConnect);

    server.on("/api/devices", HTTP_OPTIONS, handleOptionsRequest);
    server.on("/api/devices", HTTP_GET, handleGetDevices);

    server.on("/api/control", HTTP_OPTIONS, handleOptionsRequest);
    server.on("/api/control", HTTP_PUT, [](AsyncWebServerRequest *request) {}, NULL, handleControl);

    server.on("/api/validate", HTTP_OPTIONS, handleOptionsRequest);
    server.on("/api/validate", HTTP_POST, handleValidateRequest);

    // Setup API endpoints with OPTIONS handling
    server.onNotFound([](AsyncWebServerRequest *request)
    {
        if (request->method() == HTTP_OPTIONS) {
            handleOptionsRequest(request);
        } else {
            DynamicJsonDocument errorResp(200);
            errorResp["message"] = "Not Found";
            errorResp["status"] = false;
            String jsonResponse;
            serializeJson(errorResp, jsonResponse);
            AsyncWebServerResponse *resp = request->beginResponse(404, "application/json", jsonResponse);
            addCorsHeaders(resp);
            request->send(resp);
        }
    });

    // Start server
    server.begin();
}

void loop()
{
    static unsigned long lastWiFiCheck = 0;
    const unsigned long WIFI_CHECK_INTERVAL = 30000; // Check every 30 seconds

    // Check WiFi connection status periodically
    if (millis() - lastWiFiCheck >= WIFI_CHECK_INTERVAL)
    {
        lastWiFiCheck = millis();

        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("WiFi connection lost!");

            // Try to reconnect
            WiFi.disconnect();
            delay(1000);
            connectToWiFi();
        }
    }

    // The async web server handles requests in the background
    delay(100);
}

// Function to save WiFi credentials
void saveWiFiCredentials(const char* ssid, const char* password) {
    preferences.begin("wifi", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);
    preferences.putBool("configured", true);
    preferences.end();
}

// Save WiFi mode (AP or STA) persistently
void saveWiFiMode(Mode m) {
    preferences.begin("wifi", false);
    // store as unsigned int (0 = AP, 1 = STA)
    preferences.putUInt("mode", (unsigned int)m);
    preferences.end();
}

// Load saved WiFi mode; default to AP when not present
Mode loadWiFiMode() {
    preferences.begin("wifi", true);
    unsigned int v = preferences.getUInt("mode", (unsigned int)AP);
    preferences.end();
    return (v == (unsigned int)STA) ? STA : AP;
}

// Function to load WiFi credentials
bool loadWiFiCredentials(String &ssid, String &password) {
    preferences.begin("wifi", true);
    bool configured = preferences.getBool("configured", false);
    if (configured) {
        ssid = preferences.getString("ssid", "");
        password = preferences.getString("password", "");
    }
    preferences.end();
    return configured;
}

// Function to setup AP mode
void setupAPMode() {
    Serial.println("Setting up Access Point...");
    WiFi.mode(WIFI_AP);
    // Use the requested AP network: gateway and AP IP 192.168.10.1
    IPAddress apGateway(192,168,10,1);
    IPAddress apIP = apGateway; // AP IP will be 192.168.10.1

    IPAddress apSubnet(255,255,255,0);
    WiFi.softAPConfig(apIP, apGateway, apSubnet);
    WiFi.softAP(ESP32_SSID, ESP32_PASSWORD);
    Serial.println("Access Point Started");
    Serial.print("AP IP address: ");
    Serial.println(apIP);
}

// Function to handle WiFi connection
void connectToWiFi() {
    if (wifiMode == AP) {
        setupAPMode();
        return;
    }

    String saved_ssid, saved_password;
    if (!loadWiFiCredentials(saved_ssid, saved_password)) {
        Serial.println("No WiFi credentials found");
        return;
    }

    // First try DHCP to learn the gateway/subnet, then set a unique static IP derived from MAC
    Serial.println("Connecting to WiFi (DHCP) to obtain network info...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(saved_ssid.c_str(), saved_password.c_str());

    // Wait for connection for up to 10 seconds
    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        delay(1000);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("\nFailed to connect (DHCP)");
        return;
    }

    // We have a DHCP address; obtain gateway/subnet
    IPAddress gw = WiFi.gatewayIP();
    IPAddress subnet = WiFi.subnetMask();
    Serial.println();
    Serial.print("DHCP IP: "); Serial.println(WiFi.localIP());
    Serial.print("Gateway: "); Serial.println(gw);

    // Derive a stable, unique host address from MAC, but special-case common router
    String mac = WiFi.macAddress();
    uint8_t macLast = 0;
    int idx = mac.lastIndexOf(':');
    if (idx >= 0) {
        String last = mac.substring(idx + 1);
        macLast = (uint8_t)strtoul(last.c_str(), NULL, 16);
    }

    IPAddress staticIP = gw; // copy network prefix
    // Special-case: if gateway is 192.168.1.1 assign 192.168.1.200
    if (gw == IPAddress(192,168,1,1)) {
        staticIP = IPAddress(192,168,1,200);
    } else {
        // compute candidate in the .10 - .250 range based on MAC
        uint8_t candidate = 10 + (macLast % 240);
        if (candidate == gw[3]) candidate = (candidate < 250) ? candidate + 1 : candidate - 1;
        staticIP[3] = candidate;
    }

    // Apply static IP by reconfiguring then reconnecting
    Serial.print("Configuring static IP: "); Serial.println(staticIP);
    WiFi.disconnect();
    delay(200);
    if (!WiFi.config(staticIP, gw, subnet)) {
        Serial.println("Failed to set static IP (WiFi.config returned false)");
    }

    WiFi.begin(saved_ssid.c_str(), saved_password.c_str());
    attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
        delay(1000);
        Serial.print("+");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected with static IP");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFailed to connect with static IP, falling back to DHCP IP");
        // optionally fall back to DHCP — try to reconnect normally
        WiFi.disconnect();
        delay(200);
        WiFi.begin(saved_ssid.c_str(), saved_password.c_str());
    }
}

// Handle OPTIONS requests for CORS
void handleOptionsRequest(AsyncWebServerRequest *request)
{
    AsyncWebServerResponse *response = request->beginResponse(204);
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Max-Age", "86400"); // 24 hours cache for preflight
    request->send(response);
}

// Function to add CORS headers to responses
void addCorsHeaders(AsyncWebServerResponse *response)
{
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
}

// Utility function to validate token
bool isValidToken(const char *token)
{
    return (token != nullptr && strcmp(token, AUTH_TOKEN) == 0);
}

// Helper function to check authorization and send unauthorized response if needed
bool checkAuthorization(AsyncWebServerRequest *request)
{
    if (!request->hasHeader("Authorization"))
    {
        DynamicJsonDocument errorResp(200);
        errorResp["message"] = "Token missing";
        errorResp["status"] = false;

        String jsonResponse;
        serializeJson(errorResp, jsonResponse);
        AsyncWebServerResponse *resp = request->beginResponse(401, "application/json", jsonResponse);
        addCorsHeaders(resp);
        request->send(resp);
        return false;
    }

    const AsyncWebHeader *h = request->getHeader("Authorization");
    String authHeader = h->value();

    // Check for "Bearer " prefix
    if (!authHeader.startsWith("Bearer "))
    {
        DynamicJsonDocument errorResp(200);
        errorResp["message"] = "Invalid authorization format. Use 'Bearer <token>'";
        errorResp["status"] = false;

        String jsonResponse;
        serializeJson(errorResp, jsonResponse);
        AsyncWebServerResponse *resp = request->beginResponse(401, "application/json", jsonResponse);
        addCorsHeaders(resp);
        request->send(resp);
        return false;
    }

    // Extract token (remove "Bearer " prefix)
    String tokenValue = authHeader.substring(7); // "Bearer " is 7 characters
    const char *token = tokenValue.c_str();

    if (!isValidToken(token))
    {
        DynamicJsonDocument errorResp(200);
        errorResp["message"] = "Invalid token";
        errorResp["status"] = false;

        String jsonResponse;
        serializeJson(errorResp, jsonResponse);
        AsyncWebServerResponse *resp = request->beginResponse(401, "application/json", jsonResponse);
        addCorsHeaders(resp);
        request->send(resp);
        return false;
    }

    return true;
}

// Request handlers
void handleConnect(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    // Only process the complete body
    if (index + len != total)
        return;

    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, data);

    DynamicJsonDocument response(200);
    int statusCode = 200;

    if (error)
    {
        response["message"] = "Invalid JSON";
        response["status"] = false;
        statusCode = 400;
    }
    else
    {
        const char *password = doc["password"];
        if (password && strcmp(password, AUTH_PASSWORD) == 0)
        {
            response["message"] = "Authentication successful";
            response["status"] = true;
            JsonObject data = response.createNestedObject("data");
            data["token"] = AUTH_TOKEN;
        }
        else
        {
            response["message"] = "Invalid password";
            response["status"] = false;
            statusCode = 401;
        }
    }

    String jsonResponse;
    serializeJson(response, jsonResponse);
    AsyncWebServerResponse *resp = request->beginResponse(statusCode, "application/json", jsonResponse);
    addCorsHeaders(resp);
    request->send(resp);
}

void handleGetDevices(AsyncWebServerRequest *request)
{
    // Check authorization first
    if (!checkAuthorization(request))
    {
        return;
    }

    // Authorization valid, proceed with getting devices
    DynamicJsonDocument response(1024);
    response["message"] = "Devices retrieved";
    response["status"] = true;

    JsonArray data = response.createNestedArray("data");
    for (const Device &device : deviceManager.getAllDevices())
    {
        JsonObject deviceObj = data.createNestedObject();
        deviceObj["id"] = device.id;
        deviceObj["title"] = device.title;
        deviceObj["type"] = device.type;
        deviceObj["gpioPin"] = device.gpioPin;
        deviceObj["status"] = device.status;
    }

    String jsonResponse;
    serializeJson(response, jsonResponse);
    AsyncWebServerResponse *resp = request->beginResponse(200, "application/json", jsonResponse);
    addCorsHeaders(resp);
    request->send(resp);
}

void handleControl(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    // Only process the complete body
    if (index + len != total)
        return;

    // Check authorization first
    if (!checkAuthorization(request))
    {
        return;
    }

    DynamicJsonDocument response(200);

    // Token is valid, process the request
    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, data);

    int statusCode = 200;
    if (error)
    {
        response["message"] = "Invalid JSON";
        response["status"] = false;
        statusCode = 400;
    }
    else
    {
        int deviceId = doc["device"];
        bool status = doc["status"];

        if (deviceManager.updateDeviceStatus(deviceId, status))
        {
            response["message"] = "Device updated";
            response["status"] = true;
        }
        else
        {
            response["message"] = "Device not found";
            response["status"] = false;
            statusCode = 404;
        }
    }

    String jsonResponse;
    serializeJson(response, jsonResponse);
    AsyncWebServerResponse *resp = request->beginResponse(statusCode, "application/json", jsonResponse);
    addCorsHeaders(resp);
    request->send(resp);
}

void handleValidate(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
    // Check authorization first
    if (!checkAuthorization(request))
    {
        return;
    }

    DynamicJsonDocument response(200);

    // If we got here, the token is valid
    response["message"] = "Token valid";
    response["status"] = true;

    String jsonResponse;
    serializeJson(response, jsonResponse);
    AsyncWebServerResponse *resp = request->beginResponse(200, "application/json", jsonResponse);
    addCorsHeaders(resp);
    request->send(resp);
}

// Request-only handler for /api/validate (handles POST requests without a body)
void handleValidateRequest(AsyncWebServerRequest *request)
{
    // Reuse checkAuthorization which handles missing/invalid tokens and sends error responses
    if (!checkAuthorization(request))
    {
        return;
    }

    DynamicJsonDocument response(200);
    response["message"] = "Token valid";
    response["status"] = true;

    String jsonResponse;
    serializeJson(response, jsonResponse);
    AsyncWebServerResponse *resp = request->beginResponse(200, "application/json", jsonResponse);
    addCorsHeaders(resp);
    request->send(resp);
}

// Handler for WiFi mode changes
void handleWiFiMode(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // Check authorization first
    if (!checkAuthorization(request)) {
        return;
    }

    // Only process the complete body
    if (index + len != total)
        return;

    DynamicJsonDocument doc(200);
    DeserializationError error = deserializeJson(doc, data);

    DynamicJsonDocument response(200);
    int statusCode = 200;

    if (error) {
        response["message"] = "Invalid JSON";
        response["status"] = false;
        statusCode = 400;
    } else {
        const char* mode = doc["mode"];
        
        if (mode) {
            if (strcmp(mode, "AP") == 0 || strcmp(mode, "STA") == 0) {
                wifiMode = (strcmp(mode, "AP") == 0) ? AP : STA;
                // Persist the mode so it survives reboot
                saveWiFiMode(wifiMode);

                response["message"] = String("Mode changed to ") + mode + ". Please restart the device manually to apply this change.";
                response["status"] = true;
                
                // Send response; do NOT restart automatically (user requested)
                String jsonResponse;
                serializeJson(response, jsonResponse);
                AsyncWebServerResponse *resp = request->beginResponse(statusCode, "application/json", jsonResponse);
                addCorsHeaders(resp);
                request->send(resp);
                return;
            } else {
                response["message"] = "Invalid mode. Use 'AP' or 'STA'";
                response["status"] = false;
                statusCode = 400;
            }
        } else {
            response["message"] = "Mode not specified";
            response["status"] = false;
            statusCode = 400;
        }
    }

    String jsonResponse;
    serializeJson(response, jsonResponse);
    AsyncWebServerResponse *resp = request->beginResponse(statusCode, "application/json", jsonResponse);
    addCorsHeaders(resp);
    request->send(resp);
}

// Handler for WiFi setup endpoint
void handleWiFiSetup(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // Require authorization
    if (!checkAuthorization(request)) {
        return;
    }

    // Only process the complete body
    if (index + len != total)
        return;

    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, data);

    DynamicJsonDocument response(200);
    int statusCode = 200;

    if (error) {
        response["message"] = "Invalid JSON";
        response["status"] = false;
        statusCode = 400;
    } else {
        const char* ssid = doc["ssid"];
        const char* password = doc["password"];

        if (ssid && password && strlen(ssid) > 0 && strlen(password) > 0) {
            // Save the credentials only; do NOT attempt to connect here
            saveWiFiCredentials(ssid, password);

            response["message"] = "WiFi credentials saved please change Access mode";
            response["status"] = true;
        } else {
            response["message"] = "Missing SSID or password";
            response["status"] = false;
            statusCode = 400;
        }
    }

    String jsonResponse;
    serializeJson(response, jsonResponse);
    AsyncWebServerResponse *resp = request->beginResponse(statusCode, "application/json", jsonResponse);
    addCorsHeaders(resp);
    request->send(resp);
}