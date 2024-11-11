#include <Arduino_DIH.h>
#include <ArduinoJson.h>

Arduino_DIH dih(Serial);

void setup() {
    dih.begin(9600);
    dih.setDeviceID("device_001");
}

void loop() {
    dih.loop();

    // Only send data if registered
    if (dih.isRegistered()) {
        int sensorValue = analogRead(A0);
        dih.sendData("temperature", String(sensorValue));
    }
    delay(1000);
}
