//! \file
//! \brief Welcome user! (just a manager of portfolios).

namespace analpaper {
  struct Settings: public Sqlite::StructBackup<Settings>,
                   public Client::Broadcast<Settings>,
                   public Client::Clickable {
    string currency = "";
      bool zeroed   = true;
    private_ref:
      const KryptoNinja &K;
    public:
      Settings(const KryptoNinja &bot)
        : StructBackup(bot)
        , Broadcast(bot)
        , Clickable(bot)
        , K(bot)
      {};
      void from_json(const json &j) {
        currency = j.value("currency", K.gateway->quote);
        zeroed   = j.value("zeroed", zeroed);
        if (currency.empty()) currency = K.gateway->quote;
        K.clicked(this);
      };
      void click(const json &j) override {
        from_json(j);
        backup();
        broadcast();
      };
      mMatter about() const override {
        return mMatter::QuotingParameters;
      };
    private:
      string explain() const override {
        return "Settings";
      };
      string explainKO() const override {
        return "using default values for %";
      };
  };
  static void to_json(json &j, const Settings &k) {
    j = {
      {"currency", k.currency},
      {  "zeroed", k.zeroed  }
    };
  };
  static void from_json(const json &j, Settings &k) {
    k.from_json(j);
  };

  struct Portfolio {
    Wallet wallet;
    unordered_map<string, Price> prices;
    unordered_map<string, pair<Price, Amount>> precisions;
    Price price;
    pair<Price, Amount> precision;
  };
  static void to_json(json &j, const Portfolio &k) {
    j = {
      {         "wallet", k.wallet          },
      {          "price", k.price           },
      { "pricePrecision", k.precision.first },
      {"amountPrecision", k.precision.second}
    };
  };

  struct Portfolios: public Client::Broadcast<Portfolios> {
    Settings settings;
    unordered_map<string, Portfolio> portfolio;
    private_ref:
      const KryptoNinja &K;
    public:
      Portfolios(const KryptoNinja &bot)
        : Broadcast(bot)
        , settings(bot)
        , K(bot)
      {};
      Price calc(const string &from) const {
        if (from == settings.currency)
          return 1;
        if (portfolio.at(from).prices.contains(settings.currency))
          return portfolio.at(from).prices.at(settings.currency);
        else for (const auto &it : portfolio.at(from).prices)
          if (portfolio.contains(it.first)) {
            if (portfolio.at(it.first).prices.contains(settings.currency))
              return it.second * portfolio.at(it.first).prices.at(settings.currency);
            else for (const auto &_it : portfolio.at(it.first).prices)
              if (portfolio.contains(_it.first)
                and portfolio.at(_it.first).prices.contains(settings.currency)
              ) return it.second * _it.second * portfolio.at(_it.first).prices.at(settings.currency);
          }
        return 0;
      };
      void calc() {
        for (auto &it : portfolio) {
          portfolio.at(it.first).wallet.value = (
            portfolio.at(it.first).price = calc(it.first)
          ) * portfolio.at(it.first).wallet.total;
          portfolio.at(it.first).precision = portfolio.at(it.first).precisions.contains(settings.currency)
            ? portfolio.at(it.first).precisions.at(settings.currency) : (pair<Price, Amount>){1e-8, 1e-8};
        }
        if (ratelimit()) return;
        broadcast();
        K.repaint(true);
      };
      bool ready() const {
        const bool err = portfolio.empty();
        if (err and Tspent > 21e+3)
          K.warn("QE", "Unable to display portfolios, missing wallet data", 3e+3);
        return !err;
      };
      mMatter about() const override {
        return mMatter::Position;
      };
    private:
      bool ratelimit() {
        return !read_soon();
      };
  };
  static void to_json(json &j, const Portfolios &k) {
    j = json::array();
    for (const auto &it : k.portfolio)
      if (it.second.price
        and (k.settings.zeroed or it.second.wallet.total)
      ) j.push_back(it.second);
  };

  struct Market {
    string web,
           base,
           quote,
           symbol;
     Price price,
           spread,
           raw_spread,
           ask,
           bid;
    double open;
    Amount volume,
           raw_volume;
  };
  static void to_json(json &j, const Market &k) {
    j = {
      {   "web", k.web   },
      {  "base", k.base  },
      { "quote", k.quote },
      {"symbol", k.symbol},
      { "price", k.price },
      {"spread", k.spread},
      {   "ask", k.ask   },
      {   "bid", k.bid   },
      {  "open", k.open  },
      {"volume", k.volume}
    };
  };

  struct Markets: public Client::Broadcast<Markets> {
    unordered_map<string, unordered_map<string, Market>> market;
    private_ref:
      const KryptoNinja &K;
    public:
      Markets(const KryptoNinja &bot)
        : Broadcast(bot)
        , K(bot)
      {};
      void add(const Ticker &raw) {
        market[raw.base][raw.quote] = {
          K.gateway->web(raw.base, raw.quote),
          raw.base,
          raw.quote,
          raw.symbol,
          raw.price,
          0,
          raw.spread,
          raw.bestask,
          raw.bestbid,
          raw.open,
          0,
          raw.volume
        };
      };
      void calc(const string &base, const string &quote, const Price &volume, const Price &spread) {
        market[base][quote].volume = volume;
        market[base][quote].spread = spread;
        if (ratelimit()) return;
        broadcast();
      };
      mMatter about() const override {
        return mMatter::MarketData;
      };
    private:
      bool ratelimit() {
        return !read_soon(1e+3);
      };
  };
  static void to_json(json &j, const Markets &k) {
    j = k.market;
  };

  struct Tickers: public Client::Clicked {
    Markets markets;
    private_ref:
      const KryptoNinja &K;
            Portfolios  &portfolios;
    public:
      Tickers(const KryptoNinja &bot, Portfolios &p)
        : Clicked(bot, {
            {&p.settings, [&]() { calc(); }}
          })
        , markets(bot)
        , K(bot)
        , portfolios(p)
      {};
      void read_from_gw(const Ticker &raw) {
        Price  pricePrecision  = 1e-8;
        Amount amountPrecision = 1e-8;
        if (K.gateway->precisions.contains(raw.symbol)) {
          pricePrecision  = K.gateway->precisions.at(raw.symbol).first;
          amountPrecision = K.gateway->precisions.at(raw.symbol).second;
        }
        add(raw.base,  raw.quote,     raw.price, pricePrecision, amountPrecision);
        add(raw.quote, raw.base,  1 / raw.price, amountPrecision, pricePrecision);
        portfolios.calc();
        markets.add(raw);
        markets.calc(
          raw.base,
          raw.quote,
          raw.volume * portfolios.portfolio.at(raw.base).price,
          raw.spread * portfolios.portfolio.at(raw.quote).price
        );
      };
    private:
      void calc() {
        portfolios.calc();
        for (auto &it : markets.market)
          for (auto &_it : it.second) {
            markets.calc(
              it.first,
              _it.first,
              _it.second.raw_volume * portfolios.portfolio.at(it.first).price,
              _it.second.raw_spread * portfolios.portfolio.at(_it.first).price
            );
          }
      };
      void add(const string &base, const string &quote, const Price &price, const Price &pricePrecision, const Amount &amountPrecision) {
        portfolios.portfolio[base].prices[quote] = price;
        portfolios.portfolio[base].precisions[quote] = {pricePrecision, amountPrecision};
        if (portfolios.portfolio.at(base).wallet.currency.empty())
          portfolios.portfolio.at(base).wallet.currency = base;
      };
  };

  struct Wallets {
    private_ref:
      Portfolios &portfolios;
    public:
      Wallets(Portfolios &p)
        : portfolios(p)
      {};
      void read_from_gw(const Wallet &raw) {
        portfolios.portfolio[raw.currency].wallet = raw;
        portfolios.calc();
      };
  };

  struct Orders: public System::Orderbook,
                 public Client::Broadcast<Orders> {
    private_ref:
      const KryptoNinja &K;
    public:
      Orders(const KryptoNinja &bot)
        : Orderbook(bot)
        , Broadcast(bot)
        , K(bot)
      {
        withExternal = true;
      };
      void read_from_gw(const Order &) {
        broadcast();
      };
      mMatter about() const override {
        return mMatter::OrderStatusReports;
      };
      bool realtime() const override {
        return false;
      };
      json blob() const override {
        return working(false);
      };
  };
  static void to_json(json &j, const Orders &k) {
    j = k.blob();
  };

  struct Semaphore: public Client::Broadcast<Semaphore>,
                    public Hotkey::Keymap {
    Connectivity greenGateway = Connectivity::Disconnected;
    private_ref:
      const KryptoNinja &K;
    public:
      Semaphore(const KryptoNinja &bot)
        : Broadcast(bot)
        , Keymap(bot, {
            {'Q',  [&]() { exit(); }},
            {'q',  [&]() { exit(); }},
            {'\e', [&]() { exit(); }}
          })
        , K(bot)
      {};
      bool online() const {
        return (bool)greenGateway;
      };
      void read_from_gw(const Connectivity &raw) {
        greenGateway = raw;
        broadcast();
        K.repaint();
      };
      mMatter about() const override {
        return mMatter::Connectivity;
      };
  };
  static void to_json(json &j, const Semaphore &k) {
    j = {
      {"online", k.greenGateway}
    };
  };

  struct Product: public Client::Broadcast<Product> {
    private_ref:
      const KryptoNinja &K;
    public:
      Product(const KryptoNinja &bot)
        : Broadcast(bot)
        , K(bot)
      {};
      json to_json() const {
        return {
          {   "exchange", K.gateway->exchange                         },
          {       "base", K.gateway->base                             },
          {      "quote", K.gateway->quote                            },
          {     "symbol", K.gateway->symbol                           },
          {  "webMarket", K.gateway->web()                            },
          {  "webOrders", K.gateway->web(true)                        },
          {  "tickPrice", K.gateway->decimal.price.stream.precision() },
          {   "tickSize", K.gateway->decimal.amount.stream.precision()},
          {  "stepPrice", K.gateway->decimal.price.step               },
          {   "stepSize", K.gateway->decimal.amount.step              },
          {    "minSize", K.gateway->minSize                          },
          {       "inet", K.arg<string>("interface")                  },
          {"environment", K.arg<string>("title")                      },
          { "matryoshka", K.arg<string>("matryoshka")                 },
          {     "source", K_SOURCE " " K_BUILD                        }
        };
      };
      mMatter about() const override {
        return mMatter::ProductAdvertisement;
      };
  };
  static void to_json(json &j, const Product &k) {
    j = k.to_json();
  };

  struct Memory: public Client::Broadcast<Memory> {
    public:
      unsigned int orders_60s = 0;
    private:
      Product product;
    private_ref:
      const KryptoNinja &K;
    public:
      Memory(const KryptoNinja &bot)
        : Broadcast(bot)
        , product(bot)
        , K(bot)
      {};
      void timer_60s() {
        broadcast();
        orders_60s = 0;
      };
      json to_json() const {
        return {
          {  "addr", K.gateway->unlock           },
          {  "freq", orders_60s                  },
          { "theme", K.arg<int>("ignore-moon")
                       + K.arg<int>("ignore-sun")},
          {"memory", K.memSize()                 },
          {"dbsize", K.dbSize()                  }
        };
      };
      mMatter about() const override {
        return mMatter::ApplicationState;
      };
  };
  static void to_json(json &j, const Memory &k) {
    j = k.to_json();
  };

  struct ButtonSubmitNewOrder: public Client::Clickable {
    private_ref:
      const KryptoNinja &K;
    public:
      ButtonSubmitNewOrder(const KryptoNinja &bot)
        : Clickable(bot)
        , K(bot)
      {};
      void click(const json &j) override {
        if (j.is_object()
          and !j.value("symbol", "").empty()
          and j.value("price", 0.0)
          and j.value("quantity", 0.0)
        ) {
          json order = j;
          order["manual"]  = true;
          order["orderId"] = K.gateway->randId();
          K.clicked(this, order);
        }
      };
      mMatter about() const override {
        return mMatter::SubmitNewOrder;
      };
  };
  struct ButtonCancelOrder: public Client::Clickable {
    private_ref:
      const KryptoNinja &K;
    public:
      ButtonCancelOrder(const KryptoNinja &bot)
        : Clickable(bot)
        , K(bot)
      {};
      void click(const json &j) override {
        if (j.is_object() and !j.value("orderId", "").empty())
          K.clicked(this, j.at("orderId").get<string>());
      };
      mMatter about() const override {
        return mMatter::CancelOrder;
      };
  };
  struct InputEditOrder: public Client::Clickable {
    private_ref:
      const KryptoNinja          &K;
            ButtonSubmitNewOrder &submit;
            ButtonCancelOrder    &cancel;
    public:
      InputEditOrder(const KryptoNinja &bot, ButtonSubmitNewOrder &s, ButtonCancelOrder &c)
        : Clickable(bot)
        , K(bot)
        , submit(s)
        , cancel(c)
      {};
      void click(const json &j) override {
        cancel.click(j);
        submit.click(j);
      };
      mMatter about() const override {
        return mMatter::EditOrder;
      };
  };

  struct Buttons {
    ButtonSubmitNewOrder submit;
    ButtonCancelOrder    cancel;
    InputEditOrder       edit;
    Buttons(const KryptoNinja &bot)
      : submit(bot)
      , cancel(bot)
      , edit(bot, submit, cancel)
    {};
  };

  struct Broker: public Client::Clicked {
          Memory memory;
       Semaphore semaphore;
    private_ref:
      const KryptoNinja &K;
    public:
      Broker(const KryptoNinja &bot, const Buttons &b)
        : Clicked(bot, {
            {&b.submit, [&](const json &j) { K.place(j); }},
            {&b.cancel, [&](const json &j) { K.cancel(j); }}
          })
        , memory(bot)
        , semaphore(bot)
        , K(bot)
      {};
      bool ready() const {
        return semaphore.online();
      };
  };

  class Engine {
    public:
      Portfolios portfolios;
         Tickers ticker;
         Wallets wallet;
         Buttons button;
          Orders orders;
          Broker broker;
    public:
      Engine(const KryptoNinja &bot)
        : portfolios(bot)
        , ticker(bot, portfolios)
        , wallet(portfolios)
        , button(bot)
        , orders(bot)
        , broker(bot, button)
      {};
      void read(const Connectivity &rawdata) {
        broker.semaphore.read_from_gw(rawdata);
      };
      void read(const Ticker &rawdata) {
        ticker.read_from_gw(rawdata);
      };
      void read(const Wallet &rawdata) {
        wallet.read_from_gw(rawdata);
      };
      void read(const Order &rawdata) {
        orders.read_from_gw(rawdata);
      };
      void timer_1s(const unsigned int &tick) {
        if (!(tick % 60))
          broker.memory.timer_60s();
      };
  };
}
