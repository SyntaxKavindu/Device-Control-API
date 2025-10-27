#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <vector>
#include "Device.h"

class DeviceManager {
private:
    std::vector<Device> devices;

public:
    bool addDevice(int id, const char* title, const char* type, int gpioPin, bool status);
    bool updateDeviceStatus(int id, bool status);
    bool deleteDevice(int id);
    Device* getDevice(int id);
    const std::vector<Device>& getAllDevices() const;
};

#endif // DEVICEMANAGER_H