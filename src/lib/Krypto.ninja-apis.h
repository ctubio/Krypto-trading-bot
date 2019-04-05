#ifndef K_APIS_H_
#define K_APIS_H_
//! \file
//! \brief External exchange API integrations.

namespace ₿ {
  enum class Connectivity: unsigned int { Disconnected, Connected };
  enum class       Status: unsigned int { WaitingToWork, Working, Terminated, WaitingToTerminate, Rejected };
  enum class         Side: unsigned int { Bid, Ask };
  enum class  TimeInForce: unsigned int { GTC, IOC, FOK };
  enum class    OrderType: unsigned int { Limit, Market };

  struct mLevel {
     Price price;
    Amount size;
  };
  static void to_json(json &j, const mLevel &k) {
    j = {
      {"price", k.price}
    };
    if (k.size) j["size"] = k.size;
  };
  struct mLevels {
    vector<mLevel> bids,
                   asks;
  };
  static void to_json(json &j, const mLevels &k) {
    j = {
      {"bids", k.bids},
      {"asks", k.asks}
    };
  };

  struct mWallet {
    CoinId currency;
    Amount amount,
           held,
           total,
           value;
    double profit;
    static void reset(const Amount &a, const Amount &h, mWallet *const wallet) {
      wallet->total = (wallet->amount = a)
                    + (wallet->held   = h);
    };
  };
  static void to_json(json &j, const mWallet &k) {
    j = {
      {"amount", k.amount},
      {  "held", k.held  },
      { "value", k.value },
      {"profit", k.profit}
    };
  };
  struct mWallets {
    mWallet base,
            quote;
  };
  static void to_json(json &j, const mWallets &k) {
    j = {
      { "base", k.base },
      {"quote", k.quote}
    };
  };

  struct mTrade {
      Side side;
     Price price;
    Amount quantity;
     Clock time;
  };
  static void to_json(json &j, const mTrade &k) {
    j = {
      {    "side", k.side    },
      {   "price", k.price   },
      {"quantity", k.quantity},
      {    "time", k.time    }
    };
  };

  struct mOrder: public mTrade {
           bool isPong;
         RandId orderId,
                exchangeId;
         Status status;
         Amount tradeQuantity;
      OrderType type;
    TimeInForce timeInForce;
           bool disablePostOnly;
          Clock latency;
    static void update(const mOrder &raw, mOrder *const order) {
      if (!order) return;
      if (Status::Terminated ==                        raw.status
        and                        order->status    == Status::WaitingToWork
      )                            order->status     = Status::Rejected;
      else if (Status::Working == (order->status     = raw.status
      ) and !order->latency)       order->latency    = raw.time - order->time;
                                   order->time       = raw.time;
      if (!raw.exchangeId.empty()) order->exchangeId = raw.exchangeId;
      if (raw.price)               order->price      = raw.price;
      if (raw.quantity)            order->quantity   = raw.quantity;
    };
    static const bool replace(const Price &price, const bool &isPong, mOrder *const order) {
      if (!order
        or order->exchangeId.empty()
      ) return false;
      order->price  = price;
      order->isPong = isPong;
      order->time   = Tstamp;
      return true;
    };
    static const bool cancel(mOrder *const order) {
      if (!order
        or order->exchangeId.empty()
        or order->status == Status::WaitingToWork
        or order->status == Status::WaitingToTerminate
      ) return false;
      order->status = Status::WaitingToTerminate;
      order->time   = Tstamp;
      return true;
    };
  };
  static void to_json(json &j, const mOrder &k) {
    j = {
      {        "orderId", k.orderId        },
      {     "exchangeId", k.exchangeId     },
      {           "side", k.side           },
      {       "quantity", k.quantity       },
      {           "type", k.type           },
      {         "isPong", k.isPong         },
      {          "price", k.price          },
      {    "timeInForce", k.timeInForce    },
      {         "status", k.status         },
      {"disablePostOnly", k.disablePostOnly},
      {           "time", k.time           },
      {        "latency", k.latency        }
    };
  };
  static void from_json(const json &j, mOrder &k) {
    k.price           = j.value("price", 0.0);
    k.quantity        = j.value("quantity", 0.0);
    k.side            = j.value("side", "") == "Bid"
                          ? Side::Bid
                          : Side::Ask;
    k.type            = j.value("type", "") == "Limit"
                          ? OrderType::Limit
                          : OrderType::Market;
    k.timeInForce     = j.value("timeInForce", "") == "GTC"
                          ? TimeInForce::GTC
                          : (j.value("timeInForce", "") == "FOK"
                            ? TimeInForce::FOK
                            : TimeInForce::IOC);
    k.isPong          = false;
    k.disablePostOnly = true;
  };

  class Curl {
    private:
      static mutex waiting_reply;
    public:
      static const char *inet;
      static const json xfer(const string &url, const long &timeout = 13) {
        return perform(url, [&](CURL *curl) {
          curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        }, timeout == 13);
      };
      static const json xfer(const string &url, const string &post) {
        return perform(url, [&](CURL *curl) {
          struct curl_slist *h_ = nullptr;
          h_ = curl_slist_append(h_, "Content-Type: application/x-www-form-urlencoded");
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h_);
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.data());
        });
      };
      static const json perform(const string &url, function<void(CURL *curl)> setopt, bool debug = true) {
        lock_guard<mutex> lock(waiting_reply);
        string reply;
        CURLcode res = CURLE_FAILED_INIT;
        CURL *curl = curl_easy_init();
        if (curl) {
          setopt(curl);
          curl_easy_setopt(curl, CURLOPT_USERAGENT, "K");
          curl_easy_setopt(curl, CURLOPT_INTERFACE, inet);
          curl_easy_setopt(curl, CURLOPT_URL, url.data());
          curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write);
          curl_easy_setopt(curl, CURLOPT_WRITEDATA, &reply);
          res = curl_easy_perform(curl);
          curl_easy_cleanup(curl);
        }
        return (debug and res != CURLE_OK)
          ? (json){ {"error", string("CURL Error: ") + curl_easy_strerror(res)} }
          : (json::accept(reply)
              ? json::parse(reply)
              : json::object()
            );
      };
    private:
      static size_t write(void *buf, size_t size, size_t nmemb, void *reply) {
        ((string*)reply)->append((char*)buf, size *= nmemb);
        return size;
      };
  };

  class Text {
    public:
      static string strL(string input) {
        transform(input.begin(), input.end(), input.begin(), ::tolower);
        return input;
      };
      static string strU(string input) {
        transform(input.begin(), input.end(), input.begin(), ::toupper);
        return input;
      };
      static string B64(const string &input) {
        BIO *bio, *b64;
        BUF_MEM *bufferPtr;
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);
        BIO_set_close(bio, BIO_CLOSE);
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, input.data(), input.length());
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);
        const string output(bufferPtr->data, bufferPtr->length);
        BIO_free_all(bio);
        return output;
      };
      static string B64_decode(const string &input) {
        BIO *bio, *b64;
        char output[input.length()];
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new_mem_buf(input.data(), input.length());
        bio = BIO_push(b64, bio);
        BIO_set_close(bio, BIO_CLOSE);
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        int len = BIO_read(bio, output, input.length());
        BIO_free_all(bio);
        return string(output, len);
      };
      static string SHA256(const string &input, const bool &hex = false) {
        return SHA(input, hex, ::SHA256, SHA256_DIGEST_LENGTH);
      };
      static string SHA512(const string &input) {
        return SHA(input, false, ::SHA512, SHA512_DIGEST_LENGTH);
      };
      static string HMAC1(const string &key, const string &input, const bool &hex = false) {
        return HMAC(key, input, hex, EVP_sha1, SHA_DIGEST_LENGTH);
      };
      static string HMAC256(const string &key, const string &input, const bool &hex = false) {
        return HMAC(key, input, hex, EVP_sha256, SHA256_DIGEST_LENGTH);
      };
      static string HMAC512(const string &key, const string &input, const bool &hex = false) {
        return HMAC(key, input, hex, EVP_sha512, SHA512_DIGEST_LENGTH);
      };
      static string HMAC384(const string &key, const string &input, const bool &hex = false) {
        return HMAC(key, input, hex, EVP_sha384, SHA384_DIGEST_LENGTH);
      };
    private:
      static string SHA(
        const string  &input,
        const bool    &hex,
        unsigned char *(*md)(const unsigned char*, size_t, unsigned char*),
        const int     &digest_len
      ) {
        unsigned char digest[digest_len];
        md((unsigned char*)input.data(), input.length(), (unsigned char*)&digest);
        char output[digest_len * 2 + 1];
        for (unsigned int i = 0; i < digest_len; i++)
          sprintf(&output[i * 2], "%02x", (unsigned int)digest[i]);
        return hex ? HEX(output) : output;
      };
      static string HMAC(
        const string &key,
        const string &input,
        const bool   &hex,
        const EVP_MD *(evp_md)(),
        const int    &digest_len
      ) {
        unsigned char* digest;
        digest = ::HMAC(
          evp_md(),
          input.data(), input.length(),
          (unsigned char*)key.data(), key.length(),
          nullptr, nullptr
        );
        char output[digest_len * 2 + 1];
        for (unsigned int i = 0; i < digest_len; i++)
          sprintf(&output[i * 2], "%02x", (unsigned int)digest[i]);
        return hex ? HEX(output) : output;
      };
      static string HEX(const string &input) {
        const unsigned int len = input.length();
        string output;
        for (unsigned int i = 0; i < len; i += 2)
          output.push_back(
            (char)(int)strtol(input.substr(i, 2).data(), nullptr, 16)
          );
        return output;
      };
  };

  class Random {
    public:
      static const unsigned long long int64() {
        static random_device rd;
        static mt19937_64 gen(rd());
        return uniform_int_distribution<unsigned long long>()(gen);
      };
      static const RandId int45Id() {
        return to_string(int64()).substr(0, 10);
      };
      static const RandId int32Id() {
        return to_string(int64()).substr(0,  8);
      };
      static const RandId char16Id() {
        string id = string(16, ' ');
        for (auto &it : id) {
         const int offset = int64() % (26 + 26 + 10);
         if (offset < 26)           it = 'a' + offset;
         else if (offset < 26 + 26) it = 'A' + offset - 26;
         else                       it = '0' + offset - 26 - 26;
        }
        return id;
      };
      static const RandId uuid36Id() {
        string uuid = string(36, ' ');
        uuid[8]  =
        uuid[13] =
        uuid[18] =
        uuid[23] = '-';
        uuid[14] = '4';
        unsigned long long rnd = int64();
        for (auto &it : uuid)
          if (it == ' ') {
            if (rnd <= 0x02) rnd = 0x2000000 + (int64() * 0x1000000) | 0;
            rnd >>= 4;
            const int offset = (uuid[17] != ' ' and uuid[19] == ' ')
              ? ((rnd & 0xf) & 0x3) | 0x8
              : rnd & 0xf;
            if (offset < 10) it = '0' + offset;
            else             it = 'a' + offset - 10;
          }
        return uuid;
      };
      static const RandId uuid32Id() {
        RandId uuid = uuid36Id();
        uuid.erase(remove(uuid.begin(), uuid.end(), '-'), uuid.end());
        return uuid;
      }
  };

  class GwExchangeData {
    public_friend:
      class Decimal {
        public:
          stringstream stream;
        public:
          Decimal()
          {
            stream << fixed;
          };
          const double truncate(const double &input) const {
            const double points = pow(10, -1 * stream.precision());
            return floor(input / points) * points;
          };
          const string str(const double &input) {
            stream.str("");
            stream << truncate(input);
            return stream.str();
          };
      };
    public:
      struct {
        Decimal price,
                amount,
                percent;
      } decimal;
      uWS::Group<uWS::CLIENT> *api = nullptr;
      function<void(const mOrder&)>       write_mOrder;
      function<void(const mTrade&)>       write_mTrade;
      function<void(const mLevels&)>      write_mLevels;
      function<void(const mWallets&)>     write_mWallets;
      function<void(const Connectivity&)> write_Connectivity;
      bool askForFees      = false,
           askForReplace   = false,
           askForCancelAll = false;
      const RandId (*randId)() = nullptr;
      virtual const bool askForData(const unsigned int &tick) = 0;
      virtual const bool waitForData() = 0;
      void place(const mOrder *const order) {
        place(
          order->orderId,
          order->side,
          decimal.price.str(order->price),
          decimal.amount.str(order->quantity),
          order->type,
          order->timeInForce,
          order->disablePostOnly
        );
      };
      void replace(const mOrder *const order) {
        replace(
          order->exchangeId,
          decimal.price.str(order->price)
        );
      };
      void cancel(const mOrder *const order) {
        cancel(
          order->orderId,
          order->exchangeId
        );
      };
//BO non-free Gw library functions from build-*/local/lib/K-*.a (it just redefines all virtual gateway class members below).
/**/  virtual bool ready(uS::Loop *const) = 0;                               // wait for exchange and register data handlers
/**/  virtual void replace(RandId, string) {};                               // call         async orders data from exchange
/**/  virtual void place(RandId, Side, string, string, OrderType, TimeInForce, bool) = 0, // async orders like above / below
/**/               cancel(RandId, RandId) = 0,                               // call         async orders data from exchange
/**/               close() = 0;                                              // disconnect but without waiting for reconnect
/**/protected:
/**/  virtual bool            async_wallet() { return false; };              // call         async wallet data from exchange
/**/  virtual vector<mWallets> sync_wallet() { return {}; };                 // call and read sync wallet data from exchange
/**/  virtual vector<mLevels>  sync_levels() { return {}; };                 // call and read sync levels data from exchange
/**/  virtual vector<mTrade>   sync_trades() { return {}; };                 // call and read sync trades data from exchange
/**/  virtual vector<mOrder>   sync_orders() { return {}; };                 // call and read sync orders data from exchange
/**/  virtual vector<mOrder>   sync_cancelAll() = 0;                         // call and read sync orders data from exchange
//EO non-free Gw library functions from build-*/local/lib/K-*.a (it just redefines all virtual gateway class members above).
      future<vector<mWallets>> replyWallets;
      future<vector<mLevels>> replyLevels;
      future<vector<mTrade>> replyTrades;
      future<vector<mOrder>> replyOrders;
      future<vector<mOrder>> replyCancelAll;
      const bool askForNeverAsyncData(const unsigned int &tick) {
        bool waiting = false;
        if ((askForFees
          and !(askForFees = false)
          ) or !(tick % 15)) waiting |= !(async_wallet() or !askFor(replyWallets, [&]() { return sync_wallet(); }));
        if (askForCancelAll
          and !(tick % 300)) waiting |= askFor(replyCancelAll, [&]() { return sync_cancelAll(); });
        return waiting;
      };
      const bool askForSyncData(const unsigned int &tick) {
        bool waiting = false;
        if (!(tick % 2))     waiting |= askFor(replyOrders, [&]() { return sync_orders(); });
                             waiting |= askForNeverAsyncData(tick);
        if (!(tick % 3))     waiting |= askFor(replyLevels, [&]() { return sync_levels(); });
        if (!(tick % 60))    waiting |= askFor(replyTrades, [&]() { return sync_trades(); });
        return waiting;
      };
      const bool waitForNeverAsyncData() {
        return waitFor(replyWallets,   write_mWallets)
             | waitFor(replyCancelAll, write_mOrder);
      };
      const bool waitForSyncData() {
        return waitFor(replyOrders,    write_mOrder)
             | waitForNeverAsyncData()
             | waitFor(replyLevels,    write_mLevels)
             | waitFor(replyTrades,    write_mTrade);
      };
      template<typename T1, typename T2> const bool askFor(
              future<vector<T1>> &reply,
        const T2                 &read
      ) {
        bool waiting = reply.valid();
        if (!waiting) {
          reply = ::async(launch::async, read);
          waiting = true;
        }
        return waiting;
      };
      template<typename T> const unsigned int waitFor(
              future<vector<T>>        &reply,
        const function<void(const T&)> &write
      ) {
        bool waiting = reply.valid();
        if (waiting and reply.wait_for(chrono::nanoseconds(0)) == future_status::ready) {
          for (T &it : reply.get()) write(it);
          waiting = false;
        }
        return waiting;
      };
  };

  class GwExchange: public GwExchangeData {
    public:
      using Report = vector<pair<string, string>>;
      unsigned int countdown = 0;
        string exchange, apikey,
               secret,   pass,
               http,     ws,
               fix,      unlock;
        CoinId base,     quote;
           int version  = 0,
               maxLevel = 0,
               debug    = 0;
         Price minTick  = 0;
        Amount minSize  = 0,
               makeFee  = 0,
               takeFee  = 0;
      virtual const json handshake() = 0;
      const json handshake(const bool &nocache) {
        json reply;
        const string cache = "/var/lib/K/cache/handshake"
              + ('.' + exchange)
              +  '.' + base
              +  '.' + quote
              +  '.' + "json";
        fstream file;
        struct stat st;
        if (!nocache
          and access(cache.data(), R_OK) != -1
          and !stat(cache.data(), &st)
          and Tstamp - 25200e+3 < st.st_mtime * 1e+3
        ) {
          file.open(cache, fstream::in);
          reply = json::parse(file);
        } else
          reply = handshake();
        minTick = reply.value("minTick", 0.0);
        minSize = reply.value("minSize", 0.0);
        makeFee = reply.value("makeFee", 0.0);
        takeFee = reply.value("takeFee", 0.0);
        if (!file.is_open() and minTick and minSize) {
          file.open(cache, fstream::out | fstream::trunc);
          file << reply.dump();
        }
        if (file.is_open()) file.close();
        return reply.value("reply", json::object());
      };
      void end(const bool &dustybot = false) {
        if (dustybot)
          log("--dustybot is enabled, remember to cancel manually any open order.");
        else if (write_mOrder) {
          log("Attempting to cancel all open orders, please wait.");
          for (mOrder &it : sync_cancelAll()) write_mOrder(it);
          log("cancel all open orders OK");
        }
        close();
        api->close();
      };
      void report(Report notes, const bool &nocache) {
        decimal.price.stream.precision(abs(log10(minTick)));
        decimal.amount.stream.precision(minTick < 1e-8 ? 10 : 8);
        decimal.percent.stream.precision(2);
        for (auto it : (Report){
          {"symbols", base + "/" + quote},
          {"minTick", decimal.amount.str(minTick)              },
          {"minSize", decimal.amount.str(minSize)              },
          {"makeFee", decimal.percent.str(makeFee * 1e+2) + "%"},
          {"takeFee", decimal.percent.str(takeFee * 1e+2) + "%"}
        }) notes.push_back(it);
        string note = "handshake:";
        for (auto &it : notes)
          if (!it.second.empty())
            note += "\n- " + it.first + ": " + it.second;
        log((nocache ? "" : "cached ") + note);
      };
      void latency(const string &reason, const function<void()> &fn) {
        log("latency check", "start");
        const Clock Tstart = Tstamp;
        fn();
        const Clock Tstop  = Tstamp;
        log("latency check", "stop");
        const unsigned int Tdiff = Tstop - Tstart;
        log(reason + " took", to_string(Tdiff) + "ms of your time");
        string result = "This result is ";
        if      (Tdiff < 2e+2) result += "very good; most traders don't enjoy such speed!";
        else if (Tdiff < 5e+2) result += "good; most traders get the same result";
        else if (Tdiff < 7e+2) result += "a bit bad; most traders get better results";
        else if (Tdiff < 1e+3) result += "bad; consider moving to another server/network";
        else                   result += "very bad; move to another server/network";
        log(result);
      };
      function<void(const string&, const string&, const string&)> printer;
    protected:
      void log(const string &reason, const string &highlight = "") {
        if (printer) printer(
          string(reason.find(">>>") != reason.find("<<<")
            ? "DEBUG "
            : "GW "
          ) + exchange,
          reason,
          highlight
        );
      };
      void reconnect(const string &reason) {
        countdown = 7;
        log("WS " + reason + ", reconnecting in " + to_string(countdown) + "s.");
      };
  };

  class Gw: public GwExchange {
    public:
//BO non-free Gw library functions from build-*/local/lib/K-*.a (it just returns a derived gateway class based on argument).
/**/  static Gw* new_Gw(const string&); // may return too a nullptr instead of a child gateway class, if string is unknown..
//EO non-free Gw library functions from build-*/local/lib/K-*.a (it just returns a derived gateway class based on argument).
  };

  class GwApiREST: public Gw {
    public:
      const bool askForData(const unsigned int &tick) override {
        return askForSyncData(tick);
      };
      const bool waitForData() override {
        return waitForSyncData();
      };
  };
  class GwApiWS: public Gw {
    public:
      GwApiWS()
      { countdown = 1; };
      const bool askForData(const unsigned int &tick) override {
        return askForNeverAsyncData(tick);
      };
      const bool waitForData() override {
        return waitForNeverAsyncData();
      };
  };

  class GwNull: public GwApiREST {
    public:
      GwNull()
      {
        randId = Random::uuid36Id;
      };
    public:
      const json handshake() override {
        return {
          {"minTick", 1e-2   },
          {"minSize", 1e-2   },
          {  "reply", nullptr}
        };
      };
  };
  class GwHitBtc: public GwApiWS {
    public:
      GwHitBtc()
      {
        http   = "https://api.hitbtc.com/api/2";
        ws     = "wss://api.hitbtc.com/api/2/ws";
        randId = Random::uuid32Id;
      };
      const json handshake() override {
        const json reply = Curl::xfer(http + "/public/symbol/" + base + quote);
        return {
          {"minTick", stod(reply.value("tickSize", "0"))            },
          {"minSize", stod(reply.value("quantityIncrement", "0"))   },
          {"makeFee", stod(reply.value("provideLiquidityRate", "0"))},
          {"takeFee", stod(reply.value("takeLiquidityRate", "0"))   },
          {  "reply", reply                                         }
        };
      };
    protected:
      static const json xfer(const string &url, const string &auth, const string &post) {
        return Curl::perform(url, [&](CURL *curl) {
          curl_easy_setopt(curl, CURLOPT_USERPWD, auth.data());
          curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.data());
        });
      };
  };
  class GwBequant: virtual public GwHitBtc {
    public:
      GwBequant()
      {
        http = "https://api.bequant.io/api/2";
        ws   = "wss://api.bequant.io/api/2/ws";
      };
  };
  class GwCoinbase: public GwApiWS,
                    public FIX::NullApplication {
    public:
      GwCoinbase()
      {
        http   = "https://api.pro.coinbase.com";
        ws     = "wss://ws-feed.pro.coinbase.com";
        fix    = "fix.pro.coinbase.com:4198";
        randId = Random::uuid36Id;
      };
      const json handshake() override {
        const json reply = Curl::xfer(http + "/products/" + base + "-" + quote);
        return {
          {"minTick", stod(reply.value("quote_increment", "0"))},
          {"minSize", stod(reply.value("base_min_size", "0"))  },
          {  "reply", reply                                    }
        };
      };
    protected:
      static const json xfer(const string &url, const string &h1, const string &h2, const string &h3, const string &h4, const bool &rm) {
        return Curl::perform(url, [&](CURL *curl) {
          struct curl_slist *h_ = nullptr;
          h_ = curl_slist_append(h_, ("CB-ACCESS-KEY: " + h1).data());
          h_ = curl_slist_append(h_, ("CB-ACCESS-SIGN: " + h2).data());
          h_ = curl_slist_append(h_, ("CB-ACCESS-TIMESTAMP: " + h3).data());
          h_ = curl_slist_append(h_, ("CB-ACCESS-PASSPHRASE: " + h4).data());
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h_);
          if (rm) curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
        });
      };
  };
  class GwBitfinex: public GwApiWS {
    public:
      GwBitfinex()
      {
        http   = "https://api.bitfinex.com/v1";
        ws     = "wss://api.bitfinex.com/ws/2";
        randId = Random::int45Id;
        askForReplace = true;
      };
      const json handshake() override {
        const json reply1 = Curl::xfer(http + "/pubticker/" + base + quote);
        Price minTick = 0,
              minSize = 0;
        if (reply1.find("last_price") != reply1.end()) {
          ostringstream price_;
          price_ << scientific << stod(reply1.value("last_price", "0"));
          string price = price_.str();
          for (string::iterator it=price.begin(); it!=price.end();)
            if (*it == '+' or *it == '-') break;
            else it = price.erase(it);
          istringstream iss("1e" + to_string(fmax(stod(price),-4)-4));
          iss >> minTick;
        }
        const json reply2 = Curl::xfer(http + "/symbols_details");
        if (reply2.is_array())
          for (const json &it : reply2)
            if (it.find("pair") != it.end() and it.value("pair", "") == Text::strL(base + quote)) {
              minSize = stod(it.value("minimum_order_size", "0"));
              break;
            }
        return {
          {"minTick", minTick         },
          {"minSize", minSize         },
          {  "reply", {reply1, reply2}}
        };
      };
    protected:
      static const json xfer(const string &url, const string &post, const string &h1, const string &h2) {
        return Curl::perform(url, [&](CURL *curl) {
          struct curl_slist *h_ = nullptr;
          h_ = curl_slist_append(h_, ("X-BFX-APIKEY: " + h1).data());
          h_ = curl_slist_append(h_, ("X-BFX-PAYLOAD: " + post).data());
          h_ = curl_slist_append(h_, ("X-BFX-SIGNATURE: " + h2).data());
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h_);
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.data());
        });
      };
  };
  class GwEthfinex: virtual public GwBitfinex {
    public:
      GwEthfinex()
      {
        http = "https://api.ethfinex.com/v1";
        ws   = "wss://api.ethfinex.com/ws/2";
      };
  };
  class GwFCoin: public GwApiWS {
    public:
      GwFCoin()
      {
        http   = "https://api.fcoin.com/v2/";
        ws     = "wss://api.fcoin.com/v2/ws";
        randId = Random::char16Id;
      };
      const json handshake() override {
        const json reply = Curl::xfer(http + "public/symbols");
        Price minTick = 0,
              minSize = 0;
        if (reply.find("data") != reply.end() and reply.at("data").is_array())
          for (const json &it : reply.at("data"))
            if (it.find("name") != it.end() and it.value("name", "") == Text::strL(base + quote)) {
              istringstream iss(
                "1e-" + to_string(it.value("price_decimal", 0))
                + " 1e-" + to_string(it.value("amount_decimal", 0))
              );
              iss >> minTick >> minSize;
              break;
            }
        return {
          {"minTick", minTick},
          {"minSize", minSize},
          {  "reply", reply  }
        };
      };
    protected:
      static const json xfer(const string &url, const string &h1, const string &h2, const string &h3, const string &post = "") {
        return Curl::perform(url, [&](CURL *curl) {
          struct curl_slist *h_ = nullptr;
          if (!post.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.data());
            h_ = curl_slist_append(h_, "Content-Type: application/json;charset=UTF-8");
          }
          h_ = curl_slist_append(h_, ("FC-ACCESS-KEY: " + h1).data());
          h_ = curl_slist_append(h_, ("FC-ACCESS-SIGNATURE: " + h2).data());
          h_ = curl_slist_append(h_, ("FC-ACCESS-TIMESTAMP: " + h3).data());
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h_);
        });
      };
  };
  class GwKraken: public GwApiREST {
    public:
      GwKraken()
      {
        http   = "https://api.kraken.com";
        randId = Random::int32Id;
      };
      const json handshake() override {
        const json reply = Curl::xfer(http + "/0/public/AssetPairs?pair=" + base + quote);
        Price minTick = 0,
              minSize = 0;
        if (reply.find("result") != reply.end())
          for (json::const_iterator it = reply.at("result").cbegin(); it != reply.at("result").cend(); ++it)
            if (it.value().find("pair_decimals") != it.value().end()) {
              istringstream iss(
                "1e-" + to_string(it.value().value("pair_decimals", 0))
                + " 1e-" + to_string(it.value().value("lot_decimals", 0))
              );
              iss >> minTick >> minSize;
              break;
            }
        return {
          {"minTick", minTick},
          {"minSize", minSize},
          {  "reply", reply  }
        };
      };
    protected:
      static const json xfer(const string &url, const string &h1, const string &h2, const string &post) {
        return Curl::perform(url, [&](CURL *curl) {
          struct curl_slist *h_ = nullptr;
          h_ = curl_slist_append(h_, ("API-Key: " + h1).data());
          h_ = curl_slist_append(h_, ("API-Sign: " + h2).data());
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h_);
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.data());
        });
      };
  };
  class GwPoloniex: public GwApiREST {
    public:
      GwPoloniex()
      {
        http   = "https://poloniex.com";
        randId = Random::int45Id;
      };
      const json handshake() override {
        const json reply = Curl::xfer(http + "/public?command=returnTicker")
                             .value(quote + "_" + base, json::object());
        return {
          {"minTick", reply.empty()
                        ? 0
                        : 1e-8     },
          {"minSize", 1e-3         },
          {  "reply", reply        }
        };
      };
    protected:
      static const json xfer(const string &url, const string &post, const string &h1, const string &h2) {
        return Curl::perform(url, [&](CURL *curl) {
          struct curl_slist *h_ = nullptr;
          h_ = curl_slist_append(h_, "Content-Type: application/x-www-form-urlencoded");
          h_ = curl_slist_append(h_, ("Key: " + h1).data());
          h_ = curl_slist_append(h_, ("Sign: " + h2).data());
          curl_easy_setopt(curl, CURLOPT_HTTPHEADER, h_);
          curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.data());
        });
      };
  };
}

#endif
