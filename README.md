[***REFUGEES WELCOME!***](http://www.refugeesaid.eu/rab-campaign/)

[![Release](https://img.shields.io/github/release/ctubio/Krypto-trading-bot.svg)](https://github.com/ctubio/Krypto-trading-bot/releases)
[![Platform](https://img.shields.io/badge/platform-unix--like-lightgray.svg)](https://www.gnu.org/)
[![Software License](https://img.shields.io/badge/license-ISC-111111.svg)](https://raw.githubusercontent.com/ctubio/Krypto-trading-bot/master/LICENSE)
[![Software License](https://img.shields.io/badge/license-MIT-111111.svg)](https://raw.githubusercontent.com/ctubio/Krypto-trading-bot/master/COPYING)

[`K.js`](https://github.com/ctubio/Krypto-trading-bot) is a very low latency cryptocurrency [market making](https://github.com/ctubio/Krypto-trading-bot/blob/master/MANUAL.md#what-is-market-making) trading bot with a full featured [web interface](https://github.com/ctubio/Krypto-trading-bot#web-ui) and [backtester](https://github.com/ctubio/Krypto-trading-bot/blob/master/MANUAL.md#how-can-i-test-new-trading-strategies), it directly connects to [several cryptocoin exchanges](https://github.com/ctubio/Krypto-trading-bot/tree/master/etc#configuration-options). On a decent machine, it reacts to market data by placing and canceling orders in under a millisecond.

[![Build Status](https://img.shields.io/travis/ctubio/Krypto-trading-bot/master.svg?label=test%20build)](https://travis-ci.org/ctubio/Krypto-trading-bot)
[![Coverage Status](https://img.shields.io/coveralls/ctubio/Krypto-trading-bot/master.svg?label=code%20coverage)](https://coveralls.io/r/ctubio/Krypto-trading-bot?branch=master)
[![Quality Status](https://img.shields.io/codacy/grade/d48a59c313504f7988e3df031665f90f/master.svg)](https://www.codacy.com/app/ctubio/Krypto-trading-bot)
[![Dependency Status](https://img.shields.io/david/ctubio/Krypto-trading-bot.svg)](https://david-dm.org/ctubio/Krypto-trading-bot)
[![Open Issues](https://img.shields.io/github/issues/ctubio/Krypto-trading-bot.svg)](https://github.com/ctubio/Krypto-trading-bot/issues)

### <img src="https://assets-cdn.github.com/images/icons/emoji/unicode/1f4be.png" align="middle" /> Latest version at https://github.com/ctubio/Krypto-trading-bot <img src="https://assets-cdn.github.com/images/icons/emoji/unicode/1f51e.png" align="middle" /> <img src="https://assets-cdn.github.com/images/icons/emoji/unicode/1f4b8.png" align="middle" />

[![Total Downloads](https://img.shields.io/npm/dt/hacktimer.svg)](https://github.com/ctubio/Krypto-trading-bot)
[![Week Downloads](https://img.shields.io/npm/dw/hacktimer.svg)](https://github.com/ctubio/Krypto-trading-bot)
[![Month Downloads](https://img.shields.io/npm/dm/hacktimer.svg)](https://github.com/ctubio/Krypto-trading-bot)
[![Day Downloads](https://img.shields.io/npm/dy/hacktimer.svg)](https://github.com/ctubio/Krypto-trading-bot)

Runs on the latest node.js (v7 or v8). Persistence is achieved using a built-in server-less SQLite C++ interface. Installation via Docker is supported, but manual installation in a dedicated Debian 8, CentOS 7 or macOS 10.12 instance is recommended.

![Web UI Preview](https://raw.githubusercontent.com/ctubio/Krypto-trading-bot/master/dist/img/web_ui_preview.png)

The web UI is compatible with most web browsers/devices/resolutions, but Firefox or Chrome at 1600px are recommended. Doesn't require configuration of any web server (unless installed behind your own reverse proxy).

### Compatible Exchanges

||with Post-Only Orders support|without Post-Only|
|---|---|---|
|**without Maker fees**|[Coinbase GDAX](https://www.gdax.com/)<br> &#10239; _REST + WebSocket + FIX_||
|**with Maker and Taker fees**|[Bitfinex](https://www.bitfinex.com/)<br> &#10239; _REST + WebSocket_<br><br>[Poloniex](https://www.poloniex.com/)<br> &#10239; _REST_|[OKCoin.com](https://www.okcoin.com/)<br>[OKCoin.cn](https://www.okcoin.cn/)<br> &#10239; _REST + WebSocket_<br><br>[HitBTC](https://hitbtc.com/)<br> &#10239; _REST + WebSocket_<br><br>[Korbit](https://www.korbit.co.kr/)<br> &#10239; _REST_|

All currency pairs are supported, otherwise please open a [new issue](https://github.com/ctubio/Krypto-trading-bot/issues/new?title=Missing%20currency%20pair) to easily include any missing currency that you would like.

### Docker Installation

See [dist/Dockerfile](https://github.com/ctubio/Krypto-trading-bot/tree/master/dist#dockerfile) section if you use winy (because the Manual Installation only works on unix-like platforms).

### Manual Installation

1. Ensure your target machine has installed node v7 or v8 (see `node -v` or `nodejs -v`), g++ will be installed automatically.

2. Run in any location that you wish (feel free to customize the suggested folder name `K`):
```
 $ git clone ssh://git@github.com/ctubio/Krypto-trading-bot K
 $ cd K
 $ cp etc/K.json.dist etc/K.json
 $ vim etc/K.json
 $ npm start
```

See [configuration](https://github.com/ctubio/Krypto-trading-bot/tree/master/etc#configuration-options) section while setting up the configuration options in your new config file `etc/K.json`.

`npm` will run `K.js` in the background using [forever](https://www.npmjs.com/package/forever). But before, it will install all local dependencies in `node_modules` folder and will compile TypeScript in `src` folder to CommonJS in `app` folder if it was not already done before.

Troubleshooting:

 * Do not install or execute the application as root; if you really want to install as root use `npm install --unsafe-perm`.

 * Create a temporary [swap file](https://stackoverflow.com/questions/17173972/how-do-you-add-swap-to-an-ec2-instance) (after install you can swapoff) if the installation fails with error: `virtual memory exhausted: Cannot allocate memory`.

 * Run `rm -rf node_modules && npm install` if the application stops working after `npm run latest` (sometimes outdated dependencies are not deleted).

 Optional:

 * Install the system daemon script `dist/K-init.sh` (to make use of `service K start` from anywhere instead of `cd path/to/K && npm start`) see [dist](https://github.com/ctubio/Krypto-trading-bot/tree/master/dist) folder.

 * Replace the certificate at `dist/sslcert` folder with your own, see [web ui](https://github.com/ctubio/Krypto-trading-bot#web-ui) section. But, the certificate provided is a fully featured default openssl, that you may just need to authorise in your browser.

### Configuration

See [etc](https://github.com/ctubio/Krypto-trading-bot/tree/master/etc) folder.

### Upgrade to the latest commit

Feel free anytime to check if there are new modifications with `npm run diff`.

Once you decide that is time to upgrade, execute `npm run latest` to download and install the latest modifications in your remote branch (or directly `npm run reinstall` to skip the display of the new commit messages).

After install the latest version, all running instances will be restarted.

### Multiple instances party time

Please note, an "instance" is in fact a config file under `etc` folder; using a single machine and the same source folder, you can run as many instances as config files you have in `etc` folder (limited by the available free RAM).

You can list the current instances running anytime with `npm run list`.

Simple commands like `npm start`, `npm stop` or `npm restart` (without any config file defined) will use the default config file `etc/K.json`.

To run multiple instances using a collection of config files:

1. Create a new config file with `cp etc/K.json etc/X.json` (with any name but `.json` extension).

    1. Edit the value of `WebClientListenPort` in the new config file to set a new port, so all applications have a unique port to display the UI.

    2. Edit the values of `BotIdentifier`, `EXCHANGE` and `TradedPair` in the new config file as you alternatively desire.

2. Run the new instance with `npm start --K.js:config=X.json`, also the commands `npm stop` and `npm restart` allow the parameter `--K.js:config=`, the value is simply the filename of the config file under `etc` folder that you want to run; this value will also be used as the `uid` of the process executed by `forever`.

3. Open in the web browser the different pages of the ports of the different running instances, or display the UI of all instances together in a single page using the MATRYOSHKA link in the footer and the config option `MatryoshkaUrl`.

After multiple config files are setup under `etc` folder, to control them all together instead of one by one, the commands `npm run startall`, `npm run stopall` and `npm run restartall` are also available, just remember that config files with a filename starting with underscore symbol "_" will be skipped.

### Application Usage

1. Open your web browser to connect to HTTPS port `3000` (or value of `WebClientListenPort`) of the machine running K. If you're running K locally on Mac/Windows on Docker, replace "localhost" with the address returned by `boot2docker ip`.

2. Read up on how to use K and market making in the [manual](https://github.com/ctubio/Krypto-trading-bot/blob/master/MANUAL.md).

3. Set up trading parameters to your liking in the web UI. Click the "BTC/USD" button so it is green to start making markets.

### Web UI

Once `K.js` is up and running, visit HTTPS port `3000` (or value of `WebClientListenPort`) of the machine on which it is running to view the admin view. There are inputs for quoting parameters, grids to display market orders, market trades, your trades, your order history, your positions, and a big button with the currency pair you are trading. When you're ready, click that button green to begin sending out quotes. The UI uses a healthy mixture of socket.io and angularjs observed with reactivexjs.

If you want to generate your own certificate see [SSL for internal usage](http://www.akadia.com/services/ssh_test_certificate.html).

In case you really want to use plain HTTP, remove the files `server.crt` and `server.key` inside `dist/sslcert` folder.

### Databases

Each currency pair of each exchange will use a different sqlite database file.

All database files are located at `/data/db/K.*.db`, where `*` is the identifier with format `exchange.base_currency.quote_currency`; it is located outside the application path to survive reinstalls and wild `rm -rf path/to/K`.

You can copy any `.db` file to another machine when migrating or as a backup.

If a database file do not exists, the application will create it on boot; otherwise, it will load it and reuse it.

To see the data of each database file you can use https://github.com/sqlitebrowser/sqlitebrowser or similars.

### Charts

The metrics are not saved anywhere, is just UI data collected with a visibility retention of 6 hours, to display over time:

 * Market Fair Value with High and Low Prices
 * Trades Complete
 * Target Position for BTC currency (TBP)
 * Target Position for Fiat currency
 * STDEV and EWMA values for Quote Protection and APR
 * Amount available in wallet for buy
 * Amount held in open trades for buy
 * Amount available in wallet for sell
 * Amount held in open trades for sell
 * Total amount available and held at both sides in BTC currency
 * Total amount available and held at both sides in Fiat currency

### Test units and Build notes

Feel free to run `npm test` anytime.

To rebuild the application with your modifications, please run `npm install` or directly `npm run postinstall`.

To rebuild the C++ shared objects with your modifications, please run `node-gyp rebuild`.

To pipe the output to stdout, execute the application in the foreground with `nodejs K.js` or `node K.js`.

To ignore the output, execute the application in the background with `forever start K.js` or with the alias `npm start`.

To debug the server code with chrome-devtools, attach the node debugger with `nodejs --inspect K.js` (from your local, you can open a ssh tunnel to access it with `ssh -N -L 9229:127.0.0.1:9229 user@host`).

Passing a config filename as a parameter after `K.js` is also allowed, like `nodejs K.js X.json`.

### Unreleased Changelog:

Added built-in C++ WWW Server to replace expressjs and socketio.

Added built-in SQLite C++ interface to replace external mongodb server.

Added Poloniex API.

### Release 3.0 Changelog:

Updated application name to K because of Kira.

Added nodejs7, typescript2, angular4 and reactivexjs.

Added cleanup of bandwidth, source code, dependencies and installation steps.

Added many quoting parameters thanks to Camille92 genius suggestions.

Added support for multiple instances/config files with nested matryoshka UI.

Added npm scripts, david-dm, travis-ci, coveralls and codacy.

Added historical charts to replace grafana.

Added C++ math functions.

Updated OKCoin API (since https://www.okcoin.com/t-354.html).

Updated Bitfinex API v2.

Added GDAX FIX API with stunnel.

Added Korbit API.

### Release 2.0 Changelog:

Added new quoting styles PingPong, Boomerang, AK-47.

Added cleanup of database records, memory usage and log recording.

Added audio notices, realtime wallet display, and grafana integration.

Added https, dark theme and new UI elements.

Added a bit of love to Kira.

### Release 1.0 Changelog:

see the upstream project [michaelgrosner/tribeca](https://github.com/michaelgrosner/tribeca).

### Donations

nope, this project doesn't have maintenance costs. but you can donate to your favorite developer today! (or tomorrow!)

or see the upstream project [michaelgrosner/tribeca](https://github.com/michaelgrosner/tribeca).

or donate your time with programming or financial suggestions in the topical IRC channel **##tradingBot** at irc.domirc.net on port 6697 (SSL), or 6667 (plain) or feel free to make any question, but questions technically are not donations.
