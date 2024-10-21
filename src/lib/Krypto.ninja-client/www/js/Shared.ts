import {NgModule, Component, enableProdMode, OnInit} from '@angular/core';
import {platformBrowserDynamic}                      from '@angular/platform-browser-dynamic';
import {BrowserModule}                               from '@angular/platform-browser';
import {FormsModule}                                 from '@angular/forms';

import * as Highcharts         from 'highcharts';
require('highcharts/highcharts-more')(Highcharts);
export {Highcharts};
import {HighchartsChartModule} from 'highcharts-angular';

import {RowNode}      from 'ag-grid-community';
import {AgGridModule} from 'ag-grid-angular';

import {Socket, Models} from 'lib/K';

@Component({
  selector: 'K',
  template: `<div>
    <div [hidden]="state.online !== null"
      style="padding:42px;transform:rotate(-6deg);text-shadow: 1px 1px #0000009e;">
      <h4 class="text-danger text-center">
        <i class="beacon exc-{{ exchange_icon }}-s" style="font-size:69px;"></i>
        <br /><br />
        {{ product.environment ? product.environment+' is d' : 'D' }}isconnected
      </h4>
    </div>
    <div [hidden]="state.online === null">
      <div class="container-fluid">
          <div id="hud"
            [ngClass]="state.online ? 'bg-success' : 'bg-danger'">
            <client
              [state]="state"
              [product]="product"
              [addr]="addr"
              [tradeFreq]="tradeFreq"
              (onFooter)="onFooter($event)"></client>
          </div>
      </div>
    </div>
    <address>
      <a rel="noreferrer" target="_blank"
        href="{{ homepage }}/blob/master/README.md">README</a> -
      <a rel="noreferrer" target="_blank"
        href="{{ homepage }}/blob/master/doc/MANUAL.md">MANUAL</a> -
      <a rel="noreferrer" target="_blank"
        href="{{ homepage }}">SOURCE</a> -
      <span [hidden]="state.online === null">
        <span [hidden]="!product.inet">
          <span title="non-default Network Interface for outgoing traffic">{{ product.inet }}</span> -
        </span>
        <span title="Server used RAM"
          style="margin-top: 6px;display: inline-block;">{{ server_memory }}</span> -
        <span title="Client used RAM"
          style="margin-top: 6px;display: inline-block;">{{ client_memory }}</span> -
        <span title="Database Size"
          style="margin-top: 6px;display: inline-block;">{{ db_size }}</span> -
        <span [innerHTML]="footer"></span>
      </span>
      <a href="#"
        (click)="changeTheme()">{{ system_theme == 'light' ? 'DARK' : 'LIGHT' }}</a> -
      <a href="#"
        (click)="openMatryoshka()">MATRYOSHKA</a> -
      <a rel="noreferrer" target="_blank"
        href="{{ homepage }}/issues/new?title=%5Btopic%5D%20short%20and%20sweet%20description&body=description%0Aplease,%20consider%20to%20add%20all%20possible%20details%20%28if%20any%29%20about%20your%20new%20feature%20request%20or%20bug%20report%0A%0A%2D%2D%2D%0A%60%60%60%0Aapp%20exchange%3A%20{{ product.exchange }}/{{ product.base+'/'+product.quote }}%0Aapp%20version%3A%20undisclosed%0AOS%20distro%3A%20undisclosed%0A%60%60%60%0A![300px-spock_vulcan-salute3](https://cloud.githubusercontent.com/assets/1634027/22077151/4110e73e-ddb3-11e6-9d84-358e9f133d34.png)">CREATE ISSUE</a> -
      <a rel="noreferrer" target="_blank"
        href="https://github.com/ctubio/Krypto-trading-bot/discussions/new">HELP</a> -
      <a rel="noreferrer" target="_blank"
        href="https://discord.gg/jAX7GEzcWD">CHAT</a>
    </address>
    <iframe
      (window:resize)="resizeMatryoshka()"
      id="matryoshka"
      src="about:blank"></iframe>
  </div>`
})
export class KComponent implements OnInit {

  private homepage: string = 'https://github.com/ctubio/Krypto-trading-bot';
  private exchange_icon: string;
  private tradeFreq: number = 0;
  private addr: string;
  private footer: string;

  private server_memory: string = '0KB';
  private client_memory: string = '0KB';
  private db_size: string = '0KB';

  private user_theme: string = null;
  private system_theme: string = null;

  private state: Models.ExchangeState = new Models.ExchangeState();
  private product: Models.ProductAdvertisement = new Models.ProductAdvertisement();

  ngOnInit() {
    new Socket.Client();

    new Socket.Subscriber(Models.Topics.ProductAdvertisement)
      .registerSubscriber(this.onProduct);

    new Socket.Subscriber(Models.Topics.Connectivity)
      .registerSubscriber((o: Models.ExchangeState) => { this.state = o; })
      .registerDisconnectedHandler(() => { this.state.online = null; });

    new Socket.Subscriber(Models.Topics.ApplicationState)
      .registerSubscriber(this.onAppState);

    window.addEventListener("message", e => {
      if (!e.data.indexOf) return;

      if (e.data.indexOf('height=') === 0) {
        document.getElementById('matryoshka').style.height = e.data.replace('height=', '');
        this.resizeMatryoshka();
      }
    }, false);
  };

  private onFooter(o: string) {
    this.footer = o;
  };

  private openMatryoshka = () => {
    const url = window.prompt('Enter the URL of another instance:', this.product.matryoshka || 'https://');
    document.getElementById('matryoshka').setAttribute('src', url || 'about:blank');
    document.getElementById('matryoshka').style.height = (url && url != 'https://') ? '589px' : '0px';
  };

  private resizeMatryoshka = () => {
    if (window.parent === window) return;
    window.parent.postMessage('height=' + document.getElementsByTagName('body')[0].getBoundingClientRect().height + 'px', '*');
  };

  private setTheme = () => {
    if (this.system_theme==(document.body.classList.contains('theme-dark')?'dark':'light'))
      return;
    const rand = Math.random() * 360;
    document.body.style.backgroundColor = this.system_theme=='dark' ? '#222' : '#FFF';
    document.body.style.backgroundImage = this.system_theme=='dark'
      ? 'radial-gradient(circle, #0a0b0d 0%, hwb('+rand+' 3% 87% / 0.69) 100%)'
      : 'radial-gradient(circle, hwb('+rand+'deg 75% 15%) 0%, hwb('+(rand+50)+'deg 75% 15% / 0.69) 100%)';
    document.getElementById('hud').style.backgroundImage = document.body.style.backgroundImage.replace('100%', '0%').replace(' 0.69) ', ' 1) ');    
    if (this.system_theme=='dark') document.body.classList.add('theme-dark');
    else document.body.classList.remove('theme-dark');
    [...document.getElementsByTagName('ag-grid-angular')].forEach(o => {
      o.classList.add('ag-theme-alpine' + (this.system_theme=='dark'?'-dark':''));
      o.classList.remove('ag-theme-alpine' + (this.system_theme=='dark'?'':'-dark'));
    });
  };

  private changeTheme = () => {
    this.user_theme = this.user_theme!==null
                  ? (this.user_theme  == 'light' ? 'dark' : 'light')
                  : (this.system_theme== 'light' ? 'dark' : 'light');
    this.system_theme = this.user_theme;
    this.setTheme();
  };

  private getTheme = (hour: number) => {
    return this.user_theme!==null
         ? this.user_theme
         : ((hour<9 || hour>=21)?'dark':'light');
  };

  private onAppState = (o : Models.ApplicationState) => {
    this.server_memory = bytesToSize(o.memory, 0);
    this.client_memory = bytesToSize((<any>window.performance).memory ? (<any>window.performance).memory.usedJSHeapSize : 1, 0);
    this.db_size = bytesToSize(o.dbsize, 0);
    this.tradeFreq = (o.freq);
    this.user_theme = this.user_theme!==null ? this.user_theme : (o.theme==1 ? 'light' : (o.theme==2 ? 'dark' : this.user_theme));
    this.system_theme = this.getTheme((new Date).getHours());
    this.setTheme();
    this.addr = o.addr;
  }

  private onProduct = (o : Models.ProductAdvertisement) => {
    if (this.product.source && this.product.source != o.source)
      window.location.reload();
    window.document.title = '[' + o.environment + ']';
    this.exchange_icon = o.exchange.toLowerCase();
    this.product = o;
    setTimeout(() => {window.dispatchEvent(new Event('resize'))}, 0);
    console.log(
      "%c" + this.product.source + " started at " + (new Date().toISOString().slice(11, -1)) + "  %c" + this.homepage,
      "color:green;font-size:32px;",
      "color:red;font-size:16px;"
    );
  };
};

export function bootstrapModule(declarations: any[]) {
  @NgModule({
    imports: [
      BrowserModule,
      FormsModule,
      HighchartsChartModule,
      AgGridModule
    ],
    declarations: [KComponent].concat(declarations),
    bootstrap: [KComponent]
  })
  class KModule {};

  enableProdMode();
  platformBrowserDynamic().bootstrapModule(KModule);
};

export function bytesToSize(input: number, precision: number) {
  if (!input) return '0KB';
  let unit: string[] = ['', 'K', 'M', 'G', 'T', 'P'];
  let index: number = Math.floor(Math.log(input) / Math.log(1024));
  return index >= unit.length
    ? input + 'B'
    : (input / Math.pow(1024, index))
        .toFixed(precision) + unit[index] + 'B';
};

export function playAudio(basename: string) {
  let audio = new Audio('audio/' + basename + '.mp3');
  audio.volume = 0.5;
  audio.play();
};

export function resetRowData(name: string, val: string, node: RowNode): string[] {
  var dir   = '';
  var _val  = num(val);
  var _data = num(node.data[name]);
  if      (_val > _data) dir = 'up-data';
  else if (_val < _data) dir = 'down-data';
  if (dir != '') {
    node.data['dir_' + name] = dir;
    node.setDataValue(name, val);
  }
  return dir != '' ? [name] : [];
};

export function str(val: number, decimals: number): string {
  return val.toLocaleString([].concat(navigator.languages), {
    minimumFractionDigits: decimals,
    maximumFractionDigits: decimals
  });
};

var exp = null;
export function num(val: string): number {
  if (exp) return parseFloat(val.replace(exp, ''));
  exp = new RegExp('[^\\d-' + str(0, 1).replace(/0/g, '') + ']', 'g');
  return num(val);
};

export function comparator(valueA, valueB, nodeA, nodeB, isInverted) {
  return num(valueA) - num(valueB);
};
