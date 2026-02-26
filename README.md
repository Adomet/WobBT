# Wolf Of Binance Back Tester

Wolf Of Binance Back Tester

## Linux (AWS EC2) build and run

Install dependencies (Ubuntu/Debian):

```bash
sudo apt update
sudo apt install -y build-essential cmake libcurl4-openssl-dev libssl-dev zlib1g-dev python3-dev
```

For **static linking** (libcurl, libssl, libcrypto embedded in executable):

```bash
sudo apt install -y libcurl4-openssl-dev libssl-dev zlib1g-dev  # static .a libs
```

Build:

```bash
cd WobBT
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Build with **static linking** (standalone executable, fewer .so dependencies):

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DSTATIC_LINK=ON
cmake --build build -j
```

Run:

```bash
./build/WobBT
```

Set API keys securely before running (Linux):

```bash
export BINANCE_API_KEY="YFcu43KTUUgWpfEr9fGP1AaIV5gRqpVceSA0VQzh3qgcv9eztlppgmfwJ4lvbgAc"
export BINANCE_API_SECRET="YFcu43KTUUgWpfEr9fGP1AaIV5gRqpVceSA0VQzh3qgcv9eztlppgmfwJ4lvbgAc"
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