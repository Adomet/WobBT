# Wolf Of Binance Back Tester

Wolf Of Binance Back Tester

## Linux (AWS EC2) build and run

Install dependencies (Ubuntu/Debian):

```bash
sudo apt update
sudo apt install -y build-essential cmake libcurl4-openssl-dev libssl-dev python3
```

`CMake 2.8.12+` is supported for this project.

Build:

```bash
cd WobBT
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Run:

```bash
./build/WobBT
```

Set API keys securely before running (Linux):

```bash
export BINANCE_API_KEY="your_api_key"
export BINANCE_API_SECRET="your_api_secret"
./build/WobBT
```

Or use a local `.env` file in the `WobBT` directory:

```bash
cat > .env << 'EOF'
BINANCE_API_KEY=your_api_key
BINANCE_API_SECRET=your_api_secret
EOF
./build/WobBT
```

Priority: shell environment variables override values from `.env`.