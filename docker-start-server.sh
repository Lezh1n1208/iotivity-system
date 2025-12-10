#!/bin/bash

echo "🚀 Starting IoT Gateway (Simulated Raspberry Pi)"
echo "================================================"

# Start Flask backend
echo "🐍 Starting Flask Backend on port 5000..."
cd /app/backend && python3 app.py &
BACKEND_PID=$!
echo "✅ Backend started (PID: $BACKEND_PID)"

# Start OCF Server
echo "🔌 Starting OCF Server on port 5683..."
exec /app/server/ocfserver