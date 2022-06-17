//
// Created by dsporykhin on 13.06.22.
//

#ifndef BASE_ESP8266_MQTT_DOORSCONTROLLER_H
#define BASE_ESP8266_MQTT_DOORSCONTROLLER_H


#include "ServoController.h"

class DoorsController {
private:
    ServoController *smoke_pipe_control;
    ServoController *oxygen_door_control;
    ServoController *upper_door_control;

    double smoke_pipe_value;
    double oxygen_door_value;
    double upper_door_value;

    bool door_opened;

    ServosHardwareSettings *servos_hardware_settings;

    void applyStatus();
public:
    DoorsController(ServosHardwareSettings *servos_hardware_settings);

    void setSmokePipeValue(double value);
    double getSmokePipeValue();

    void setOxygenDoorValue(double value);
    double getOxygenDoorValue();

    void setUpperDoorValue(double value);
    double getUpperDoorValue();

    void setDoopOpened(bool door_opened);
    bool isDoorOpened();
};


#endif //BASE_ESP8266_MQTT_DOORSCONTROLLER_H
