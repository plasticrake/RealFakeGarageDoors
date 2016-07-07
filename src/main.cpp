#include "Arduino.h"
#include "config.h"
#include "sha256.h"
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//==============================================================================
// Constants
//==============================================================================
const int MSG_SIZE = HASH_LENGTH + 1;  // Hash plus command
const int SECOND = 1000;
const int MINUTE = SECOND * 60;
const int HOUR = MINUTE * 60;
const int DAY = HOUR * 24;

enum Command {
  CMD_STATUS = 0,
  CMD_OPEN_FRONT = 1,
  CMD_OPEN_REAR = 2,
  CMD_SHUTDOWN = 3
};

enum Error {
  INVALID_HASH = 4,
  INVALID_COMMAND = 5,
  THROTTLE_RATE_EXCEEDED = 6
};

enum GarageDoorIndex {
  FRONT = 1,
  REAR = 2
};

//==============================================================================

struct OpenLog {
  static const uint32_t LOG_SIZE = MAX_OPEN_COUNT_PER_DAY + 1;

  uint32_t log[LOG_SIZE];
  size_t logLength;
  size_t openCount;  // all time total

  bool throttleRateExceeded;

  OpenLog() : logLength(0), openCount(0), throttleRateExceeded(false) {
    memset(log, 0, LOG_SIZE);
  }

  void open() {
    if (logLength >= LOG_SIZE) {
      logLength = 0;  // rollover
    }
    log[logLength] = millis();
    openCount++;
    logLength++;
  }

  uint32_t openCountSince(uint32_t since) {
    uint32_t count = 0;
    for (size_t i = 0; i < LOG_SIZE; i++) {
      if (log[i] > 0 && log[i] >= since) {
        count++;
      }
    }
    return count;
  }

  uint32_t openCountLastMinute() {
    uint32_t since = millis();
    if (since < MINUTE) { since = MINUTE; }
    return openCountSince(since - MINUTE);
  }

  uint32_t openCountLastHour() {
    uint32_t since = millis();
    if (since < HOUR) { since = HOUR; }
    return openCountSince(since - HOUR);
  }

  uint32_t openCountLastDay() {
    uint32_t since = millis();
    if (since < DAY) { since = DAY; }
    return openCountSince(since - DAY);
  }

  bool checkThrottleRateExceeded() {
    // Once throttle rate has been met, will always return true
    // Device must be reset to continute to function
    if ((openCountLastMinute() > MAX_OPEN_COUNT_PER_MINUTE) ||
        (openCountLastHour() > MAX_OPEN_COUNT_PER_HOUR) ||
        (openCountLastDay() > MAX_OPEN_COUNT_PER_DAY)) {
      throttleRateExceeded = true;
    }
    return throttleRateExceeded;
  }
};

OpenLog openLog;
MDNSResponder mdns;
WiFiServer server = WiFiServer(PORT);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void printHash(uint8_t* hash) {
  int i;
  for (i = 0; i < 32; i++) {
    Serial.print("0123456789abcdef"[hash[i] >> 4]);
    Serial.print("0123456789abcdef"[hash[i] & 0xf]);
  }
  Serial.println();
}

int openGarageDoor(GarageDoorIndex doorIndex) {
  openLog.open();
  if (openLog.checkThrottleRateExceeded()) {
    return THROTTLE_RATE_EXCEEDED;
  }

  Serial.printf("Opening Garage Door: %i...", doorIndex);

  uint8_t pin;
  switch (doorIndex) {
    case FRONT:
      pin = FRONT_DOOR_PIN;
      break;
    case REAR:
      pin = REAR_DOOR_PIN;
      break;
  }

  digitalWrite(pin, HIGH);
  delay(250);
  digitalWrite(pin, LOW);

  Serial.println("Done");

  return 0;
}

int executeCommand(Command command) {
  switch (command) {
    case CMD_STATUS:
      return 0;  // Do nothing, just needs to respond to client
      break;
    case CMD_OPEN_FRONT:
      return openGarageDoor(FRONT);
      break;
    case CMD_OPEN_REAR:
      return openGarageDoor(REAR);
      break;
    case CMD_SHUTDOWN:
      // Sleep for 21 years
      ESP.deepSleep(999999999 * 999999999U, WAKE_NO_RFCAL);
      break;
    default:
      return INVALID_COMMAND;
      break;
  }
}

uint8_t* genHash(unsigned long epochTime) {
  Sha256.initHmac((uint8_t*)HASH_KEY, sizeof(HASH_KEY) - 1);
  Sha256.print(epochTime);
  return Sha256.resultHmac();
}

int parseMessage(uint8_t* msg, unsigned long epochTime, Command& cmd) {
  for (int8_t i = -2; i <= 2; i++) {
    uint8_t* hash = genHash(epochTime + i);
    if (memcmp(msg, hash, HASH_LENGTH) == 0) {
      Serial.println("Hash Match");
      cmd = (Command)(*(msg + HASH_LENGTH));
      return 0;
    }
  }
  return INVALID_HASH;
}

//==============================================================================
// setup()
//==============================================================================
void setup() {
  Serial.begin(115200);

  pinMode(FRONT_DOOR_PIN, OUTPUT);
  pinMode(REAR_DOOR_PIN, OUTPUT);

  Serial.printf("\nConnecting to %s", WIFI_SSID);
  if (WiFi.SSID() != WIFI_SSID) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }

  WiFi.hostname(WIFI_HOSTNAME);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }

  Serial.print("\nWiFi connected\n");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  if (!mdns.begin(WIFI_HOSTNAME)) {
    Serial.printf("mDNS Hostname: %s\n", "ERROR");
  } else {
    Serial.printf("mDNS Hostname: %s\n", WIFI_HOSTNAME);
  }

  timeClient.begin();

  server.begin();
  Serial.printf("Port: %i\n", PORT);
}

//==============================================================================
// loop()
//==============================================================================
void loop() {
  timeClient.update();

  if (WiFiClient client = server.available()) {
    while (client.connected()) {
      if (client.available() > 0) {
        Serial.println("Client Connected... Receiving Data");

        uint8_t response;
        unsigned long epochTime = timeClient.getEpochTime();

        uint8_t buffer[MSG_SIZE];
        client.readBytes(buffer, MSG_SIZE);

        Command cmd = CMD_STATUS;
        int ret;
        if ((ret = parseMessage(buffer, epochTime, cmd)) == 0) {
          ret = executeCommand(cmd);
        }
        if (ret == 0) {
          response = cmd;  // Echo back command signaling success
        } else {
          response = ret;  // Respond with Error Code
        }

        Serial.printf("Response: %i\n", response);
        client.print(response, 1);
        client.stop();
        Serial.println("Client Disconnected");
      }
    }
  }

  static uint32_t lastStatus = millis();
  if (millis() - lastStatus > 1000) {
    Serial.print('.');
    if (openLog.throttleRateExceeded) { Serial.print('X'); }
    lastStatus = millis();
  }
}
