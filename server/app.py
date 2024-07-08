import random
import time
from flask import Flask, request, jsonify
from paho.mqtt import client as mqtt_client

broker = 'broker.emqx.io'
port = 1883
topic_generate = "esmail/ali/generate"
topic_decode = "esmail/ali/decode"
topic_response = "esmail/ali/response"
client_id = f'python-mqtt-{random.randint(0, 1000)}'

app = Flask(__name__)
mqtt_client_obj = None
last_response = ""


def connect_mqtt():
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
            client.subscribe(topic_response)
        else:
            print(f"Failed to connect, return code {rc}")

    def on_message(client, userdata, msg):
        global last_response
        print(f"Received message from topic `{msg.topic}`: {msg.payload.decode()}")
        last_response = msg.payload.decode()

    global mqtt_client_obj
    mqtt_client_obj = mqtt_client.Client(mqtt_client.CallbackAPIVersion.VERSION1, client_id)
    mqtt_client_obj.on_connect = on_connect
    mqtt_client_obj.on_message = on_message
    mqtt_client_obj.connect(broker, port)
    return mqtt_client_obj


@app.route('/generate', methods=['GET'])
def generate_data():
    global last_response
    last_response = ""
    if mqtt_client_obj:
        print("Publishing to generate topic")
        result = mqtt_client_obj.publish(topic_generate, "")
        if result[0] == 0:
            print("Message published successfully")
        else:
            print("Failed to publish message")

        time.sleep(2)  # Ensure sufficient wait time for ESP32 response

        return jsonify({"data": last_response})
    else:
        return jsonify({"error": "MQTT client not connected"})


@app.route('/decode', methods=['POST'])
def decode_data():
    global last_response
    last_response = ""
    nmea_data = request.files['file'].read().decode()
    if mqtt_client_obj:
        print("Publishing to decode topic")
        result = mqtt_client_obj.publish(topic_decode, nmea_data)
        if result[0] == 0:
            print("Message published successfully")
        else:
            print("Failed to publish message")

        time.sleep(2)  # Ensure sufficient wait time for ESP32 response

        return jsonify({"data": last_response})
    else:
        return jsonify({"error": "MQTT client not connected"})


@app.route('/')
def index():
    return '''
    <!DOCTYPE HTML>
    <html>
    <head>
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
        <button class="button" onclick="document.getElementById('fileInput').click()">Upload NMEA File</button>
        <input type="file" id="fileInput" style="display: none;" onchange="uploadFile()">
        <pre id="output"></pre>
        <script>
            function generateData() {
                fetch('/generate')
                    .then(response => response.json())
                    .then(data => {
                        document.getElementById('output').textContent = data.data;
                    });
            }
            function uploadFile() {
                const fileInput = document.getElementById('fileInput');
                const file = fileInput.files[0];
                const formData = new FormData();
                formData.append('file', file);
                fetch('/decode', {
                    method: 'POST',
                    body: formData
                })
                .then(response => response.json())
                .then(data => {
                    document.getElementById('output').textContent = data.data;
                });
            }
        </script>
    </body>
    </html>
    '''


if __name__ == '__main__':
    mqtt_client_obj = connect_mqtt()
    mqtt_client_obj.loop_start()
    app.run(host='0.0.0.0', port=5000)
