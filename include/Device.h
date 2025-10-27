#ifndef DEVICE_H
#define DEVICE_H

struct Device {
    int id;
    char title[20];
    char type[10];
    int gpioPin;
    bool status;
};

#endif // DEVICE_H