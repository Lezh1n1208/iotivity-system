from flask import Flask, request, jsonify, render_template
from flask_socketio import SocketIO

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

latest_data = {"temperature": 0.0, "humidity": 0.0, "timestamp": 0}


@app.route('/')
def dashboard():
    return render_template('dashboard.html')


@app.route('/sensor', methods=['POST'])
def receive_sensor_data():
    global latest_data
    data = request.get_json()
    if data:
        latest_data = data
        print(f"[Backend] Received: {data}")
        socketio.emit('sensor_update', data)
    return jsonify({"status": "ok"})


@app.route('/api/sensors', methods=['GET'])
def get_sensors():
    return jsonify(latest_data)


@app.route('/latest', methods=['GET'])
def get_latest():
    return jsonify(latest_data)


if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000, debug=True)
