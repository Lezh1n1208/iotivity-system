#!/bin/bash
set -e

echo "🚀 Starting IoT Gateway"
echo "========================"

# Function to cleanup on exit
cleanup() {
    echo "🛑 Stopping services..."
    kill $FLASK_PID $OCF_PID 2>/dev/null || true
    wait
    echo "✅ Cleanup complete"
}

trap cleanup SIGTERM SIGINT EXIT

# Start Flask backend
echo "🐍 Starting Flask Backend..."
cd /app/backend
python3 app.py &
FLASK_PID=$!

# Wait for Flask to be ready (with retry)
echo "⏳ Waiting for Flask to be ready..."
MAX_RETRIES=30
RETRY_COUNT=0

while [ $RETRY_COUNT -lt $MAX_RETRIES ]; do
    if curl -f http://localhost:5000/api/sensors > /dev/null 2>&1; then
        echo "✅ Flask Backend ready!"
        break
    fi
    RETRY_COUNT=$((RETRY_COUNT + 1))
    sleep 1
done

if [ $RETRY_COUNT -eq $MAX_RETRIES ]; then
    echo "❌ Flask failed to start in 30s"
    exit 1
fi

# Start OCF Server
echo "📡 Starting OCF Server..."
/app/server/ocfserver &
OCF_PID=$!

echo "✅ All services started!"
echo "   - Flask PID: $FLASK_PID"
echo "   - OCF PID: $OCF_PID"
echo ""
echo "📊 Dashboard: http://localhost:5000"
echo "📡 CoAP Server: coap://172.20.0.10:5683"
echo ""

# Wait for both processes (restart if one dies)
while true; do
    if ! kill -0 $FLASK_PID 2>/dev/null; then
        echo "⚠️ Flask Backend died! Restarting..."
        cd /app/backend
        python3 app.py &
        FLASK_PID=$!
    fi
    
    if ! kill -0 $OCF_PID 2>/dev/null; then
        echo "⚠️ OCF Server died! Restarting..."
        /app/server/ocfserver &
        OCF_PID=$!
    fi
    
    sleep 5
done