#ifndef K_QP_H_
#define K_QP_H_

namespace K {
<<<<<<< HEAD
  struct Qp {
    int               widthPing                     = 2;
    double            widthPingPercentage           = decimal_cast<2>("0.25").getAsDouble();
    int               widthPong                     = 2;
    double            widthPongPercentage           = decimal_cast<2>("0.25").getAsDouble();
    bool              widthPercentage               = false;
    bool              bestWidth                     = true;
    double            buySize                       = decimal_cast<2>("0.02").getAsDouble();
    int               buySizePercentage             = 7;
    bool              buySizeMax                    = false;
    double            sellSize                      = decimal_cast<2>("0.01").getAsDouble();
    int               sellSizePercentage            = 7;
    bool              sellSizeMax                   = false;
    mPingAt           pingAt                        = mPingAt::BothSides;
    mPongAt           pongAt                        = mPongAt::ShortPingFair;
    mQuotingMode      mode                          = mQuotingMode::AK47;
    int               bullets                       = 2;
    double            range                         = decimal_cast<1>("0.5").getAsDouble();
    mFairValueModel   fvModel                       = mFairValueModel::BBO;
    int               targetBasePosition            = 1;
    int               targetBasePositionPercentage  = 50;
    double            positionDivergence            = decimal_cast<1>("0.9").getAsDouble();
    int               positionDivergencePercentage  = 21;
    bool              percentageValues              = false;
    mAutoPositionMode autoPositionMode              = mAutoPositionMode::EWMA_LS;
    mAPR              aggressivePositionRebalancing = mAPR::Off;
    mSOP              superTrades                   = mSOP::Off;
    double            tradesPerMinute               = decimal_cast<1>("0.9").getAsDouble();
    int               tradeRateSeconds              = 69;
    bool              quotingEwmaProtection         = true;
    int               quotingEwmaProtectionPeridos  = 200;
    mSTDEV            quotingStdevProtection        = mSTDEV::Off;
    bool              quotingStdevBollingerBands    = false;
    double            quotingStdevProtectionFactor  = decimal_cast<1>("1.0").getAsDouble();
    int               quotingStdevProtectionPeriods = 1200;
    double            ewmaSensiblityPercentage      = decimal_cast<1>("0.5").getAsDouble();
    int               longEwmaPeridos               = 200;
    int               mediumEwmaPeridos             = 100;
    int               shortEwmaPeridos              = 50;
    int               aprMultiplier                 = 2;
    int               sopWidthMultiplier            = 2;
    int               delayAPI                      = 0;
    bool              cancelOrdersAuto              = false;
    int               cleanPongsAuto                = 0;
    double            profitHourInterval            = decimal_cast<1>("0.5").getAsDouble();
    bool              audio                         = false;
    int               delayUI                       = 7;
    mMoveit            moveit                        = mMoveit::unknown;
    mMovemomentum     movement                  = mMovemomentum::unknown;
    double            upnormallow                   = decimal_cast<1>("-0.5").getAsDouble();
    double            upnormalhigh                   = decimal_cast<1>("0.5").getAsDouble();
    double            upmidlow                      = decimal_cast<1>("0.5").getAsDouble();
    double            upmidhigh                     = decimal_cast<1>("1").getAsDouble();
    double            upfastlow                     = decimal_cast<1>("1").getAsDouble();
    double            dnnormallow                   = decimal_cast<1>("-0.5").getAsDouble();
    double            dnnormalhigh                   = decimal_cast<1>("0.5").getAsDouble();
    double            dnmidlow                      = decimal_cast<1>("-0.5").getAsDouble();
    double            dnmidhigh                     = decimal_cast<1>("-1").getAsDouble();
    double            dnfastlow                     = decimal_cast<1>("-1").getAsDouble();
    double            asp_low                     = decimal_cast<1>("-5").getAsDouble();
    double            asp_high                     = decimal_cast<1>("5").getAsDouble();
    bool              aspactive                     = false;
    double            aspvalue                      = decimal_cast<1>("0").getAsDouble();
  } qp;
  class QP {
    public:
      static void main(Local<Object> exports) {
        Isolate* isolate = exports->GetIsolate();
        Qp *qp = new Qp;
        Local<Object> qpRepo_ = Object::New(isolate);
        qpRepo_->Set(FN::v8S("widthPing"), Number::New(isolate, qp->widthPing));
        qpRepo_->Set(FN::v8S("widthPingPercentage"), Number::New(isolate, qp->widthPingPercentage));
        qpRepo_->Set(FN::v8S("widthPong"), Number::New(isolate, qp->widthPong));
        qpRepo_->Set(FN::v8S("widthPongPercentage"), Number::New(isolate, qp->widthPongPercentage));
        qpRepo_->Set(FN::v8S("widthPercentage"), Boolean::New(isolate, qp->widthPercentage));
        qpRepo_->Set(FN::v8S("bestWidth"), Boolean::New(isolate, qp->bestWidth));
        qpRepo_->Set(FN::v8S("buySize"), Number::New(isolate, qp->buySize));
        qpRepo_->Set(FN::v8S("buySizePercentage"), Number::New(isolate, qp->buySizePercentage));
        qpRepo_->Set(FN::v8S("buySizeMax"), Boolean::New(isolate, qp->buySizeMax));
        qpRepo_->Set(FN::v8S("sellSize"), Number::New(isolate, qp->sellSize));
        qpRepo_->Set(FN::v8S("sellSizePercentage"), Number::New(isolate, qp->sellSizePercentage));
        qpRepo_->Set(FN::v8S("sellSizeMax"), Boolean::New(isolate, qp->sellSizeMax));
        qpRepo_->Set(FN::v8S("pingAt"), Number::New(isolate, (int)qp->pingAt));
        qpRepo_->Set(FN::v8S("pongAt"), Number::New(isolate, (int)qp->pongAt));
        qpRepo_->Set(FN::v8S("mode"), Number::New(isolate, (int)qp->mode));
        qpRepo_->Set(FN::v8S("bullets"), Number::New(isolate, qp->bullets));
        qpRepo_->Set(FN::v8S("range"), Number::New(isolate, qp->range));
        qpRepo_->Set(FN::v8S("fvModel"), Number::New(isolate, (int)qp->fvModel));
        qpRepo_->Set(FN::v8S("targetBasePosition"), Number::New(isolate, qp->targetBasePosition));
        qpRepo_->Set(FN::v8S("targetBasePositionPercentage"), Number::New(isolate, qp->targetBasePositionPercentage));
        qpRepo_->Set(FN::v8S("positionDivergence"), Number::New(isolate, qp->positionDivergence));
        qpRepo_->Set(FN::v8S("positionDivergencePercentage"), Number::New(isolate, qp->positionDivergencePercentage));
        qpRepo_->Set(FN::v8S("percentageValues"), Boolean::New(isolate, qp->percentageValues));
        qpRepo_->Set(FN::v8S("autoPositionMode"), Number::New(isolate, (int)qp->autoPositionMode));
        qpRepo_->Set(FN::v8S("aggressivePositionRebalancing"), Number::New(isolate, (int)qp->aggressivePositionRebalancing));
        qpRepo_->Set(FN::v8S("superTrades"), Number::New(isolate, (int)qp->superTrades));
        qpRepo_->Set(FN::v8S("tradesPerMinute"), Number::New(isolate, qp->tradesPerMinute));
        qpRepo_->Set(FN::v8S("tradeRateSeconds"), Number::New(isolate, qp->tradeRateSeconds));
        qpRepo_->Set(FN::v8S("quotingEwmaProtection"), Boolean::New(isolate, qp->quotingEwmaProtection));
        qpRepo_->Set(FN::v8S("quotingEwmaProtectionPeridos"), Number::New(isolate, qp->quotingEwmaProtectionPeridos));
        qpRepo_->Set(FN::v8S("quotingStdevProtection"), Number::New(isolate, (int)qp->quotingStdevProtection));
        qpRepo_->Set(FN::v8S("quotingStdevBollingerBands"), Boolean::New(isolate, qp->quotingStdevBollingerBands));
        qpRepo_->Set(FN::v8S("quotingStdevProtectionFactor"), Number::New(isolate, qp->quotingStdevProtectionFactor));
        qpRepo_->Set(FN::v8S("quotingStdevProtectionPeriods"), Number::New(isolate, qp->quotingStdevProtectionPeriods));
        qpRepo_->Set(FN::v8S("ewmaSensiblityPercentage"), Number::New(isolate, qp->ewmaSensiblityPercentage));
        qpRepo_->Set(FN::v8S("longEwmaPeridos"), Number::New(isolate, qp->longEwmaPeridos));
        qpRepo_->Set(FN::v8S("mediumEwmaPeridos"), Number::New(isolate, qp->mediumEwmaPeridos));
        qpRepo_->Set(FN::v8S("shortEwmaPeridos"), Number::New(isolate, qp->shortEwmaPeridos));
        qpRepo_->Set(FN::v8S("aprMultiplier"), Number::New(isolate, qp->aprMultiplier));
        qpRepo_->Set(FN::v8S("sopWidthMultiplier"), Number::New(isolate, qp->sopWidthMultiplier));
        qpRepo_->Set(FN::v8S("delayAPI"), Number::New(isolate, qp->delayAPI));
        qpRepo_->Set(FN::v8S("cancelOrdersAuto"), Boolean::New(isolate, qp->cancelOrdersAuto));
        qpRepo_->Set(FN::v8S("cleanPongsAuto"), Number::New(isolate, qp->cleanPongsAuto));
        qpRepo_->Set(FN::v8S("profitHourInterval"), Number::New(isolate, qp->profitHourInterval));
        qpRepo_->Set(FN::v8S("audio"), Boolean::New(isolate, qp->audio));
        qpRepo_->Set(FN::v8S("delayUI"), Number::New(isolate, qp->delayUI));
        qpRepo_->Set(FN::v8S("moveit"), Number::New(isolate, (int)qp->moveit));
        qpRepo_->Set(FN::v8S("movement"), Number::New(isolate, (int)qp->movement));
        qpRepo_->Set(FN::v8S("upnormallow"), Number::New(isolate, qp->upnormallow));
        qpRepo_->Set(FN::v8S("upnormalhigh"), Number::New(isolate, qp->upnormalhigh));
        qpRepo_->Set(FN::v8S("upmidlow"), Number::New(isolate, qp->upmidlow));
        qpRepo_->Set(FN::v8S("upmidhigh"), Number::New(isolate, qp->upmidhigh));
        qpRepo_->Set(FN::v8S("upfastlow"), Number::New(isolate, qp->upfastlow));
        qpRepo_->Set(FN::v8S("dnnormallow"), Number::New(isolate, qp->dnnormallow));
        qpRepo_->Set(FN::v8S("dnnormalhigh"), Number::New(isolate, qp->dnnormalhigh));
        qpRepo_->Set(FN::v8S("dnmidlow"), Number::New(isolate, qp->dnmidlow));
        qpRepo_->Set(FN::v8S("dnmidhigh"), Number::New(isolate, qp->dnmidhigh));
        qpRepo_->Set(FN::v8S("dnfastlow"), Number::New(isolate, qp->dnfastlow));
        qpRepo_->Set(FN::v8S("asp_low"), Number::New(isolate, qp->asp_low));
        qpRepo_->Set(FN::v8S("asp_high"), Number::New(isolate, qp->asp_high));
        qpRepo_->Set(FN::v8S("aspactive"), Boolean::New(isolate, qp->aspactive));
        qpRepo_->Set(FN::v8S("aspvalue"), Boolean::New(isolate, qp->aspvalue));

        MaybeLocal<Array> maybe_props;
        Local<Array> props;
        maybe_props = qpRepo_->GetOwnPropertyNames(Context::New(isolate));
        if (!maybe_props.IsEmpty()) {
          props = maybe_props.ToLocalChecked();
          for(uint32_t i=0; i < props->Length(); i++) {
            string k = CF::cfString(FN::S8v(props->Get(i)->ToString()), false);
            if (k != "") qpRepo_->Set(props->Get(i)->ToString(), Number::New(isolate, stod(k)));
          }
=======
  static json defQP {
    {  "widthPing",                     2                                      },
    {  "widthPingPercentage",           decimal_cast<2>("0.25").getAsDouble()  },
    {  "widthPong",                     2                                      },
    {  "widthPongPercentage",           decimal_cast<2>("0.25").getAsDouble()  },
    {  "widthPercentage",               false                                  },
    {  "bestWidth",                     true                                   },
    {  "buySize",                       decimal_cast<2>("0.02").getAsDouble()  },
    {  "buySizePercentage",             7                                      },
    {  "buySizeMax",                    false                                  },
    {  "sellSize",                      decimal_cast<2>("0.01").getAsDouble()  },
    {  "sellSizePercentage",            7                                      },
    {  "sellSizeMax",                   false                                  },
    {  "pingAt",                        (int)mPingAt::BothSides                },
    {  "pongAt",                        (int)mPongAt::ShortPingFair            },
    {  "mode",                          (int)mQuotingMode::AK47                },
    {  "bullets",                       2                                      },
    {  "range",                         decimal_cast<1>("0.5").getAsDouble()   },
    {  "fvModel",                       (int)mFairValueModel::BBO              },
    {  "targetBasePosition",            1                                      },
    {  "targetBasePositionPercentage",  50                                     },
    {  "positionDivergence",            decimal_cast<1>("0.9").getAsDouble()   },
    {  "positionDivergencePercentage",  21                                     },
    {  "percentageValues",              false                                  },
    {  "autoPositionMode",              (int)mAutoPositionMode::EWMA_LS        },
    {  "aggressivePositionRebalancing", (int)mAPR::Off                         },
    {  "superTrades",                   (int)mSOP::Off                         },
    {  "tradesPerMinute",               decimal_cast<1>("0.9").getAsDouble()   },
    {  "tradeRateSeconds",              69                                     },
    {  "quotingEwmaProtection",         true                                   },
    {  "quotingEwmaProtectionPeridos",  200                                    },
    {  "quotingStdevProtection",        (int)mSTDEV::Off                       },
    {  "quotingStdevBollingerBands",    false                                  },
    {  "quotingStdevProtectionFactor",  decimal_cast<1>("1.0").getAsDouble()   },
    {  "quotingStdevProtectionPeriods", 1200                                   },
    {  "ewmaSensiblityPercentage",      decimal_cast<1>("0.5").getAsDouble()   },
    {  "longEwmaPeridos",               200                                    },
    {  "mediumEwmaPeridos",             100                                    },
    {  "shortEwmaPeridos",              50                                     },
    {  "aprMultiplier",                 2                                      },
    {  "sopWidthMultiplier",            2                                      },
    {  "delayAPI",                      0                                      },
    {  "cancelOrdersAuto",              false                                  },
    {  "cleanPongsAuto",                0                                      },
    {  "profitHourInterval",            decimal_cast<1>("0.5").getAsDouble()   },
    {  "audio",                         false                                  },
    {  "delayUI",                       7                                      }
  };
  static vector<string> boolQP = {
    "widthPercentage", "bestWidth", "sellSizeMax", "buySizeMax", "percentageValues",
    "quotingEwmaProtection", "quotingStdevBollingerBands", "cancelOrdersAuto", "audio"
  };
  class QP {
    public:
      static void main(Local<Object> exports) {
        for (json::iterator it = defQP.begin(); it != defQP.end(); ++it) {
          string k = CF::cfString(it.key(), false);
          if (k != "") defQP[it.key()] = k;
>>>>>>> upstream/master
        }
        qpRepo = defQP;
        json qpa = DB::load(uiTXT::QuotingParametersChange);
        if (qpa.size())
          for (json::iterator it = qpa["/0"_json_pointer].begin(); it != qpa["/0"_json_pointer].end(); ++it)
            qpRepo[it.key()] = it.value();
        cleanBool();
        UI::uiSnap(uiTXT::QuotingParametersChange, &onSnap);
        UI::uiHand(uiTXT::QuotingParametersChange, &onHand);
        EV::evUp("QuotingParameters", qpRepo);
        NODE_SET_METHOD(exports, "qpRepo", QP::_qpRepo);
      }
    private:
      static void _qpRepo(const FunctionCallbackInfo<Value> &args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);
        Local<Object> o = Object::New(isolate);
        for (json::iterator it = qpRepo.begin(); it != qpRepo.end(); ++it)
          if (it.value().is_number()) o->Set(FN::v8S(it.key()), Number::New(isolate, it.value()));
          else if (it.value().is_boolean()) o->Set(FN::v8S(it.key()), Boolean::New(isolate, it.value()));
        args.GetReturnValue().Set(o);
      };
      static json onSnap(json z) {
        return { qpRepo };
      };
      static json onHand(json k) {
        if (k["buySize"].get<double>() > 0
          && k["sellSize"].get<double>() > 0
          && k["buySizePercentage"].get<double>() > 0
          && k["sellSizePercentage"].get<double>() > 0
          && k["widthPing"].get<double>() > 0
          && k["widthPong"].get<double>() > 0
          && k["widthPingPercentage"].get<double>() > 0
          && k["widthPongPercentage"].get<double>() > 0
        ) {
          if ((mQuotingMode)k["mode"].get<int>() == mQuotingMode::Depth)
            k["widthPercentage"] = false;
          qpRepo = k;
          cleanBool();
          DB::insert(uiTXT::QuotingParametersChange, k);
          EV::evUp("QuotingParameters", k);
        }
        UI::uiSend(uiTXT::QuotingParametersChange, k);
        return {};
      };
      static void cleanBool() {
        for (vector<string>::iterator it = boolQP.begin(); it != boolQP.end(); ++it)
          if (qpRepo[*it].is_number()) qpRepo[*it] = qpRepo[*it].get<int>() != 0;
      };
  };
}

#endif
