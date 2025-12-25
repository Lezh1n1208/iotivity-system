# IoT Gateway Project README

## Overview

This project provides an IoT gateway solution with a backend Flask server and frontend OCF (Open CoAP) clients. It supports temperature and humidity data collection and transmission via HTTP and CoAP protocols.

### Quick Install Guide
1. Clone the repository:
```bash
git clone https://github.com/yourusername/iotivity-<repository_name>
```

2. Install dependencies:
```bash
cd iotivity-backend && python3 setup.py install
```

## Features

1. **Backend (Server)**
   - Handles sensor data via HTTP API.
   - Maintains state for latest temperature and humidity values.
   - Supports CoAP protocol for more efficient IoT communication.

2. **Frontend (OCF Clients)**
   - Client applications for mobile and desktop access.
   - Simple interface to view IoT data.
   - Built with Docker containers for easy deployment.

3. **CoAP Support**
   - Data delivery via HTTP requests.
   - Random fluctuations in sensor data for simulation.

4. **Configuration**
   - Docker networks provide isolated environments.
   - Environmental variables allow customization of server IP and CoAP port.

## Setup

### Server Setup
1. Clone the IoT server container:
```bash
docker build ./server/Dockerfile.server
docker run -it --name iot-server -p 5000:5000 python3 app.py &
```

2. For local development:
```bash
cd /app/backend && python3 app.py &
```

### Client Setup
1. Clone either client container:
   ```bash
   docker build ./client/Dockerfile.client && \
   docker run -it --name iot-client -p 5000:5000 start_client.sh
   ```

2. Or use a script for manual execution:
```bash
./docker-start-client.sh
```

## Usage

### Server Usage (from Python CLI)
```python3 app.py```

### Client Usage (from CoAP)
```bash
curl -X POST "http://localhost:5683" \
  --data-binary "@client.cbor" "temperature":24.0 "humidity":61.0
```

## Quick Start

#### Running the Server
1. Build the server:
   ```bash
   docker build ./server/Dockerfile.server -t iot-server .
   ```
2. Run in detached mode:
   ```bash
   docker run -d --detach -p 5000:5000 python3 app.py &
   ```

#### Running a Client (from Terminal)
```bash
curl -X POST "http://localhost:5683" \
  --data-binary "@client.cbor" "temperature":24.0 "humidity":61.0
```

## Legal Notes

- This project is provided as-is.
- No liability for misuse or damage.

## Contact

For questions, contact [your email](mailto:your@email.com) or visit the repository's issues page.

---

This README provides a comprehensive guide to using and deploying the IoT gateway project. Let me know if you need further details!