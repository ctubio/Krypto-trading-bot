import {Component, Input} from '@angular/core';

import {Socket, Models} from 'lib/K';

@Component({
  selector: 'submit-order',
  template: `<table class="table table-responsive" style="width: 715px;margin-left: 10px;">
    <thead>
      <tr>
        <th style="width:100px;">Symbol:</th>
        <th style="width:100px;">Side:</th>
        <th style="width:100px;">Price:</th>
        <th style="width:100px;">Size:</th>
        <th style="width:100px;">TIF:</th>
        <th style="width:100px;">Type:</th>
      </tr>
    </thead>
    <tbody>
      <tr>
        <td>
          <select class="form-control input-sm" [(ngModel)]="symbol">
            <option
              *ngFor="let sym of availableSymbols"
              [ngValue]="sym"
            >{{ sym }}</option>
          </select>
        </td>
        <td>
          <select id="selectSide" class="form-control input-sm" [(ngModel)]="side">
            <option
              *ngFor="let option of availableSides"
              [ngValue]="option.val"
            >{{ option.str }}</option>
          </select>
        </td>
        <td>
          <input
            id="orderPriceInput"
            class="form-control input-sm"
            type="number"
            min="0"
            [(ngModel)]="price" />
        </td>
        <td>
          <input
            id="orderSizeInput"
            class="form-control input-sm"
            type="number"
            min="0"
            [(ngModel)]="quantity" />
        </td>
        <td>
          <select class="form-control input-sm" [(ngModel)]="timeInForce">
            <option
              *ngFor="let option of availableTifs"
              [ngValue]="option.val"
            >{{ option.str }}</option>
          </select>
        </td>
        <td>
          <select class="form-control input-sm" [(ngModel)]="type">
            <option
              *ngFor="let option of availableOrderTypes"
              [ngValue]="option.val"
            >{{ option.str }}</option>
          </select>
        </td>
        <td style="width:70px;" rowspan="2" class="text-center">
          <button
            type="button"
            class="btn btn-default"
            (click)="submitManualOrder()"
          >Submit</button>
        </td>
      </tr>
    </tbody>
  </table>`
})
export class SubmitComponent {

  private symbol: string;
  private side: Models.Side = Models.Side.Bid;
  private price: number;
  private quantity: number;
  private timeInForce: Models.TimeInForce = Models.TimeInForce.GTC;
  private type: Models.OrderType = Models.OrderType.Limit;

  private availableSides:      Models.Map[] = Models.getMap(Models.Side);
  private availableTifs:       Models.Map[] = Models.getMap(Models.TimeInForce);
  private availableOrderTypes: Models.Map[] = Models.getMap(Models.OrderType);
  private availableSymbols:        string[] = [];

  private fireCxl: Socket.IFire<Models.OrderRequestFromUI> = new Socket.Fire(Models.Topics.SubmitNewOrder);


  @Input() set markets(o: any) {
    if (!this.availableSymbols.length)
      for (let x in o)
        for (let z in o[x])
          if (!this.availableSymbols.filter(s => s == o[x][z].symbol).length) {
            this.availableSymbols.push(o[x][z].symbol);
            this.availableSymbols.sort();
          }
  };

  private submitManualOrder = () => {
    if (this.price && this.quantity)
      this.fireCxl.fire(new Models.OrderRequestFromUI(
        this.symbol,
        this.side,
        this.price,
        this.quantity,
        this.timeInForce,
        this.type
      ));
  };
};
