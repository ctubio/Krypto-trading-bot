import {Component, Input} from '@angular/core';

import * as Models from './models';

@Component({
  selector: 'market-quoting',
  template: `<div class="tradeSafety2" style="margin-top:-4px;padding-top:0px;padding-right:0px;"><div style="padding-top:0px;padding-right:0px;">
      Market Width: <span class="{{ diffMD ? \'text-danger\' : \'text-muted\' }}">{{ diffMD | number:'1.'+product.fixed+'-'+product.fixed }}</span>,
      Quote Width: <span class="{{ diffPx ? \'text-danger\' : \'text-muted\' }}">{{ diffPx | number:'1.'+product.fixed+'-'+product.fixed }}</span>, Quotes: <span title="New Quotes in memory" class="{{ quotesInMemoryNew ? \'text-danger\' : \'text-muted\' }}">{{ quotesInMemoryNew }}</span>/<span title="Working Quotes in memory" class="{{ quotesInMemoryWorking ? \'text-danger\' : \'text-muted\' }}">{{ quotesInMemoryWorking }}</span>/<span title="Other Quotes in memory" class="{{ quotesInMemoryDone ? \'text-danger\' : \'text-muted\' }}">{{ quotesInMemoryDone }}</span>
      <div style="padding-left:0px;">Wallet TBP: <span class="text-danger">{{ targetBasePosition | number:'1.3-3' }}</span>, pDiv: <span class="text-danger">{{ positionDivergence | number:'1.3-3' }}</span>, APR: <span class="{{ sideAPRSafety!=\'Off\' ? \'text-danger\' : \'text-muted\' }}">{{ sideAPRSafety }}</span></div>
      </div></div><div style="padding-right:4px;padding-left:4px;padding-top:4px;"><table class="marketQuoting table table-hover table-responsive text-center">
      <tr class="active">
        <td>bidSize&nbsp;</td>
        <td>bidPrice</td>
        <td>askPrice</td>
        <td>askSize&nbsp;</td>
      </tr>
      <tr class="info">
        <th *ngIf="bidStatus == 'Live'" class="text-danger">{{ qBidSz | number:'1.4-4' }}<span *ngIf="!qBidSz">&nbsp;</span></th>
        <th *ngIf="bidStatus == 'Live'" class="text-danger">{{ qBidPx | number:'1.'+product.fixed+'-'+product.fixed }}</th>
        <th *ngIf="bidStatus != 'Live'" colspan="2" class="text-danger" title="Bids Quote Status">{{ bidStatus }}</th>
        <th *ngIf="askStatus == 'Live'" class="text-danger">{{ qAskPx | number:'1.'+product.fixed+'-'+product.fixed }}</th>
        <th *ngIf="askStatus == 'Live'" class="text-danger">{{ qAskSz | number:'1.4-4' }}<span *ngIf="!qAskSz">&nbsp;</span></th>
        <th *ngIf="askStatus != 'Live'" colspan="2" class="text-danger" title="Ask Quote Status">{{ askStatus }}</th>
      </tr>
      <tr class="active" *ngFor="let level of levels; let i = index">
        <td *ngIf="i == 1 && levels.length == 4" colspan="4"><div class="text-danger" style="height:174px;"><br />To <a href="{{ product.advert.homepage }}/blob/master/README.md#unlock" target="_blank">unlock</a> all market levels<br />and to collaborate with the development..<br /><br />make an acceptable Pull Request on github,<br/>or send 0.01210000 BTC or more to:<br /><a href="https://www.blocktrail.com/BTC/address/{{ a }}" target="_blank">{{ a }}</a><br /><br />Wait 0 confirmations and restart this bot.<!-- you can remove this message, but obviously the missing market levels will not be displayed magically. the market levels will be only displayed if the also displayed address is credited with 0.01210000 BTC. Note that if you make a Pull Request i will credit the payment for you easy, just let me know in the description of the PR what is the BTC Address displayed in your bot. --></div></td>
        <td *ngIf="i != 1 || levels.length != 4" [ngClass]="level.bidClass"><div style="z-index:2;position:relative;" [ngClass]="'bidsz' + i + ' num'">{{ level.bidSize | number:'1.4-4' }}</div><div style="float:right;margin-right:19px;"><div [ngClass]="level.bidClassVisual">&nbsp;</div></div></td>
        <td *ngIf="i != 1 || levels.length != 4" [ngClass]="level.bidClass"><div [ngClass]="'bidsz' + i">{{ level.bidPrice | number:'1.'+product.fixed+'-'+product.fixed }}</div></td>
        <td *ngIf="i != 1 || levels.length != 4" [ngClass]="level.askClass"><div [ngClass]="'asksz' + i">{{ level.askPrice | number:'1.'+product.fixed+'-'+product.fixed }}</div></td>
        <td *ngIf="i != 1 || levels.length != 4" [ngClass]="level.askClass"><div style="float:left;"><div [ngClass]="level.askClassVisual">&nbsp;</div></div><div style="z-index:2;position:relative;" [ngClass]="'asksz' + i + ' num'">{{ level.askSize | number:'1.4-4' }}</div></td>
      </tr>
    </table></div>`
})
export class MarketQuotingComponent {

  public levels: any[];
  public qBidSz: number;
  public qBidPx: number;
  public qAskPx: number;
  public qAskSz: number;
  public orderBids: any[];
  public orderAsks: any[];
  public bidStatus: string;
  public askStatus: string;
  public quotesInMemoryNew: number;
  public quotesInMemoryWorking: number;
  public quotesInMemoryDone: number;
  public diffMD: number;
  public diffPx: number;
  public noBidReason: string;
  public noAskReason: string;
  private targetBasePosition: number;
  private positionDivergence: number;
  private sideAPRSafety: string;

  @Input() product: Models.ProductState;

  @Input() a: string;

  @Input() set online(online: boolean) {
    if (online) return;
    this.clearQuote();
    this.updateQuoteClass();
  }

  @Input() set setOrderList(o: any[]) {
    this.updateQuote(o);
  }

  @Input() set setTargetBasePosition(o: Models.TargetBasePositionValue) {
    if (o == null) {
      this.targetBasePosition = null;
      this.sideAPRSafety = null;
      this.positionDivergence = null;
    } else {
      this.targetBasePosition = o.tbp;
      this.sideAPRSafety = o.sideAPR || 'Off';
      this.positionDivergence = o.pDiv;
    }
  }

  @Input() set setQuoteStatus(o) {
    if (o == null) {
      this.bidStatus = Models.QuoteStatus[1];
      this.askStatus = Models.QuoteStatus[1];
      this.quotesInMemoryNew = 0;
      this.quotesInMemoryWorking = 0;
      this.quotesInMemoryDone = 0;
    } else {
      this.bidStatus = Models.QuoteStatus[o.bidStatus];
      this.askStatus = Models.QuoteStatus[o.askStatus];
      this.quotesInMemoryNew = o.quotesInMemoryNew;
      this.quotesInMemoryWorking = o.quotesInMemoryWorking;
      this.quotesInMemoryDone = o.quotesInMemoryDone;
    }
  }

  private clearQuote = () => {
    this.orderBids = [];
    this.orderAsks = [];
  }

  @Input() set setMarketData(update: Models.Market) {
    if (update == null || typeof update.bids == "undefined" || typeof update.asks == "undefined" || !update.bids || !update.asks || !update.bids.length || !update.asks.length) {
      this.levels = [];
      return;
    }

    for (var i: number = 0; i < this.orderAsks.length; i++)
      if (!update.asks.filter(x => x.price===this.orderAsks[i].price).length) {
        for (var j: number = 0; j < update.asks.length;j++)
          if (update.asks[j].price>this.orderAsks[i].price) break;
        update.asks.splice(j-(j==update.asks.length?0:1), 0, {price:this.orderAsks[i].price, size:this.orderAsks[i].quantity});
        update.asks = update.asks.slice(0, -1);
      }
    for (var i: number = 0; i < this.orderBids.length; i++)
      if (!update.bids.filter(x => x.price===this.orderBids[i].price).length) {
        for (var j: number = 0; j < update.bids.length;j++)
          if (update.bids[j].price<this.orderBids[i].price) break;
        update.bids.splice(j-(j==update.bids.length?0:1), 0, {price:this.orderBids[i].price, size:this.orderBids[i].quantity});
        update.bids = update.bids.slice(0, -1);
      }

    var _levels = [];
    for (var j: number = 0; j < update.asks.length; j++) {
      if (j >= _levels.length) _levels[j] = <any>{};
      _levels[j] = Object.assign(_levels[j], { askPrice: update.asks[j].price, askSize: update.asks[j].size });
    }

    for (var j: number = 0; j < update.bids.length; j++) {
      if (j >= _levels.length) _levels[j] = <any>{};
      _levels[j] = Object.assign(_levels[j], { bidPrice: update.bids[j].price, bidSize: update.bids[j].size });
      if (j==0) this.diffMD = _levels[j].askPrice - _levels[j].bidPrice;
      else if (j==1) this.diffPx = Math.max((this.qAskPx && this.qBidPx) ? this.qAskPx - this.qBidPx : 0, 0);
    }

    var modAsk: number;
    var modBid: number;
    for (var i: number = this.levels.length;i--;) {
      if (i >= _levels.length) {
        _levels[i] = <any>{};
        continue;
      }
      modAsk = 2;
      modBid = 2;
      for (var h: number = _levels.length;h--;) {
        if (modAsk===2 && this.levels[i].askPrice===_levels[h].askPrice)
          modAsk = this.levels[i].askSize!==_levels[h].askSize ? 1 : 0;
        if (modBid===2 && this.levels[i].bidPrice===_levels[h].bidPrice)
          modBid = this.levels[i].bidSize!==_levels[h].bidSize ? 1 : 0;
        if (modBid!==2 && modAsk!==2) break;
      }
      _levels[i] = Object.assign(_levels[i], { bidMod: modBid, askMod: modAsk });
    }
    for (var h: number = _levels.length;h--;) {
      modAsk = 0;
      modBid = 0;
      for (var i: number = this.levels.length;i--;) {
        if (!modAsk && this.levels[i].askPrice===_levels[h].askPrice)
          modAsk = 1;
        if (!modBid && this.levels[i].bidPrice===_levels[h].bidPrice)
          modBid = 1;
        if (modBid && modAsk) break;
      }
      if (!modBid) _levels[h] = Object.assign(_levels[h], { bidMod: 1 });
      if (!modAsk) _levels[h] = Object.assign(_levels[h], { askMod: 1 });
    }
    this.updateQuoteClass(_levels);
  }

  private updateQuote = (o) => {
    if (!o || (typeof o.length == 'number' && !o.length)) {
      this.clearQuote();
      return;
    } else if (typeof o.length == 'number' && typeof o[0] == 'object') {
      this.clearQuote();
      return o.forEach(x => setTimeout(this.updateQuote(x), 0));
    }

    const orderSide = o.side === Models.Side.Bid ? 'orderBids' : 'orderAsks';
    if (o.orderStatus == Models.OrderStatus.Cancelled
      || o.orderStatus == Models.OrderStatus.Complete
    ) this[orderSide] = this[orderSide].filter(x => x.orderId !== o.orderId);
    else if (!this[orderSide].filter(x => x.orderId === o.orderId).length)
      this[orderSide].push({
        orderId: o.orderId,
        side: o.side,
        price: o.price,
        quantity: o.quantity,
      });

    if (this.orderBids.length) {
      var bid = this.orderBids.reduce((a,b)=>a.price>b.price?a:b);
      this.qBidPx = bid.price;
      this.qBidSz = bid.quantity;
    } else {
      this.qBidPx = null;
      this.qBidSz = null;
    }
    if (this.orderAsks.length) {
      var ask = this.orderAsks.reduce((a,b)=>a.price<b.price?a:b);
      this.qAskPx = ask.price;
      this.qAskSz = ask.quantity;
    } else {
      this.qAskPx = null;
      this.qAskSz = null;
    }

    this.updateQuoteClass();
  }

  private forEach = (array, callback) => {
    for (var i = 0; i < array.length; i++)
      callback.call(window, array[i]);
  }

  private updateQuoteClass = (levels?: any[]) => {
    if (document.body.className != "visible") return;
    if (levels && levels.length > 0) {
      for (let i = 0; i < levels.length; i++) {
        if (i >= this.levels.length) this.levels[i] = <any>{ };
        if (levels[i].bidMod===1)
          this.forEach(document.querySelectorAll('.bidsz'+i), function (el) {
            if (el.className.indexOf('num')>-1 && el.className.indexOf('buy')==-1) el.className += ' buy';
          });
        if (levels[i].askMod===1)
          this.forEach(document.querySelectorAll('.asksz'+i), function (el) {
            if (el.className.indexOf('num')>-1 && el.className.indexOf('sell')==-1) el.className += ' sell';
          });
        this.forEach(document.querySelectorAll('.bidsz'+i), function (el) {
          el.style.opacity = levels[i].bidMod===2?0.4:1.0;
        });
        this.forEach(document.querySelectorAll('.asksz'+i), function (el) {
          el.style.opacity = levels[i].askMod===2?0.4:1.0;
        });
        setTimeout(() => {
          this.forEach(document.querySelectorAll('.bidsz'+i), function (el) {
            el.style.opacity = levels[i].bidMod===2?0.0:1.0;
          });
          this.forEach(document.querySelectorAll('.asksz'+i), function (el) {
            el.style.opacity =  levels[i].askMod===2?0.0:1.0;
          });
          setTimeout(() => {
            this.levels[i] = Object.assign(this.levels[i], { bidPrice: levels[i].bidPrice, bidSize: levels[i].bidSize, askPrice: levels[i].askPrice, askSize: levels[i].askSize });
            this.levels[i].bidClass = 'active';
            for (var j = 0; j < this.orderBids.length; j++)
              if (this.orderBids[j].price === this.levels[i].bidPrice)
                this.levels[i].bidClass = 'success buy';
            this.levels[i].bidClassVisual = String('vsBuy visualSize').concat(<any>Math.round(Math.max(Math.min((Math.log(this.levels[i].bidSize)/Math.log(2))*4,19),1)));
            this.levels[i].askClass = 'active';
            for (var j = 0; j < this.orderAsks.length; j++)
              if (this.orderAsks[j].price === this.levels[i].askPrice)
                this.levels[i].askClass = 'success sell';
            this.levels[i].askClassVisual = String('vsAsk visualSize').concat(<any>Math.round(Math.max(Math.min((Math.log(this.levels[i].askSize)/Math.log(2))*4,19),1)));
            setTimeout(() => {
              this.forEach(document.querySelectorAll('.asksz'+i), function (el) {
                el.style.opacity = 1.0;
                if (el.className.indexOf('num')>-1)
                  el.className = el.className.replace(' sell', '');
              });
              this.forEach(document.querySelectorAll('.bidsz'+i), function (el) {
                el.style.opacity = 1.0;
                if (el.className.indexOf('num')>-1)
                  el.className = el.className.replace(' buy', '');
              });
            }, 1);
          }, 0);
        }, 221);
      }
    }
  }
}
