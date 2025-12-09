#!/bin/bash

echo "📱 Starting OCF Client"
echo "======================"

# Wait for server to be ready
echo "⏳ Waiting for server to start..."
sleep 5

echo "✅ Starting client..."
exec /app/ocfclient