# kalshiTrader

A small C++ trading bot skeleton for interacting with Kalshi (placeholder).

Requirements

- CMake >= 3.20
- A C++20 toolchain
- vcpkg (manifest mode)

Quick start

1. Install vcpkg (if not already): https://github.com/microsoft/vcpkg

2. From the project root, bootstrap vcpkg and install dependencies (manifest mode will read `vcpkg.json`).

3. Configure and build with CMake (example):

```bash
# from project root
git clone https://github.com/microsoft/vcpkg.git ./vcpkg
./vcpkg/bootstrap-vcpkg.sh

# configure
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$(pwd)/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

Project layout

- `src/` - application sources
- `include/` - public headers
- `tests/` - unit tests (Catch2)

Next steps

- Implement Kalshi REST client in `src/kalshi`
- Add config and credential management
- Implement strategy loop and risk management

Using a global vcpkg

If you prefer a single, global vcpkg installation instead of vendoring it in the project, bootstrap vcpkg once and point CMake to that toolchain file when configuring. For example, if you install vcpkg at `/Users/jiminryu/Documents/vcpkg`:

```bash
# bootstrap (only once)
git clone https://github.com/microsoft/vcpkg.git /Users/jiminryu/Documents/vcpkg
/Users/jiminryu/Documents/vcpkg/bootstrap-vcpkg.sh

# Configure the project to use the global vcpkg
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=/Users/jiminryu/Documents/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

Notes:

- Keep `vcpkg.json` in the project root — this manifest is what vcpkg reads to install the correct packages for the project.
- CI should also bootstrap or cache the same vcpkg installation to ensure reproducible builds.

## Run

The application reads credentials from environment variables. At minimum set:

- `KALSHI_API_KEY_ID` — your Kalshi API key id
- `KALSHI_PRIVATE_KEY` — your RSA private key in PEM format. If storing in an env var, replace real newlines with `\n` and this app will normalize them back.
- Optional: `KALSHI_CHANNELS` (comma-separated channels like `ticker,orderbook_delta`) and `KALSHI_MARKET_TICKER` for orderbook subscriptions.

Example (macOS / Linux):

```bash
export KALSHI_API_KEY_ID="your-key-id"
# If your PEM contains newlines, convert them to literal \n when exporting
export KALSHI_PRIVATE_KEY="$'-----BEGIN PRIVATE KEY-----\nMIIEv...\n-----END PRIVATE KEY-----'"
export KALSHI_CHANNELS="ticker,orderbook_delta"
export KALSHI_MARKET_TICKER="KXHARRIS24-LSV"
./build/src/kalshi_trader_app
```

The client will create the required authentication headers, open a websocket to `wss://api.kalshi.com/trade-api/ws/v2`, and print live messages to stdout.
