import Models = require("../share/models");
import Utils = require("./utils");
import Statistics = require("./statistics");
import FairValue = require("./fair-value");
import moment = require("moment");
import Broker = require("./broker");
export class TargetBasePositionManager {
  public sideAPR: string;

  private newWidth: Models.IStdev = null;
  private newQuote: number = null;
  private newShort: number = null;
  private newMedium: number = null;
  private newLong: number = null;
  private fairValue: number = null;
  public latestLong: number = null;
  private latestMedium: number = null;
  public latestShort: number = null;

  public EMASHORT: number[] = [];

  public set quoteEwma(quoteEwma: number) {
    this.newQuote = quoteEwma;
  }
  public set widthStdev(widthStdev: Models.IStdev) {
    this.newWidth = widthStdev;
  }

  private _newTargetPosition: number = 0;
  private _lastPosition: number = null;

  private _latest: Models.TargetBasePositionValue = null;
  public get latestTargetPosition(): Models.TargetBasePositionValue {
    return this._latest;
  }

  constructor(
    private _minTick: number,
    private _dbInsert,
    private _fvAgent: FairValue.FairValueEngine,
    private _ewma: Statistics.EWMATargetPositionCalculator,
    private _qpRepo,
    private _positionBroker: Broker.PositionBroker,
    private _uiSnap,
    private _uiSend,
    private _evOn,
    private _evUp,
    initRfv: Models.RegularFairValue[],
    initTBP: Models.TargetBasePositionValue[]
  ) {
    if (initTBP.length && typeof initTBP[0].tbp != "undefined") {
      this._latest = initTBP[0];
      console.info(new Date().toISOString().slice(11, -1), 'tbp', 'Loaded from DB:', this._latest.tbp);
    }
    if (initRfv !== null && initRfv.length) {
      this.latestLong = initRfv[0].ewmaLong;
      this.latestMedium = initRfv[0].ewmaMedium;
      this.latestShort = initRfv[0].ewmaShort;
    }





    _uiSnap(Models.Topics.TargetBasePosition, () => [this._latest]);
    _uiSnap(Models.Topics.EWMAChart, () => [this.fairValue?new Models.EWMAChart(
      this.newWidth,
      this.newQuote?Utils.roundNearest(this.newQuote, this._minTick):null,
      this.newShort?Utils.roundNearest(this.newShort, this._minTick):null,
      this.newMedium?Utils.roundNearest(this.newMedium, this._minTick):null,
      this.newLong?Utils.roundNearest(this.newLong, this._minTick):null,
      this.fairValue?Utils.roundNearest(this.fairValue, this._minTick):null
    ):null]);
    this._evOn('PositionBroker', this.recomputeTargetPosition);
    this._evOn('QuotingParameters', () => setTimeout(() => this.recomputeTargetPosition(), moment.duration(121)));
    setInterval(this.updateEwmaValues, moment.duration(1, 'minutes'));
  }

  private recomputeTargetPosition = () => {
    const params = this._qpRepo();
    if (params === null || this._positionBroker.latestReport === null) {
      console.info(new Date().toISOString().slice(11, -1), 'tbp', 'Unable to compute tbp [ qp | pos ] = [', !!params, '|', !!this._positionBroker.latestReport, ']');
      return;
    }
    const targetBasePosition: number = (params.autoPositionMode === Models.AutoPositionMode.Manual)
      ? (params.percentageValues
        ? params.targetBasePositionPercentage * this._positionBroker.latestReport.value / 100
        : params.targetBasePosition)
      : ((1 + this._newTargetPosition) / 2) * this._positionBroker.latestReport.value;

    if (this._latest === null || Math.abs(this._latest.tbp - targetBasePosition) > 1e-4 || this.sideAPR !== this._latest.sideAPR) {
      this._latest = new Models.TargetBasePositionValue(targetBasePosition, this.sideAPR);
      this._evUp('TargetPosition');
      this._uiSend(Models.Topics.TargetBasePosition, this._latest, true);
      this._dbInsert(Models.Topics.TargetBasePosition, this._latest);
      console.info(new Date().toISOString().slice(11, -1), 'tbp', 'recalculated', this._latest.tbp);

      let fairFV: number = this._fvAgent.latestFairValue.price;

    //  this._uiSend(Models.Topics.FairValue,   fairValue , true);
    //  this._dbInsert(Models.Topics.FairValue,   fairValue );

    if (this._fvAgent.latestFairValue === null) {
      console.info(new Date().toISOString().slice(11, -1), 'tbp', 'Unable to update ewma');
      return;
    }

    let movement: number = ((this.newShort - this.newLong) / ((this.newShort + this.newLong) / 2)) * 100 ;

      params.aspvalue = (this.newShort * 100 / this.newLong) - 100 ;
    //  console.info(new Date().toISOString().slice(11, -1), 'ASP2', 'Find the bug:', mrdebug )
      console.info(new Date().toISOString().slice(11, -1), 'ASP2', 'Fair Value recalculated:', this._fvAgent.latestFairValue.price )
      console.info(new Date().toISOString().slice(11, -1), 'ASP2', 'New Short Value:', (this.newShort ) )
      console.info(new Date().toISOString().slice(11, -1), 'ASP2', 'New Long Value:', (this.newLong) )
      console.info(new Date().toISOString().slice(11, -1), 'ASP2', 'recalculated', params.aspvalue )
      console.info(new Date().toISOString().slice(11, -1), 'ASP2', 'Movement', movement )
      console.info(new Date().toISOString().slice(11, -1), 'ASP2', 'EMA SHORT: ', this.newShort);
    

      //this.EMASHORT.push(this.newShort);
      //this._EMASHORT = this._EMASHORT.slice(-(params.shortEwmaPeridos));
  //    this._EMALONG.push(newLong);
    //  this._EMALONG = this._EMALONG.slice(-(params.longEwmaPeridos));





//   EMA.calculate({period : params.shortEwmaPeridos, values : this.EMASHORT});
   //sma.getResult()
//  console.info(new Date().toISOString().slice(11, -1), 'EMZZ: ',   EMA.getResult() )
//  console.info(new Date().toISOString().slice(11, -1), 'EMZZ: ',   this.EMASHORT )
      if(this.newShort > this.newLong) {
        // Going up!
        params.moveit = Models.mMoveit.up;
        console.info(new Date().toISOString().slice(11, -1), 'MoveMent: ',   Models.mMoveit[params.moveit] )
        if (params.upnormallow > movement && params.upnormalhigh < movement) {
          params.movement = Models.mMovemomentum.normal;
        } else if ( movement > params.upmidlow && movement < params.upmidhigh )
        {
          params.movement = Models.mMovemomentum.mid;
        } else if (movement > params.upfastlow )
        {
          params.movement = Models.mMovemomentum.fast;
        }


      } else if(this.newShort  < this.newLong) {
        // Going down
        params.moveit = Models.mMoveit.down;
        console.info(new Date().toISOString().slice(11, -1), 'MoveMent: ',   Models.mMoveit[params.moveit] )
        if (movement > params.dnnormallow  &&  movement < params.dnnormalhigh ) {
          params.movement = Models.mMovemomentum.normal;
        } else if ( movement < params.dnmidlow  &&  movement > params.dnmidhigh )
        {
          params.movement = Models.mMovemomentum.mid;
        } else if (movement < params.dnfastlow )
        {
          params.movement = Models.mMovemomentum.fast;
        }
      } else {
        console.info(new Date().toISOString().slice(11, -1), 'Movement', 'I Suck at math: ' )

      }
      console.info(new Date().toISOString().slice(11, -1), 'Movement', 'Speed: ', Models.mMovemomentum[params.movement] )



    }
  };

  private updateEwmaValues = () => {
    if (this._fvAgent.latestFairValue === null) {
      console.info(new Date().toISOString().slice(11, -1), 'tbp', 'Unable to update ewma');
      return;
    }
    //  const params = this._qpRepo();

    this.fairValue = this._fvAgent.latestFairValue.price;

    this.newShort = this._ewma.addNewShortValue(this.fairValue);
    this.newMedium = this._ewma.addNewMediumValue(this.fairValue);
    this.newLong = this._ewma.addNewLongValue(this.fairValue);
    this._newTargetPosition = this._ewma.computeTBP(this.fairValue, this.newLong, this.newMedium, this.newShort);
    // console.info(new Date().toISOString().slice(11, -1), 'tbp', 'recalculated ewma [ FV | L | M | S ] = [',this.fairValue,'|',this.newLong,'|',this.newMedium,'|',this.newShort,']');
    this.recomputeTargetPosition();
//params.shortEMAArray.push(this.newShort);


    this._uiSend(Models.Topics.EWMAChart, new Models.EWMAChart(
      this.newWidth,
      this.newQuote?Utils.roundNearest(this.newQuote, this._minTick):null,
      Utils.roundNearest(this.newShort, this._minTick),
      Utils.roundNearest(this.newMedium, this._minTick),
      Utils.roundNearest(this.newLong, this._minTick),
      Utils.roundNearest(this.fairValue, this._minTick)
    ), true);

    this._dbInsert(Models.Topics.EWMAChart, new Models.RegularFairValue(this.fairValue, this.newLong, this.newMedium, this.newShort));
  };
}
