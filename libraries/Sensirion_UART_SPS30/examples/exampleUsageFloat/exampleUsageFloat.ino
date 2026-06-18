/*
 * Copyright (c) 2025, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <Arduino.h>
#include <SensirionUartSps30.h>

// Adjust as needed for you Arduino board.
// [Serial, Serial1, Serial2, etc.]
#define SENSOR_SERIAL_INTERFACE Serial1
// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

SensirionUartSps30 sensor;

static char errorMessage[64];
static int16_t error;

void setup() {

    // initialize serial interface for logging
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }

    // initialize serial interface for sensor communication
    SENSOR_SERIAL_INTERFACE.begin(115200);
    while (!SENSOR_SERIAL_INTERFACE) {
        delay(100);
    }

    sensor.begin(SENSOR_SERIAL_INTERFACE);

    sensor.stopMeasurement();
    int8_t serialNumber[32] = {0};
    int8_t productType[9] = {0};
    error = sensor.readSerialNumber(serialNumber, 32);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute readSerialNumber(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    Serial.print("serialNumber: ");
    Serial.print((const char*)serialNumber);
    Serial.println();
    error = sensor.readProductType(productType, 9);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute readProductType(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    Serial.print("productType: ");
    Serial.print((const char*)productType);
    Serial.println();
    error = sensor.startMeasurement(SPS30_OUTPUT_FORMAT_OUTPUT_FORMAT_FLOAT);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute startMeasurement(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
}

void loop() {

    float mc1p0 = 0;
    float mc2p5 = 0;
    float mc4p0 = 0;
    float mc10p0 = 0;
    float nc0p5 = 0;
    float nc1p0 = 0;
    float nc2p5 = 0;
    float nc4p0 = 0;
    float nc10p0 = 0;
    float typicalParticleSize = 0;
    delay(1000);
    error = sensor.readMeasurementValuesFloat(mc1p0, mc2p5, mc4p0, mc10p0,
                                              nc0p5, nc1p0, nc2p5, nc4p0,
                                              nc10p0, typicalParticleSize);
    if (error != NO_ERROR) {
        Serial.print("Error trying to execute readMeasurementValuesFloat(): ");
        errorToString(error, errorMessage, sizeof errorMessage);
        Serial.println(errorMessage);
        return;
    }
    Serial.print("mc1p0: ");
    Serial.print(mc1p0);
    Serial.print("\t");
    Serial.print("mc2p5: ");
    Serial.print(mc2p5);
    Serial.print("\t");
    Serial.print("mc4p0: ");
    Serial.print(mc4p0);
    Serial.print("\t");
    Serial.print("mc10p0: ");
    Serial.print(mc10p0);
    Serial.print("\t");
    Serial.print("nc0p5: ");
    Serial.print(nc0p5);
    Serial.print("\t");
    Serial.print("nc1p0: ");
    Serial.print(nc1p0);
    Serial.print("\t");
    Serial.print("nc2p5: ");
    Serial.print(nc2p5);
    Serial.print("\t");
    Serial.print("nc4p0: ");
    Serial.print(nc4p0);
    Serial.print("\t");
    Serial.print("nc10p0: ");
    Serial.print(nc10p0);
    Serial.print("\t");
    Serial.print("typicalParticleSize: ");
    Serial.print(typicalParticleSize);
    Serial.println();
}
