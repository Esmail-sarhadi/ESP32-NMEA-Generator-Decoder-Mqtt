#include <WiFi.h>
#include <PubSubClient.h>

// Replace these with your network credentials
const char* ssid = "charon";
const char* password = "12121212";
const char *mqtt_broker = "broker.emqx.io";
const int mqtt_port = 1883;
const char *topic = "emqx/esp32";

// MQTT Topics
const char *topic_generate = "esmail/ali/generate";
const char *topic_decode = "esmail/ali/decode";
const char *topic_response = "esmail/ali/response";

String generateRandomNMEAData();
String generateRandomTime();
String generateRandomCoordinate(bool isLatitude);
String generateRandomSatelliteCount();
String calculateChecksum(String nmea);
String decodeNMEAData(String nmea_data);
bool process_gpgga_line(const String& line, struct GPGGAData& data);

struct GPGGAData {
    String time;
    String latitude;
    String latitude_direction;
    String longitude;
    String longitude_direction;
    String satellite_count;
};

WiFiClient espClient;
PubSubClient client(espClient);
void callback(char *topic, byte *payload, unsigned int length);

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect("ESP32Client")) {
            Serial.println("connected");
            client.subscribe(topic_generate);
            client.subscribe(topic_decode);
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200, SERIAL_8N1, 3, 1);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the Wi-Fi network");

    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);

    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str())) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    client.publish(topic, "Hi, I'm ESP32 ^^");
    
    client.subscribe(topic_generate);
    client.subscribe(topic);
    client.subscribe(topic_decode);
}

void callback(char *topic, byte *payload, unsigned int length) {
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
    Serial.println(message);
    if (String(topic) == topic_generate) {
        String nmea_data = generateRandomNMEAData();
        client.publish(topic_response, nmea_data.c_str());
        String decoded_data = decodeNMEAData(nmea_data);
        client.publish(topic_response, decoded_data.c_str());
    } else if (String(topic) == topic_decode) {
        
    }
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
}

String generateRandomNMEAData() {
    String nmea = "";
    nmea += "$GPGGA," + generateRandomTime() + "," + generateRandomCoordinate(true) + "," + generateRandomCoordinate(false) + ",1," + generateRandomSatelliteCount() + ",1.0,1.0,M,1.0,M,,";
    nmea += "*" + calculateChecksum(nmea) + "\n";
    return nmea;
}

String generateRandomTime() {
    char buffer[7];
    snprintf(buffer, sizeof(buffer), "%02d%02d%02d", random(0, 24), random(0, 60), random(0, 60));
    return String(buffer);
}

String generateRandomCoordinate(bool isLatitude) {
    char buffer[12];
    int degrees = isLatitude ? random(0, 90) : random(0, 180);
    float minutes = random(0, 6000) / 100.0;
    char direction = isLatitude ? (random(0, 2) == 0 ? 'N' : 'S') : (random(0, 2) == 0 ? 'E' : 'W');
    snprintf(buffer, sizeof(buffer), "%02d%07.4f,%c", degrees, minutes, direction);
    return String(buffer);
}

String generateRandomSatelliteCount() {
    return String(random(0, 13));
}

String calculateChecksum(String nmea) {
    byte checksum = 0;
    for (int i = 1; i < nmea.length(); i++) {
        checksum ^= nmea[i];
    }
    char buffer[3];
    snprintf(buffer, sizeof(buffer), "%02X", checksum);
    return String(buffer);
}

String decodeNMEAData(String nmea_data) {
    String decoded_data = "";
    int start = 0;
    int end = nmea_data.indexOf('\n');
    while (end != -1) {
        String line = nmea_data.substring(start, end);
        if (line.indexOf("$GPGGA") != -1) {
            GPGGAData data;
            if (process_gpgga_line(line, data)) {
                decoded_data += "Time: " + data.time + ", ";
                decoded_data += "Latitude: " + data.latitude + " " + data.latitude_direction + ", ";
                decoded_data += "Longitude: " + data.longitude + " " + data.longitude_direction + ", ";
                decoded_data += "Satellite Count: " + data.satellite_count + "\n";
            }
        }
        start = end + 1;
        end = nmea_data.indexOf('\n', start);
    }
    return decoded_data;
}

bool process_gpgga_line(const String& line, struct GPGGAData& data) {
    int indices[15];
    int idx_count = 0;
    for (int i = 0; i < line.length(); i++) {
        if (line[i] == ',' || line[i] == '*') {
            indices[idx_count++] = i;
        }
    }
    if (idx_count != 15) {
        return false;
    }
    data.time = line.substring(indices[0] + 1, indices[1]);
    data.latitude = line.substring(indices[1] + 1, indices[2]);
    data.latitude_direction = line.substring(indices[2] + 1, indices[3]);
    data.longitude = line.substring(indices[3] + 1, indices[4]);
    data.longitude_direction = line.substring(indices[4] + 1, indices[5]);
    data.satellite_count = line.substring(indices[6] + 1, indices[7]);
    return true;
}
