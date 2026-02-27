#!/bin/bash

APP_NAME="WobBT"
PROJECT_DIR="./WobBT/WobBT"

echo "=== REBUILD STARTED ==="

# 1️⃣ Running process varsa kapat
PID=$(pgrep -f "$PROJECT_DIR/build/$APP_NAME")

if [ ! -z "$PID" ]; then
    echo "Running $APP_NAME found. PID: $PID"
    echo "Stopping process..."
    kill -15 $PID
    sleep 2

    if ps -p $PID > /dev/null
    then
        echo "Force killing..."
        kill -9 $PID
    fi
else
    echo "No running $APP_NAME process found."
fi

# 2️⃣ Proje klasörüne git
cd $PROJECT_DIR || { echo "Project directory not found!"; exit 1; }

# 3️⃣ Git pull
echo "Pulling latest changes..."
git pull || { echo "Git pull failed!"; exit 1; }

# 4️⃣ Build temizle
echo "Removing old build..."
rm -rf build

# 5️⃣ Configure
echo "Configuring CMake..."
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release || { echo "CMake failed!"; exit 1; }

# 6️⃣ Build
echo "Building..."
cmake --build build || { echo "Build failed!"; exit 1; }

# 7️⃣ Start
echo "Starting application..."
nohup ./build/$APP_NAME > output.log 2>&1 &

echo "=== REBUILD COMPLETE ==="