#!/bin/bash
# filepath: /home/lezh1n/Workspace/Project/IoT/source-code/docker-start-server.sh

echo "🚀 Starting IoT Gateway"

cd /app/backend && python3 app.py &
exec /app/server/ocfserver