#!/bin/bash
echo "🚀 Starting IoT Gateway"

cd /app/backend && python3 app.py &
exec /app/server/ocfserver