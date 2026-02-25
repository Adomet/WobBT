# Wolf Of Binance Back Tester

Wolf Of Binance Back Tester

## Linux (AWS EC2) build and run

Install dependencies (Ubuntu/Debian):

```bash
sudo apt update
sudo
```

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