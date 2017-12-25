#ifndef K_KM_H_
#define K_KM_H_

namespace K {
  enum class mExchange: unsigned int { Null, HitBtc, OkCoin, Coinbase, Bitfinex, Kraken, OkEx, BitfinexMargin, Korbit, Poloniex };
  enum class mConnectivity: unsigned int { Disconnected, Connected };
  enum class mStatus: unsigned int { New, Working, Complete, Cancelled };
  enum class mSide: unsigned int { Bid, Ask, Both };
  enum class mTimeInForce: unsigned int { IOC, FOK, GTC };
  enum class mOrderType: unsigned int { Limit, Market };
  enum class mPingAt: unsigned int { BothSides, BidSide, AskSide, DepletedSide, DepletedBidSide, DepletedAskSide, StopPings };
  enum class mPongAt: unsigned int { ShortPingFair, LongPingFair, ShortPingAggressive, LongPingAggressive };
  enum class mQuotingMode: unsigned int { Top, Mid, Join, InverseJoin, InverseTop, HamelinRat, Depth };
  enum class mQuotingSafety: unsigned int { Off, PingPong, Boomerang, AK47 };
  enum class mQuoteState: unsigned int { Live, Disconnected, DisabledQuotes, MissingData, UnknownHeld, TBPHeld, MaxTradesSeconds, WaitingPing, DepletedFunds, Crossed };
  enum class mFairValueModel: unsigned int { BBO, wBBO };
  enum class mAutoPositionMode: unsigned int { Manual, EWMA_LS, EWMA_LMS, EWMA_4 };
  enum class mPDivMode: unsigned int { Manual, Linear, Sine, SQRT, Switch};
  enum class mAPR: unsigned int { Off, Size, SizeWidth };
  enum class mSOP: unsigned int { Off, Trades, Size, TradesSize };
  enum class mSTDEV: unsigned int { Off, OnFV, OnFVAPROff, OnTops, OnTopsAPROff, OnTop, OnTopAPROff };
  enum class mPortal: unsigned char { Hello = '=', Kiss = '-' };
  enum class mMatter: unsigned char {
    FairValue            = 'a', Quote                = 'b', ActiveSubscription = 'c', Connectivity       = 'd', MarketData       = 'e',
    QuotingParameters    = 'f', SafetySettings       = 'g', Product            = 'h', OrderStatusReports = 'i',
    ProductAdvertisement = 'j', ApplicationState     = 'k', Notepad            = 'l', ToggleSettings     = 'm',
    Position             = 'n',                             SubmitNewOrder     = 'p', CancelOrder        = 'q', MarketTrade      = 'r',
    Trades               = 's',ExternalValuation     = 't', QuoteStatus        = 'u', TargetBasePosition = 'v', TradeSafetyValue = 'w',
    CancelAllOrders      = 'x', CleanAllClosedTrades = 'y', CleanAllTrades     = 'z', CleanTrade         = 'A',
    TradesChart          = 'B', WalletChart          = 'C', EWMAChart          = 'D', MarketDataLongTerm = 'G'
  };
  static          bool operator! (mConnectivity k_)                   { return !(unsigned int)k_; };
  static mConnectivity operator* (mConnectivity _k, mConnectivity k_) { return (mConnectivity)((unsigned int)_k * (unsigned int)k_); };
  struct mQuotingParams {
    double            widthPing                       = 2.0;
    double            widthPingPercentage             = 0.25;
    double            widthPong                       = 2.0;
    double            widthPongPercentage             = 0.25;
    bool              widthPercentage                 = false;
    bool              bestWidth                       = true;
    double            buySize                         = 0.02;
    unsigned int      buySizePercentage               = 7;
    bool              buySizeMax                      = false;
    double            sellSize                        = 0.01;
    unsigned int      sellSizePercentage              = 7;
    bool              sellSizeMax                     = false;
    mPingAt           pingAt                          = mPingAt::BothSides;
    mPongAt           pongAt                          = mPongAt::ShortPingFair;
    mQuotingMode      mode                            = mQuotingMode::Top;
    mQuotingSafety    safety                          = mQuotingSafety::Boomerang;
    unsigned int      bullets                         = 2;
    double            range                           = 0.5;
    double            rangePercentage                 = 5.0;
    mFairValueModel   fvModel                         = mFairValueModel::BBO;
    double            targetBasePosition              = 1.0;
    unsigned int      targetBasePositionPercentage    = 50;
    double            positionDivergence              = 0.9;
    double            positionDivergenceMin           = 0.4;
    unsigned int      positionDivergencePercentage    = 21;
    unsigned int      positionDivergencePercentageMin = 10;
    mPDivMode         positionDivergenceMode          = mPDivMode::Manual;
    bool              percentageValues                = false;
    mAutoPositionMode autoPositionMode                = mAutoPositionMode::EWMA_LS;
    mAPR              aggressivePositionRebalancing   = mAPR::Off;
    mSOP              superTrades                     = mSOP::Off;
    double            tradesPerMinute                 = 0.9;
    unsigned int      tradeRateSeconds                = 3;
    bool              protectionEwmaWidthPing         = false;
    bool              protectionEwmaQuotePrice        = true;
    unsigned int      protectionEwmaPeriods           = 200;
    mSTDEV            quotingStdevProtection          = mSTDEV::Off;
    bool              quotingStdevBollingerBands      = false;
    double            quotingStdevProtectionFactor    = 1.0;
    unsigned int      quotingStdevProtectionPeriods   = 1200;
    double            ewmaSensiblityPercentage        = 0.5;
    unsigned int      veryLongEwmaPeriods             = 400;
    unsigned int      longEwmaPeriods                 = 200;
    unsigned int      mediumEwmaPeriods               = 100;
    unsigned int      shortEwmaPeriods                = 50;
    double            aprMultiplier                   = 2;
    double            sopWidthMultiplier              = 2;
    double            sopSizeMultiplier               = 2;
    double            sopTradesMultiplier             = 2;
    bool              cancelOrdersAuto                = false;
    double            cleanPongsAuto                  = 0.0;
    double            profitHourInterval              = 0.5;
    bool              audio                           = false;
    unsigned int      delayUI                         = 7;
    bool              _matchPings                     = true;
  };
  static void to_json(json& j, const mQuotingParams& k) {
    j = {
      {                      "widthPing", k.widthPing                      },
      {            "widthPingPercentage", k.widthPingPercentage            },
      {                      "widthPong", k.widthPong                      },
      {            "widthPongPercentage", k.widthPongPercentage            },
      {                "widthPercentage", k.widthPercentage                },
      {                      "bestWidth", k.bestWidth                      },
      {                        "buySize", k.buySize                        },
      {              "buySizePercentage", k.buySizePercentage              },
      {                     "buySizeMax", k.buySizeMax                     },
      {                       "sellSize", k.sellSize                       },
      {             "sellSizePercentage", k.sellSizePercentage             },
      {                    "sellSizeMax", k.sellSizeMax                    },
      {                         "pingAt", k.pingAt                         },
      {                         "pongAt", k.pongAt                         },
      {                           "mode", k.mode                           },
      {                         "safety", k.safety                         },
      {                        "bullets", k.bullets                        },
      {                          "range", k.range                          },
      {                "rangePercentage", k.rangePercentage                },
      {                        "fvModel", k.fvModel                        },
      {             "targetBasePosition", k.targetBasePosition             },
      {   "targetBasePositionPercentage", k.targetBasePositionPercentage   },
      {             "positionDivergence", k.positionDivergence             },
      {   "positionDivergencePercentage", k.positionDivergencePercentage   },
      {          "positionDivergenceMin", k.positionDivergenceMin          },
      {"positionDivergencePercentageMin", k.positionDivergencePercentageMin},
      {         "positionDivergenceMode", k.positionDivergenceMode         },
      {               "percentageValues", k.percentageValues               },
      {               "autoPositionMode", k.autoPositionMode               },
      {  "aggressivePositionRebalancing", k.aggressivePositionRebalancing  },
      {                    "superTrades", k.superTrades                    },
      {                "tradesPerMinute", k.tradesPerMinute                },
      {               "tradeRateSeconds", k.tradeRateSeconds               },
      {        "protectionEwmaWidthPing", k.protectionEwmaWidthPing        },
      {       "protectionEwmaQuotePrice", k.protectionEwmaQuotePrice       },
      {          "protectionEwmaPeriods", k.protectionEwmaPeriods          },
      {         "quotingStdevProtection", k.quotingStdevProtection         },
      {     "quotingStdevBollingerBands", k.quotingStdevBollingerBands     },
      {   "quotingStdevProtectionFactor", k.quotingStdevProtectionFactor   },
      {  "quotingStdevProtectionPeriods", k.quotingStdevProtectionPeriods  },
      {       "ewmaSensiblityPercentage", k.ewmaSensiblityPercentage       },
      {            "veryLongEwmaPeriods", k.veryLongEwmaPeriods            },
      {                "longEwmaPeriods", k.longEwmaPeriods                },
      {              "mediumEwmaPeriods", k.mediumEwmaPeriods              },
      {               "shortEwmaPeriods", k.shortEwmaPeriods               },
      {                  "aprMultiplier", k.aprMultiplier                  },
      {             "sopWidthMultiplier", k.sopWidthMultiplier             },
      {              "sopSizeMultiplier", k.sopSizeMultiplier              },
      {            "sopTradesMultiplier", k.sopTradesMultiplier            },
      {               "cancelOrdersAuto", k.cancelOrdersAuto               },
      {                 "cleanPongsAuto", k.cleanPongsAuto                 },
      {             "profitHourInterval", k.profitHourInterval             },
      {                          "audio", k.audio                          },
      {                        "delayUI", k.delayUI                        }
    };
  };
  static void from_json(const json& j, mQuotingParams& k) {
    json o = j;
    for (json::iterator it = o.begin(); it != o.end();)
      if (it.value().is_null()) it = o.erase(it); else ++it;
    mQuotingParams def;
    k.widthPing                       = fmax(1e-8,            o.value("widthPing", def.widthPing));
    k.widthPingPercentage             = fmin(1e+2, fmax(1e-1, o.value("widthPingPercentage", def.widthPingPercentage)));
    k.widthPong                       = fmax(1e-8,            o.value("widthPong", def.widthPong));
    k.widthPongPercentage             = fmin(1e+2, fmax(1e-1, o.value("widthPongPercentage", def.widthPongPercentage)));
    k.widthPercentage                 =                       o.value("widthPercentage", def.widthPercentage);
    k.bestWidth                       =                       o.value("bestWidth", def.bestWidth);
    k.buySize                         = fmax(1e-8,            o.value("buySize", def.buySize));
    k.buySizePercentage               = fmin(1e+2, fmax(1,    o.value("buySizePercentage", def.buySizePercentage)));
    k.buySizeMax                      =                       o.value("buySizeMax", def.buySizeMax);
    k.sellSize                        = fmax(1e-8,            o.value("sellSize", def.sellSize));
    k.sellSizePercentage              = fmin(1e+2, fmax(1,    o.value("sellSizePercentage", def.sellSizePercentage)));
    k.sellSizeMax                     =                       o.value("sellSizeMax", def.sellSizeMax);
    k.pingAt                          =                       o.value("pingAt", def.pingAt);
    k.pongAt                          =                       o.value("pongAt", def.pongAt);
    k.mode                            =                       o.value("mode", def.mode);
    k.safety                          =                       o.value("safety", def.safety);
    k.bullets                         = fmin(10, fmax(1,      o.value("bullets", def.bullets)));
    k.range                           =                       o.value("range", def.range);
    k.rangePercentage                 = fmin(1e+2, fmax(1e-1, o.value("rangePercentage", def.rangePercentage)));
    k.fvModel                         =                       o.value("fvModel", def.fvModel);
    k.targetBasePosition              =                       o.value("targetBasePosition", def.targetBasePosition);
    k.targetBasePositionPercentage    = fmin(1e+2, fmax(0,    o.value("targetBasePositionPercentage", def.targetBasePositionPercentage)));
    k.positionDivergenceMin           =                       o.value("positionDivergenceMin", def.positionDivergenceMin);
    k.positionDivergenceMode          =                       o.value("positionDivergenceMode", def.positionDivergenceMode);
    k.positionDivergence              =                       o.value("positionDivergence", def.positionDivergence);
    k.positionDivergencePercentage    = fmin(1e+2, fmax(0,    o.value("positionDivergencePercentage", def.positionDivergencePercentage)));
    k.positionDivergencePercentageMin = fmin(1e+2, fmax(0,    o.value("positionDivergencePercentageMin", def.positionDivergencePercentageMin)));
    k.percentageValues                =                       o.value("percentageValues", def.percentageValues);
    k.autoPositionMode                =                       o.value("autoPositionMode", def.autoPositionMode);
    k.aggressivePositionRebalancing   =                       o.value("aggressivePositionRebalancing", def.aggressivePositionRebalancing);
    k.superTrades                     =                       o.value("superTrades", def.superTrades);
    k.tradesPerMinute                 =                       o.value("tradesPerMinute", def.tradesPerMinute);
    k.tradeRateSeconds                = fmax(0,               o.value("tradeRateSeconds", def.tradeRateSeconds));
    k.protectionEwmaWidthPing         =                       o.value("protectionEwmaWidthPing", def.protectionEwmaWidthPing);
    k.protectionEwmaQuotePrice        =                       o.value("protectionEwmaQuotePrice", def.protectionEwmaQuotePrice);
    k.protectionEwmaPeriods           = fmax(1,               o.value("protectionEwmaPeriods", def.protectionEwmaPeriods));
    k.quotingStdevProtection          =                       o.value("quotingStdevProtection", def.quotingStdevProtection);
    k.quotingStdevBollingerBands      =                       o.value("quotingStdevBollingerBands", def.quotingStdevBollingerBands);
    k.quotingStdevProtectionFactor    =                       o.value("quotingStdevProtectionFactor", def.quotingStdevProtectionFactor);
    k.quotingStdevProtectionPeriods   = fmax(1,               o.value("quotingStdevProtectionPeriods", def.quotingStdevProtectionPeriods));
    k.ewmaSensiblityPercentage        =                       o.value("ewmaSensiblityPercentage", def.ewmaSensiblityPercentage);
    k.veryLongEwmaPeriods             = fmax(1,               o.value("veryLongEwmaPeriods", def.veryLongEwmaPeriods));
    k.longEwmaPeriods                 = fmax(1,               o.value("longEwmaPeriods", def.longEwmaPeriods));
    k.mediumEwmaPeriods               = fmax(1,               o.value("mediumEwmaPeriods", def.mediumEwmaPeriods));
    k.shortEwmaPeriods                = fmax(1,               o.value("shortEwmaPeriods", def.shortEwmaPeriods));
    k.aprMultiplier                   =                       o.value("aprMultiplier", def.aprMultiplier);
    k.sopWidthMultiplier              =                       o.value("sopWidthMultiplier", def.sopWidthMultiplier);
    k.sopSizeMultiplier               =                       o.value("sopSizeMultiplier", def.sopSizeMultiplier);
    k.sopTradesMultiplier             =                       o.value("sopTradesMultiplier", def.sopTradesMultiplier);
    k.cancelOrdersAuto                =                       o.value("cancelOrdersAuto", def.cancelOrdersAuto);
    k.cleanPongsAuto                  =                       o.value("cleanPongsAuto", def.cleanPongsAuto);
    k.profitHourInterval              =                       o.value("profitHourInterval", def.profitHourInterval);
    k.audio                           =                       o.value("audio", def.audio);
    k.delayUI                         = fmax(0,               o.value("delayUI", def.delayUI));
    if (k.mode == mQuotingMode::Depth) k.widthPercentage = false;
    k._matchPings = k.safety == mQuotingSafety::Boomerang or k.safety == mQuotingSafety::AK47;
  };
  struct mPair {
    string base,
           quote;
    mPair():
      base(""), quote("")
    {};
    mPair(string b, string q):
      base(b), quote(q)
    {};
  };
  static void to_json(json& j, const mPair& k) {
    j = {
      { "base", k.base },
      {"quote", k.quote}
    };
  };
  static void from_json(const json& j, mPair& k) {
    json o = j;
    for (json::iterator it = o.begin(); it != o.end();)
      if (it.value().is_null()) it = o.erase(it); else ++it;
    k.base  = o.value("base", "");
    k.quote = o.value("quote", "");
  };
  struct mWallet {
    double amount,
           held;
    string currency;
    mWallet():
      amount(0), held(0), currency("")
    {};
    mWallet(double a, double h, string c):
      amount(a), held(h), currency(c)
    {};
  };
  static void to_json(json& j, const mWallet& k) {
    j = {
      {  "amount", k.amount  },
      {    "held", k.held    },
      {"currency", k.currency}
    };
  };
  struct mProfit {
           double baseValue,
                  quoteValue;
    unsigned long time;
    mProfit():
      baseValue(0), quoteValue(0), time(0)
    {};
    mProfit(double b, double q, unsigned long t):
      baseValue(b), quoteValue(q), time(t)
    {};
  };
  static void to_json(json& j, const mProfit& k) {
    j = {
      { "baseValue", k.baseValue },
      {"quoteValue", k.quoteValue},
      {      "time", k.time      }
    };
  };
  static void from_json(const json& j, mProfit& k) {
    json o = j;
    for (json::iterator it = o.begin(); it != o.end();)
      if (it.value().is_null()) it = o.erase(it); else ++it;
    k.baseValue  = o.value("baseValue", 0.0);
    k.quoteValue = o.value("quoteValue", 0.0);
    k.time       = o.value("time", (unsigned long)0);
  };
  struct mSafety {
    double buy,
           sell,
           combined,
           buyPing,
           sellPing;
    mSafety():
      buy(0), sell(0), combined(0), buyPing(-1), sellPing(-1)
    {};
    mSafety(double b, double s, double c, double bP, double sP):
      buy(b), sell(s), combined(c), buyPing(bP), sellPing(sP)
    {};
  };
  static void to_json(json& j, const mSafety& k) {
    j = {
      {     "buy", k.buy              },
      {    "sell", k.sell             },
      {"combined", k.combined         },
      { "buyPing", fmax(0, k.buyPing) },
      {"sellPing", fmax(0, k.sellPing)}
    };
  };
  struct mPosition {
       double baseAmount,
              quoteAmount,
              baseHeldAmount,
              quoteHeldAmount,
              baseValue,
              quoteValue,
              profitBase,
              profitQuote;
        mPair pair;
    mPosition():
      baseAmount(0), quoteAmount(0), baseHeldAmount(0), quoteHeldAmount(0), baseValue(0), quoteValue(0), profitBase(0), profitQuote(0), pair(mPair())
    {};
    mPosition(double bA, double qA, double bH, double qH, double bV, double qV, double bP, double qP, mPair p):
      baseAmount(bA), quoteAmount(qA), baseHeldAmount(bH), quoteHeldAmount(qH), baseValue(bV), quoteValue(qV), profitBase(bP), profitQuote(qP), pair(p)
    {};
    bool empty() {
      return !baseValue;
    };
  };
  static void to_json(json& j, const mPosition& k) {
    j = {
      {     "baseAmount", k.baseAmount     },
      {    "quoteAmount", k.quoteAmount    },
      { "baseHeldAmount", k.baseHeldAmount },
      {"quoteHeldAmount", k.quoteHeldAmount},
      {      "baseValue", k.baseValue      },
      {     "quoteValue", k.quoteValue     },
      {     "profitBase", k.profitBase     },
      {    "profitQuote", k.profitQuote    },
      {           "pair", k.pair           }
    };
  };
  struct mTrade {
           string tradeId;
            mSide side;
            mPair pair;
           double price,
                  quantity,
                  value,
                  Kqty,
                  Kvalue,
                  Kprice,
                  Kdiff,
                  feeCharged;
    unsigned long time,
                  Ktime;
             bool loadedFromDB;

    mTrade():
      tradeId(""), pair(mPair()), price(0), quantity(0), side((mSide)0), time(0), value(0), Ktime(0), Kqty(0), Kprice(0), Kvalue(0), Kdiff(0), feeCharged(0), loadedFromDB(false)
    {};
    mTrade(double p, double q, unsigned long t):
      tradeId(""), pair(mPair()), price(p), quantity(q), side((mSide)0), time(t), value(0), Ktime(0), Kqty(0), Kprice(0), Kvalue(0), Kdiff(0), feeCharged(0), loadedFromDB(false)
    {};
    mTrade(double p, double q, mSide s):
      tradeId(""), pair(mPair()), price(p), quantity(q), side(s), time(0), value(0), Ktime(0), Kqty(0), Kprice(0), Kvalue(0), Kdiff(0), feeCharged(0), loadedFromDB(false)
    {};
    mTrade(string i, mPair P, double p, double q, mSide S, unsigned long t, double v, unsigned long Kt, double Kq, double Kp, double Kv, double Kd, double f, bool l):
      tradeId(i), pair(P), price(p), quantity(q), side(S), time(t), value(v), Ktime(Kt), Kqty(Kq), Kprice(Kp), Kvalue(Kv), Kdiff(Kd), feeCharged(f), loadedFromDB(l)
    {};
  };
  static void to_json(json& j, const mTrade& k) {
    if (k.tradeId.length()) j = {
      {     "tradeId", k.tradeId     },
      {        "time", k.time        },
      {        "pair", k.pair        },
      {       "price", k.price       },
      {    "quantity", k.quantity    },
      {        "side", k.side        },
      {       "value", k.value       },
      {       "Ktime", k.Ktime       },
      {        "Kqty", k.Kqty        },
      {      "Kprice", k.Kprice      },
      {      "Kvalue", k.Kvalue      },
      {       "Kdiff", k.Kdiff       },
      {  "feeCharged", k.feeCharged  },
      {"loadedFromDB", k.loadedFromDB},
    };
    else j = {
      {    "time", k.time    },
      {    "pair", k.pair    },
      {   "price", k.price   },
      {"quantity", k.quantity},
      {    "side", k.side    }
    };
  };
  static void from_json(const json& j, mTrade& k) {
    json o = j;
    for (json::iterator it = o.begin(); it != o.end();)
      if (it.value().is_null()) it = o.erase(it); else ++it;
    k.tradeId      = o.value("tradeId", "");
    k.pair         = o.value("pair", json::object());
    k.price        = o.value("price", 0.0);
    k.quantity     = o.value("quantity", 0.0);
    k.side         = o.value("side", (mSide)0);
    k.time         = o.value("time", (unsigned long)0);
    k.value        = o.value("value", 0.0);
    k.Ktime        = o.value("Ktime", (unsigned long)0);
    k.Kqty         = o.value("Kqty", 0.0);
    k.Kprice       = o.value("Kprice", 0.0);
    k.Kvalue       = o.value("Kvalue", 0.0);
    k.Kdiff        = o.value("Kdiff", 0.0);
    k.feeCharged   = o.value("feeCharged", 0.0);
    k.loadedFromDB = o.value("loadedFromDB", false);
  };
  struct mOrder {
           string orderId,
                  exchangeId;
            mPair pair;
            mSide side;
           double price,
                  quantity,
                  tradeQuantity;
       mOrderType type;
     mTimeInForce timeInForce;
             mStatus orderStatus;
             bool isPong,
                  preferPostOnly;
    unsigned long time,
                  computationalLatency;
    mOrder():
      orderId(""), exchangeId(""), pair(mPair()), side((mSide)0), quantity(0), type((mOrderType)0), isPong(false), price(0), timeInForce((mTimeInForce)0), orderStatus((mStatus)0), preferPostOnly(false), tradeQuantity(0), time(0), computationalLatency(0)
    {};
    mOrder(string o, mStatus s):
      orderId(o), exchangeId(""), pair(mPair()), side((mSide)0), quantity(0), type((mOrderType)0), isPong(false), price(0), timeInForce((mTimeInForce)0), orderStatus(s), preferPostOnly(false), tradeQuantity(0), time(0), computationalLatency(0)
    {};
    mOrder(string o, string e, mStatus s, double p, double q, double Q):
      orderId(o), exchangeId(e), pair(mPair()), side((mSide)0), quantity(q), type((mOrderType)0), isPong(false), price(p), timeInForce((mTimeInForce)0), orderStatus(s), preferPostOnly(false), tradeQuantity(Q), time(0), computationalLatency(0)
    {};
    mOrder(string o, mPair P, mSide S, double q, mOrderType t, bool i, double p, mTimeInForce F, mStatus s, bool O):
      orderId(o), exchangeId(""), pair(P), side(S), quantity(q), type(t), isPong(i), price(p), timeInForce(F), orderStatus(s), preferPostOnly(O), tradeQuantity(0), time(0), computationalLatency(0)
    {};
    string quantity2str() {
      stringstream ss;
      ss << setprecision(8) << fixed << quantity;
      return ss.str();
    };
    string tradeQuantity2str() {
      stringstream ss;
      ss << setprecision(8) << fixed << tradeQuantity;
      return ss.str();
    };
    string price2str() {
      stringstream ss;
      ss << setprecision(8) << fixed << price;
      return ss.str();
    };
  };
  static void to_json(json& j, const mOrder& k) {
    j = {
      {             "orderId", k.orderId             },
      {          "exchangeId", k.exchangeId          },
      {                "pair", k.pair                },
      {                "side", k.side                },
      {            "quantity", k.quantity            },
      {                "type", k.type                },
      {              "isPong", k.isPong              },
      {               "price", k.price               },
      {         "timeInForce", k.timeInForce         },
      {         "orderStatus", k.orderStatus         },
      {      "preferPostOnly", k.preferPostOnly      },
      {       "tradeQuantity", k.tradeQuantity       },
      {                "time", k.time                },
      {"computationalLatency", k.computationalLatency}
    };
  };
  struct mLevel {
    double price,
           size;
    mLevel():
      price(0), size(0)
    {};
    mLevel(double p, double s):
      price(p), size(s)
    {};
  };
  static void to_json(json& j, const mLevel& k) {
    j = {
      {"price", k.price},
      { "size", k.size }
    };
  };
  struct mLevels {
    vector<mLevel> bids,
                   asks;
    mLevels():
      bids({}), asks({})
    {};
    mLevels(vector<mLevel> b, vector<mLevel> a):
      bids(b), asks(a)
    {};
    bool empty() {
      return bids.empty() or asks.empty();
    };
  };
  static void to_json(json& j, const mLevels& k) {
    j = {
      {"bids", k.bids},
      {"asks", k.asks}
    };
  };
  struct mQuote {
    mLevel bid,
           ask;
      bool isBidPong,
           isAskPong;
    mQuote():
      bid(mLevel()), ask(mLevel()), isBidPong(false), isAskPong(false)
    {};
    mQuote(mLevel b, mLevel a):
      bid(b), ask(a), isBidPong(false), isAskPong(false)
    {};
    mQuote(mLevel b, mLevel a, bool bP, bool aP):
      bid(b), ask(a), isBidPong(bP), isAskPong(aP)
    {};
  };
  static void to_json(json& j, const mQuote& k) {
    j = {
      {"bid", k.bid},
      {"ask", k.ask}
    };
  };
  struct mQuoteStatus {
     mQuoteState bidStatus,
                 askStatus;
    unsigned int quotesInMemoryNew,
                 quotesInMemoryWorking,
                 quotesInMemoryDone;
    mQuoteStatus():
      bidStatus((mQuoteState)0), askStatus((mQuoteState)0), quotesInMemoryNew(0), quotesInMemoryWorking(0), quotesInMemoryDone(0)
    {};
    mQuoteStatus(mQuoteState b, mQuoteState a, unsigned int n, unsigned int w, unsigned int d):
      bidStatus(b), askStatus(a), quotesInMemoryNew(n), quotesInMemoryWorking(w), quotesInMemoryDone(d)
    {};
  };
  static void to_json(json& j, const mQuoteStatus& k) {
    j = {
      {"bidStatus",             k.bidStatus},
      {"askStatus",             k.askStatus},
      {"quotesInMemoryNew",     k.quotesInMemoryNew},
      {"quotesInMemoryWorking", k.quotesInMemoryWorking},
      {"quotesInMemoryDone",    k.quotesInMemoryDone}
    };
  };
  static const char kB64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                     "abcdefghijklmnopqrstuvwxyz"
                                     "0123456789+/";
  static char RBLACK[] = "\033[0;30m", RRED[]    = "\033[0;31m", RGREEN[] = "\033[0;32m", RYELLOW[] = "\033[0;33m",
              RBLUE[]  = "\033[0;34m", RPURPLE[] = "\033[0;35m", RCYAN[]  = "\033[0;36m", RWHITE[]  = "\033[0;37m",
              BBLACK[] = "\033[1;30m", BRED[]    = "\033[1;31m", BGREEN[] = "\033[1;32m", BYELLOW[] = "\033[1;33m",
              BBLUE[]  = "\033[1;34m", BPURPLE[] = "\033[1;35m", BCYAN[]  = "\033[1;36m", BWHITE[]  = "\033[1;37m";
  static WINDOW *wBorder = nullptr,
                *wLog = nullptr;
  static vector<function<void()>*> gwEndings;
  class Gw {
    public:
      static Gw *config(string, string, string, int, string, string, string, string, string, string);
      string (*randId)() = 0;
      function<void(mOrder)>        evDataOrder;
      function<void(mTrade)>        evDataTrade;
      function<void(mWallet)>       evDataWallet;
      function<void(mLevels)>       evDataLevels;
      function<void(mConnectivity)> evConnectOrder,
                                    evConnectMarket;
      uWS::Hub                *hub     = nullptr;
      uWS::Group<uWS::CLIENT> *gwGroup = nullptr;
      mExchange exchange = (mExchange)0;
         int version = 0;
      double makeFee = 0,  minTick = 0,
             takeFee = 0,  minSize = 0;
      string base    = "", quote   = "",
             name    = "", symbol  = "",
             apikey  = "", secret  = "",
             user    = "", pass    = "",
             ws      = "", http    = "";
      virtual   void wallet() = 0,
                     levels() = 0,
                     send(string, mSide, string, string, mOrderType, mTimeInForce, bool, unsigned long) = 0,
                     cancel(string, string, mSide, unsigned long) = 0,
                     cancelAll() = 0,
                     close() = 0;
      virtual string A() = 0;
  };
  class Klass {
    protected:
      Gw             *gw = nullptr;
      mQuotingParams *qp = nullptr;
      Klass          *config = nullptr,
                     *events = nullptr,
                     *memory = nullptr,
                     *client = nullptr,
                     *broker = nullptr,
                     *market = nullptr,
                     *wallet = nullptr,
                     *engine = nullptr;
      virtual void load(int argc, char** argv) {};
      virtual void load() {};
      virtual void waitTime() {};
      virtual void waitData() {};
      virtual void waitUser() {};
      virtual void run() {};
    public:
      void main(int argc, char** argv) {
        load(argc, argv);
        run();
      };
      void wait() {
        load();
        waitTime();
        waitData();
        waitUser();
        run();
      };
      void gwLink(Gw *k) { gw = k; };
      void qpLink(mQuotingParams *k) { qp = k; };
      void cfLink(Klass *k) { config = k; };
      void evLink(Klass *k) { events = k; };
      void dbLink(Klass *k) { memory = k; };
      void uiLink(Klass *k) { client = k; };
      void ogLink(Klass *k) { broker = k; };
      void mgLink(Klass *k) { market = k; };
      void pgLink(Klass *k) { wallet = k; };
      void qeLink(Klass *k) { engine = k; };
  };
  class kLass: public Klass {
    private:
      mQuotingParams p;
    public:
      void link(Klass *EV, Klass *DB, Klass *UI, Klass *QP, Klass *OG, Klass *MG, Klass *PG, Klass *QE, Klass *GW) {
        Klass *CF = (Klass*)this;
        EV->gwLink(gw);                 UI->gwLink(gw);                 OG->gwLink(gw); MG->gwLink(gw); PG->gwLink(gw); QE->gwLink(gw); GW->gwLink(gw);
        EV->cfLink(CF); DB->cfLink(CF); UI->cfLink(CF);                 OG->cfLink(CF); MG->cfLink(CF); PG->cfLink(CF); QE->cfLink(CF); GW->cfLink(CF);
                        DB->evLink(EV); UI->evLink(EV); QP->evLink(EV); OG->evLink(EV); MG->evLink(EV); PG->evLink(EV); QE->evLink(EV); GW->evLink(EV);
                                        UI->dbLink(DB); QP->dbLink(DB); OG->dbLink(DB); MG->dbLink(DB); PG->dbLink(DB);
                                                        QP->uiLink(UI); OG->uiLink(UI); MG->uiLink(UI); PG->uiLink(UI); QE->uiLink(UI); GW->uiLink(UI);
                                                        QP->qpLink(&p); OG->qpLink(&p); MG->qpLink(&p); PG->qpLink(&p); QE->qpLink(&p); GW->qpLink(&p);
                                                                                        MG->ogLink(OG); PG->ogLink(OG); QE->ogLink(OG);
                                                                                                        PG->mgLink(MG); QE->mgLink(MG);
                                                                                                                        QE->pgLink(PG);
                                                                                                                                        GW->qeLink(QE);
      };
  };
}

#endif
