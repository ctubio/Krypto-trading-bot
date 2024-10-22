import {Component, Input} from '@angular/core';

import {GridOptions, GridApi, RowNode} from 'ag-grid-community';

import {Shared, Models} from 'lib/K';

@Component({
  selector: 'takers',
  template: `<ag-grid-angular
    class="ag-theme-alpine{{ onGridTheme() }} marketTrades"
    style="height: 616px;width: 100%;"
    (window:resize)="onGridReady($event)"
    (gridReady)="onGridReady($event)"
    [gridOptions]="grid"></ag-grid-angular>`
})
export class TakersComponent {

  @Input() product: Models.ProductAdvertisement;

  @Input() set taker(o: Models.MarketTrade[]) {
    this.addRowData(o);
  };

  private api: GridApi;

  private grid: GridOptions = <GridOptions>{
    overlayLoadingTemplate: `<span class="ag-overlay-no-rows-center">empty history</span>`,
    overlayNoRowsTemplate: `<span class="ag-overlay-no-rows-center">empty history</span>`,
    defaultColDef: { sortable: true, resizable: true, flex: 1 },
    rowHeight:25,
    headerHeight:25,
    animateRows:false,
    getRowId: (params: any) => params.data.id,
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
      cellRenderer: (params) => `<span class="val">` + params.value + `</span>` + ` <i class="beacon sym-_default-s sym-` + this.product.quote.toLowerCase() + `-s" ></i>`,
      cellClassRules: {
        'sell': 'data.side == "Ask"',
        'buy': 'data.side == "Bid"'
      }
    }, {
      field: 'quantity',
      width: 50,
      headerName: 'qty',
      cellRenderer: (params) => `<span class="val">` + params.value + `</span>` + ` <i class="beacon sym-_default-s sym-` + this.product.base.toLowerCase() + `-s" ></i>`,
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

  private onGridReady($event: any) {
    if ($event.api) this.api = $event.api;
  };

  private onGridTheme() {
    return document.body.classList.contains('theme-dark')
      ? '-dark' : '';
  };

  private addRowData = (o: Models.MarketTrade[]) => {
    if (!this.api) return;

    if (!o.length) this.api.setGridOption('rowData', []);
    else {
      var add: any[] = [];
      o.forEach(o => {
        add.push({
          id: Math.random().toString(),
          price: Shared.str(o.price, this.product.tickPrice),
          quantity: Shared.str(o.quantity, this.product.tickSize),
          time: o.time,
          recent: true,
          side: Models.Side[o.side]
        });
      });

      this.api.applyTransactionAsync({add}, () => {
        var txn: any = {
          update: [],
          remove: []
        };
        this.api.forEachNodeAfterFilterAndSort((node: RowNode, index: number) => {
          if (index > 30)
            txn.remove.push({id: node.data.id});
          else if (node.data.recent && Math.abs(o[o.length-1].time - node.data.time) > 7000)
            txn.update.push(Object.assign(node.data, {recent: false}));
        });
        this.api.applyTransaction(txn);
      });
    }
  };
};
