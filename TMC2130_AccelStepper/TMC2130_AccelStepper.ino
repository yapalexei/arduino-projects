/**
 * Author Teemu Mäntykallio
 * Initializes the library and turns the motor in alternating directions.
*/

#define EN_PIN    21  // Nano v3:   16 Mega:    38  //enable (CFG6)
#define DIR_PIN   2  //            19          55  //direction
#define STEP_PIN  4  //            18          54  //step
#define CS_PIN    5  //            17          40  //chip select

constexpr uint32_t steps_per_mm = 80;

#include <TMC2130Stepper.h>
TMC2130Stepper driver = TMC2130Stepper(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN);

#include <AccelStepper.h>
AccelStepper stepper = AccelStepper(stepper.DRIVER, STEP_PIN, DIR_PIN, 6,7,true);

void setup() {
    SPI.begin();
    Serial.begin(9600);
    while(!Serial);
    Serial.println("Start...");
    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    driver.begin();             // Initiate pins and registeries
    driver.rms_current(600);    // Set stepper current to 600mA. The command is the same as command TMC2130.setCurrent(600, 0.11, 0.5);
    driver.stealthChop(1);      // Enable extremely quiet stepping
    driver.stealth_autoscale(1);
    driver.microsteps(16);

    stepper.setMaxSpeed(10*steps_per_mm); // 100mm/s @ 80 steps/mm
    stepper.setAcceleration(50*steps_per_mm); // 2000mm/s^2
    stepper.setEnablePin(EN_PIN);
    stepper.setPinsInverted(false, false, false);
    stepper.enableOutputs();
}

void loop() {
    if (stepper.distanceToGo() == 0) {
        stepper.disableOutputs();
        // digitalWrite(EN_PIN, HIGH);
        delay(2000);
        stepper.move(50*steps_per_mm); // Move 100mm
        stepper.enableOutputs();
        // digitalWrite(EN_PIN, LOW);
    }
    stepper.run();
}
