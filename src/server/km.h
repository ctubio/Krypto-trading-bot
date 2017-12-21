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
  enum class uiBIT: unsigned char { Hello = '=', Kiss = '-' };
  enum class uiTXT: unsigned char {
    FairValue = 'a', Quote = 'b', ActiveSubscription = 'c', Connectivity = 'd', MarketData = 'e',
    QuotingParameters = 'f', SafetySettings = 'g', Product = 'h', OrderStatusReports = 'i',
    ProductAdvertisement = 'j', ApplicationState = 'k', Notepad = 'l', ToggleSettings = 'm',
    Position = 'n', SubmitNewOrder = 'p', CancelOrder = 'q', MarketTrade = 'r', Trades = 's',
    ExternalValuation = 't', QuoteStatus = 'u', TargetBasePosition = 'v', TradeSafetyValue = 'w',
    CancelAllOrders = 'x', CleanAllClosedTrades = 'y', CleanAllTrades = 'z', CleanTrade = 'A',
    TradesChart = 'B', WalletChart = 'C', EWMAChart = 'D', MarketDataLongTerm = 'G'
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
      {"widthPing", k.widthPing},
      {"widthPingPercentage", k.widthPingPercentage},
      {"widthPong", k.widthPong},
      {"widthPongPercentage", k.widthPongPercentage},
      {"widthPercentage", k.widthPercentage},
      {"bestWidth", k.bestWidth},
      {"buySize", k.buySize},
      {"buySizePercentage", k.buySizePercentage},
      {"buySizeMax", k.buySizeMax},
      {"sellSize", k.sellSize},
      {"sellSizePercentage", k.sellSizePercentage},
      {"sellSizeMax", k.sellSizeMax},
      {"pingAt", k.pingAt},
      {"pongAt", k.pongAt},
      {"mode", k.mode},
      {"safety", k.safety},
      {"bullets", k.bullets},
      {"range", k.range},
      {"rangePercentage", k.rangePercentage},
      {"fvModel", k.fvModel},
      {"targetBasePosition", k.targetBasePosition},
      {"targetBasePositionPercentage", k.targetBasePositionPercentage},
      {"positionDivergence", k.positionDivergence},
      {"positionDivergencePercentage", k.positionDivergencePercentage},
      {"positionDivergenceMin", k.positionDivergenceMin},
      {"positionDivergencePercentageMin", k.positionDivergencePercentageMin},
      {"positionDivergenceMode", k.positionDivergenceMode},
      {"percentageValues", k.percentageValues},
      {"autoPositionMode", k.autoPositionMode},
      {"aggressivePositionRebalancing", k.aggressivePositionRebalancing},
      {"superTrades", k.superTrades},
      {"tradesPerMinute", k.tradesPerMinute},
      {"tradeRateSeconds", k.tradeRateSeconds},
      {"protectionEwmaWidthPing", k.protectionEwmaWidthPing},
      {"protectionEwmaQuotePrice", k.protectionEwmaQuotePrice},
      {"protectionEwmaPeriods", k.protectionEwmaPeriods},
      {"quotingStdevProtection", k.quotingStdevProtection},
      {"quotingStdevBollingerBands", k.quotingStdevBollingerBands},
      {"quotingStdevProtectionFactor", k.quotingStdevProtectionFactor},
      {"quotingStdevProtectionPeriods", k.quotingStdevProtectionPeriods},
      {"ewmaSensiblityPercentage", k.ewmaSensiblityPercentage},
      {"veryLongEwmaPeriods", k.veryLongEwmaPeriods},
      {"longEwmaPeriods", k.longEwmaPeriods},
      {"mediumEwmaPeriods", k.mediumEwmaPeriods},
      {"shortEwmaPeriods", k.shortEwmaPeriods},
      {"aprMultiplier", k.aprMultiplier},
      {"sopWidthMultiplier", k.sopWidthMultiplier},
      {"sopSizeMultiplier", k.sopSizeMultiplier},
      {"sopTradesMultiplier", k.sopTradesMultiplier},
      {"cancelOrdersAuto", k.cancelOrdersAuto},
      {"cleanPongsAuto", k.cleanPongsAuto},
      {"profitHourInterval", k.profitHourInterval},
      {"audio", k.audio},
      {"delayUI", k.delayUI}
    };
  };
  static void from_json(const json& j, mQuotingParams& k) {
    if (j.end() != j.find("widthPing")) k.widthPing = fmax(1e-8, j.at("widthPing").get<double>());
    if (j.end() != j.find("widthPingPercentage")) k.widthPingPercentage = fmin(1e+2, fmax(1e-1, j.at("widthPingPercentage").get<double>()));
    if (j.end() != j.find("widthPong")) k.widthPong = fmax(1e-8, j.at("widthPong").get<double>());
    if (j.end() != j.find("widthPongPercentage")) k.widthPongPercentage = fmin(1e+2, fmax(1e-1, j.at("widthPongPercentage").get<double>()));
    if (j.end() != j.find("widthPercentage")) k.widthPercentage = j.at("widthPercentage").get<bool>();
    if (j.end() != j.find("bestWidth")) k.bestWidth = j.at("bestWidth").get<bool>();
    if (j.end() != j.find("buySize")) k.buySize = fmax(1e-8, j.at("buySize").get<double>());
    if (j.end() != j.find("buySizePercentage")) k.buySizePercentage = fmin(1e+2, max(1, j.at("buySizePercentage").get<int>()));
    if (j.end() != j.find("buySizeMax")) k.buySizeMax = j.at("buySizeMax").get<bool>();
    if (j.end() != j.find("sellSize")) k.sellSize = fmax(1e-8, j.at("sellSize").get<double>());
    if (j.end() != j.find("sellSizePercentage")) k.sellSizePercentage = fmin(1e+2, max(1, j.at("sellSizePercentage").get<int>()));
    if (j.end() != j.find("sellSizeMax")) k.sellSizeMax = j.at("sellSizeMax").get<bool>();
    if (j.end() != j.find("pingAt")) k.pingAt = j.at("pingAt").get<mPingAt>();
    if (j.end() != j.find("pongAt")) k.pongAt = j.at("pongAt").get<mPongAt>();
    if (j.end() != j.find("mode")) k.mode = j.at("mode").get<mQuotingMode>();
    if (j.end() != j.find("safety")) k.safety = j.at("safety").get<mQuotingSafety>();
    if (j.end() != j.find("bullets")) k.bullets = min(10, max(1, j.at("bullets").get<int>()));
    if (j.end() != j.find("range")) k.range = j.at("range").get<double>();
    if (j.end() != j.find("rangePercentage")) k.rangePercentage = fmin(1e+2, fmax(1e-1, j.at("rangePercentage").get<double>()));
    if (j.end() != j.find("fvModel")) k.fvModel = j.at("fvModel").get<mFairValueModel>();
    if (j.end() != j.find("targetBasePosition")) k.targetBasePosition = j.at("targetBasePosition").get<double>();
    if (j.end() != j.find("targetBasePositionPercentage")) k.targetBasePositionPercentage = fmin(1e+2, max(0, j.at("targetBasePositionPercentage").get<int>()));
    if (j.end() != j.find("positionDivergenceMin")) k.positionDivergenceMin = j.at("positionDivergenceMin").get<double>();
    if (j.end() != j.find("positionDivergenceMode")) k.positionDivergenceMode = j.at("positionDivergenceMode").get<mPDivMode>();
    if (j.end() != j.find("positionDivergence")) k.positionDivergence = j.at("positionDivergence").get<double>();
    if (j.end() != j.find("positionDivergencePercentage")) k.positionDivergencePercentage = fmin(1e+2, max(0, j.at("positionDivergencePercentage").get<int>()));
    if (j.end() != j.find("positionDivergencePercentageMin")) k.positionDivergencePercentageMin = fmin(1e+2, max(0, j.at("positionDivergencePercentageMin").get<int>()));
    if (j.end() != j.find("percentageValues")) k.percentageValues = j.at("percentageValues").get<bool>();
    if (j.end() != j.find("autoPositionMode")) k.autoPositionMode = j.at("autoPositionMode").get<mAutoPositionMode>();
    if (j.end() != j.find("aggressivePositionRebalancing")) k.aggressivePositionRebalancing = j.at("aggressivePositionRebalancing").get<mAPR>();
    if (j.end() != j.find("superTrades")) k.superTrades = j.at("superTrades").get<mSOP>();
    if (j.end() != j.find("tradesPerMinute")) k.tradesPerMinute = j.at("tradesPerMinute").get<double>();
    if (j.end() != j.find("tradeRateSeconds")) k.tradeRateSeconds = max(0, j.at("tradeRateSeconds").get<int>());
    if (j.end() != j.find("protectionEwmaWidthPing")) k.protectionEwmaWidthPing = j.at("protectionEwmaWidthPing").get<bool>();
    if (j.end() != j.find("protectionEwmaQuotePrice")) k.protectionEwmaQuotePrice = j.at("protectionEwmaQuotePrice").get<bool>();
    if (j.end() != j.find("protectionEwmaPeriods")) k.protectionEwmaPeriods = max(1, j.at("protectionEwmaPeriods").get<int>());
    if (j.end() != j.find("quotingStdevProtection")) k.quotingStdevProtection = j.at("quotingStdevProtection").get<mSTDEV>();
    if (j.end() != j.find("quotingStdevBollingerBands")) k.quotingStdevBollingerBands = j.at("quotingStdevBollingerBands").get<bool>();
    if (j.end() != j.find("quotingStdevProtectionFactor")) k.quotingStdevProtectionFactor = j.at("quotingStdevProtectionFactor").get<double>();
    if (j.end() != j.find("quotingStdevProtectionPeriods")) k.quotingStdevProtectionPeriods = max(1, j.at("quotingStdevProtectionPeriods").get<int>());
    if (j.end() != j.find("ewmaSensiblityPercentage")) k.ewmaSensiblityPercentage = j.at("ewmaSensiblityPercentage").get<double>();
    if (j.end() != j.find("veryLongEwmaPeriods")) k.veryLongEwmaPeriods = max(1, j.at("veryLongEwmaPeriods").get<int>());
    if (j.end() != j.find("longEwmaPeriods")) k.longEwmaPeriods = max(1, j.at("longEwmaPeriods").get<int>());
    if (j.end() != j.find("mediumEwmaPeriods")) k.mediumEwmaPeriods = max(1, j.at("mediumEwmaPeriods").get<int>());
    if (j.end() != j.find("shortEwmaPeriods")) k.shortEwmaPeriods = max(1, j.at("shortEwmaPeriods").get<int>());
    if (j.end() != j.find("aprMultiplier")) k.aprMultiplier = j.at("aprMultiplier").get<double>();
    if (j.end() != j.find("sopWidthMultiplier")) k.sopWidthMultiplier = j.at("sopWidthMultiplier").get<double>();
    if (j.end() != j.find("sopSizeMultiplier")) k.sopSizeMultiplier = j.at("sopSizeMultiplier").get<double>();
    if (j.end() != j.find("sopTradesMultiplier")) k.sopTradesMultiplier = j.at("sopTradesMultiplier").get<double>();
    if (j.end() != j.find("cancelOrdersAuto")) k.cancelOrdersAuto = j.at("cancelOrdersAuto").get<bool>();
    if (j.end() != j.find("cleanPongsAuto")) k.cleanPongsAuto = j.at("cleanPongsAuto").get<double>();
    if (j.end() != j.find("profitHourInterval")) k.profitHourInterval = j.at("profitHourInterval").get<double>();
    if (j.end() != j.find("audio")) k.audio = j.at("audio").get<bool>();
    if (j.end() != j.find("delayUI")) k.delayUI = max(0, j.at("delayUI").get<int>());
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
      {"base", k.base},
      {"quote", k.quote}
    };
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
      {"amount", k.amount},
      {"held", k.held},
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
      {"baseValue", k.baseValue},
      {"quoteValue", k.quoteValue},
      {"time", k.time}
    };
  };
  static void from_json(const json& j, mProfit& k) {
    if (j.end() != j.find("baseValue")) k.baseValue = j.at("baseValue").get<double>();
    if (j.end() != j.find("quoteValue")) k.quoteValue = j.at("quoteValue").get<double>();
    if (j.end() != j.find("time")) k.time = j.at("time").get<unsigned long>();
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
      {"buy", k.buy},
      {"sell", k.sell},
      {"combined", k.combined},
      {"buyPing", fmax(0, k.buyPing)},
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
      {"baseAmount", k.baseAmount},
      {"quoteAmount", k.quoteAmount},
      {"baseHeldAmount", k.baseHeldAmount},
      {"quoteHeldAmount", k.quoteHeldAmount},
      {"baseValue", k.baseValue},
      {"quoteValue", k.quoteValue},
      {"profitBase", k.profitBase},
      {"profitQuote", k.profitQuote},
      {"pair", k.pair}
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
      {"tradeId", k.tradeId},
      {"time", k.time},
      {"pair", k.pair},
      {"price", k.price},
      {"quantity", k.quantity},
      {"side", k.side},
      {"value", k.value},
      {"Ktime", k.Ktime},
      {"Kqty", k.Kqty},
      {"Kprice", k.Kprice},
      {"Kvalue", k.Kvalue},
      {"Kdiff", k.Kdiff},
      {"feeCharged", k.feeCharged},
      {"loadedFromDB", k.loadedFromDB},
    };
    else j = {
      {"time", k.time},
      {"pair", k.pair},
      {"price", k.price},
      {"quantity", k.quantity},
      {"side", k.side}
    };
  };
  static void from_json(const json& j, mTrade& k) {
    if (j.end() != j.find("tradeId")) k.tradeId = j.at("tradeId").get<string>();
    if (j.end() != j.find("pair")) k.pair = mPair(j["/pair/base"_json_pointer].get<string>(), j["/pair/quote"_json_pointer].get<string>());
    if (j.end() != j.find("price")) k.price = j.at("price").get<double>();
    if (j.end() != j.find("quantity")) k.quantity = j.at("quantity").get<double>();
    if (j.end() != j.find("side")) k.side = j.at("side").get<mSide>();
    if (j.end() != j.find("time")) k.time = j.at("time").get<unsigned long>();
    if (j.end() != j.find("value")) k.value = j.at("value").get<double>();
    if (j.end() != j.find("Ktime")) k.Ktime = j.at("Ktime").get<unsigned long>();
    if (j.end() != j.find("Kqty")) k.Kqty = j.at("Kqty").get<double>();
    if (j.end() != j.find("Kprice")) k.Kprice = j.at("Kprice").get<double>();
    if (j.end() != j.find("Kvalue")) k.Kvalue = j.at("Kvalue").get<double>();
    if (j.end() != j.find("Kdiff")) k.Kdiff = j.at("Kdiff").get<double>();
    if (j.end() != j.find("feeCharged")) k.feeCharged = j.at("feeCharged").get<double>();
    if (j.end() != j.find("loadedFromDB")) k.loadedFromDB = j.at("loadedFromDB").get<bool>();
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
      {"orderId", k.orderId},
      {"exchangeId", k.exchangeId},
      {"pair", k.pair},
      {"side", k.side},
      {"quantity", k.quantity},
      {"type", k.type},
      {"isPong", k.isPong},
      {"price", k.price},
      {"timeInForce", k.timeInForce},
      {"orderStatus", k.orderStatus},
      {"preferPostOnly", k.preferPostOnly},
      {"tradeQuantity", k.tradeQuantity},
      {"time", k.time},
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
    json b, a;
    for (vector<mLevel>::const_iterator it = k.bids.begin(); it != k.bids.end(); ++it)
      b.push_back({{"price", it->price}, {"size", it->size}});
    for (vector<mLevel>::const_iterator it = k.asks.begin(); it != k.asks.end(); ++it)
      a.push_back({{"price", it->price}, {"size", it->size}});
    j = {{"bids", b}, {"asks", a}};
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
      {"bid", {
        {"price", k.bid.price},
        {"size", k.bid.size}
      }},
      {"ask", {
        {"price", k.ask.price},
        {"size", k.ask.size}
      }}
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
      {"bidStatus", k.bidStatus},
      {"askStatus", k.askStatus},
      {"quotesInMemoryNew", k.quotesInMemoryNew},
      {"quotesInMemoryWorking", k.quotesInMemoryWorking},
      {"quotesInMemoryDone", k.quotesInMemoryDone}
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
