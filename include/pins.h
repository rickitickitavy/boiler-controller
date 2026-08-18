#ifndef BOILERCONTROLLER_PINS_H
#define BOILERCONTROLLER_PINS_H

// Servo doors
#define SMOKE_SERVO_PIN 25
#define OXYGEN_SERVO_PIN 26
#define UPPER_SERVO_PIN 27

// Actuators
#define EMERGENCY_VALVE_PIN 0
#define PUMP_1_PIN 4
#define PUMP_2_PIN 13
#define PUMP_3_PIN 33
#define PUMP_4_PIN 32

// Sensors
#define ONE_WIRE_PIN 14
#define ONE_WIRE_PIN_2 32
#define FLOW_SENSOR_PIN 39
#define MAIN_DOOR_SENSOR_PIN 34

// Display / touch
#define DISPLAY_CS_PIN 17
#define DISPLAY_DC_PIN 15
#define DISPLAY_RST_PIN 16
#define TOUCH_CS 12
#define TOUCH_PEN 35

// Bus / misc
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22
#define EXTERNAL_WDT_PIN 5

#endif
