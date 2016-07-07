//==============================================================================
// Configuration
//==============================================================================
const char WIFI_SSID[] = "Interdimensional WiFi";
const char WIFI_PASS[] = "Unity173";
const char WIFI_HOSTNAME[] = "RealFakeGarageDoors";
const char HASH_KEY[] = "wubbalubbadubdub";
const int PORT = 3893;

const int FRONT_DOOR_PIN = 14;
const int REAR_DOOR_PIN = 5;

// Throttle Rate, once met device must be restarted
const int MAX_OPEN_COUNT_PER_MINUTE = 6;
const int MAX_OPEN_COUNT_PER_HOUR = 12;
const int MAX_OPEN_COUNT_PER_DAY = 24;
