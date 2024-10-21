import {Component, Input, Output, EventEmitter} from '@angular/core';

import {GridOptions, GridApi, IRowNode, RowNode, ColDef} from 'ag-grid-community';

import {Socket, Shared, Models} from 'lib/K';

@Component({
  selector: 'trades',
  template: `<ag-grid-angular
    class="ag-theme-alpine"
    style="height: 479px;width: 99.80%;"
    (window:resize)="onGridReady($event)"
    (gridReady)="onGridReady($event)"
    (cellClicked)="onCellClicked($event)"
    [gridOptions]="grid"></ag-grid-angular>`
})
export class TradesComponent {

  private audio: boolean;
  private hasPongs: boolean;
  private headerNameMod: string = "";

  private fireCxl: Socket.IFire<Models.CleanTradeRequestFromUI> = new Socket.Fire(Models.Topics.CleanTrade);

  @Input() product: Models.ProductAdvertisement;

  @Input() set quotingParameters(o: Models.QuotingParameters) {
    this.addRowConfig(o);
  };

  @Input() set trade(o: Models.Trade) {
    this.addRowData(o);
  };

  @Output() onTradesLength = new EventEmitter<number>();

  @Output() onTradesMatchedLength = new EventEmitter<number>();

  @Output() onTradesChartData = new EventEmitter<Models.TradeChart>();

  private api: GridApi;

  private grid: GridOptions = <GridOptions>{
    overlayLoadingTemplate: `<span class="ag-overlay-no-rows-center">0 closed orders</span>`,
    overlayNoRowsTemplate: `<span class="ag-overlay-no-rows-center">0 closed orders</span>`,
    defaultColDef: { sortable: true, resizable: true, flex: 1 },
    rowHeight:28,
    headerHeight:25,
    animateRows:true,
    getRowId: (params: any) => params.data.tradeId,
    columnDefs: [{
      width: 30,
      field: 'cancel',
      headerName: 'cxl',
      suppressSizeToFit: true,
      cellRenderer: (params) => {
        return `<button type="button" class="btn btn-danger btn-xs">
          <span data-action-type="remove">&#10006;</span>
        </button>`;
      }
    }, {
      width: 95,
      field:'time',
      sort: 'desc',
      headerValueGetter:(params) => this.headerNameMod + 'time',
      suppressSizeToFit: true,
      comparator: (valueA: number, valueB: number, nodeA: RowNode, nodeB: RowNode, isInverted: boolean) => {
          return (nodeA.data.Ktime||nodeA.data.time) - (nodeB.data.Ktime||nodeB.data.time);
      },
      cellRenderer: (params) => {
        var d = new Date(params.value||0);
        return (d.getDate()+'')
          .padStart(2, "0")+'/'+((d.getMonth()+1)+'')
          .padStart(2, "0")+' '+(d.getHours()+'')
          .padStart(2, "0")+':'+(d.getMinutes()+'')
          .padStart(2, "0")+':'+(d.getSeconds()+'')
          .padStart(2, "0");
      }
    }, {
      width: 95,
      field:'Ktime',
      headerName:'⇋time',
      hide:true,
      suppressSizeToFit: true,
      cellRenderer: (params) => {
        if (params.value==0) return '';
        var d = new Date(params.value);
        return (d.getDate()+'')
          .padStart(2, "0")+'/'+((d.getMonth()+1)+'')
          .padStart(2, "0")+' '+(d.getHours()+'')
          .padStart(2, "0")+':'+(d.getMinutes()+'')
          .padStart(2, "0")+':'+(d.getSeconds()+'')
          .padStart(2, "0");
      }
    }, {
      width: 50,
      field:'side',
      headerName:'side',
      suppressSizeToFit: true,
      cellClassRules: {
        'sell': 'x == "Ask"',
        'buy': 'x == "Bid"',
        'kira': 'x == "&#10564;"'
      },
      cellRenderer: (params) => params.value === '&#10564;'
        ? '<span style="font-size:21px;padding-left:3px;font-weight:600;">' + params.value + '</span>'
        : params.value
    }, {
      width: 80,
      field:'price',
      headerValueGetter:(params) => this.headerNameMod + 'price',
      cellRenderer: (params) => `<span class="val">` + params.value + `</span>` + ` <i class="beacon sym-_default-s sym-` + this.product.quote.toLowerCase() + `-s" ></i>`,
      cellClassRules: {
        'sell': 'data._side == "Ask"',
        'buy': 'data._side == "Bid"'
      }
    }, {
      width: 95,
      field:'quantity',
      headerValueGetter:(params) => this.headerNameMod + 'qty',
      cellRenderer: (params) => `<span class="val">` + params.value + `</span>` + ` <i class="beacon sym-_default-s sym-` + this.product.base.toLowerCase() + `-s" ></i>`,
      suppressSizeToFit: true,
      cellClassRules: {
        'sell': 'data._side == "Ask"',
        'buy': 'data._side == "Bid"'
      }
    }, {
      width: 69,
      field:'value',
      headerValueGetter:(params) => this.headerNameMod + 'value',
      cellRenderer: (params) => `<span class="val">` + params.value + `</span>` + ` <i class="beacon sym-_default-s sym-` + this.product.quote.toLowerCase() + `-s" ></i>`,
      cellClassRules: {
        'sell': 'data._side == "Ask"',
        'buy': 'data._side == "Bid"'
      }
    }, {
      width: 75,
      field:'Kvalue',
      headerName:'⥄value',
      cellRenderer: (params) => `<span class="val">` + params.value + `</span>` + ` <i class="beacon sym-_default-s sym-` + this.product.quote.toLowerCase() + `-s" ></i>`,
      cellClassRules: {
        'buy': 'data._side == "Ask"',
        'sell': 'data._side == "Bid"'
      }
    }, {
      width: 85,
      field:'Kqty',
      headerName:'⥄qty',
      suppressSizeToFit: true,
      cellRenderer: (params) => `<span class="val">` + params.value + `</span>` + ` <i class="beacon sym-_default-s sym-` + this.product.base.toLowerCase() + `-s" ></i>`,
      cellClassRules: {
        'buy': 'data._side == "Ask"',
        'sell': 'data._side == "Bid"'
      }
    }, {
      width: 80,
      field:'Kprice',
      headerName:'⥄price',
      cellRenderer: (params) => `<span class="val">` + params.value + `</span>` + ` <i class="beacon sym-_default-s sym-` + this.product.quote.toLowerCase() + `-s" ></i>`,
      cellClassRules: {
        'buy': 'data._side == "Ask"',
        'sell': 'data._side == "Bid"'
      }
    }, {
      width: 65,
      field:'delta',
      headerName:'delta',
      cellClassRules: {
        'kira': 'data.side == "&#10564;"'
      },
      cellRenderer: (params) => params.value
        ? `<span class="val">` + params.value + `</span>` + ` <i class="beacon sym-_default-s sym-` + this.product.quote.toLowerCase() + `-s" ></i>`
        : ''
    }]
  };

  private onGridReady($event: any) {
    if ($event.api) this.api = $event.api;
  };

  private onCellClicked = ($event) => {
    if ($event.event.target.getAttribute('data-action-type') != 'remove') return;
    this.fireCxl.fire(new Models.CleanTradeRequestFromUI($event.data.tradeId));
  }

  private addRowConfig = (o: Models.QuotingParameters) => {
    this.audio = o.audio;

    this.hasPongs = (o.safety === Models.QuotingSafety.Boomerang || o.safety === Models.QuotingSafety.AK47);

    this.headerNameMod = this.hasPongs ? "➜" : "";

    if (!this.api) return;

    this.grid.columnDefs.map((x: ColDef)  => {
      if (['Ktime','Kqty','Kprice','Kvalue','delta'].indexOf(x.field) > -1)
        this.api.setColumnsVisible([x.field], this.hasPongs);
      return x;
    });

    this.api.refreshHeader();

    this.emitLengths();
  };

  private addRowData = (o: Models.Trade) => {
    if (!this.api) return;

    if (o === null) this.api.setGridOption('rowData', []);
    else {
      var node: IRowNode = this.api.getRowNode(o.tradeId);
      if (o.Kqty < 0) {
        if (node)
          this.api.applyTransaction({remove: [node.data]});
      } else {
        var edit = {
          time: o.time,
          quantity: Shared.str(o.quantity, this.product.tickSize),
          value: Shared.str(o.value, this.product.tickPrice),
          Ktime: o.Ktime,
          Kqty: o.Kqty ? Shared.str(o.Kqty, this.product.tickSize) : '',
          Kprice: o.Kprice ? Shared.str(o.Kprice, this.product.tickPrice) : '',
          Kvalue: o.Kvalue ? Shared.str(o.Kvalue, this.product.tickPrice) : '',
          delta: Shared.str(o.delta, 8),
          side: o.Kqty >= o.quantity ? '&#10564;' : (o.side === Models.Side.Ask ? "Ask" : "Bid"),
          _side: o.side === Models.Side.Ask ? "Ask" : "Bid",
        };

        if (node) node.setData(Object.assign(node.data, edit));
        else this.api.applyTransaction({add: [Object.assign(edit, {
          tradeId: o.tradeId,
          price: Shared.str(o.price, this.product.tickPrice)
        })]});

        if (o.loadedFromDB === false) {
          if (this.audio) Shared.playAudio(o.isPong?'1':'0');

          this.onTradesChartData.emit(new Models.TradeChart(
            (o.isPong && o.Kprice)?o.Kprice:o.price,
            (o.isPong && o.Kprice)?(o.side === Models.Side.Ask ? Models.Side.Bid : Models.Side.Ask):o.side,
            (o.isPong && o.Kprice)?o.Kqty:o.quantity,
            (o.isPong && o.Kprice)?o.Kvalue:o.value,
            o.isPong
          ));
        }
      }
    }

    this.emitLengths();
  };

  private emitLengths = () => {
    this.onTradesLength.emit(this.api.getDisplayedRowCount());
    var tradesMatched: number = 0;
    if (this.hasPongs) {
      this.api.forEachNode((node: RowNode) => {
        if (node.data.Kqty) tradesMatched++;
      });
    } else tradesMatched = -1;
    this.onTradesMatchedLength.emit(tradesMatched);
  };
};
