import {Component, Input} from '@angular/core';

import {Socket, Models} from 'lib/K';

@Component({
  selector: 'settings',
  template: `<div id="portfolios_settings">
    <div>
      <select class="form-control input-lg" id="base_select"
        title="base currency"
        [(ngModel)]="params.currency"
        (ngModelChange)="submitSettings()">
        <option [ngValue]="product.base">{{ product.base }}</option>
        <option [ngValue]="product.quote">{{ product.quote }}</option>
      </select>
      <input type="checkbox" id="zeroed_checkbox"
        title="show zeroed balances"
        [(ngModel)]="params.zeroed"
        (ngModelChange)="submitSettings()">
    </div>
    <h2>
      <a rel="noreferrer" target="_blank"
        href="{{ product.webOrders }}"
        title="full balance"><span id="full_balance" >0.00000000</span><i class="beacon sym-_default-s sym-{{ params.currency.toLowerCase() }}-s" ></i></a>
    </h2>
  </div>`
})
export class SettingsComponent {

  private params: Models.PortfolioParameters = JSON.parse(JSON.stringify(new Models.PortfolioParameters()));
  private pending: boolean = false;

  private fireCxl: Socket.IFire<Models.PortfolioParameters> = new Socket.Fire(Models.Topics.QuotingParametersChange);

  @Input() product: Models.ProductAdvertisement;

  @Input() set settings(o: Models.PortfolioParameters) {
    this.params = JSON.parse(JSON.stringify(o));
    this.pending = false;
    setTimeout(() => {window.dispatchEvent(new Event('resize'))}, 0);
  };

  private submitSettings = () => {
    this.pending = true;
    this.fireCxl.fire(this.params);
  };
};
