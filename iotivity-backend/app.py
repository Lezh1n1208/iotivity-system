from flask import Flask, request, jsonify, render_template
from flask_socketio import SocketIO
import time
import json
import os

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

# Shared state file for OCF server
STATE_FILE = "/tmp/sensor_state.json"

latest_data = {
    "temperature": None,
    "humidity": None,
    "timestamp": 0,
    "sensor_connected": False
}


def save_sensor_state():
    """Save sensor data to shared file for OCF server"""
    with open(STATE_FILE, 'w') as f:
        json.dump(latest_data, f)


def init_sensor_state():
    """Initialize empty state"""
    save_sensor_state()


@app.route('/')
def dashboard():
    return render_template('dashboard.html')


@app.route('/sensor', methods=['POST'])
def receive_sensor_data():
    global latest_data
    data = request.get_json()
    if data:
        latest_data = {
            "temperature": data.get("temperature"),
            "humidity": data.get("humidity"),
            "timestamp": int(time.time()),
            "sensor_connected": True
        }
        print(
            f"✅ [Backend] ESP8266 Data: T={latest_data['temperature']}°C, H={latest_data['humidity']}%")

        # Save to shared file
        save_sensor_state()

        # Broadcast to web dashboard
        socketio.emit('sensor_update', latest_data)
    return jsonify({"status": "ok"})


@app.route('/api/sensors', methods=['GET'])
def get_sensors():
    # Check if sensor data is stale (>15 seconds old)
    now = int(time.time())
    if now - latest_data['timestamp'] > 15:
        latest_data['sensor_connected'] = False
        save_sensor_state()

    return jsonify(latest_data)


@app.route('/latest', methods=['GET'])
def get_latest():
    return jsonify(latest_data)


if __name__ == '__main__':
    print("🚀 Flask Backend Starting...")
    init_sensor_state()
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)
