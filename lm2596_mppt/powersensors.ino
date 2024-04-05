void setup_powersensors(){
    if (!ina_solar.begin()) {
        Serial.println(F("Failed to find INA219 chip (solar side)"));
        while (1) { delay(10); }
    }
    if (!ina_batt.begin()) {
        Serial.println(F("Failed to find INA219 chip (battery side)"));
        while (1) { delay(10); }
    }
}

void read_powersensors(){
    input_power = ina_solar.getPower_mW();
    output_power = ina_batt.getPower_mW();
    panel_voltage = ina_solar.getBusVoltage_V();
    battery_voltage = ina_batt.getBusVoltage_V();
}

