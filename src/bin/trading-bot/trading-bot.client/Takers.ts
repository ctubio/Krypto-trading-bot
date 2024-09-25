import {Component, Input} from '@angular/core';

import {GridOptions, GridApi, RowNode} from 'ag-grid-community';

import {Shared, Models} from 'lib/K';

@Component({
  selector: 'takers',
  template: `<ag-grid-angular
    class="{{ onGridTheme() }} marketTrades"
    style="height: 616px;width: 100%;"
    (window:resize)="onGridReady($event)"
    (gridReady)="onGridReady($event)"
    [gridOptions]="grid"></ag-grid-angular>`
})
export class TakersComponent {

  @Input() product: Models.ProductAdvertisement;

  @Input() set taker(o: Models.MarketTrade) {
    this.addRowData(o);
  };

  private api: GridApi;

  private grid: GridOptions = <GridOptions>{
    overlayLoadingTemplate: `<span class="ag-overlay-no-rows-center">empty history</span>`,
    overlayNoRowsTemplate: `<span class="ag-overlay-no-rows-center">empty history</span>`,
    defaultColDef: { sortable: true, resizable: true, flex: 1 },
    rowHeight:21,
    headerHeight:21,
    columnDefs: [{
      field: 'time',
      width: 82,
      headerName: 'time',
      sort: 'desc',
      suppressSizeToFit: true,
      cellClassRules: {
        'text-muted': '!data.recent'
      },
      cellRenderer: (params) => {
        var d = new Date(params.value||0);
        return (d.getHours()+'')
          .padStart(2, "0")+':'+(d.getMinutes()+'')
          .padStart(2, "0")+':'+(d.getSeconds()+'')
          .padStart(2, "0")+','+(d.getMilliseconds()+'')
          .padStart(3, "0");
      }
    }, {
      field: 'price',
      width: 85,
      headerName: 'price',
      cellClassRules: {
        'sell': 'data.side == "Ask"',
        'buy': 'data.side == "Bid"'
      }
    }, {
      field: 'quantity',
      width: 50,
      headerName: 'qty',
      cellClassRules: {
        'sell': 'data.side == "Ask"',
        'buy': 'data.side == "Bid"'
      }
    }, {
      field: 'side',
      width: 40,
      headerName: 'side',
      cellClassRules: {
        'sell': 'x == "Ask"',
        'buy': 'x == "Bid"'
      },
    }]
  };

  private onGridReady(event: any) {
    this.api = event.api;
    Shared.currencyHeaders(this.api, this.product.base, this.product.quote);
  };

  private onGridTheme() {
    return "ag-theme-alpine"+((document.getElementById('daynight').getAttribute('href').indexOf('dark') != -1)?'-dark':'');
  };

  private addRowData = (o: Models.MarketTrade) => {
    if (!this.api) return;

    if (o === null) this.api.setGridOption('rowData', []);
    else {
      this.api.applyTransaction({add: [{
        price: o.price.toFixed(this.product.tickPrice),
        quantity: o.quantity.toFixed(this.product.tickSize),
        time: o.time,
        recent: true,
        side: Models.Side[o.side]
      }]});

      this.api.forEachNode((node: RowNode) => {
        if (Math.abs(o.time - node.data.time) > 3600000)
          this.api.applyTransaction({remove: [node.data]});
        else if (node.data.recent && Math.abs(o.time - node.data.time) > 7000)
          node.setData(Object.assign(node.data, {recent: false}));
      });
    }
  };
};
