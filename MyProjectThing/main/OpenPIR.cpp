// //IR SENSOR ********************************************************************
// #define PIR_AOUT A0  // PIR analog output on A0
// #define PIR_DOUT 2   // PIR digital output on D2
// #define LED_PIN  13  // LED to illuminate on motion
// #define PRINT_TIME 100 // Rate of serial printouts
// unsigned long lastPrint = 0; // Keep track of last serial out
// void readDigitalValue();
// void printAnalogValue();
// void readDigitalValue()
// {
//   // The OpenPIR's digital output is active high
//   int motionStatus = digitalRead(PIR_DOUT);
//
//   // If motion is detected, turn the onboard LED on:
//   if (motionStatus == HIGH)
//     digitalWrite(LED_PIN, HIGH);
//   else // Otherwise turn the LED off:
//     digitalWrite(LED_PIN, LOW);
// }
//
// void printAnalogValue()
// {
//   if ( (lastPrint + PRINT_TIME) < millis() )
//   {
//     lastPrint = millis();
//     // Read in analog value:
//     unsigned int analogPIR = analogRead(PIR_AOUT);
//     // Convert 10-bit analog value to a voltage
//     // (Assume high voltage is 5.0V.)
//     float voltage = (float) analogPIR / 1024.0 * 5.0;
//     // Print the reading from the digital pin.
//     // Mutliply by 5 to maintain scale with AOUT.
//     Serial.print(5 * digitalRead(PIR_DOUT));
//     Serial.print(',');    // Print a comma
//     Serial.print(2.5);    // Print the upper limit
//     Serial.print(',');    // Print a comma
//     Serial.print(1.7);    // Print the lower limit
//     Serial.print(',');    // Print a comma
//     Serial.print(voltage); // Print voltage
//     Serial.println();
//   }
// }
//
// pinMode(PIR_AOUT, INPUT);
// pinMode(PIR_DOUT, INPUT);
// pinMode(LED_PIN, OUTPUT);
// digitalWrite(LED_PIN, LOW);
//
// readDigitalValue();
// // Read A pin, print that value to serial port:
// printAnalogValue();
