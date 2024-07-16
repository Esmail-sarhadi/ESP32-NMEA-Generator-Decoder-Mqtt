# ESP32 NMEA Data Generator and Decoder

This project demonstrates how to use an ESP32 microcontroller to generate and decode NMEA (National Marine Electronics Association) data. The ESP32 connects to a WiFi network and communicates with a public MQTT broker to publish and subscribe to specific topics. The project includes generating random NMEA sentences, decoding them, and displaying the results via a web interface using Flask and MQTT.

## Features
- Connects ESP32 to a WiFi network.
- Uses MQTT to communicate with a public broker.
- Generates random NMEA sentences.
- Decodes NMEA sentences.
- Displays the generated and decoded data on a web interface.

## Components
- ESP32 microcontroller.
- WiFi network.
- MQTT broker (e.g., broker.emqx.io).
- Flask web server.

## Getting Started

### Prerequisites
- ESP32 development board.
- Arduino IDE.
- Python 3.x.
- Flask.
- paho-mqtt.

### Installation

1. **ESP32 Setup:**
   - Install the [ESP32 board](https://github.com/espressif/arduino-esp32) in Arduino IDE.
   - Install the necessary libraries in Arduino IDE:
     ```cpp
     #include <WiFi.h>
     #include <PubSubClient.h>
     ```

2. **Flask and MQTT Setup:**
   - Install Flask and paho-mqtt:
     ```bash
     pip install flask paho-mqtt
     ```

### Usage

1. **ESP32 Code:**
   - Upload the provided ESP32 code to your ESP32 development board.
   - Example snippet from `main.cpp`:
     ```cpp
     void setup() {
         Serial.begin(115200);
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
             if (client.connect(client_id.c_str())) {
                 client.subscribe(topic_generate);
                 client.subscribe(topic_decode);
             } else {
                 delay(2000);
             }
         }
         client.publish(topic, "Hi, I'm ESP32 ^^");
     }
     ```

2. **Flask Server:**
   - Run the Flask server:
     ```bash
     python app.py
     ```
   - Example snippet from `app.py`:
     ```python
     @app.route('/generate', methods=['GET'])
     def generate_data():
         global last_response
         last_response = ""
         if mqtt_client_obj:
             result = mqtt_client_obj.publish(topic_generate, "")
             if result[0] == 0:
                 print("Message published successfully")
             else:
                 print("Failed to publish message")

             time.sleep(2)  # Ensure sufficient wait time for ESP32 response

             return jsonify({"data": last_response})
         else:
             return jsonify({"error": "MQTT client not connected"})
     ```

3. **Web Interface:**
   - Access the web interface by navigating to `http://<your_server_ip>:5000` in your web browser.
   - Example snippet from `index.html`:
     ```html
     <!DOCTYPE html>
     <html lang="en">
     <head>
         <meta charset="UTF-8">
         <title>ESP32 NMEA Data</title>
         <style>
             body { font-family: Arial, sans-serif; text-align: center; }
             .button { display: inline-block; padding: 10px 20px; font-size: 16px; cursor: pointer; }
             .button:hover { background-color: #ddd; }
         </style>
     </head>
     <body>
         <h1>ESP32 NMEA Data</h1>
         <button class="button" onclick="generateData()">Generate Sample Data</button>
         <pre id="output"></pre>
         <script>
             function generateData() {
                 fetch('/generate')
                     .then(response => response.json())
                     .then(data => {
                         document.getElementById('output').textContent = data.data;
                     });
             }
         </script>
     </body>
     </html>
     ```

### Files

- **ESP32 Code:**
  - `main.cpp`: Contains the code for the ESP32 to connect to WiFi, generate, and decode NMEA data.

- **Flask Server:**
  - `app.py`: Contains the Flask server code to handle requests and interact with the MQTT broker.

- **Web Interface:**
  - `index.html`: Simple HTML file for the web interface.

### Example

After setting up, you can generate random NMEA sentences and see the decoded information on the web interface.

## Contributing

If you would like to contribute to this project, please open an issue or submit a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- This project uses the ESP32 microcontroller, Arduino IDE, Flask, and paho-mqtt.
<h2 id="donation">Donation</h2>

<p>If you find this project helpful, consider making a donation:</p>
<p><a href="https://nowpayments.io/donation?api_key=REWCYVC-A1AMFK3-QNRS663-PKJSBD2&source=lk_donation&medium=referral" target="_blank">
     <img src="https://nowpayments.io/images/embeds/donation-button-black.svg" alt="Crypto donation button by NOWPayments">
</a></p>
