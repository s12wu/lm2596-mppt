// Arduino Uno

#include <MCP42010.h>
// SPI pins for Arduino UNO are CS=2, CLK=4, MOSI=5
MCP42010 mcp_digipot(2, 4, 5);

#include <Wire.h>
#include <Adafruit_INA219.h>
Adafruit_INA219 ina_solar(0x40); // (no solder bridges)
Adafruit_INA219 ina_batt(0x41);  // bridge A0

/*
#define MCP_LOW_R 53      //ohms when set to 0 - measure using a multimeter
#define MCP_HIGH_R 10180  //ohms when set to 255
*/
// These are the raw values the digipot should be set to to get a specific voltage at the solar panel. Obtained by experimenting
#define MCP_AT_MPP 38       // 16 V panel voltage = MPP
#define MCP_OPENCIRCUIT 15  // > 20.5 V panel voltage = Open circuit, the load is basically disconnected
uint8_t digipot = MCP_AT_MPP;

/*
Solar Panel characteristics: Open circuit voltage 20V, Maximum Power Point 16V

The charge process happens in three stages:
1. Full Power:  If the battery voltage is below the maximum voltage even while charging it,
                we can operate at the MPP (16V).

2. Almost full: If the battery voltage at the MPP exceeds the maximum value,
                we must reduce the charging power, i.e. leave the MPP.
                The panel voltage is increased and thus the power is reduced so that
                the battery voltage remains at the maximum.
                The fuller the battery gets, the lower the charging power will be.

3. Full:        At some point, the target panel voltage will be higher than the open circuit voltage.
                This effectively disconnects the solar panel and stops charging.

*/
enum charge_modes {
    FULLPOWER,
    ALMOSTFULL,
    FULL
};
enum charge_modes charge_status = FULLPOWER;


float cutoff_voltage; // the cutoff voltage for the currently set battery chemistry

float panel_voltage, battery_voltage, input_power, output_power;

#define VOLTAGE_THRESHOLD 0.03


void setup() {
    pinMode(12, OUTPUT);
    digitalWrite(12, LOW);

    Serial.begin(115200);

    Serial.println(F("Hello World"));
    mcp_digipot.setPot(1, digipot);

    setup_powersensors();
    setup_chemistry_swich();
}

void loop() {
    read_chemistry_switch();
    read_powersensors();

    Serial.print(String(battery_voltage) + " V\t" + String(output_power/1000, 3) + " W out\t" + String(panel_voltage) + " V in\t" + String(cutoff_voltage) + " V\t");


    if (charge_status == FULLPOWER) {
        Serial.println(F("Full power"));

        if (battery_voltage > cutoff_voltage) {
            Serial.println(F("Switching from Full Power mode to Almost Full"));
            charge_status = ALMOSTFULL;
        }
    }

    if (charge_status == ALMOSTFULL) {
        Serial.print(F("Almost full, error "));

        float error = battery_voltage - cutoff_voltage;
        Serial.print(error);

        if (abs(error) <= VOLTAGE_THRESHOLD) {
            Serial.println(F(" - nothing to do"));
        }

        else if (error > VOLTAGE_THRESHOLD) {
            Serial.print(F(" - vbat too high, decreasing power, dp "));
            digipot -= 1;
            Serial.println(digipot);

            if (digipot <= MCP_OPENCIRCUIT) {
                Serial.println(F("Reached open circuit, battery completely charged."));
                charge_status = FULL;
                digipot = MCP_OPENCIRCUIT;
            }
        }

        else if (error < -VOLTAGE_THRESHOLD) {
            Serial.print(F(" - vbat too low, increasing power, dp "));
            digipot += 1;
            Serial.println(digipot);

            if (digipot >= MCP_AT_MPP) {
                Serial.println(F("reached MPP, switching back to full power"));
                charge_status = FULLPOWER;
                digipot = MCP_AT_MPP;
            }
        }
    }

    if (charge_status == FULL) {
        Serial.println("Battery fully charged");
        if (battery_voltage < cutoff_voltage - VOLTAGE_THRESHOLD) {
            Serial.println("Battery voltage too low, switching to almost full mode");
            digipot = MCP_OPENCIRCUIT;
            charge_status = ALMOSTFULL;
        }
    }


    mcp_digipot.setPot(1, digipot);
    delay(1000);
}
