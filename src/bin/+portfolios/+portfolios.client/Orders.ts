import {Component, Input} from '@angular/core';

import {GridOptions, GridApi} from 'ag-grid-community';

import {Socket, Models} from 'lib/K';

@Component({
  selector: 'orders',
  template: `<div id="openorders">
    <h2>
      <span [hidden]="!!symbols">0 open orders</span>
      <a
        *ngFor="let x of symbols"
        (click)="applyFilter(x.symbol)"
        [ngStyle]="{'cursor':'pointer','padding': '0px 20px 0px 10px', 'font-weight': filter==x.symbol?600:300}"
      >{{ x.symbol }}<sup title="{{x.count}} open orders">({{x.count}})</sup></a>
    </h2>
    <ag-grid-angular
      [hidden]="!filter || !symbols"
      class="ag-theme-alpine ag-theme-big"
      style="width: 100%;"
      (window:resize)="onGridReady($event)"
      (gridReady)="onGridReady($event)"
      (cellClicked)="onCellClicked($event)"
      [gridOptions]="grid"></ag-grid-angular>
  </div>`
})
export class OrdersComponent {

  private fireCxl: Socket.IFire<Models.OrderCancelRequestFromUI> = new Socket.Fire(Models.Topics.CancelOrder);

  @Input() product: Models.ProductAdvertisement;

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
    isExternalFilterPresent: () => !!this.filter,
    doesExternalFilterPass: (node) => (
      !this.filter || node.data.symbol == this.filter
    ),
    columnDefs: [{
      width: 30,
      field: "cancel",
      headerName: 'cxl',
      suppressSizeToFit: true,
      cellRenderer: (params) => `<button type="button" class="btn btn-danger btn-xs">
          <span data-action-type="remove"'>&times;</span>
        </button>`
    }, {
      width: 82,
      field: 'time',
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
      width: 40,
      field: 'side',
      headerName: 'side',
      suppressSizeToFit: true,
      cellClassRules: {
        'sell': 'data.side == "Ask"',
        'buy': 'data.side == "Bid"'
      },
      cellRenderer: (params) => (
        params.data.pong
          ? '&#10564;'
          : '&#10140;'
        ) + params.value
    }, {
      width: 74,
      field: 'price',
      headerName: 'price',
      sort: 'desc',
      cellClassRules: {
        'sell': 'data.side == "Ask"',
        'buy': 'data.side == "Bid"'
      }
    }, {
      width: 95,
      field: 'quantity',
      headerName: 'qty',
      suppressSizeToFit: true,
      cellClassRules: {
        'sell': 'data.side == "Ask"',
        'buy': 'data.side == "Bid"'
      }
    }, {
      width: 74,
      field: 'value',
      headerName: 'value',
      cellClassRules: {
        'sell': 'data.side == "Ask"',
        'buy': 'data.side == "Bid"'
      }
    }, {
      width: 55,
      field: 'type',
      headerName: 'type',
      suppressSizeToFit: true
    }, {
      width: 40,
      field: 'tif',
      headerName: 'tif'
    }, {
      width: 45,
      field: 'lat',
      headerName: 'lat'
    }, {
      width: 110,
      field: 'exchangeId',
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

  private applyFilter = (filter) => {
    if (this.filter && this.filter == filter)
      this.filter = "";
    else this.filter = filter;
    this.api.onFilterChanged();
  };

  private addRowData = (o: Models.Order[]) => {
    if (!this.api) return;

    var add = [];
    this.symbols.forEach(s => s.count = 0);

    o.forEach(o => {
      add.push({
        symbol: o.symbol,
        orderId: o.orderId,
        exchangeId: o.exchangeId,
        side: Models.Side[o.side],
        price: o.price, //.toFixed(this.product.tickPrice)
        value: (Math.round(o.quantity * o.price * 100) / 100), //.toFixed(this.product.tickPrice)
        type: Models.OrderType[o.type],
        tif: Models.TimeInForce[o.timeInForce],
        lat: o.latency + 'ms',
        quantity: o.quantity, //.toFixed(this.product.tickSize)
        pong: o.isPong,
        time: o.time
      });

      this.addSymbol(o.symbol);
    });

    this.api.setGridOption('rowData', []);

    if (add.length) this.api.applyTransaction({add: add});
  };

  private addSymbol(sym: string) {
    if (!this.symbols.filter(s => s.symbol == sym).length) {
      this.symbols.push({
        symbol: sym,
        count:  0
      });
      this.symbols.sort((a,b) => a.symbol.localeCompare(b.symbol));
    }
    this.symbols.forEach(s => {
      if (s.symbol == sym)
        s.count++;
    });
  };
};
