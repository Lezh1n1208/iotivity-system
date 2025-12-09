#!/bin/bash

echo "🚀 Starting IoT Gateway (Simulated Raspberry Pi)"
echo "================================================"

# Start Flask backend
echo "🐍 Starting Flask Backend on port 5000..."
cd /app/backend
python3 app.py > /var/log/backend.log 2>&1 &
BACKEND_PID=$!

sleep 3

if kill -0 $BACKEND_PID 2>/dev/null; then
    echo "✅ Backend started (PID: $BACKEND_PID)"
else
    echo "❌ Backend failed to start"
    cat /var/log/backend.log
    exit 1
fi

# Start OCF Server
echo "🔌 Starting OCF Server on port 5683..."
cd /app/server
exec ./ocfserver