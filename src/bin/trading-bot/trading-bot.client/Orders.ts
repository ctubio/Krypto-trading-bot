import {Component, Input} from '@angular/core';

import {GridOptions, GridApi} from 'ag-grid-community';

import {Socket, Shared, Models} from 'lib/K';

@Component({
  selector: 'orders',
  template: `<ag-grid-angular
    class="ag-theme-alpine"
    style="height: 131px;width: 99.80%;"
    (window:resize)="onGridReady($event)"
    (gridReady)="onGridReady($event)"
    (cellClicked)="onCellClicked($event)"
    [gridOptions]="grid"></ag-grid-angular>`
})
export class OrdersComponent {

  private fireCxl: Socket.IFire<Models.OrderCancelRequestFromUI> = new Socket.Fire(Models.Topics.CancelOrder);

  @Input() product: Models.ProductAdvertisement;

  @Input() set orders(o: Models.Order[]) {
    this.addRowData(o);
  };

  private api: GridApi;

  private grid: GridOptions = <GridOptions>{
    suppressNoRowsOverlay: true,
    defaultColDef: { sortable: true, resizable: true, flex: 1 },
    rowHeight:25,
    headerHeight:25,
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
    Shared.currencyHeaders(this.api, this.product.base, this.product.quote);
  };

  private onCellClicked = ($event) => {
    if ($event.event.target.getAttribute('data-action-type') != 'remove') return;
    this.fireCxl.fire(new Models.OrderCancelRequestFromUI($event.data.orderId, $event.data.exchange));
  };

  private addRowData = (o: Models.Order[]) => {
    if (!this.api) return;

    var add = [];

    o.forEach(o => {
      add.push({
        orderId: o.orderId,
        exchangeId: o.exchangeId,
        side: Models.Side[o.side],
        price: o.price.toFixed(this.product.tickPrice),
        value: (Math.round(o.quantity * o.price * 100) / 100).toFixed(this.product.tickPrice),
        type: Models.OrderType[o.type],
        tif: Models.TimeInForce[o.timeInForce],
        lat: o.latency + 'ms',
        quantity: o.quantity.toFixed(this.product.tickSize),
        pong: o.isPong,
        time: o.time
      });
    });

    this.api.setGridOption('rowData', []);

    if (add.length) this.api.applyTransaction({add: add});
  };
};
