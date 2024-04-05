// Battery voltages at which the charging is finished
#define CUTOFF_LIION_2S 8.3  // Rotkohl
#define CUTOFF_NIMH_6S 8.6   // Internal battery
#define CUTOFF_NIMH_8S 11.6  // 8S NIMH battery pack 

// Pins where the selector swich is connected to. The selected one is connected to ground.
#define SELECT_GROUND 8  //pin 8 is used as a ground pin so we can use a 4-pin connector
#define SELECT_LIION_2S_PIN 10
#define SELECT_NIMH_6S_PIN 9
#define SELECT_NIMH_8S_PIN 11

void setup_chemistry_swich() {
    pinMode(SELECT_GROUND, OUTPUT);
    digitalWrite(SELECT_GROUND, LOW);
    pinMode(SELECT_LIION_2S_PIN, INPUT_PULLUP);
    pinMode(SELECT_NIMH_6S_PIN, INPUT_PULLUP);
    pinMode(SELECT_NIMH_8S_PIN, INPUT_PULLUP);
    read_chemistry_switch();
}

// Polled in void loop(). This function reads the current switch position and changes cutoff_voltage if the user changed the setting.
// If all three pins are disconnected, we remain in the previous setting.
void read_chemistry_switch() {
    if (digitalRead(SELECT_LIION_2S_PIN) == LOW) cutoff_voltage = CUTOFF_LIION_2S;
    if (digitalRead(SELECT_NIMH_6S_PIN) == LOW) cutoff_voltage = CUTOFF_NIMH_6S;
    if (digitalRead(SELECT_NIMH_8S_PIN) == LOW) cutoff_voltage = CUTOFF_NIMH_8S;
}
