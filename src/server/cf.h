#ifndef K_CF_H_
#define K_CF_H_

namespace K {
  class CF: public kLass {
    public:
      int argPort = 3000,
          argColors = 0,
          argDebug = 0,
          argDebugEvents = 0,
          argDebugOrders = 0,
          argDebugQuotes = 0,
          argWithoutSSL = 0,
          argHeadless = 0,
          argAutobot = 0,
          argNaked = 0,
          argFree = 0;
      string argTitle = "K.sh",
             argUser = "NULL",
             argPass = "NULL",
             argMatryoshka = "https://www.example.com/",
             argExchange = "NULL",
             argCurrency = "NULL",
             argApikey = "NULL",
             argSecret = "NULL",
             argUsername = "NULL",
             argPassphrase = "NULL",
             argHttp = "NULL",
             argWss = "NULL",
             argDatabase = "",
             argWhitelist = "";
      double argEwmaShort = 0,
             argEwmaMedium = 0,
             argEwmaLong = 0,
             argEwmaVeryLong = 0;
    protected:
      void load(int argc, char** argv) {
        cout << BGREEN << "K" << RGREEN << " build " << K_BUILD << " " << K_STAMP << "." << BRED << '\n';
        int k;
        while (true) {
          int i = 0;
          static struct option args[] = {
            {"help",         no_argument,       0,               'h'},
            {"colors",       no_argument,       &argColors,        1},
            {"debug",        no_argument,       &argDebug,         1},
            {"debug-events", no_argument,       &argDebugEvents,   1},
            {"debug-orders", no_argument,       &argDebugOrders,   1},
            {"debug-quotes", no_argument,       &argDebugQuotes,   1},
            {"without-ssl",  no_argument,       &argWithoutSSL,    1},
            {"headless",     no_argument,       &argHeadless,      1},
            {"naked",        no_argument,       &argNaked,         1},
            {"autobot",      no_argument,       &argAutobot,       1},
            {"whitelist",    required_argument, 0,               'L'},
            {"matryoshka",   required_argument, 0,               'k'},
            {"exchange",     required_argument, 0,               'e'},
            {"currency",     required_argument, 0,               'c'},
            {"apikey",       required_argument, 0,               'A'},
            {"secret",       required_argument, 0,               'S'},
            {"passphrase",   required_argument, 0,               'X'},
            {"username",     required_argument, 0,               'U'},
            {"http",         required_argument, 0,               'H'},
            {"wss",          required_argument, 0,               'W'},
            {"title",        required_argument, 0,               'K'},
            {"port",         required_argument, 0,               'P'},
            {"user",         required_argument, 0,               'u'},
            {"pass",         required_argument, 0,               'p'},
            {"database",     required_argument, 0,               'd'},
            {"ewma-short",   required_argument, 0,               's'},
            {"ewma-medium",  required_argument, 0,               'm'},
            {"ewma-long",    required_argument, 0,               'l'},
            {"ewma-verylong",required_argument, 0,               'V'},
            {"free-version", no_argument,       &argFree,          1},
            {"version",      no_argument,       0,               'v'},
            {0,              0,                 0,                 0}
          };
          k = getopt_long(argc, argv, "hvd:l:m:s:p:u:v:c:e:k:P:K:W:H:U:X:S:A:", args, &i);
          if (k == -1) break;
          switch (k) {
            case 0: break;
            case 'P': argPort = stoi(optarg); break;
            case 'A': argApikey = string(optarg); break;
            case 'S': argSecret = string(optarg); break;
            case 'U': argUsername = string(optarg); break;
            case 'X': argPassphrase = string(optarg); break;
            case 'H': argHttp = string(optarg); break;
            case 'W': argWss = string(optarg); break;
            case 'e': argExchange = string(optarg); break;
            case 'c': argCurrency = string(optarg); break;
            case 'd': argDatabase = string(optarg); break;
            case 'k': argMatryoshka = string(optarg); break;
            case 'K': argTitle = string(optarg); break;
            case 'u': argUser = string(optarg); break;
            case 'p': argPass = string(optarg); break;
            case 's': argEwmaShort = stod(optarg); break;
            case 'm': argEwmaMedium = stod(optarg); break;
            case 'l': argEwmaLong = stod(optarg); break;
            case 'V': argEwmaVeryLong = stod(optarg); break;
            case 'L': argWhitelist = string(optarg); break;
            case 'h': cout
              << RGREEN << "This is free software: the quoting engine and UI are open source," << '\n' << "feel free to hack both as you need." << '\n'
              << RGREEN << "This is non-free software: the exchange integrations are licensed" << '\n' << "by and under the law of my grandma, feel free to crack all." << '\n'
              << RGREEN << "  questions: " << RYELLOW << "https://earn.com/analpaper/" << '\n'
              << BGREEN << "K" << RGREEN << " bugkiller: " << RYELLOW << "https://github.com/ctubio/Krypto-trading-bot/issues/new" << '\n'
              << RGREEN << "  downloads: " << RYELLOW << "ssh://git@github.com/ctubio/Krypto-trading-bot" << '\n';
            case '?': cout
              << FN::uiT() << "Usage:" << BYELLOW << " ./K.sh [arguments]" << '\n'
              << FN::uiT() << "[arguments]:" << '\n'
              << FN::uiT() << RWHITE << "-h, --help                - show this help and quit." << '\n'
              << FN::uiT() << RWHITE << "    --autobot             - automatically start trading on boot." << '\n'
              << FN::uiT() << RWHITE << "    --naked               - do not display CLI, print output to stdout instead." << '\n'
              << FN::uiT() << RWHITE << "    --headless            - do not listen for UI connections," << '\n'
              << FN::uiT() << RWHITE << "                            ignores '--without-ssl', '--whitelist' and '--port'." << '\n'
              << FN::uiT() << RWHITE << "    --without-ssl         - do not use HTTPS for UI connections (use HTTP only)." << '\n'
              << FN::uiT() << RWHITE << "-L, --whitelist=IP        - set IP or csv of IPs to allow UI connections," << '\n'
              << FN::uiT() << RWHITE << "                            alien IPs will get a zip-bomb instead." << '\n'
              << FN::uiT() << RWHITE << "-P, --port=NUMBER         - set NUMBER of an open port to listen for UI connections." << '\n'
              << FN::uiT() << RWHITE << "-u, --user=WORD           - set allowed WORD as username for UI connections," << '\n'
              << FN::uiT() << RWHITE << "                            mandatory but may be 'NULL'." << '\n'
              << FN::uiT() << RWHITE << "-p, --pass=WORD           - set allowed WORD as password for UI connections," << '\n'
              << FN::uiT() << RWHITE << "                            mandatory but may be 'NULL'." << '\n'
              << FN::uiT() << RWHITE << "-e, --exchange=NAME       - set exchange NAME for trading, mandatory one of:" << '\n'
              << FN::uiT() << RWHITE << "                            'COINBASE', 'BITFINEX', 'HITBTC', 'OKCOIN'," << '\n'
              << FN::uiT() << RWHITE << "                            'KORBIT', 'POLONIEX' or 'NULL'." << '\n'
              << FN::uiT() << RWHITE << "-c, --currency=PAIRS      - set currency pairs for trading (use format" << '\n'
              << FN::uiT() << RWHITE << "                            with '/' separator, like 'BTC/EUR')." << '\n'
              << FN::uiT() << RWHITE << "-A, --apikey=WORD         - set (never share!) WORD as api key for trading," << '\n'
              << FN::uiT() << RWHITE << "                            mandatory." << '\n'
              << FN::uiT() << RWHITE << "-S, --secret=WORD         - set (never share!) WORD as api secret for trading," << '\n'
              << FN::uiT() << RWHITE << "                            mandatory." << '\n'
              << FN::uiT() << RWHITE << "-X, --passphrase=WORD     - set (never share!) WORD as api passphrase for trading," << '\n'
              << FN::uiT() << RWHITE << "                            mandatory but may be 'NULL'." << '\n'
              << FN::uiT() << RWHITE << "-U, --username=WORD       - set (never share!) WORD as api username for trading," << '\n'
              << FN::uiT() << RWHITE << "                            mandatory but may be 'NULL'." << '\n'
              << FN::uiT() << RWHITE << "-H, --http=URL            - set URL of api HTTP/S endpoint for trading," << '\n'
              << FN::uiT() << RWHITE << "                            mandatory." << '\n'
              << FN::uiT() << RWHITE << "-W, --wss=URL             - set URL of api SECURE WS endpoint for trading," << '\n'
              << FN::uiT() << RWHITE << "                            mandatory." << '\n'
              << FN::uiT() << RWHITE << "-d, --database=PATH       - set alternative PATH to database filename," << '\n'
              << FN::uiT() << RWHITE << "                            default PATH is '/data/db/K.*.*.*.db'," << '\n'
              << FN::uiT() << RWHITE << "                            any route to a filename is valid," << '\n'
              << FN::uiT() << RWHITE << "                            or use ':memory:' (see sqlite.org/inmemorydb.html)." << '\n'
              << FN::uiT() << RWHITE << "-s, --ewma-short=PRICE    - set initial ewma short value," << '\n'
              << FN::uiT() << RWHITE << "                            overwrites the value from the database." << '\n'
              << FN::uiT() << RWHITE << "-m, --ewma-medium=PRICE   - set initial ewma medium value," << '\n'
              << FN::uiT() << RWHITE << "                            overwrites the value from the database." << '\n'
              << FN::uiT() << RWHITE << "-l, --ewma-long=PRICE     - set initial ewma long value," << '\n'
              << FN::uiT() << RWHITE << "                            overwrites the value from the database." << '\n'
              << FN::uiT() << RWHITE << "-V, --ewma-verylong=PRICE - set initial ewma verylong value," << '\n'
              << FN::uiT() << RWHITE << "                            overwrites the value from the database." << '\n'
              << FN::uiT() << RWHITE << "    --debug-events        - print detailed output about event handlers." << '\n'
              << FN::uiT() << RWHITE << "    --debug-orders        - print detailed output about exchange messages." << '\n'
              << FN::uiT() << RWHITE << "    --debug-quotes        - print detailed output about quoting engine." << '\n'
              << FN::uiT() << RWHITE << "    --debug               - print detailed output about all the (previous) things!" << '\n'
              << FN::uiT() << RWHITE << "    --colors              - print highlighted output." << '\n'
              << FN::uiT() << RWHITE << "-k, --matryoshka=URL      - set Matryoshka link URL of the next UI." << '\n'
              << FN::uiT() << RWHITE << "-K, --title=WORD          - set WORD as UI title to identify different bots." << '\n'
              << FN::uiT() << RWHITE << "    --free-version        - work with all market levels but slowdown with 1 XMR hash." << '\n'
              << FN::uiT() << RWHITE << "-v, --version             - show current build version and quit." << '\n'
              << RGREEN << "  more help: " << RYELLOW << "https://github.com/ctubio/Krypto-trading-bot/blob/master/MANUAL.md" << '\n'
              << BGREEN << "K" << RGREEN << " questions: " << RYELLOW << "irc://irc.domirc.net:6667/##tradingBot" << '\n'
              << RGREEN << "  home page: " << RYELLOW << "https://ca.rles-tub.io./trades" << '\n';
              exit(EXIT_SUCCESS);
              break;
            case 'v': exit(EXIT_SUCCESS);
            default: abort();
          }
        }
        if (optind < argc) {
          cout << "ARG" << RRED << " Errrror:" << BRED << " non-option ARGV-elements: ";
          while(optind < argc) cout << argv[optind++];
          cout << '\n';
          exit(EXIT_SUCCESS);
        }
        if (argExchange == "") {
          cout << "ARG" << RRED << " Errrror:" << BRED << " Missing mandatory argument \"--exchange\", at least." << '\n';
          exit(EXIT_SUCCESS);
        }
        if (argDebug)
          argDebugEvents =
          argDebugOrders =
          argDebugQuotes = argDebug;
        if (!argColors)
          RBLACK[0] = RRED[0]    = RGREEN[0] = RYELLOW[0] =
          RBLUE[0]  = RPURPLE[0] = RCYAN[0]  = RWHITE[0]  =
          BBLACK[0] = BRED[0]    = BGREEN[0] = BYELLOW[0] =
          BBLUE[0]  = BPURPLE[0] = BCYAN[0]  = BWHITE[0]  = argColors;
        if (argDatabase == "")
          argDatabase = string("/data/db/K.")
            + to_string((int)exchange())
            + '.' + base()
            + '.' + quote() + ".db";
      };
      void run() {
        if (!argNaked) FN::screen(argColors, argExchange, argCurrency);
        mExchange e = exchange();
        gw = Gw::E(e);
        gw->exchange = e;
        gw->name = argExchange;
        gw->base = base();
        gw->quote = quote();
        gw->apikey = argApikey;
        gw->secret = argSecret;
        gw->user = argUsername;
        gw->pass = argPassphrase;
        gw->http = argHttp;
        gw->ws = argWss;
        gw->version = argFree;
      };
    private:
      string base() {
        string k_ = argCurrency;
        string k = k_.substr(0, k_.find("/"));
        if (k == k_) FN::logExit("CF", "Invalid currency pair! Must be in the format of BASE/QUOTE, eg BTC/EUR.", EXIT_SUCCESS);
        return FN::S2u(k);
      };
      string quote() {
        string k_ = argCurrency;
        string k = k_.substr(k_.find("/")+1);
        if (k == k_) FN::logExit("CF", "Invalid currency pair! Must be in the format of BASE/QUOTE, eg BTC/EUR", EXIT_SUCCESS);
        return FN::S2u(k);
      };
      mExchange exchange() {
        string k = FN::S2l(argExchange);
        if (k == "coinbase") return mExchange::Coinbase;
        else if (k == "okcoin") return mExchange::OkCoin;
        else if (k == "okex") return mExchange::OkEx;
        else if (k == "bitfinex") return mExchange::Bitfinex;
        else if (k == "hitbtc") return mExchange::HitBtc;
        else if (k == "kraken") return mExchange::Kraken;
        else if (k == "korbit") return mExchange::Korbit;
        else if (k == "poloniex") return mExchange::Poloniex;
        else if (k != "null") FN::logExit("CF", string("Invalid configuration value \"") + k + "\" as EXCHANGE. See https://github.com/ctubio/Krypto-trading-bot/tree/master/etc#configuration-options for more information", EXIT_SUCCESS);
        return mExchange::Null;
      };
  };
}

#endif