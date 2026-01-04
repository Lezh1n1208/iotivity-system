#!/bin/bash
set -e

echo "📱 Starting OCF Client"
echo "======================"

# Get server IP from environment or use default
SERVER_IP=${SERVER_IP:-172.20.0.10}

echo "🎯 Target Server: $SERVER_IP:5683"

# Wait for server to be discoverable
echo "⏳ Waiting for OCF Server to be discoverable..."
MAX_RETRIES=30
RETRY_COUNT=0

while [ $RETRY_COUNT -lt $MAX_RETRIES ]; do
    # Try to ping server
    if ping -c 1 -W 1 $SERVER_IP > /dev/null 2>&1; then
        echo "✅ Server reachable!"
        break
    fi
    RETRY_COUNT=$((RETRY_COUNT + 1))
    echo "   Retry $RETRY_COUNT/$MAX_RETRIES..."
    sleep 1
done

if [ $RETRY_COUNT -eq $MAX_RETRIES ]; then
    echo "❌ Server not reachable after 30s"
    exit 1
fi

# Additional delay for OCF server initialization
echo "⏳ Waiting for OCF Server initialization..."
sleep 5

echo "🚀 Starting OCF Client..."
exec /app/ocfclient