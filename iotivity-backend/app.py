from flask import Flask, request, jsonify
import time

app = Flask(__name__)

# In-memory storage
latest_data = {"temperature": 0.0, "humidity": 0.0, "timestamp": 0}


@app.route('/sensor', methods=['POST'])
def receive_sensor():
    """ESP8266 gửi data đến đây"""
    global latest_data
    data = request.json
    latest_data = {
        "temperature": data.get("temperature", 0.0),
        "humidity": data.get("humidity", 0.0),
        "timestamp": int(time.time())
    }
    print(f"[Backend] Received: {latest_data}")
    return jsonify({"status": "ok"}), 200


@app.route('/latest', methods=['GET'])
def get_latest():
    """OCF Server lấy data từ đây"""
    return jsonify(latest_data), 200


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
