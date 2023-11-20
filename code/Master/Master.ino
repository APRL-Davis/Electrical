#include <NativeEthernet.h>
#include <NativeEthernetUdp.h>
#include <SPI.h>
#include <inttypes.h>
// A lot of placeholders and Unknowns for now

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 177);

unsigned int localPort = 8888;      // local port to listen on

// the following lines should go away and use the structs and types instead
byte sensors1[4];
byte sensors2[4];
byte data[8];
char cDataPacket[64];

// define some values for sensors
#define NUM_SENSORS 2
#define SENSOR_READINGS_PER_PACKET 64
#define CS1 10
#define CS2 9

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// Timing settings
IntervalTimer sensorTimer;
volatile bool readyToReadSensors = false;
const int sensorInterval = 1000; // in microseconds (f = 1000 Hz => T = 1 ms = 1000 us)

// GLOBAL VARIABLES
bool skipDelay;
bool readyToSendData = false;
size_t currentSensorReadingIndex = 0; // index of the current sensor reading
size_t currentPacketID = 0; // index of the current packet

// sensor data structures for transmission
typedef struct sensorData{
  time_t timestamp;
  uint16_t sensorID;
  double sensorValue;
} sensorData;

typedef struct dataPacket{
  time_t timestamp;
  size_t packetID;
  sensorData sensorDataReadings[SENSOR_READINGS_PER_PACKET];
} dataPacket;

// buffers for receiving and sending data
char packetBuffer[64];  // buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged";        // a string to send back
// one buffer for sending and one for sensing writing
dataPacket outgoingDataPacketBuffers[2];
uint8_t currentDataPacketBuffer;

// Structures and settings for sensors
enum sensorType {THERMOCOUPLE, PRESSURE};
enum sensorSerialType {sstSPI, sstI2C, sstUART};
enum SPIPins {CSPin = 0, SCKPin = 1, MISOPin = 2, MOSIPin = 3};

typedef struct {
  uint32_t speed;
  // SPI: pins[0] = CS, pins[1] = SCK, pins[2] = MISO, pins[3] = MOSI
  // I2C: pins[0] = SDA, pins[1] = SCL
  // UART: pins[0] = RX, pins[1] = TX
  uint8_t pins[4];
  sensorType type;
  sensorSerialType serialType;
  uint16_t sensorID;
  uint8_t bitorder;
} sensorSettings;

// setup sensors
sensorSettings sensors[2] = {
  {1000000, {CS1, 13, 12, 11}, THERMOCOUPLE, sstSPI, 1, MSBFIRST},
  {1000000, {CS2, 13, 12, 11}, PRESSURE, sstSPI, 2, MSBFIRST}
};

void setup() {
  
  SPI.begin();

  pinMode(CS1, OUTPUT);
  pinMode(CS2, OUTPUT);

  // You can use Ethernet.init(pin) to configure the CS pin
  //Ethernet.init(10);  // Most Arduino shields
  //Ethernet.init(5);   // MKR ETH shield
  //Ethernet.init(0);   // Teensy 2.0
  //Ethernet.init(20);  // Teensy++ 2.0
  //Ethernet.init(15);  // ESP8266 with Adafruit Featherwing Ethernet
  //Ethernet.init(33);  // ESP32 with Adafruit Featherwing Ethernet

  // start the Ethernet
  Ethernet.begin(mac, ip);

  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
  }

  // start UDP
  Udp.begin(localPort);

  // Begin timers
  sensorTimer.begin(sensorTimerISR, sensorInterval);

}

void loop() {

  // Run through the possible events in this loop starting with
  // the highest-priority events and shortest execution times

  // start with sensor data collection
  if (readyToReadSensors) {
    readyToReadSensors = false;
    readSensors();
    skipDelay = true;
  }

  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i=0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // send a reply to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
  }

  // if there's data to send, send a UDP packet
  if (readyToSendData) {
    readyToSendData = false;
    // send the packet reply
    // ...
    // send the raw dataPacketBuffer over UDP;

  }

  if (!skipDelay) {
    delayMicroseconds(50);
  } else {
    skipDelay = false;
  }
  
}

void readSensors() {
  // read the sensors and store them in the data array
  unsigned int lengthOfSensors = sizeof(sensors)/sizeof(sensorSettings);
  for (unsigned int i = 0; i < lengthOfSensors; i++) {
    switch (sensors[i].type) {
      case THERMOCOUPLE:
        outgoingDataPacketBuffers[currentDataPacketBuffer]
          .sensorDataReadings[currentSensorReadingIndex]
          .sensorValue = readThermocoupleSensor(sensors[i]);
        outgoingDataPacketBuffers[currentDataPacketBuffer]
          .sensorDataReadings[currentSensorReadingIndex]
          .sensorID = sensors[i].sensorID;
        currentSensorReadingIndex++;
        break;
      case PRESSURE:
        outgoingDataPacketBuffers[currentDataPacketBuffer]
          .sensorDataReadings[currentSensorReadingIndex]
          .sensorValue = readPressureSensor(sensors[i]);
        outgoingDataPacketBuffers[currentDataPacketBuffer]
          .sensorDataReadings[currentSensorReadingIndex]
          .sensorID = sensors[i].sensorID;
        currentSensorReadingIndex++;
        break;
      default:
        break;
    }
    if (currentSensorReadingIndex >= SENSOR_READINGS_PER_PACKET) {
      currentSensorReadingIndex = 0;
      outgoingDataPacketBuffers[currentDataPacketBuffer].timestamp = micros();
      outgoingDataPacketBuffers[currentDataPacketBuffer].packetID = currentPacketID;
      currentPacketID++;
      currentDataPacketBuffer = !currentDataPacketBuffer;
      readyToSendData = true;
    }
  }
}

void sensorTimerISR() {
  readyToReadSensors = true;
}

double readThermocoupleSensor(sensorSettings settings) {
  // Read the thermocouple sensor on the given pin
  
  double sensorValue = 0;
  uint32_t rawValue = 0;
  // byte rawByte;
  
  if (settings.serialType == sstSPI) {
    SPI.beginTransaction(SPISettings(settings.speed, settings.bitorder, SPI_MODE0));
    digitalWrite(settings.pins[CSPin], LOW);
    delayMicroseconds(10);
    rawValue = SPI.transfer16(0b0010000000000100);
    digitalWrite(settings.pins[CSPin], HIGH);
    SPI.endTransaction();
  } else if (settings.serialType == sstI2C) {
    // ...
  } else if (settings.serialType == sstUART) {
    // ...
  }

  // process sensorBytes into sensorValue

  sensorValue = double(rawValue);

  return sensorValue;
}

double readPressureSensor(sensorSettings settings) {
  // Read the pressure sensor on the given pin
  // ...
  double sensorValue = 0;
  return sensorValue;
}