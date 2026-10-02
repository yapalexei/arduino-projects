#include <ESP32-TWAI-CAN.hpp>

#define CAN_TX   6  // Connects to CTX
#define CAN_RX   7  // Connects to CRX

CanFrame rxFrame; // Create frame to read 

void setup() {
  // Set up serial for debugging
  Serial.begin(115200);
  delay(500);

  // Set the pins
  ESP32Can.setPins(CAN_TX, CAN_RX);
  Serial.println("test3");

  // Start the CAN bus at 500 kbps
  if(ESP32Can.begin(ESP32Can.convertSpeed(500))) {
      Serial.println("CAN bus started!");
  } else {
      Serial.println("CAN bus failed!");
  }
}

void loop() {

  // canSender();  // call function to send data through CAN
  canReceiver(); // call function to recieve data through CAN
}

void canSender() {
  // send packet: id is 11 bits, packet can contain up to 8 bytes of data
  Serial.print("Sending packet ... ");

  CanFrame testFrame = { 0 };
  testFrame.identifier = 0x12;  // Sets the ID
  testFrame.extd = 0; // Set extended frame to false
  testFrame.data_length_code = 8; // Set length of data - change depending on data sent
  testFrame.data[0] = '1'; // Write data to buffer. data is not sent until writeFrame() is called.
  testFrame.data[1] = '2';
  testFrame.data[2] = '3';
  testFrame.data[3] = '4';
  testFrame.data[4] = '5';
  testFrame.data[5] = '6';
  testFrame.data[6] = '7';
  testFrame.data[7] = '8';

  ESP32Can.writeFrame(testFrame); // transmit frame
  Serial.println("done");

  delay(1000);
}

void canReceiver() {
  // try to parse packet
  if(ESP32Can.readFrame(rxFrame, 700)) { // 1000 is the timeout value
    // Communicate that a packet was recieved
    Serial.printf("Received frame: %03X \r\n", rxFrame.identifier);

    // Communicate packet information
    for(int i = 0; i <= rxFrame.data_length_code - 1; i ++) {
      Serial.print((char) rxFrame.data[i]); // Transmit value from the frame 
    }
  } else {
    Serial.println("No frame recieved");
  }
}