import {NgZone, Component, Inject, Input, OnInit} from '@angular/core';
import moment = require('moment');

import Models = require('../share/models');
import {SubscriberFactory} from './shared_directives';

@Component({
  selector: 'market-quoting',
  template: `<div class="tradeSafety2" style="margin-top:-4px;padding-top:0px;padding-right:0px;"><div style="padding-top:0px;padding-right:0px;">
      Market Width: <span class="{{ diffMD ? \'text-danger\' : \'text-muted\' }}">{{ diffMD | number:'1.'+product.fixed+'-'+product.fixed }}</span>,
      Quote Width: <span class="{{ diffPx ? \'text-danger\' : \'text-muted\' }}">{{ diffPx | number:'1.'+product.fixed+'-'+product.fixed }}</span>
      <div style="padding-left:0px;">Wallet TBP: <span class="text-danger">{{ targetBasePosition | number:'1.3-3' }}</span>, APR: <span class="{{ sideAPRSafety!=\'Off\' ? \'text-danger\' : \'text-muted\' }}">{{ sideAPRSafety }}</span>, Quotes: <span title="New Quotes in memory" class="{{ quotesInMemoryNew ? \'text-danger\' : \'text-muted\' }}">{{ quotesInMemoryNew }}</span>/<span title="Working Quotes in memory" class="{{ quotesInMemoryWorking ? \'text-danger\' : \'text-muted\' }}">{{ quotesInMemoryWorking }}</span>/<span title="Other Quotes in memory" class="{{ quotesInMemoryDone ? \'text-danger\' : \'text-muted\' }}">{{ quotesInMemoryDone }}</span></div>
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
        <td [ngClass]="level.bidClass"><div style="z-index:2;position:relative;" [ngClass]="'bidsz' + i + ' num'">{{ level.bidSize | number:'1.4-4' }}</div><div style="float:right;margin-right:19px;"><div [ngClass]="level.bidClassVisual">&nbsp;</div></div></td>
        <td [ngClass]="level.bidClass"><div [ngClass]="'bidsz' + i">{{ level.bidPrice | number:'1.'+product.fixed+'-'+product.fixed }}</div></td>
        <td [ngClass]="level.askClass"><div [ngClass]="'asksz' + i">{{ level.askPrice | number:'1.'+product.fixed+'-'+product.fixed }}</div></td>
        <td [ngClass]="level.askClass"><div style="float:left;"><div [ngClass]="level.askClassVisual">&nbsp;</div></div><div style="z-index:2;position:relative;" [ngClass]="'asksz' + i + ' num'">{{ level.askSize | number:'1.4-4' }}</div></td>
      </tr>
    </table></div>`
})
export class MarketQuotingComponent implements OnInit {

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
  private sideAPRSafety: string;
  @Input() product: Models.ProductState;

  @Input() set connected(connected: boolean) {
    if (connected) return;
    this.clearQuote();
    this.updateQuoteClass();
  }

  constructor(
    @Inject(NgZone) private zone: NgZone,
    @Inject(SubscriberFactory) private subscriberFactory: SubscriberFactory
  ) {
    this.clearMarket();
    this.clearQuote();
  }

  ngOnInit() {
    [
      [Models.Topics.MarketData, this.updateMarket, this.clearMarket],
      [Models.Topics.OrderStatusReports, this.updateQuote, this.clearQuote],
      [Models.Topics.QuoteStatus, this.updateQuoteStatus, this.clearQuoteStatus],
      [Models.Topics.TargetBasePosition, this.updateTargetBasePosition, this.clearTargetBasePosition]
    ].forEach(x => (<T>(topic: string, updateFn, clearFn) => {
      this.subscriberFactory
        .getSubscriber<T>(this.zone, topic)
        .registerConnectHandler(clearFn)
        .registerSubscriber(updateFn);
    }).call(this, x[0], x[1], x[2]));
  }

  private clearMarket = () => {
    this.levels = [];
  }

  private clearTargetBasePosition = () => {
    this.targetBasePosition = null;
    this.sideAPRSafety = null;
  }

  private clearQuote = () => {
    this.orderBids = [];
    this.orderAsks = [];
  }

  private clearQuoteStatus = () => {
    this.bidStatus = Models.QuoteStatus[1];
    this.askStatus = Models.QuoteStatus[1];
    this.quotesInMemoryNew = 0;
    this.quotesInMemoryWorking = 0;
    this.quotesInMemoryDone = 0;
  }

  private updateTargetBasePosition = (value : Models.TargetBasePositionValue) => {
    if (value == null) return;
    this.targetBasePosition = value.data;
    this.sideAPRSafety = value.sideAPR || 'Off';
  }

  private updateMarket = (update: Models.Timestamped<any[]>) => {
    if (update == null) {
      this.clearMarket();
      return;
    }

    for (var i: number = 0; i < this.orderAsks.length; i++)
      if (!update.data[1].filter(x => x===this.orderAsks[i].price).length) {
        for (var j: number = 0; j < update.data[1].length;j++)
          if (update.data[1][j++]>this.orderAsks[i].price) break;
        update.data[1].splice(j-(j==update.data[1].length?0:1), 0, this.orderAsks[i].price, this.orderAsks[i].quantity);
        update.data[1] = update.data[1].slice(0, -2);
      }
    for (var i: number = 0; i < this.orderBids.length; i++)
      if (!update.data[0].filter(x => x===this.orderBids[i].price).length) {
        for (var j: number = 0; j < update.data[0].length;j++)
          if (update.data[0][j++]<this.orderBids[i].price) break;
        update.data[0].splice(j-(j==update.data[0].length?0:1), 0, this.orderBids[i].price, this.orderBids[i].quantity);
        update.data[0] = update.data[0].slice(0, -2);
      }

    var _levels = [];
    for (var i: number = 0, j: number = 0; i < update.data[1].length; i++, j++) {
      if (j >= _levels.length) _levels[j] = <any>{};
      _levels[j] = Object.assign(_levels[j], { askPrice: update.data[1][i], askSize: update.data[1][++i] });
    }

    for (var i: number = 0, j: number = 0; i < update.data[0].length; i++, j++) {
      if (j >= _levels.length) _levels[j] = <any>{};
      _levels[j] = Object.assign(_levels[j], { bidPrice: update.data[0][i], bidSize: update.data[0][++i] });
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

  private updateQuote = (o: Models.Timestamped<any[]>) => {
    if (typeof o.data[0] === 'object') {
      this.clearQuote();
      return o.data.forEach(x => setTimeout(this.updateQuote(x), 0));
    }
    const orderSide = o.data[2] === Models.Side.Bid ? 'orderBids' : 'orderAsks';
    if (o.data[1] == Models.OrderStatus.Cancelled
      || o.data[1] == Models.OrderStatus.Complete
    ) this[orderSide] = this[orderSide].filter(x => x.orderId !== o.data[0]);
    else if (!this[orderSide].filter(x => x.orderId === o.data[0]).length)
      this[orderSide].push({
        orderId: o.data[0],
        side: o.data[2],
        quantity: o.data[5],
        price: o.data[4]
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

  private updateQuoteStatus = (status: Models.TwoSidedQuoteStatus) => {
    if (status == null) {
      this.clearQuoteStatus();
      return;
    }

    this.bidStatus = Models.QuoteStatus[status.bidStatus];
    this.askStatus = Models.QuoteStatus[status.askStatus];
    this.quotesInMemoryNew = status.quotesInMemoryNew;
    this.quotesInMemoryWorking = status.quotesInMemoryWorking;
    this.quotesInMemoryDone = status.quotesInMemoryDone;
  }

  private updateQuoteClass = (levels?: any[]) => {
    if (document.body.className != "visible") return;
    if (levels && levels.length > 0) {
      for (let i = 0; i < levels.length; i++) {
        if (i >= this.levels.length) this.levels[i] = <any>{ };
        if (levels[i].bidMod===1) (<any>jQuery)('.bidsz'+i+'.num').addClass('buy');
        if (levels[i].askMod===1) (<any>jQuery)('.asksz'+i+'.num').addClass('sell');
        (<any>jQuery)('.bidsz'+i).css( 'opacity', levels[i].bidMod===2?0.4:1.0 );
        (<any>jQuery)('.asksz'+i).css( 'opacity', levels[i].askMod===2?0.4:1.0 );
        setTimeout(() => {
          (<any>jQuery)('.bidsz'+i).css( 'opacity', levels[i].bidMod===2?0.0:1.0 );
          (<any>jQuery)('.asksz'+i).css( 'opacity', levels[i].askMod===2?0.0:1.0 );
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
            setTimeout(() => { (<any>jQuery)('.asksz'+i+', .bidsz'+i).css( 'opacity', 1.0 ); (<any>jQuery)('.asksz'+i+'.num'+', .bidsz'+i+'.num').removeClass('sell').removeClass('buy'); }, 1);
          }, 0);
        }, 221);
      }
    }
  }
}
