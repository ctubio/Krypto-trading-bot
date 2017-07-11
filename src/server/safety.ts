import Models = require("../share/models");
import Utils = require("./utils");
import Broker = require("./broker");
import Publish = require("./publish");
import moment = require('moment');
import FairValue = require("./fair-value");
import QuotingParameters = require("./quoting-parameters");
import PositionManagement = require("./position-management");

interface ITrade {
    price: number;
    quantity: number;
    time: Date;
}

export class SafetyCalculator {
    private _latest: Models.TradeSafety = null;
    public get latest() { return this._latest; }
    public set latest(val: Models.TradeSafety) {
        if (!this._latest || Math.abs(val.combined - this._latest.combined) > 1e-3
          || Math.abs(val.buyPing - this._latest.buyPing) >= 1e-2
          || Math.abs(val.sellPong - this._latest.sellPong) >= 1e-2) {
            this._latest = val;
            this._evUp('Safety');
            this._publisher.publish(Models.Topics.TradeSafetyValue, this.latest, true);
        }
    }

    private _buys: ITrade[] = [];
    private _sells: ITrade[] = [];

    public targetPosition: PositionManagement.TargetBasePositionManager;

    constructor(
      private _timeProvider: Utils.ITimeProvider,
      private _fvEngine: FairValue.FairValueEngine,
      private _qpRepo: QuotingParameters.QuotingParametersRepository,
      private _positionBroker: Broker.PositionBroker,
      private _orderBroker: Broker.OrderBroker,
      private _publisher: Publish.Publisher,
      private _evOn,
      private _evUp
    ) {
      _publisher.registerSnapshot(Models.Topics.TradeSafetyValue, () => [this.latest]);
      this._evOn('OrderTradeBroker', this.onTrade);

      this._evOn('QuotingParameters', this.computeQtyLimit);
      _timeProvider.setInterval(this.computeQtyLimit, moment.duration(1, "seconds"));
    }

    private onTrade = (ut: Models.Trade) => {
        if (this.isOlderThan(ut.time)) return;

        this[ut.side === Models.Side.Ask ? '_sells' : '_buys'].push(<ITrade>{
          price: ut.price,
          quantity: ut.quantity,
          time: ut.time
        });

        this.computeQtyLimit();
    };

    private isOlderThan(time: Date) {
        return Math.abs(this._timeProvider.utcNow().valueOf() - time.valueOf()) > this._qpRepo.latest.tradeRateSeconds * 1000;
    }

    private computeQtyLimit = () => {
        var fv = this._fvEngine.latestFairValue;
        if (!fv || !this.targetPosition.latestTargetPosition || !this._positionBroker.latestReport) return;
        const settings = this._qpRepo.latest;
        const latestPosition = this._positionBroker.latestReport;
        let buySize: number  = (settings.percentageValues && latestPosition != null)
            ? settings.buySizePercentage * latestPosition.value / 100
            : settings.buySize;
        let sellSize: number = (settings.percentageValues && latestPosition != null)
              ? settings.sellSizePercentage * latestPosition.value / 100
              : settings.sellSize;
        const targetBasePosition = this.targetPosition.latestTargetPosition.data;
        const totalBasePosition = latestPosition.baseAmount + latestPosition.baseHeldAmount;
        if (settings.aggressivePositionRebalancing != Models.APR.Off && settings.buySizeMax) buySize = Math.max(buySize, targetBasePosition - totalBasePosition);
        if (settings.aggressivePositionRebalancing != Models.APR.Off && settings.sellSizeMax) sellSize = Math.max(sellSize, totalBasePosition - targetBasePosition);

        var buyPing = 0;
        var sellPong = 0;
        var buyPq = 0;
        var sellPq = 0;
        var _buyPq = 0;
        var _sellPq = 0;
        var trades = this._orderBroker.tradesMemory;
        var widthPong = (settings.widthPercentage)
            ? settings.widthPongPercentage * fv.price / 100
            : settings.widthPong;
        if (settings.pongAt == Models.PongAt.ShortPingFair || settings.pongAt == Models.PongAt.ShortPingAggressive) {
          trades.sort((a: Models.Trade, b: Models.Trade) => a.price>b.price?1:(a.price<b.price?-1:0));
          for (var ti = 0;ti<trades.length;ti++) {
            if ((!fv.price || (fv.price>trades[ti].price && fv.price-settings.widthPong<trades[ti].price)) && ((settings.mode !== Models.QuotingMode.Boomerang && settings.mode !== Models.QuotingMode.HamelinRat && settings.mode !== Models.QuotingMode.AK47) || trades[ti].Kqty<trades[ti].quantity) && trades[ti].side == Models.Side.Bid && buyPq<sellSize) {
              _buyPq = Math.min(sellSize - buyPq, trades[ti].quantity);
              buyPing += trades[ti].price * _buyPq;
              buyPq += _buyPq;
            }
            if (buyPq>=sellSize) break;
          }
          trades.sort((a: Models.Trade, b: Models.Trade) => a.price<b.price?1:(a.price>b.price?-1:0));
        } else if (settings.pongAt == Models.PongAt.LongPingFair || settings.pongAt == Models.PongAt.LongPingAggressive)
          trades.sort((a: Models.Trade, b: Models.Trade) => a.price>b.price?1:(a.price<b.price?-1:0));
        if (!buyPq) for (var ti = 0;ti<trades.length;ti++) {
          if ((!fv.price || fv.price>trades[ti].price) && ((settings.mode !== Models.QuotingMode.Boomerang && settings.mode !== Models.QuotingMode.HamelinRat && settings.mode !== Models.QuotingMode.AK47) || trades[ti].Kqty<trades[ti].quantity) && trades[ti].side == Models.Side.Bid && buyPq<sellSize) {
            _buyPq = Math.min(sellSize - buyPq, trades[ti].quantity);
            buyPing += trades[ti].price * _buyPq;
            buyPq += _buyPq;
          }
          if (buyPq>=sellSize) break;
        }
        if (settings.pongAt == Models.PongAt.ShortPingFair || settings.pongAt == Models.PongAt.ShortPingAggressive) {
          trades.sort((a: Models.Trade, b: Models.Trade) => a.price<b.price?1:(a.price>b.price?-1:0));
          for (var ti = 0;ti<trades.length;ti++) {
            if ((!fv.price || (fv.price<trades[ti].price && fv.price+settings.widthPong>trades[ti].price)) && ((settings.mode !== Models.QuotingMode.Boomerang && settings.mode !== Models.QuotingMode.HamelinRat && settings.mode !== Models.QuotingMode.AK47) || trades[ti].Kqty<trades[ti].quantity) && trades[ti].side == Models.Side.Ask && sellPq<buySize) {
              _sellPq = Math.min(buySize - sellPq, trades[ti].quantity);
              sellPong += trades[ti].price * _sellPq;
              sellPq += _sellPq;
            }
            if (sellPq>=buySize) break;
          }
          trades.sort((a: Models.Trade, b: Models.Trade) => a.price>b.price?1:(a.price<b.price?-1:0));
        } else if (settings.pongAt == Models.PongAt.LongPingFair || settings.pongAt == Models.PongAt.LongPingAggressive)
          trades.sort((a: Models.Trade, b: Models.Trade) => a.price<b.price?1:(a.price>b.price?-1:0));
        if (!sellPq) for (var ti = 0;ti<trades.length;ti++) {
          if ((!fv.price || fv.price<trades[ti].price) && ((settings.mode !== Models.QuotingMode.Boomerang && settings.mode !== Models.QuotingMode.HamelinRat && settings.mode !== Models.QuotingMode.AK47) || trades[ti].Kqty<trades[ti].quantity) && trades[ti].side == Models.Side.Ask && sellPq<buySize) {
            _sellPq = Math.min(buySize - sellPq, trades[ti].quantity);
            sellPong += trades[ti].price * _sellPq;
            sellPq += _sellPq;
          }
          if (sellPq>=buySize) break;
        }

        if (buyPq) buyPing /= buyPq;
        if (sellPq) sellPong /= sellPq;

        this._buys = this._buys.filter(o => !this.isOlderThan(o.time))
          .sort(function(a,b){return a.price>b.price?-1:(a.price<b.price?1:0)});
        this._sells = this._sells.filter(o => !this.isOlderThan(o.time))
          .sort(function(a,b){return a.price>b.price?1:(a.price<b.price?-1:0)});

        // don't count good trades against safety
        while (this._buys.length && this._sells.length) {
            var sell = this._sells.slice(-1).pop();
            var buy = this._buys.slice(-1).pop();
            if (sell.price >= buy.price) {

                var sellQty = sell.quantity;
                var buyQty = buy.quantity;

                buy.quantity -= sellQty;
                sell.quantity -= buyQty;

                if (buy.quantity < 1e-4) this._buys.pop();
                if (sell.quantity < 1e-4) this._sells.pop();
            }
            else {
                break;
            }
        }

        this.latest = new Models.TradeSafety(
          ((t: ITrade[]) => t.reduce((sum, t) => sum + t.quantity, 0) / buySize)(this._buys),
          ((t: ITrade[]) => t.reduce((sum, t) => sum + t.quantity, 0) / sellSize)(this._sells),
          ((t: ITrade[]) => t.reduce((sum, t) => sum + t.quantity, 0) / (buySize + sellSize / 2))(this._buys.concat(this._sells)),
          buyPing,
          sellPong,
          this._timeProvider.utcNow()
        );
    };
}
