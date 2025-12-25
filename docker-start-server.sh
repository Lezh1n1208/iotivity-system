#!/bin/bash
set -e

echo "🚀 Starting IoT Gateway"
echo "========================"

# Start Flask backend first (to initialize state file)
echo "🐍 Starting Flask Backend..."
cd /app/backend
python3 app.py &
FLASK_PID=$!

# Wait for Flask to initialize
sleep 3

# Start OCF Server
echo "📡 Starting OCF Server..."
/app/server/ocfserver &
OCF_PID=$!

# Wait for any process to exit
wait -n

# Exit with status of process that exited first
exit $?