#ifndef K_GW_H_
#define K_GW_H_

namespace K {
  class GW: public Klass {
    private:
      mConnectivity gwAutoStart = mConnectivity::Disconnected,
                    gwQuotingState = mConnectivity::Disconnected,
                    gwConnectOrder = mConnectivity::Disconnected,
                    gwConnectMarket = mConnectivity::Disconnected,
                    gwConnectExchange = mConnectivity::Disconnected;
      unsigned int gwT_5m = 0;
    protected:
      void load() {
        gwEndings.back() = &happyEnding;
        if (((CF*)config)->argAutobot) gwAutoStart = mConnectivity::Connected;
        handshake(gw->exchange);
      };
      void waitTime() {
        ((EV*)events)->tServer->setData(this);
        ((EV*)events)->tServer->start([](Timer *handle) {
          GW *k = (GW*)handle->data;
          ((EV*)k->events)->debug("GW tServer timer");
          k->gw->wallet();
          if (k->qp->cancelOrdersAuto) {
            if (!k->gwT_5m++) k->gw->cancelAll();
            else if (k->gwT_5m == 20) k->gwT_5m = 0;
          }
        }, 0, 15e+3);
      };
      void waitData() {
        gw->evConnectOrder = [&](mConnectivity k) {
          serverSemaphore(mGatewayType::OrderEntry, k);
        };
        gw->evConnectMarket = [&](mConnectivity k) {
          serverSemaphore(mGatewayType::MarketData, k);
          if (k == mConnectivity::Disconnected)
            gw->evDataLevels(mLevels());
        };
        gw->levels();
      };
      void waitUser() {
        ((UI*)client)->welcome(uiTXT::ProductAdvertisement, &helloProduct);
        ((UI*)client)->welcome(uiTXT::ExchangeConnectivity, &helloStatus);
        ((UI*)client)->welcome(uiTXT::ActiveState, &helloState);
        ((UI*)client)->clickme(uiTXT::ActiveState, &kissState);
      };
      void run() {
        ((EV*)events)->start();
      };
    private:
      function<void()> happyEnding = [&]() {
        ((EV*)events)->stop([&](){
          FN::log(string("GW ") + gw->name, "Attempting to cancel all open orders, please wait.");
          gw->cancelAll();
          FN::log(string("GW ") + gw->name, "cancell all open orders OK");
        });
      };
      function<json()> helloProduct = [&]() {
        return (json){{
          {"exchange", (int)gw->exchange},
          {"pair", {{"base", gw->base}, {"quote", gw->quote}}},
          {"minTick", gw->minTick},
          {"environment", ((CF*)config)->argTitle},
          {"matryoshka", ((CF*)config)->argMatryoshka},
          {"homepage", "https://github.com/ctubio/Krypto-trading-bot"}
        }};
      };
      function<json()> helloStatus = [&]() {
        return (json){{{"status", (int)gwConnectExchange}}};
      };
      function<json()> helloState = [&]() {
        return (json){{{"state",  (int)gwQuotingState}}};
      };
      function<void(json)> kissState = [&](json k) {
        if (!k.is_object() or !k["state"].is_number()) {
          FN::logWar("JSON", "Missing state at kissState, ignored");
          return;
        }
        mConnectivity autoStart = (mConnectivity)k["state"].get<int>();
        if (autoStart != gwAutoStart) {
          gwAutoStart = autoStart;
          clientSemaphore();
        }
      };
      void serverSemaphore(mGatewayType gwT, mConnectivity gwS) {
        if (gwT == mGatewayType::MarketData) {
          if (gwConnectMarket == gwS) return;
          gwConnectMarket = gwS;
        } else if (gwT == mGatewayType::OrderEntry) {
          if (gwConnectOrder == gwS) return;
          gwConnectOrder = gwS;
        }
        gwConnectExchange = gwConnectMarket == mConnectivity::Connected and gwConnectOrder == mConnectivity::Connected
          ? mConnectivity::Connected : mConnectivity::Disconnected;
        clientSemaphore();
        ((UI*)client)->send(uiTXT::ExchangeConnectivity, {{"status", (int)gwConnectExchange}});
      };
      void clientSemaphore() {
        mConnectivity quotingState = gwConnectExchange;
        if (quotingState == mConnectivity::Connected) quotingState = gwAutoStart;
        if (quotingState != gwQuotingState) {
          gwQuotingState = quotingState;
          FN::log(string("GW ") + gw->name, "Quoting state changed to", gwQuotingState == mConnectivity::Connected ? "CONNECTED" : "DISCONNECTED");
          ((UI*)client)->send(uiTXT::ActiveState, {{"state", (int)gwQuotingState}});
        }
        ((QE*)engine)->gwConnectButton = gwQuotingState;
        ((QE*)engine)->gwConnectExchange = gwConnectExchange;
      };
      void handshake(mExchange e) {
        json reply;
        if (e == mExchange::Coinbase) {
          FN::stunnel();
          gw->randId = FN::uuidId;
          gw->symbol = FN::S2u(string(gw->base) + "-" + gw->quote);
          reply = FN::wJet(string(gw->http) + "/products/" + gw->symbol);
          gw->minTick = stod(reply.value("quote_increment", "0"));
          gw->minSize = stod(reply.value("base_min_size", "0"));
        }
        else if (e == mExchange::HitBtc) {
          gw->randId = FN::charId;
          gw->symbol = FN::S2u(string(gw->base) + gw->quote);
          reply = FN::wJet(string(gw->http) + "/public/symbol/" + gw->symbol);
          gw->minTick = stod(reply.value("tickSize", "0"));
          gw->minSize = stod(reply.value("quantityIncrement", "0"));
        }
        else if (e == mExchange::Bitfinex) {
          gw->randId = FN::int64Id;
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
        else if (e == mExchange::OkCoin or e == mExchange::OkEx) {
          gw->randId = FN::charId;
          gw->symbol = FN::S2l(string(gw->base) + "_" + gw->quote);
          gw->minTick = 0.001;
          gw->minSize = 0.001;
        }
        else if (e == mExchange::Kraken) {
          gw->randId = FN::int64Id;
          gw->symbol = FN::S2u(string(gw->base) + gw->quote);
          reply = FN::wJet(string(gw->http) + "/0/public/AssetPairs?pair=" + gw->symbol);
          if (reply.find("result") != reply.end())
            for (json::iterator it = reply["result"].begin(); it != reply["result"].end(); ++it) {
              if (it.value().find("pair_decimals") != it.value().end()) {
                stringstream os(string("1e-") + to_string(it.value().value("pair_decimals", 0)));
                os >> gw->minTick;
                gw->symbol = it.key();
                gw->base = it.value().value("base", gw->base);
                gw->quote = it.value().value("quote", gw->quote);
              }
              break;
            }
          gw->minSize = 0.01;
        }
        else if (e == mExchange::Korbit) {
          gw->randId = FN::int64Id;
          gw->symbol = FN::S2l(string(gw->base) + "_" + gw->quote);
          reply = FN::wJet(string(gw->http) + "/constants");
          if (reply.find(gw->symbol.substr(0,3).append("TickSize")) != reply.end()) {
            gw->minTick = reply.value(gw->symbol.substr(0,3).append("TickSize"), 0.0);
            gw->minSize = 0.015;
          }
        }
        else if (e == mExchange::Poloniex) {
          gw->randId = FN::int64Id;
          gw->symbol = FN::FN::S2u(string(gw->base) + "_" + gw->quote);
          reply = FN::wJet(string(gw->http) + "/public?command=returnTicker");
          if (reply.find(gw->symbol) != reply.end()) {
            istringstream os(string("1e-").append(to_string(6-reply[gw->symbol]["last"].get<string>().find("."))));
            os >> gw->minTick;
            gw->minSize = 0.01;
          }
        }
        else if (e == mExchange::Null) {
          gw->randId = FN::int64Id;
          gw->symbol = FN::FN::S2u(string(gw->base) + "_" + gw->quote);
          gw->minTick = 0.01;
          gw->minSize = 0.01;
        }
        if (gw->minTick and gw->minSize) {
          FN::log(string("GW ") + gw->name, "allows client IP");
          stringstream ss;
          ss << setprecision(8) << fixed << '\n'
            << "- autoBot: " << (((CF*)config)->argAutobot ? "yes" : "no") << '\n'
            << "- symbols: " << gw->symbol << '\n'
            << "- minTick: " << gw->minTick << '\n'
            << "- minSize: " << gw->minSize << '\n'
            << "- makeFee: " << gw->makeFee << '\n'
            << "- takeFee: " << gw->takeFee;
          FN::log(string("GW ") + gw->name + ":", ss.str());
        } else FN::logExit("CF", "Unable to fetch data from " + gw->name + " symbol \"" + gw->symbol + "\", possible error message: " + reply.dump(), EXIT_FAILURE);
      };
  };
}

#endif
