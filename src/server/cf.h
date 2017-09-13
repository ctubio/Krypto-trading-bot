#ifndef K_CF_H_
#define K_CF_H_

namespace K {
  static Gw* gw;
  static Gw* gW;
  extern json cf;
  extern string cFname;
  class CF {
    public:
      static void internal() {
        string k = string(getenv("KCONFIG") != NULL ? getenv("KCONFIG") : "K");
        cFname = string("etc/").append(k).append(".json");
        string cfname = string("etc/").append(k).append(".png");
        if (access(cFname.data(), F_OK) != -1) {
          ifstream file(cFname);
          cf = json::parse(string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>()));
          cout << FN::uiT() << "CF" << RWHITE <<" settings loaded from JSON file " << RYELLOW << k << RWHITE << " OK." << endl;
        } else if (access(cfname.data(), F_OK) != -1) {
          cFname = cfname;
          png_structp png_ptr;
          png_infop info_ptr;
          unsigned char sig[8];
          FILE *fp;
          if (!(fp = fopen(cFname.data(), "rb"))) { cout << FN::uiT() << "CF" << RRED << " Errrror:" << BRED << " Could not find and open file " << k << "." << endl; }
          else {
            fread(sig, 1, 8, fp);
            if (!png_check_sig(sig, 8)) { cout << FN::uiT() << "CF" << RRED << " Errrror:" << BRED << " Not a PNG file." << endl; }
            else {
              png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
              info_ptr = png_create_info_struct(png_ptr);
              if (!png_ptr) { cout << FN::uiT() << "CF" << RRED << " Errrror:" << BRED << " Could not allocate memory." << endl; }
              else if (setjmp(png_jmpbuf(png_ptr))) { cout << FN::uiT() << RRED << " Errrror:" << BRED << " PNG error." << endl; }
              else {
                png_init_io(png_ptr, fp);
                png_set_sig_bytes(png_ptr, 8);
                png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);
                png_textp text_ptr;
                int num_text;
                png_get_text(png_ptr, info_ptr, &text_ptr, &num_text);
                string conf = "";
                for (int i = 0; i < num_text; i++)
                  if (strcmp("K.conf", text_ptr[i].key) == 0)
                    conf = text_ptr[i].text;
                if (conf.length()) {
                  cf = json::parse(conf);
                  cout << FN::uiT() << "CF" << RWHITE << " Settings loaded from PNG file " << k << " OK." << endl;
                } else cout << FN::uiT() << "CF" << RRED << " Warrrrning:" << BRED << " No data found inside PNG file " << k << "." << endl;
              }
            }
          }
        }
        if (cfString("EXCHANGE", false) == "") cout << FN::uiT() << "CF" << RRED << " Warrrrning:" << BRED << " Settings not loaded because the config file was not found, reading ENVIRONMENT vars instead." << endl;
      };
      static void external() {
        gw = Gw::E(cfExchange());
        gw->base = cfBase();
        gw->quote = cfQuote();
        cfExchange(gw->config());
        gW = (gw->target == "NULL") ? Gw::E(mExchange::Null) : gw;
      };
      static string cfString(string k, bool r = true) {
        if (getenv(k.data()) != NULL) return string(getenv(k.data()));
        if (cf.find(k) == cf.end()) {
          if (r) {
            cout << FN::uiT() << "CF" << RRED << " Errrror:" << BRED << " Use of missing \"" << k << "\" configuration."<< endl << endl << FN::uiT() << "Hint! Make sure " << cFname << " exists or see https://github.com/ctubio/Krypto-trading-bot/blob/master/etc/K.json.dist" << endl;
            exit(1);
          } else return "";
        }
        return cf[k].is_string()
          ? cf.value(k, "")
          : (cf[k].is_number() ? to_string(cf.value(k, 0.0)) : "");
      };
      static int cfBase() {
        string k_ = cfString("TradedPair");
        string k = k_.substr(0, k_.find("/"));
        if (k == k_) { cout << FN::uiT() << "CF" << RRED << " Errrror:" << BRED << " Invalid currency pair! Must be in the format of BASE/QUOTE, eg BTC/EUR." << endl; exit(1); }
        return S2mC(k);
      };
      static int cfQuote() {
        string k_ = cfString("TradedPair");
        string k = k_.substr(k_.find("/")+1);
        if (k == k_) { cout << FN::uiT() << "CF" << RRED << " Errrror:" << BRED << " Invalid currency pair! Must be in the format of BASE/QUOTE, eg BTC/EUR." << endl; exit(1); }
        return S2mC(k);
      };
      static mExchange cfExchange() {
        string k = FN::S2l(cfString("EXCHANGE"));
        if (k == "coinbase") return mExchange::Coinbase;
        else if (k == "okcoin") return mExchange::OkCoin;
        else if (k == "bitfinex") return mExchange::Bitfinex;
        else if (k == "poloniex") return mExchange::Poloniex;
        else if (k == "korbit") return mExchange::Korbit;
        else if (k == "hitbtc") return mExchange::HitBtc;
        else if (k == "null") return mExchange::Null;
        cout << FN::uiT() << "CF" << RRED << " Errrror:" << BRED << " Invalid configuration value \"" << k << "\" as EXCHANGE. See https://github.com/ctubio/Krypto-trading-bot/tree/master/etc#configuration-options for more information." << endl;
        exit(1);
      };
      static int S2mC(string k) {
        k = FN::S2u(k);
        for (unsigned i=0; i<mCurrency.size(); ++i) if (mCurrency[i] == k) return i;
        cout << FN::uiT() << "CF" << RRED << " Errrror:" << BRED << " Use of missing \"" << k << "\" currency." << endl;
        exit(1);
      };
      static bool autoStart() {
        return "auto" == cfString("BotIdentifier").substr(0,4);
      };
    private:
      static void cfExchange(mExchange e) {
        if (e == mExchange::Coinbase) {
          system("test -n \"`/bin/pidof stunnel`\" && kill -9 `/bin/pidof stunnel`");
          system("stunnel dist/K-stunnel.conf");
          json k = FN::wJet(string(gw->http).append("/products/").append(gw->symbol));
          gw->minTick = stod(k.value("quote_increment", "0"));
          gw->minSize = stod(k.value("base_min_size", "0"));
        } else if (e == mExchange::HitBtc) {
          json k = FN::wJet(string(gw->http).append("/api/1/public/symbols"));
          if (k.find("symbols") != k.end())
            for (json::iterator it = k["symbols"].begin(); it != k["symbols"].end(); ++it)
              if (it->value("symbol", "") == gw->symbol) {
                gw->minTick = stod(it->value("step", "0"));
                gw->minSize = stod(it->value("lot", "0"));
                break;
              }
        } else if (e == mExchange::Bitfinex) {
          json k = FN::wJet(string(gw->http).append("/pubticker/").append(gw->symbol));
          if (k.find("last_price") != k.end()) {
            stringstream price_;
            price_ << scientific << stod(k.value("last_price", "0"));
            string _price_ = price_.str();
            for (string::iterator it=_price_.begin(); it!=_price_.end();)
              if (*it == '+' or *it == '-') break; else it = _price_.erase(it);
            stringstream os(string("1e").append(to_string(stod(_price_)-4)));
            os >> gw->minTick;
            gw->minSize = 0.01;
          }
        } else if (e == mExchange::OkCoin) {
          gw->minTick = "btc" == gw->symbol.substr(0,3) ? 0.01 : 0.001;
          gw->minSize = 0.01;
        } else if (e == mExchange::Korbit) {
          json k = FN::wJet(string(gw->http).append("/constants"));
          if (k.find(gw->symbol.substr(0,3).append("TickSize")) != k.end()) {
            gw->minTick = k.value(gw->symbol.substr(0,3).append("TickSize"), 0.0);
            gw->minSize = 0.015;
          }
        } else if (e == mExchange::Poloniex) {
          json k = FN::wJet(string(gw->http).append("/public?command=returnTicker"));
          if (k.find(gw->symbol) != k.end()) {
            istringstream os(string("1e-").append(to_string(6-k[gw->symbol]["last"].get<string>().find("."))));
            os >> gw->minTick;
            gw->minSize = 0.01;
          }
        } else if (e == mExchange::Null) {
          gw->minTick = 0.01;
          gw->minSize = 0.01;
        }
        if (!gw->minTick) { cout << FN::uiT() << "CF" << RRED << " Errrror:" << BRED << " Unable to fetch data from " << cfString("EXCHANGE") << " symbol \"" << gw->symbol << "\"." << endl; exit(1); }
        else { cout << FN::uiT() << "GW " << RWHITE << cfString("EXCHANGE") << " allows client IP." << endl; }
        cout << FN::uiT() << "GW " << RWHITE << setprecision(8) << fixed << cfString("EXCHANGE") << ":" << endl
          << "- autoBot: " << (autoStart() ? "yes" : "no") << endl
          << "- pair: " << gw->symbol << endl
          << "- minTick: " << gw->minTick << endl
          << "- minSize: " << gw->minSize << endl
          << "- makeFee: " << gw->makeFee << endl
          << "- takeFee: " << gw->takeFee << endl;
      };
  };
}

#endif