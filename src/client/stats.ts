import {Component, Input, OnInit} from '@angular/core';
import * as Highcharts from "highcharts";
import * as Subscribe from './subscribe';

import * as Models from './models';

@Component({
  selector: 'market-stats',
  template: `<div class="col-md-6 col-xs-6">
  <table><tr><td>
    <chart style="position:relative;top:5px;height:380px;width:700px;" [options]="fvChartOptions" (load)="saveInstance($event.context, 'fv')"></chart>
  </td><td>
    <chart style="position:relative;top:10px;height:180px;width:700px;" [options]="baseChartOptions" (load)="saveInstance($event.context, 'base')"></chart>
    <chart style="position:relative;top:11px;height:180px;width:700px;" [options]="quoteChartOptions" (load)="saveInstance($event.context, 'quote')"></chart>
  </td></tr></table>
    </div>`
})
export class StatsComponent implements OnInit {

  public positionData: Models.PositionReport;
  public targetBasePosition: number;
  public positionDivergence: number;
  public fairValue: number;
  public width: number;
  public ewmaQuote: number;
  public ewmaWidth: number;
  public ewmaShort: number;
  public ewmaMedium: number;
  public ewmaLong: number;
  public ewmaVeryLong: number;
  public stdevWidth: Models.IStdev;
  public tradesBuySize: number;
  public tradesSellSize: number;
  public fvChart: any;
  public quoteChart: any;
  public baseChart: any;
  private saveInstance = (chartInstance, chartId) => {
    this[chartId+'Chart'] = Highcharts.charts.length;
    Highcharts.charts.push(chartInstance);
  }
  private pointFormatterBase = function () {
    return this.series.type=='arearange'
      ? '<tr><td><span style="color:'+this.series.color+'">●</span>'+this.series.name+(this.series.name=="Width" && (<any>Highcharts).quotingParameters.protectionEwmaWidthPing?" EWMA":"")+' High:</td><td style="text-align:right;"> <b>'+this.high.toFixed((<any>Highcharts).customProductFixed)+' ' + ((<any>Highcharts).customQuoteCurrency) + '</b></td></tr>'
        + '<tr><td><span style="color:'+this.series.color+'">●</span>'+this.series.name+(this.series.name=="Width" && (<any>Highcharts).quotingParameters.protectionEwmaWidthPing?" EWMA":"")+' Low:</td><td style="text-align:right;"> <b>'+this.low.toFixed((<any>Highcharts).customProductFixed)+' ' + ((<any>Highcharts).customQuoteCurrency) + '</b></td></tr>'
      : ( this.series.type=='bubble'
        ? '<tr><td colspan="2"><b><span style="color:'+this.series.color+';">'+this.dir+'</span> '+this.side+'</b> (P'+this.pong+'ng)</td></tr>'
          + '<tr><td>' + 'Price:</td><td style="text-align:right;"> <b>'+this.price+' '+this.quote+'</b></td></tr>'
          + '<tr><td>' + 'Qty:</td><td style="text-align:right;"> <b>'+this.qty+' '+this.base+'</b></td></tr>'
          + '<tr><td>' + 'Value:</td><td style="text-align:right;"> <b>'+this.val+' '+this.quote+'</b></td></tr>'
        :'<tr><td><span style="color:'+this.series.color+'">' + (<any>Highcharts).customSymbols[this.series.symbol] + '</span> '+this.series.name+':</td><td style="text-align:right;"> <b>'+this.y.toFixed((<any>Highcharts).customProductFixed)+' ' + ((<any>Highcharts).customQuoteCurrency) + '</b></td></tr>'
      );
  }
  private pointFormatterQuote = function () {
    return '<tr><td><span style="color:'+this.series.color+'">' + (<any>Highcharts).customSymbols[this.series.symbol||'square'] + '</span> '+this.series.name+':</td><td style="text-align:right;"> <b>'+this.y.toFixed(8)+' ' + ((<any>Highcharts).customBaseCurrency) + '</b></td></tr>';
  }
  private syncExtremes = function (e) {
    var thisChart = this.chart;
    if (e.trigger !== 'syncExtremes') {
        (<any>Highcharts).each(Highcharts.charts, function (chart) {
            if (chart !== thisChart && chart.xAxis[0].setExtremes)
              chart.xAxis[0].setExtremes(e.min, e.max, undefined, true, { trigger: 'syncExtremes' });
        });
    }
  }
  public fvChartOptions = {
    title: 'fair value',
    chart: {
        type: 'bubble',
        width: 700,
        height: 380,
        zoomType: false,
        backgroundColor:'rgba(255, 255, 255, 0)',
    },
    plotOptions: {series: {marker: {enabled: false}}},
    navigator: {enabled: false},
    rangeSelector:{enabled: false,height:0},
    scrollbar: {enabled: false},
    credits: {enabled: false},
    xAxis: {
      type: 'datetime',
      crosshair: true,
      // events: {setExtremes: this.syncExtremes},
      labels: {enabled: false},
      gridLineWidth: 0,
      dateTimeLabelFormats: {millisecond: '%H:%M:%S',second: '%H:%M:%S',minute: '%H:%M',hour: '%H:%M',day: '%m-%d',week: '%m-%d',month: '%m',year: '%Y'}
    },
    yAxis: [{
      title: {text: 'Fair Value and Trades'},
      labels: {enabled: false},
      gridLineWidth: 0
    },{
      title: {text: 'STDEV 20'},
      labels: {enabled: false},
      opposite: true,
      gridLineWidth: 0
    },{
      title: {text: 'Market Size'},
      labels: {enabled: false},
      opposite: true,
      pointPadding: 0,
      groupPadding: 0,
      borderWidth: 0,
      shadow: false,
      gridLineWidth: 0
    }],
    legend: {
      enabled:true,
      itemStyle: {
        color: 'lightgray'
      },
      itemHoverStyle: {
        color: 'gray'
      },
      itemHiddenStyle: {
        color: 'black'
      }
    },
    tooltip: {
        shared: true,
        useHTML: true,
        headerFormat: '<small>{point.x:%A} <b>{point.x:%H:%M:%S}</b></small><table>',
        footerFormat: '</table>'
    },
    series: [{
      name: 'Fair Value',
      type: 'spline',
      lineWidth:4,
      colorIndex: 2,
      tooltip: {pointFormatter: this.pointFormatterBase},
      data: [],
      id: 'fvseries'
    },{
      name: 'Width',
      type: 'arearange',
      tooltip: {pointFormatter: this.pointFormatterBase},
      lineWidth: 0,
      colorIndex: 2,
      fillOpacity: 0.2,
      zIndex: -1,
      data: []
    },{
      name: 'Sell',
      type: 'spline',
      zIndex:1,
      colorIndex: 5,
      tooltip: {pointFormatter: this.pointFormatterBase},
      data: [],
      id: 'sellseries'
    },{
      name: 'Sell',
      type: 'bubble',
      zIndex:2,
      color: 'white',
      marker: {enabled: true,fillColor: 'transparent',lineColor: Highcharts.getOptions().colors[5]},
      tooltip: {pointFormatter: this.pointFormatterBase},
      dataLabels: {enabled: true,format: '{point.title}'},
      data: [],
      linkedTo: ':previous'
    },{
      name: 'Buy',
      type: 'spline',
      zIndex:1,
      colorIndex: 0,
      tooltip: {pointFormatter: this.pointFormatterBase},
      data: [],
      id: 'buyseries'
    },{
      name: 'Buy',
      type: 'bubble',
      zIndex:2,
      color: 'white',
      marker: {enabled: true,fillColor: 'transparent',lineColor: Highcharts.getOptions().colors[0]},
      tooltip: {pointFormatter: this.pointFormatterBase},
      dataLabels: {enabled: true,format: '{point.title}'},
      data: [],
      linkedTo: ':previous'
    },{
      name: 'EWMA Quote',
      type: 'spline',
      color: '#ffff00',
      tooltip: {pointFormatter: this.pointFormatterBase},
      data: []
    },{
      name: 'EWMA Very Long',
      type: 'spline',
      colorIndex: 6,
      tooltip: {pointFormatter: this.pointFormatterBase},
      data: []
    },{
      name: 'EWMA Long',
      type: 'spline',
      colorIndex: 6,
      tooltip: {pointFormatter: this.pointFormatterBase},
      data: []
    },{
      name: 'EWMA Medium',
      type: 'spline',
      colorIndex: 6,
      tooltip: {pointFormatter: this.pointFormatterBase},
      data: []
    },{
      name: 'EWMA Short',
      type: 'spline',
      colorIndex: 3,
      tooltip: {pointFormatter:this.pointFormatterBase},
      data: []
    },{
      name: 'STDEV Fair',
      type: 'spline',
      lineWidth:1,
      color:'#af451e',
      tooltip: {pointFormatter: this.pointFormatterBase},
      yAxis: 1,
      data: []
    },{
      name: 'STDEV Tops',
      type: 'spline',
      lineWidth:1,
      color:'#af451e',
      tooltip: {pointFormatter: this.pointFormatterBase},
      yAxis: 1,
      data: []
    },{
      name: 'STDEV TopAsk',
      type: 'spline',
      lineWidth:1,
      color:'#af451e',
      tooltip: {pointFormatter: this.pointFormatterBase},
      yAxis: 1,
      data: []
    },{
      name: 'STDEV TopBid',
      type: 'spline',
      lineWidth:1,
      color:'#af451e',
      tooltip: {pointFormatter: this.pointFormatterBase},
      yAxis: 1,
      data: []
    },{
      name: 'STDEV BBFair',
      type: 'arearange',
      tooltip: {pointFormatter: this.pointFormatterBase},
      lineWidth: 0,
      color:'#af451e',
      fillOpacity: 0.2,
      zIndex: -1,
      data: []
    },{
      name: 'STDEV BBTops',
      type: 'arearange',
      tooltip: {pointFormatter: this.pointFormatterBase},
      lineWidth: 0,
      color:'#af451e',
      fillOpacity: 0.2,
      zIndex: -1,
      data: []
    },{
      name: 'STDEV BBTop',
      type: 'arearange',
      tooltip: {pointFormatter: this.pointFormatterBase},
      lineWidth: 0,
      color:'#af451e',
      fillOpacity: 0.2,
      zIndex: -1,
      data: []
    },{
      type: 'column',
      name: 'Market Buy Size',
      tooltip: {pointFormatter: this.pointFormatterQuote},
      yAxis: 2,
      colorIndex:0,
      data: [],
      zIndex: -2
    },{
      type: 'column',
      name: 'Market Sell Size',
      tooltip: {pointFormatter: this.pointFormatterQuote},
      yAxis: 2,
      colorIndex:5,
      data: [],
      zIndex: -2
    }]
  };
  public quoteChartOptions = {
    title: 'quote wallet',
    chart: {
        width: 700,
        height: 180,
        zoomType: false,
        resetZoomButton: {theme: {display: 'none'}},
        backgroundColor:'rgba(255, 255, 255, 0)'
    },
    credits: {enabled: false},
    tooltip: {
        shared: true,
        useHTML: true,
        headerFormat: '<small>{point.x:%A} <b>{point.x:%H:%M:%S}</b></small><table>',
        footerFormat: '</table>',
        pointFormatter: this.pointFormatterBase
    },
    plotOptions: {
      area: {stacking: 'normal',connectNulls: true,marker: {enabled: false}},
      spline: {marker: {enabled: false}}
    },
    xAxis: {
      type: 'datetime',
      crosshair: true,
      // events: {setExtremes: this.syncExtremes},
      labels: {enabled: true},
      dateTimeLabelFormats: {millisecond: '%H:%M:%S',second: '%H:%M:%S',minute: '%H:%M',hour: '%H:%M',day: '%m-%d',week: '%m-%d',month: '%m',year: '%Y'}
    },
    yAxis: [{
      title: {text: 'Total Position'},
      opposite: true,
      labels: {enabled: false},
      gridLineWidth: 0
    },{
      title: {text: 'Available and Held'},
      min: 0,
      labels: {enabled: false},
      gridLineWidth: 0
    }],
    legend: {enabled: false},
    series: [{
      name: 'Total Value',
      type: 'spline',
      zIndex: 1,
      colorIndex:2,
      lineWidth:3,
      data: []
    },{
      name: 'Target',
      type: 'spline',
      yAxis: 1,
      zIndex: 2,
      colorIndex:6,
      data: []
    },{
      name: 'pDiv',
      type: 'arearange',
      lineWidth: 0,
      yAxis: 1,
      colorIndex: 6,
      fillOpacity: 0.3,
      zIndex: 0,
      marker: { enabled: false },
      data: []
    },{
      name: 'Available',
      type: 'area',
      colorIndex:0,
      fillOpacity: 0.2,
      yAxis: 1,
      data: []
    },{
      name: 'Held',
      type: 'area',
      yAxis: 1,
      marker:{symbol:'triangle-down'},
      data: []
    }]
  };
  public baseChartOptions = {
    title: 'base wallet',
    chart: {
        width: 700,
        height: 180,
        zoomType: false,
        resetZoomButton: {theme: {display: 'none'}},
        backgroundColor:'rgba(255, 255, 255, 0)'
    },
    credits: {enabled: false},
    tooltip: {
        shared: true,
        headerFormat: '<small>{point.x:%A} <b>{point.x:%H:%M:%S}</b></small><table>',
        footerFormat: '</table>',
        useHTML: true,
        pointFormatter: this.pointFormatterQuote
    },
    plotOptions: {
      area: {stacking: 'normal',connectNulls: true,marker: {enabled: false}},
      spline: {marker: {enabled: false}}
    },
    xAxis: {
      type: 'datetime',
      crosshair: true,
      // events: {setExtremes: this.syncExtremes},
      labels: {enabled: false},
      dateTimeLabelFormats: {millisecond: '%H:%M:%S',second: '%H:%M:%S',minute: '%H:%M',hour: '%H:%M',day: '%m-%d',week: '%m-%d',month: '%m',year: '%Y'}
    },
    yAxis: [{
      title: {text: 'Total Position'},
      opposite: true,
      labels: {enabled: false},
      gridLineWidth: 0
    },{
      title: {text: 'Available and Held'},
      min: 0,
      labels: {enabled: false},
      gridLineWidth: 0
    }],
    legend: {enabled: false},
    series: [{
      name: 'Total Value',
      type: 'spline',
      zIndex: 1,
      colorIndex:2,
      lineWidth:3,
      data: []
    },{
      name: 'Target (TBP)',
      type: 'spline',
      yAxis: 1,
      zIndex: 2,
      colorIndex:6,
      data: []
    },{
      name: 'pDiv',
      type: 'arearange',
      lineWidth: 0,
      yAxis: 1,
      colorIndex: 6,
      fillOpacity: 0.3,
      zIndex: 0,
      marker: { enabled: false },
      data: []
    },{
      name: 'Available',
      type: 'area',
      yAxis: 1,
      colorIndex:5,
      fillOpacity: 0.2,
      data: []
    },{
      name: 'Held',
      type: 'area',
      yAxis: 1,
      colorIndex:5,
      data: []
    }]
  };

  @Input() product: Models.ProductState;

  @Input() set setQuotingParameters(o: Models.QuotingParameters) {
    (<any>Highcharts).quotingParameters = o;
  }

  @Input() set setFairValue(o: Models.FairValue) {
    if (o == null) return;
    this.fairValue = o.price;
  }

  @Input() set setTargetBasePosition(o: Models.TargetBasePositionValue) {
    if (o == null) return;
    this.targetBasePosition = o.tbp;
    this.positionDivergence = o.pDiv;
  }

  @Input() set setPosition(o: Models.PositionReport) {
    if (o === null) return;
    let time = new Date().getTime();
    if (!(<any>Highcharts).customBaseCurrency) (<any>Highcharts).customBaseCurrency = o.pair.base;
    if (!(<any>Highcharts).customQuoteCurrency) (<any>Highcharts).customQuoteCurrency = o.pair.quote;
    this.positionData = o;
  }

  @Input() set setMarketData(o: Models.Market) {
    if (o && o.bids && o.bids.length && o.asks && o.asks.length)
      this.width = (o.asks[0].price - o.bids[0].price) / 2;
  }

  @Input() set setEWMAChartData(o: Models.EWMAChart) {
    if (o === null) return;
    this.fairValue = o.fairValue;
    if (o.ewmaQuote) this.ewmaQuote = o.ewmaQuote;
    if (o.ewmaWidth) this.ewmaWidth = o.ewmaWidth;
    if (o.ewmaShort) this.ewmaShort = o.ewmaShort;
    if (o.ewmaMedium) this.ewmaMedium = o.ewmaMedium;
    if (o.ewmaLong) this.ewmaLong = o.ewmaLong;
    if (o.ewmaVeryLong) this.ewmaVeryLong = o.ewmaVeryLong;
    if (o.stdevWidth) this.stdevWidth = o.stdevWidth;
    if (o.tradesBuySize) this.tradesBuySize = o.tradesBuySize;
    if (o.tradesSellSize) this.tradesSellSize = o.tradesSellSize;
  }

  @Input() set setTradesChartData(t: Models.TradeChart) {
    if (t===null) return;
    let time = new Date().getTime();
    Highcharts.charts[this.fvChart].series[Models.Side[t.side] == 'Bid' ? 4 : 2].addPoint([time, t.price], false);
    (<any>Highcharts).charts[this.fvChart].series[Models.Side[t.side] == 'Bid' ? 5 : 3].addPoint({
      x: time,
      y: t.price,
      z: t.quantity,
      title: (t.pong ? '¯' : '_')+(Models.Side[t.side] == 'Bid' ? 'B' : 'S'),
      dir:(Models.Side[t.side] == 'Bid' ? '▼':'▲'),
      side:(Models.Side[t.side] == 'Bid' ? 'Buy':'Sell'),
      pong:(t.pong?'o':'i'),
      price:t.price.toFixed((<any>Highcharts).customProductFixed),
      qty:t.quantity.toFixed(8),
      val:t.value.toFixed((<any>Highcharts).customProductFixed),
      quote:((<any>Highcharts).customQuoteCurrency),
      base:((<any>Highcharts).customBaseCurrency)
    }, true);
    this.updateCharts(time);
  }

  private showStats: boolean;
  @Input() set setShowStats(showStats: boolean) {
    if (!this.showStats && showStats)
      Highcharts.charts.forEach(chart => chart.redraw(false) );
    this.showStats = showStats;
  }

  private forEach = (array, callback) => {
    for (var i = 0; i < array.length; i++)
      callback.call(window, array[i]);
  }

  private extend = (out) => {
    out = out || {};

    for (var i = 1; i < arguments.length; i++) {
      if (!arguments[i])
        continue;

      for (var key in arguments[i]) {
        if (arguments[i].hasOwnProperty(key))
          out[key] = arguments[i][key];
      }
    }

    return out;
  }

  ngOnInit() {
    (<any>Highcharts).customBaseCurrency = '';
    (<any>Highcharts).customQuoteCurrency = '';
    (<any>Highcharts).customProductFixed = this.product.fixed;
    (<any>Highcharts).customSymbols = {'circle': '●','diamond': '♦','square': '■','triangle': '▲','triangle-down': '▼'};
    Highcharts.setOptions({global: {getTimezoneOffset: function () {return new Date().getTimezoneOffset(); }}});
    /*this.forEach(document.getElementsByTagName('chart'), function (el) {
      el.addEventListener('mousemove', function (e) {
        var chart, point, i, event, containerLeft, thisLeft;
        for (i = 0; i < Highcharts.charts.length; ++i) {
          chart = Highcharts.charts[i];
          containerLeft = chart.container.getBoundingClientRect().left;
          thisLeft = el.getBoundingClientRect().left;
          if (containerLeft == thisLeft && chart.container.getBoundingClientRect().top == el.getBoundingClientRect().top) continue;
          chart.pointer.reset = function () { return undefined; };
          console.log(e);
          let ev: any = this.extend(new MouseEvent('mousemove'), {
              which: 1,
              chartX: e.originalEvent.chartX,
              chartY: e.originalEvent.chartY,
              clientX: (containerLeft != thisLeft)?containerLeft - thisLeft + e.originalEvent.clientX:e.originalEvent.clientX,
              clientY: e.originalEvent.clientY,
              pageX: (containerLeft != thisLeft)?containerLeft - thisLeft + e.originalEvent.pageX:e.originalEvent.pageX,
              pageY: e.originalEvent.pageY,
              screenX: (containerLeft != thisLeft)?containerLeft - thisLeft + e.originalEvent.screenX:e.originalEvent.screenX,
              screenY: e.originalEvent.screenY
          });
          event = chart.pointer.normalize(ev);
          point = chart.series[0].searchPoint(event, true);
          if (point) {
            point.onMouseOver();
            point.series.chart.xAxis[0].drawCrosshair(event, point);
          }
        }
      });
    });
    this.forEach(document.getElementsByTagName('chart'), function (el) {
      el.addEventListener('mouseleave', function (e) {
        var chart, point, i, event;
        for (i = 0; i < Highcharts.charts.length; ++i) {
          chart = Highcharts.charts[i];
          event = chart.pointer.normalize(e.originalEvent);
          point = chart.series[0].searchPoint(event, true);
          if (point) {
            point.onMouseOut();
            chart.tooltip.hide(point);
            chart.xAxis[0].hideCrosshair();
          }
        }
      });
    });*/
    setInterval(() => this.updateCharts(new Date().getTime()), 10000);
  }

  private updateCharts = (time: number) => {
    this.removeOldPoints(time);
    if (this.fairValue) {
      if (this.stdevWidth) {
        if (this.stdevWidth.fv) Highcharts.charts[this.fvChart].series[11].addPoint([time, this.stdevWidth.fv], false);
        if (this.stdevWidth.tops) Highcharts.charts[this.fvChart].series[12].addPoint([time, this.stdevWidth.tops], false);
        if (this.stdevWidth.ask) Highcharts.charts[this.fvChart].series[13].addPoint([time, this.stdevWidth.ask], false);
        if (this.stdevWidth.bid) Highcharts.charts[this.fvChart].series[14].addPoint([time, this.stdevWidth.bid], false);
        if (this.stdevWidth.fv && this.stdevWidth.fvMean) Highcharts.charts[this.fvChart].series[15].addPoint([time, this.stdevWidth.fvMean-this.stdevWidth.fv, this.stdevWidth.fvMean+this.stdevWidth.fv], this.showStats, false, false);
        if (this.stdevWidth.tops && this.stdevWidth.topsMean) Highcharts.charts[this.fvChart].series[16].addPoint([time, this.stdevWidth.topsMean-this.stdevWidth.tops, this.stdevWidth.topsMean+this.stdevWidth.tops], this.showStats, false, false);
        if (this.stdevWidth.ask && this.stdevWidth.bid && this.stdevWidth.askMean && this.stdevWidth.bidMean) Highcharts.charts[this.fvChart].series[17].addPoint([time, this.stdevWidth.bidMean-this.stdevWidth.bid, this.stdevWidth.askMean+this.stdevWidth.ask], this.showStats, false, false);
      }
      Highcharts.charts[this.fvChart].yAxis[2].setExtremes(0, Math.max(this.tradesBuySize*4,this.tradesSellSize*4,Highcharts.charts[this.fvChart].yAxis[2].getExtremes().dataMax*4), false, true, { trigger: 'syncExtremes' });
      if (this.tradesBuySize) Highcharts.charts[this.fvChart].series[18].addPoint([time, this.tradesBuySize], false);
      if (this.tradesSellSize) Highcharts.charts[this.fvChart].series[19].addPoint([time, this.tradesSellSize], false);
      this.tradesBuySize = 0;
      this.tradesSellSize = 0;
      if (this.ewmaQuote) Highcharts.charts[this.fvChart].series[6].addPoint([time, this.ewmaQuote], false);
      if (this.ewmaVeryLong) Highcharts.charts[this.fvChart].series[7].addPoint([time, this.ewmaVeryLong], false);
      if (this.ewmaLong) Highcharts.charts[this.fvChart].series[8].addPoint([time, this.ewmaLong], false);
      if (this.ewmaMedium) Highcharts.charts[this.fvChart].series[9].addPoint([time, this.ewmaMedium], false);
      if (this.ewmaShort) Highcharts.charts[this.fvChart].series[10].addPoint([time, this.ewmaShort], false);
      Highcharts.charts[this.fvChart].series[0].addPoint([time, this.fairValue], this.showStats);
      if ((<any>Highcharts).quotingParameters.protectionEwmaWidthPing && this.ewmaWidth) Highcharts.charts[this.fvChart].series[1].addPoint([time, this.fairValue-this.ewmaWidth, this.fairValue+this.ewmaWidth], this.showStats, false, false);
      else if (this.width) Highcharts.charts[this.fvChart].series[1].addPoint([time, this.fairValue-this.width, this.fairValue+this.width], this.showStats, false, false);
    }
    if (this.positionData) {
      Highcharts.charts[this.quoteChart].yAxis[1].setExtremes(0, Math.max(this.positionData.quoteValue,Highcharts.charts[this.quoteChart].yAxis[1].getExtremes().dataMax), false, true, { trigger: 'syncExtremes' });
      Highcharts.charts[this.baseChart].yAxis[1].setExtremes(0, Math.max(this.positionData.baseValue,Highcharts.charts[this.baseChart].yAxis[1].getExtremes().dataMax), false, true, { trigger: 'syncExtremes' });
      Highcharts.charts[this.quoteChart].series[1].addPoint([time, (this.positionData.baseValue-this.targetBasePosition)*this.positionData.quoteValue/this.positionData.baseValue], false);
      Highcharts.charts[this.baseChart].series[1].addPoint([time, this.targetBasePosition], false);
      Highcharts.charts[this.quoteChart].series[2].addPoint([time, Math.max(0, this.positionData.baseValue-this.targetBasePosition-this.positionDivergence)*this.positionData.quoteValue/this.positionData.baseValue, Math.min(this.positionData.baseValue, this.positionData.baseValue-this.targetBasePosition+this.positionDivergence)*this.positionData.quoteValue/this.positionData.baseValue], this.showStats, false, false);
      Highcharts.charts[this.baseChart].series[2].addPoint([time, Math.max(0,this.targetBasePosition-this.positionDivergence), Math.min(this.positionData.baseValue, this.targetBasePosition+this.positionDivergence)], this.showStats, false, false);
      Highcharts.charts[this.quoteChart].series[0].addPoint([time, this.positionData.quoteValue], false);
      Highcharts.charts[this.quoteChart].series[3].addPoint([time, this.positionData.quoteAmount], false);
      Highcharts.charts[this.quoteChart].series[4].addPoint([time, this.positionData.quoteHeldAmount], this.showStats);
      Highcharts.charts[this.baseChart].series[0].addPoint([time, this.positionData.baseValue], false);
      Highcharts.charts[this.baseChart].series[3].addPoint([time, this.positionData.baseAmount], false);
      Highcharts.charts[this.baseChart].series[4].addPoint([time, this.positionData.baseHeldAmount], this.showStats);
    }
  }

  private removeOldPoints = (time: number) => {
    Highcharts.charts.forEach(chart => { chart.series.forEach(serie => {
      while(serie.data.length && Math.abs(time - serie.data[0].x) > (<any>Highcharts).quotingParameters.profitHourInterval * 36e+5)
        serie.data[0].remove(false);
    })});
  }
}
