import {NgZone, Component, Inject, Input, OnInit} from '@angular/core';
import {GridOptions, ColDef, RowNode} from 'ag-grid/main';

import * as Models from './models';
import {SubscriberFactory, BaseCurrencyCellComponent, QuoteCurrencyCellComponent} from './shared_directives';

@Component({
  selector: 'market-trades',
  template: `<ag-grid-angular #marketList (click)="this.loadSubscriber()" class="ag-fresh ag-dark {{ subscribed ? \'ag-subscribed\' : \'ag-not-subscribed\' }} marketTrades" style="height: 350px;width: 100%;" rowHeight="21" [gridOptions]="gridOptions"></ag-grid-angular>`
})
export class MarketTradesComponent implements OnInit {

  private gridOptions: GridOptions = <GridOptions>{};

  @Input() product: Models.ProductState;

  constructor(
    @Inject(NgZone) private zone: NgZone,
    @Inject(SubscriberFactory) private subscriberFactory: SubscriberFactory
  ) {}

  ngOnInit() {
    this.gridOptions.columnDefs = this.createColumnDefs();
    this.gridOptions.enableSorting = true;
    this.gridOptions.overlayLoadingTemplate = `<span class="ag-overlay-no-rows-center">click to view data</span>`;
    this.gridOptions.overlayNoRowsTemplate = `<span class="ag-overlay-no-rows-center">empty history</span>`;
    this.gridOptions.enableColResize = true;
  }

  private subscribed: boolean = false;
  public loadSubscriber = () => {
    if (this.subscribed) return;
    this.subscribed = true;
    this.gridOptions.rowData = [];

    this.subscriberFactory
      .getSubscriber(this.zone, Models.Topics.MarketTrade)
      .registerDisconnectedHandler(() => this.gridOptions.rowData.length = 0)
      .registerSubscriber(this.addRowData);
  }

  private createColumnDefs = (): ColDef[] => {
    return [
      { width: 82, field: 'time', headerName: 'time', cellRenderer:(params) => {
        var d = new Date(params.value||0);
        return (d.getHours()+'').padStart(2, "0")+':'+(d.getMinutes()+'').padStart(2, "0")+':'+(d.getSeconds()+'').padStart(2, "0")+','+(d.getMilliseconds()+'').padStart(3, "0");
      },sort: 'desc', cellClass: (params) => {
          return 'fs11px '+(!params.data.recent ? "text-muted" : "");
      } },
      { width: 75, field: 'price', headerName: 'price', cellClass: (params) => {
          return (params.data.side === 'Ask') ? "sell" : "buy";
      }, cellRendererFramework: QuoteCurrencyCellComponent},
      { width: 50, field: 'quantity', headerName: 'qty', cellClass: (params) => {
          return (params.data.side === 'Ask') ? "sell" : "buy";
      }, cellRendererFramework: BaseCurrencyCellComponent},
      { width: 40, field: 'side', headerName: 'side' , cellClass: (params) => {
        if (params.value === 'Bid') return 'buy';
        else if (params.value === 'Ask') return "sell";
      }}
    ];
  }

  private addRowData = (trade: Models.MarketTrade) => {
    if (!this.gridOptions.api) return;
    if (trade != null)
      this.gridOptions.api.updateRowData({add:[{
        price: trade.price,
        quantity: trade.quantity,
        time: trade.time,
        recent: true,
        side: Models.Side[trade.side],
        quoteSymbol: trade.pair.quote,
        productFixed: this.product.fixed
      }]});

    this.gridOptions.api.forEachNode((node: RowNode) => {
      if (Math.abs(trade.time - node.data.time) > 3600000)
        this.gridOptions.api.updateRowData({remove:[node.data]});
      else if (Math.abs(trade.time - node.data.time) > 7000)
        node.setData(Object.assign(node.data, {recent: false}));
    });
  }
}
