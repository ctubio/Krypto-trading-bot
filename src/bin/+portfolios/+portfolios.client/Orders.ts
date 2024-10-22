import {Component, Input} from '@angular/core';

import {GridOptions, GridApi} from 'ag-grid-community';

import {Shared, Socket, Models} from 'lib/K';

@Component({
  selector: 'orders',
  template: `<div id="openorders">
    <h2>
      <span [hidden]="!!symbols">0 open orders</span>
      <a rel="noreferrer" target="_blank" title="{{ filter }} Market"
        href="{{ orders_market }}" id="best_ask_bid" [hidden]="!filter">
          <span class="sell" title="Best Ask Price on {{ filter }}">{{ best_ask }} <i class="beacon sym-_default-s sym-{{ quote.toLowerCase() }}-s" ></i></span>
          <span class="buy" title="Best Bid Price on {{ filter }}">{{ best_bid }} <i class="beacon sym-_default-s sym-{{ quote.toLowerCase() }}-s" ></i></span>
      </a>
      <a href="#" 
        *ngFor="let x of symbols"
        (click)="applyFilter(x.symbol)"
        [ngStyle]="{'cursor':'pointer','padding': '0px 40px 0px 10px', 'font-weight': filter==x.symbol?600:300, 'opacity': filter==x.symbol?1.0:0.7}"
      >{{ x.symbol }}<span class="fracwrap"><span class="frac">
          <span title="{{x.asks}} open ask orders">{{ x.asks }}</span>
          <span class="symbol">/</span>
          <span title="{{x.bids}} open bid orders" class="bottom">{{ x.bids }}</span>
        </span></span>
       </a>
    </h2>
    <ag-grid-angular
      [hidden]="!filter || !symbols"
      class="ag-theme-alpine ag-theme-big"
      style="width: 100%;"
      (window:resize)="onGridReady($event)"
      (gridReady)="onGridReady($event)"
      (cellClicked)="onCellClicked($event)"
      (cellValueChanged)="onCellValueChanged($event)"
      [gridOptions]="grid"></ag-grid-angular>
  </div>`
})
export class OrdersComponent {

  private fireCxl: Socket.IFire<Models.OrderCancelRequestFromUI> = new Socket.Fire(Models.Topics.CancelOrder);

  private editCxl: Socket.IFire<Models.OrderEditRequestFromUI> = new Socket.Fire(Models.Topics.EditOrder);

  @Input() product: Models.ProductAdvertisement;

  private best_ask: string;
  private best_bid: string;
  private quote: string = "";
  private base: string = "";
  private orders_market: string;

  private _markets: any = null;

  @Input() set markets(o: any) {
    this._markets = o;
    this.addAskBid();
  };

  @Input() set orders(o: Models.Order[]) {
    this.addRowData(o);
  };

  private symbols: any[] = [];
  private filter: string;

  private api: GridApi;

  private grid: GridOptions = <GridOptions>{
    suppressNoRowsOverlay: true,
    defaultColDef: { sortable: true, resizable: true, flex: 1 },
    rowHeight:35,
    headerHeight:35,
    domLayout: 'autoHeight',
    getRowId: (params: any) => params.data.exchangeId,
    isExternalFilterPresent: () => !!this.filter,
    doesExternalFilterPass: (node) => (
      !this.filter || node.data.symbol == this.filter
    ),
    columnDefs: [{
      width: 20,
      field: "cancel",
      flex: 0,
      headerName: 'cxl',
      suppressSizeToFit: true,
      cellRenderer: (params) => `<button type="button" class="btn btn-danger btn-xs">
          <span data-action-type="remove"'>&#10006;</span>
        </button>`
    }, {
      width: 74,
      field: 'price',
      headerName: 'price',
      sort: 'desc',
      editable: true,
      cellEditorSelector: (params) => {
        return { component: "agNumberCellEditor", params: {
          precision: params.data.pricePrecision,
          min:  parseFloat(Math.pow(10, -params.data.pricePrecision).toFixed(params.data.pricePrecision)),
          step: parseFloat(Math.pow(10, -params.data.pricePrecision).toFixed(params.data.pricePrecision)),
          showStepperButtons: true
        } };
      },
      cellRenderer: (params) => Shared.str(params.value, params.data.pricePrecision) + ` <i class="beacon sym-_default-s sym-` + this.quote.toLowerCase() + `-s" ></i>` + ' <span title="double-click to edit" style="transform: rotate(90deg);display: inline-block;">&#9998;</span>',
      cellClassRules: {
        'sell': 'data.side == "Ask"',
        'buy': 'data.side == "Bid"'
      }
    }, {
      width: 95,
      field: 'quantity',
      headerName: 'qty',
      suppressSizeToFit: true,
      cellRenderer: (params) => Shared.str(params.value, params.data.quantityPrecision) + ` <i class="beacon sym-_default-s sym-` + this.base.toLowerCase() + `-s" ></i>`,
      cellClassRules: {
        'sell': 'data.side == "Ask"',
        'buy': 'data.side == "Bid"'
      }
    }, {
      width: 80,
      field: 'side',
      headerName: 'side',
      flex: 0,
      suppressSizeToFit: true,
      cellClassRules: {
        'sell': 'data.side == "Ask"',
        'buy': 'data.side == "Bid"'
      },
      cellRenderer: (params) => (
        params.value == "Ask"
          ? '<span style="transform: rotate(+90deg);display: inline-block;">&#10140;</span>'
          : '<span style="transform: rotate(-90deg);display: inline-block;">&#10140;</span>'
        ) + params.value
    }, {
      width: 74,
      field: 'value',
      headerName: 'value',
      cellRenderer: (params) => Shared.str(params.value, params.data.pricePrecision) + ` <i class="beacon sym-_default-s sym-` + this.quote.toLowerCase() + `-s" ></i>`,
      cellClassRules: {
        'sell': 'data.side == "Ask"',
        'buy': 'data.side == "Bid"'
      }
    }, {
      width: 100,
      field: 'time',
      flex: 0,
      headerName: 'time',
      suppressSizeToFit: true,
      cellRenderer: (params) => {
        var d = new Date(params.value||0);
        return (d.getHours()+'')
          .padStart(2, "0")+':'+(d.getMinutes()+'')
          .padStart(2, "0")+':'+(d.getSeconds()+'')
          .padStart(2, "0")+','+(d.getMilliseconds()+'')
          .padStart(3, "0");
      }
    }, {
      width: 75,
      field: 'type',
      headerName: 'type',
      flex: 0,
      suppressSizeToFit: true
    }, {
      width: 50,
      field: 'tif',
      flex: 0,
      headerName: 'tif'
    }, {
      width: 75,
      field: 'lat',
      flex: 0,
      headerName: 'lat'
    }, {
      width: 130,
      field: 'exchangeId',
      flex: 0,
      headerName: 'openOrderId',
      suppressSizeToFit: true,
      cellRenderer: (params) => params.value
        ? params.value.toString().split('-')[0]
        : ''
    }]
  };

  private onGridReady($event: any) {
    if ($event.api) this.api = $event.api;
  };

  private onCellClicked = ($event) => {
    if ($event.event.target.getAttribute('data-action-type') != 'remove') return;
    this.fireCxl.fire(new Models.OrderCancelRequestFromUI($event.data.orderId, $event.data.exchange));
  };
  
  private onCellValueChanged = ($event) => {
    this.editCxl.fire(new Models.OrderEditRequestFromUI(
      $event.data.orderId,
      parseFloat($event.data.price),
      $event.data.quantity,
      +($event.data.side == "Ask"),
      $event.data.symbol
    ));
  };

  private addAskBid = () => {
    if (!this.filter || !this._markets) return;
    loops: for (let x in this._markets)
      for (let z in this._markets[x])
        if (this.filter == this._markets[x][z].symbol) {
          var precision = this.symbols.filter(s => s.symbol == this.filter)[0].pricePrecision;
          this.best_ask = Shared.str(this._markets[x][z].ask, precision);
          this.best_bid = Shared.str(this._markets[x][z].bid, precision);
          this.orders_market = this._markets[x][z].web;
          this.base = this._markets[x][z].base;
          this.quote = this._markets[x][z].quote;
          break loops;
        }
  };

  private applyFilter = (filter) => {
    this.filter = filter;
    this.api.onFilterChanged();
    this.addAskBid();
  };

  private addRowData = (o: Models.Order[]) => {
    if (!this.api) return;

    this.symbols.forEach(s => s.bids = s.asks = 0);

    var add: any[] = [],
        update: any[] = [],
        remove: any[] = [];

    this.api.forEachNode((rowNode, index) => {
      remove.push({exchangeId: rowNode.data.exchangeId});
    });

    o.forEach(o => {
      (remove.filter(x => x.exchangeId == o.exchangeId).length
        ? update : add
      ).push({
        symbol: o.symbol,
        orderId: o.orderId,
        exchangeId: o.exchangeId,
        side: Models.Side[o.side],
        price: o.price,
        value: o.quantity * o.price,
        type: Models.OrderType[o.type],
        tif: Models.TimeInForce[o.timeInForce],
        lat: o.latency < 0 ? 'loaded' : o.latency + 'ms',
        quantity: o.quantity,
        pong: o.isPong,
        time: o.time,
        pricePrecision: -Math.log10(o.pricePrecision),
        quantityPrecision: -Math.log10(o.quantityPrecision)
      });

      remove = remove.filter(x => x.exchangeId != o.exchangeId);

      this.addSymbol(o.symbol, o.side, -Math.log10(o.pricePrecision));
    });

    this.api.applyTransaction({add, update, remove});

    if (!this.filter && this.symbols.length) {
      this.applyFilter(this.symbols[0].symbol)
    }
  };

  private addSymbol(sym: string, side: Models.Side, pricePrecision: number) {
    if (!this.symbols.filter(s => s.symbol == sym).length) {
      this.symbols.push({
        symbol: sym,
        pricePrecision: pricePrecision,
        bids:  0,
        asks:  0
      });
      this.symbols.sort((a,b) => a.symbol.localeCompare(b.symbol));
    }
    this.symbols.forEach(s => {
      if (s.symbol == sym) {
        if (side == Models.Side.Bid)
          s.bids++;
        else s.asks++;
      }
    });
  };
};
