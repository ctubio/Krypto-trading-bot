#include <nan.h>
#include "round.h"
#include "stdev.h"

namespace K {
  NAN_MODULE_INIT(InitAll) {
    Round::Init(target);
    Stdev::Init(target);
  }

  NODE_MODULE(K, InitAll)
}
