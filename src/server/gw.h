#ifndef K_GW_H_
#define K_GW_H_

namespace K {
  class GW: public Klass {
    private:
      mConnectivity gwAdminEnabled  = mConnectivity::Disconnected,
                    gwConnectOrders = mConnectivity::Disconnected,
                    gwConnectMarket = mConnectivity::Disconnected;
      unsigned int gwT_5m = 0,
                   gwT_countdown = 0;
      bool sync_levels = false,
           sync_trades = false,
           sync_orders = false;
    protected:
      void load() {                                                 _debugEvent_
        endingFn.back() = &happyEnding;
        gwAdminEnabled = (mConnectivity)((CF*)config)->argAutobot;
        handshake(gw->exchange);
      };
      void waitData() {                                             _debugEvent_
        gw->reconnect = [&](string reason) {
          gwConnect(reason);
        };
        gw->evConnectOrder = [&](mConnectivity k) {
          gwSemaphore(&gwConnectOrders, k);
        };
        gw->evConnectMarket = [&](mConnectivity k) {
          if (!gwSemaphore(&gwConnectMarket, k))
            gw->evDataLevels(mLevels());
        };
      };
      void waitTime() {                                             _debugEvent_
        if (!(sync_levels = !gw->async_levels())) gwConnect();
        sync_trades = !gw->async_trades();
        sync_orders = !gw->async_orders();
        ((EV*)events)->tServer->setData(this);
        ((EV*)events)->tServer->start([](Timer *tServer) {
          ((GW*)tServer->getData())->timer_1s();
        }, 0, 1e+3);
      };
      void waitUser() {                                             _debugEvent_
        ((UI*)client)->welcome(mMatter::Connectivity, &hello);
        ((UI*)client)->clickme(mMatter::Connectivity, &kiss);
        ((SH*)screen)->pressme(mHotkey::ESC, &hotkiss);
      };
      void run() {                                                  _debugEvent_
        ((EV*)events)->start();
      };
    private:
      function<void()> happyEnding = [&]() {
        ((EV*)events)->stop([&]() {
          if (((CF*)config)->argDustybot)
            ((SH*)screen)->log(string("GW ") + gw->name, "--dustybot is enabled, remember to cancel manually any open order.");
          else {
            ((SH*)screen)->log(string("GW ") + gw->name, "Attempting to cancel all open orders, please wait.");
            for (mOrder &it : gw->sync_cancelAll()) gw->evDataOrder(it);
            ((SH*)screen)->log(string("GW ") + gw->name, "cancel all open orders OK");
          }
          if (gw->exchange == mExchange::Coinbase) stunnel();
        });
      };
      function<void(json*)> hello = [&](json *welcome) {
        *welcome = { semaphore() };
      };
      function<void(json)> kiss = [&](json butterfly) {
        if (!butterfly.is_object() or !butterfly["state"].is_number()) return;
        mConnectivity updated = butterfly["state"].get<mConnectivity>();
        if (gwAdminEnabled != updated) {
          gwAdminEnabled = updated;
          gwAdminSemaphore();
        }
      };
      function<void()> hotkiss = [&]() {
        gwAdminEnabled = !gwAdminEnabled
          ? mConnectivity::Connected
          : mConnectivity::Disconnected;
        gwAdminSemaphore();
      };
      mConnectivity gwSemaphore(mConnectivity *current, mConnectivity updated) {
        if (*current != updated) {
          *current = updated;
          ((SH*)screen)->gwConnectExchange =
          ((QE*)engine)->gwConnectExchange = gwConnectMarket * gwConnectOrders;
          gwAdminSemaphore();
        }
        return updated;
      };
      void gwAdminSemaphore() {
        mConnectivity updated = gwAdminEnabled * ((QE*)engine)->gwConnectExchange;
        if (((QE*)engine)->gwConnectButton != updated) {
          ((SH*)screen)->gwConnectButton =
          ((QE*)engine)->gwConnectButton = updated;
          ((SH*)screen)->log(string("GW ") + gw->name, "Quoting state changed to", string(!((QE*)engine)->gwConnectButton?"DIS":"") + "CONNECTED");
        }
        ((UI*)client)->send(mMatter::Connectivity, semaphore());
        ((SH*)screen)->refresh();
      };
      json semaphore() {
        return {
          {"state", ((QE*)engine)->gwConnectButton},
          {"status", ((QE*)engine)->gwConnectExchange}
        };
      };
      inline void timer_1s() {                                      _debugEvent_
        if (gwT_countdown and gwT_countdown-- == 1)
          gw->hub->connect(gw->ws, nullptr, {}, 5000, gw->gwGroup);
        else ((QE*)engine)->timer_1s();
        if (sync_orders and !(gwT_5m % 2))
          ((EV*)events)->async(gw->orders);
        if (sync_levels and !(gwT_5m % 3))
          ((EV*)events)->async(gw->levels);
        if (!(gwT_5m % 15))
          ((EV*)events)->async(gw->wallet);
        if (sync_trades and !(gwT_5m % 60))
          ((EV*)events)->async(gw->trades);
        if (++gwT_5m == 300) {
          gwT_5m = 0;
          if (qp->cancelOrdersAuto)
            ((EV*)events)->async(gw->cancelAll);
        }
      };
      inline void gwConnect(string reason = "") {                   _debugEvent_
        if (reason.empty())
          gwT_countdown = 1;
        else {
          gwT_countdown = 7;
          ((SH*)screen)->log(string("GW ") + gw->name, string("WS ") + reason + ", reconnecting in " + to_string(gwT_countdown) + "s.");
        }
      };
      inline void stunnel(bool reboot = false) {
        system("pkill stunnel || :");
        if (reboot) system("stunnel etc/stunnel.conf");
      };
      inline void handshake(mExchange k) {
        json reply;
        if (k == mExchange::Coinbase) {
          stunnel(true);
          gw->randId = FN::uuid36Id;
          gw->symbol = FN::S2u(string(gw->base) + "-" + gw->quote);
          reply = FN::wJet(string(gw->http) + "/products/" + gw->symbol);
          gw->minTick = stod(reply.value("quote_increment", "0"));
          gw->minSize = stod(reply.value("base_min_size", "0"));
        }
        else if (k == mExchange::HitBtc) {
          gw->randId = FN::uuid32Id;
          gw->symbol = FN::S2u(string(gw->base) + gw->quote);
          reply = FN::wJet(string(gw->http) + "/public/symbol/" + gw->symbol);
          gw->minTick = stod(reply.value("tickSize", "0"));
          gw->minSize = stod(reply.value("quantityIncrement", "0"));
          gw->base = reply.value("baseCurrency", gw->base);
          gw->quote = reply.value("quoteCurrency", gw->quote);
        }
        else if (k == mExchange::Bitfinex or k == mExchange::BitfinexMargin) {
          gw->randId = FN::int45Id;
          gw->symbol = FN::S2l(string(gw->base) + gw->quote);
          reply = FN::wJet(string(gw->http) + "/pubticker/" + gw->symbol);
          if (reply.find("last_price") != reply.end()) {
            stringstream price_;
            price_ << scientific << stod(reply.value("last_price", "0"));
            string _price_ = price_.str();
            for (string::iterator it=_price_.begin(); it!=_price_.end();)
              if (*it == '+' or *it == '-') break; else it = _price_.erase(it);
            stringstream os(string("1e") + to_string(fmax(stod(_price_),-4)-4));
            os >> gw->minTick;
          }
          reply = FN::wJet(string(gw->http) + "/symbols_details");
          if (reply.is_array())
            for (json::iterator it=reply.begin(); it!=reply.end();++it)
              if (it->find("pair") != it->end() and it->value("pair", "") == gw->symbol)
                gw->minSize = stod(it->value("minimum_order_size", "0"));
        }
        else if (k == mExchange::OkCoin or k == mExchange::OkEx) {
          gw->randId = FN::char16Id;
          gw->symbol = FN::S2l(string(gw->base) + "_" + gw->quote);
          gw->minTick = 0.0001;
          gw->minSize = 0.001;
        }
        else if (k == mExchange::Kraken) {
          gw->randId = FN::int32Id;
          gw->symbol = FN::S2u(string(gw->base) + gw->quote);
          reply = FN::wJet(string(gw->http) + "/0/public/AssetPairs?pair=" + gw->symbol);
          if (reply.find("result") != reply.end())
            for (json::iterator it = reply["result"].begin(); it != reply["result"].end(); ++it)
              if (it.value().find("pair_decimals") != it.value().end()) {
                stringstream os(string("1e-") + to_string(it.value().value("pair_decimals", 0)));
                os >> gw->minTick;
                os = stringstream(string("1e-") + to_string(it.value().value("lot_decimals", 0)));
                os >> gw->minSize;
                gw->symbol = it.key();
                gw->base = it.value().value("base", gw->base);
                gw->quote = it.value().value("quote", gw->quote);
                break;
              }
        }
        else if (k == mExchange::Korbit) {
          gw->randId = FN::int45Id;
          gw->symbol = FN::S2l(string(gw->base) + "_" + gw->quote);
          reply = FN::wJet(string(gw->http) + "/constants");
          if (reply.find(gw->symbol.substr(0,3).append("TickSize")) != reply.end()) {
            gw->minTick = reply.value(gw->symbol.substr(0,3).append("TickSize"), 0.0);
            gw->minSize = 0.015;
          }
        }
        else if (k == mExchange::Poloniex) {
          gw->randId = FN::int45Id;
          gw->symbol = FN::FN::S2u(string(gw->quote) + "_" + gw->base);
          reply = FN::wJet(string(gw->http) + "/public?command=returnTicker");
          if (reply.find(gw->symbol) != reply.end()) {
            istringstream os(string("1e-").append(to_string(6-reply[gw->symbol]["last"].get<string>().find("."))));
            os >> gw->minTick;
            gw->minSize = 0.001;
          }
        }
        else if (k == mExchange::Null) {
          gw->randId = FN::uuid36Id;
          gw->symbol = FN::FN::S2u(string(gw->base) + "_" + gw->quote);
          gw->minTick = 0.01;
          gw->minSize = 0.01;
        }
        if (!gw->minTick or !gw->minSize)
          exit(_redAlert_("CF", "Unable to fetch data from " + gw->name
            + " for symbol \"" + gw->symbol + "\", possible error message: "
            + reply.dump(),
          true));
        if (k != mExchange::Null)
          ((SH*)screen)->log(string("GW ") + gw->name, "allows client IP");
        unsigned int precision = gw->minTick < 1e-8 ? 10 : 8;
        ((SH*)screen)->log(string("GW ") + gw->name + ":", string("\n")
          + "- autoBot: " + (!gwAdminEnabled ? "no" : "yes") + '\n'
          + "- symbols: " + gw->symbol + '\n'
          + "- minTick: " + FN::strX(gw->minTick, precision) + '\n'
          + "- minSize: " + FN::strX(gw->minSize, precision) + '\n'
          + "- makeFee: " + FN::strX(gw->makeFee, precision) + '\n'
          + "- takeFee: " + FN::strX(gw->takeFee, precision));
      };
  };
}

#endif
