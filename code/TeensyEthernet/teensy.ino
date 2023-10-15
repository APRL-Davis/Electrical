// SPDX-FileCopyrightText: (c) 2023 Shawn Silverman <shawn@pobox.com>
// SPDX-License-Identifier: MIT

// BroadcastChat is a simple chat application that broadcasts and
// receives text messages over UDP.
//
// This file is part of the QNEthernet library.

#include <QNEthernet.h>

using namespace qindesign::network;

// --------------------------------------------------------------------------
//  Configuration
// --------------------------------------------------------------------------

constexpr uint32_t kDHCPTimeout = 15000;  // 15 seconds

constexpr uint16_t kPort = 5190;  // Chat port

const IPAddress ipaddr = {192,0,0,10};
const IPAddress subnetmask = {255,255,255,0};
const IPAddress gw = {192,0,0,250};
const IPAddress dns1 = {1,1,1,1};

// --------------------------------------------------------------------------
//  Program State
// --------------------------------------------------------------------------

static String Command;
const int Relay1 = 6;
//const int Relay2 = 6;
//const int Relay3 = 6;
//const int Relay4 = 6;
//const int Relay5 = 6;
//const int Relay6 = 6;


// UDP port.
EthernetUDP udp;

// --------------------------------------------------------------------------
//  Main Program
// --------------------------------------------------------------------------

// Forward declarations (not really needed in the Arduino environment)
static void printPrompt();
static void receivePacket();
static void sendLine();

// Program setup.
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 4000) {
    // Wait for Serial
  }
  printf("Starting...\r\n");

  uint8_t mac[6];
  Ethernet.macAddress(mac);  // This is informative; it retrieves, not sets
  printf("MAC = %02x:%02x:%02x:%02x:%02x:%02x\r\n",
         mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  Ethernet.onLinkState([](bool state) {
    printf("[Ethernet] Link %s\r\n", state ? "ON" : "OFF");
  });

  printf("Starting Ethernet with DHCP...\r\n");
  if (!Ethernet.begin(ipaddr,subnetmask,gw,dns1)) {
    printf("Failed to start Ethernet\r\n");
    return;
  }
  if (!Ethernet.waitForLocalIP(kDHCPTimeout)) {
    printf("Failed to get IP address from DHCP\r\n");
    return;
  }

  IPAddress ip = Ethernet.localIP();
  printf("    Local IP     = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.subnetMask();
  printf("    Subnet mask  = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.broadcastIP();
  printf("    Broadcast IP = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.gatewayIP();
  printf("    Gateway      = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);
  ip = Ethernet.dnsServerIP();
  printf("    DNS          = %u.%u.%u.%u\r\n", ip[0], ip[1], ip[2], ip[3]);

  // Start UDP listening on the port
  udp.begin(kPort);

  pinMode(Relay1,OUTPUT);
  digitalWrite(Relay1,LOW); // Initial Closed
//  pinMode(Relay2,OUTPUT);
//  pinMode(Relay3,OUTPUT);
//  pinMode(Relay4,OUTPUT);
//  pinMode(Relay5,OUTPUT);
//  pinMode(Relay6,OUTPUT);
  
  printPrompt();
}

// Main program loop.
void loop() {
  //receivePacket();
  //sendLine();
  readLine(Command);
//  if (Serial.available() > 0) 
//  {  
//    actuatorCommand = Serial.parseInt();
//    switch (actuatorCommand){
//      case 11:
//            digitalWrite(relay,HIGH);
//            delay(10);
//            break;
//          
//      case 12:
//        digitalWrite(relay,LOW);
//        delay(10);
//        break;
//    }
//}
}

// --------------------------------------------------------------------------
//  Internal Functions
// --------------------------------------------------------------------------

// Control character names.
static const String kCtrlNames[]{
  "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL",
  "BS",  "HT",  "LF",  "VT",  "FF",  "CR",  "SO",  "SI",
  "DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB",
  "CAN", "EM",  "SUB", "ESC", "FS",  "GS",  "RS",  "US",
};

// Receives and prints chat packets.
static void receivePacket() {
  int size = udp.parsePacket();
  if (size < 0) {
    return;
  }

  // Get the packet data and remote address
  const uint8_t *data = udp.data();
  IPAddress ip = udp.remoteIP();

  printf("[%u.%u.%u.%u][%d] ", ip[0], ip[1], ip[2], ip[3], size);
  printf("Hello");
  // Print each character
  for (int i = 0; i < size; i++) {
    uint8_t b = data[i];
    if (b < 0x20) {
      printf("<%s>", kCtrlNames[b].c_str());
    } else if (b < 0x7f) {
      putchar(data[i]);
    } else {
      printf("<%02xh>", data[i]);
    }
  }
  printf("\r\n");
}

// Tries to read a line from the console and returns whether
// a complete line was read. This is CR/CRLF/LF EOL-aware.
static bool readLine(String &Command) {
  static bool inCR = false;  // Keeps track of CR state

  while (Serial.available() > 0) {
    int c;
    switch (c = Serial.read()) {
      case '\r':
        inCR = true;
        if (Command.equals("10")) {
          // Relay 1 Close
          printf("Relay1 Close\n\r");
          digitalWrite(Relay1,LOW);
            delay(10);
            Command = "";
            return true;
        }
        if (Command.equals("11")) {
          // Relay 1 Open
          printf("Relay1 Open\n\r");
          digitalWrite(Relay1,HIGH);
            delay(10);
            Command = "";
            return true;
        }
//        if (Command.equals("20")) {
//          // Relay 2 Close
//          digitalWrite(Relay2,LOW);
//            delay(10);
//            Command = "";
//            return true;
//        }
//        if (Command.equals("21")) {
//          // Relay 2 Open
//          digitalWrite(Relay2,HIGH);
//            delay(10);
//            Command = "";
//            return true;
//        }
//        if (Command.equals("30")) {
//          // Relay 3 Close
//          digitalWrite(Relay3,LOW);
//            delay(10);
//            Command = "";
//            return true;
//        }
//        if (Command.equals("31")) {
//          // Relay 3 Open
//          digitalWrite(Relay3,HIGH);
//            delay(10);
//            Command = "";
//            return true;
//        }
//        if (Command.equals("40")) {
//          // Relay 4 Close
//          digitalWrite(Relay4,LOW);
//            delay(10);
//            Command = "";
//            return true;
//        }
//        if (Command.equals("41")) {
//          // Relay 4 Open
//          digitalWrite(Relay4,HIGH);
//            delay(10);
//            Command = "";
//            return true;
//        }
//        if (Command.equals("50")) {
//          // Relay 5 Close
//          digitalWrite(Relay5,LOW);
//            delay(10);
//            Command = "";
//            return true;
//        }
//        if (Command.equals("51")) {
//          // Relay 5 Open
//          digitalWrite(Relay5,HIGH);
//            delay(10);
//            Command = "";
//            return true;
//        }
//        if (Command.equals("60")) {
//          // Relay 6 Close
//          digitalWrite(Relay6,LOW);
//            delay(10);
//            Command = "";
//            return true;
//        }
//        if (line.equals("61")) {
//          // Relay 6 Open
//          digitalWrite(Relay6,HIGH);
//            delay(10);
//            line = "";
//            return true;
//        }
        return true;

      case '\n':
        if (inCR) {
          // Ignore the LF
          inCR = false;
          break;
        }
        return true;

      default:
        if (c < 0) {
          return false;
        }
        inCR = false;
        Command.append(static_cast<char>(c));
    }
  }

  return false;
}

// Prints the chat prompt.
static void printPrompt() {
  printf("chat> ");
  fflush(stdout);  // printf may be line-buffered, so ensure there's output
}

// Reads from the console and sends packets.
static void sendLine() {
  static String line;

  // Read from the console and send lines
  if (readLine(line)) {
    if (line.equals("Start")) {
      printf("Started\r\n");
    }
    printf("Sent Packet\r\n");
    if (!udp.send(Ethernet.broadcastIP(), kPort,
                  reinterpret_cast<const uint8_t *>(line.c_str()),
                  line.length())) {
      printf("[Error sending]\r\n");
      printf("\r\n");
    }
    line = "";
    printPrompt();
  }
}
