#include "DeviceManager.h"
#include <Arduino.h>
#include <string.h>

bool DeviceManager::addDevice(int id, const char* title, const char* type, int gpioPin, bool status) {
    // Check if device with same ID exists
    for (const Device& device : devices) {
        if (device.id == id) return false;
    }

    Device newDevice;
    newDevice.id = id;
    strncpy(newDevice.title, title, sizeof(newDevice.title) - 1);
    strncpy(newDevice.type, type, sizeof(newDevice.type) - 1);
    newDevice.gpioPin = gpioPin;
    newDevice.status = status;

    // Initialize GPIO
    pinMode(gpioPin, OUTPUT);
    digitalWrite(gpioPin, status);

    devices.push_back(newDevice);
    return true;
}

bool DeviceManager::updateDeviceStatus(int id, bool status) {
    for (Device& device : devices) {
        if (device.id == id) {
            device.status = status;
            digitalWrite(device.gpioPin, status);
            return true;
        }
    }
    return false;
}

bool DeviceManager::deleteDevice(int id) {
    for (auto it = devices.begin(); it != devices.end(); ++it) {
        if (it->id == id) {
            devices.erase(it);
            return true;
        }
    }
    return false;
}

Device* DeviceManager::getDevice(int id) {
    for (Device& device : devices) {
        if (device.id == id) {
            return &device;
        }
    }
    return nullptr;
}

const std::vector<Device>& DeviceManager::getAllDevices() const {
    return devices;
}