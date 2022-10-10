import os

PRODUCTION = "production"
DEVELOPMENT = "development"

COIN_TARGET = "AVAX"
COIN_REFER = "USDT"

ENV = PRODUCTION #os.getenv("ENVIRONMENT", DEVELOPMENT)
DEBUG = False

DCLOG = True

BINANCE = {
  "key": "",
  "secret": ""
}
