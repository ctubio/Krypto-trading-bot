#ifndef K_EV_H_
#define K_EV_H_

#define _errorEvent_ ((EV*)events)->error
#define _debugEvent_ ((EV*)events)->debug(__PRETTY_FUNCTION__);

namespace K  {
  class EV: public Klass {
    private:
      uWS::Hub *hub = nullptr;
      Async *aEngine = nullptr;
      vector<function<void()>> asyncFn;
      future<int> hotkey;
    public:
      uWS::Group<uWS::SERVER> *uiGroup = nullptr;
      Timer *tServer = nullptr,
            *tEngine = nullptr,
            *tClient = nullptr;
      function<void(mOrder*)> ogOrder;
      function<void(mTrade*)> ogTrade;
      function<void()>        mgLevels,
                              mgEwmaQuoteProtection,
                              mgTargetPosition,
                              pgTargetBasePosition,
                              uiQuotingParameters;
    protected:
      void load() {
        gwEndings.push_back(&happyEnding);
        signal(SIGINT, quit);
        signal(SIGUSR1, wtf);
        signal(SIGABRT, wtf);
        signal(SIGSEGV, wtf);
        version();
        if (!gw) exit(error("GW", string("Unable to load a valid gateway using --exchange=") + ((CF*)config)->argExchange + " argument"));
        gw->hub = hub = new uWS::Hub(0, true);
      };
      void waitTime() {
        tServer = new Timer(hub->getLoop());
        tEngine = new Timer(hub->getLoop());
        tClient = new Timer(hub->getLoop());
      };
      void waitData() {
        aEngine = new Async(hub->getLoop());
        aEngine->setData(this);
        aEngine->start(asyncLoop);
        gw->gwGroup = hub->createGroup<uWS::CLIENT>();
      };
      void waitUser() {
        uiGroup = hub->createGroup<uWS::SERVER>(uWS::PERMESSAGE_DEFLATE);
        if (((CF*)config)->argNaked) return;
        hotkey = async(launch::async, FN::screen_events);
      };
      void run() {
        if (((CF*)config)->argDebugEvents) return;
        debug = [&](string k) {};
      };
    public:
      void start(/* KMxTWEpb9ig */) {
        THIS_WAS_A_TRIUMPH
          << "- roll-out: " << to_string(_Tstamp_) << '\n';
        hub->run();
      };
      void stop(function<void()> gwCancelAll) {
        tServer->stop();
        tEngine->stop();
        tClient->stop();
        gw->close();
        gw->gwGroup->close();
        gwCancelAll();
        asyncLoop(aEngine);
        uiGroup->close();
      };
      void listen() {
        string protocol("HTTP");
        if (!((CF*)config)->argWithoutSSL
          and (access("etc/sslcert/server.crt", F_OK) != -1) and (access("etc/sslcert/server.key", F_OK) != -1)
          and hub->listen(((CF*)config)->argPort, uS::TLS::createContext("etc/sslcert/server.crt", "etc/sslcert/server.key", ""), 0, uiGroup)
        ) protocol += "S";
        else if (!hub->listen(((CF*)config)->argPort, nullptr, 0, uiGroup))
          exit(error("IU", string("Use another UI port number, ")
            + to_string(((CF*)config)->argPort) + " seems already in use by:\n"
            + FN::output(string("netstat -anp 2>/dev/null | grep ") + to_string(((CF*)config)->argPort))
          ));
        FN::logUI(protocol, ((CF*)config)->argPort);
      };
      void deferred(function<void()> fn) {
        asyncFn.push_back(fn);
        aEngine->send();
      };
      int error(string k, string s, bool reboot = false) {
        FN::screen_quit();
        FN::logErr(k, s);
        return reboot ? EXIT_FAILURE : EXIT_SUCCESS;
      };
      function<void(string)> debug = [&](string k) {
        FN::log("DEBUG", string("EV ") + k);
      };
    private:
      function<void()> happyEnding = [&]() {
        cout << FN::uiT() << gw->name;
        for (unsigned int i = 0; i < 21; ++i)
          cout << " THE END IS NEVER";
        cout << " THE END." << '\n';
      };
      void (*asyncLoop)(Async*) = [](Async *handle) {
        EV* k = (EV*)handle->getData();
        if (!k->asyncFn.empty()) {
          for (function<void()> &it : k->asyncFn) it();
          k->asyncFn.clear();
        }
        if (k->hotkey.valid() and k->hotkey.wait_for(chrono::nanoseconds(0)) == future_status::ready) {
          int ch = k->hotkey.get();
          if (ch == 'q' or ch == 'Q')
            raise(SIGINT);
        }
      };
      void version() {
        if (access(".git", F_OK) != -1) {
          FN::output("git fetch");
          string k = changelog();
          FN::logVer(k, count(k.begin(), k.end(), '\n'));
        } else FN::logVer("", -1);
        THIS_WAS_A_TRIUMPH
          << "- upstream: " << ((CF*)config)->argExchange << '\n'
          << "- currency: " << ((CF*)config)->argCurrency << '\n';
      };
      static void halt(int last_int_alive) {
        FN::screen_quit();
        cout << '\n' << FN::uiT() << THIS_WAS_A_TRIUMPH.str();
        for (function<void()>* &it : gwEndings) (*it)();
        if (last_int_alive == EXIT_FAILURE)
          this_thread::sleep_for(chrono::seconds(3));
        cout << FN::uiT() << "K exit code " << to_string(last_int_alive) << "." << '\n';
        exit(last_int_alive);
      };
      static void quit(int last_int_alive) {
        THIS_WAS_A_TRIUMPH.str("");
        THIS_WAS_A_TRIUMPH
          << "Excellent decision! "
          << FN::wJet("https://api.icndb.com/jokes/random?escape=javascript&limitTo=[nerdy]", true)
             .value("/value/joke"_json_pointer, "let's plant a tree instead..") << '\n';
        halt(EXIT_SUCCESS);
      };
      static void  wtf(int last_int_alive) {
        ostringstream rollout(THIS_WAS_A_TRIUMPH.str());
        THIS_WAS_A_TRIUMPH.str("");
        THIS_WAS_A_TRIUMPH
          << RCYAN << "Errrror: Signal " << last_int_alive << " "  << strsignal(last_int_alive);
        if (unsupported()) upgrade();
        else {
          THIS_WAS_A_TRIUMPH
            << " (Three-Headed Monkey found):" << '\n' << rollout.str()
            << "- lastbeat: " << to_string(_Tstamp_) << '\n'
            << "- tracelog: " << '\n';
          void *k[69];
          size_t jumps = backtrace(k, 69);
          char **trace = backtrace_symbols(k, jumps);
          size_t i;
          for (i = 0; i < jumps; i++)
            THIS_WAS_A_TRIUMPH
              << trace[i] << '\n';
          free(trace);
          report();
        }
        halt(EXIT_FAILURE);
      };
      static void report() {
        THIS_WAS_A_TRIUMPH
          << '\n' << BRED << "Yikes!" << RRED
          << '\n' << "please copy and paste the error above into a new github issue (noworry for duplicates)."
          << '\n' << "If you agree, go to https://github.com/ctubio/Krypto-trading-bot/issues/new"
          << '\n' << '\n';
      };
      static void upgrade() {
        THIS_WAS_A_TRIUMPH
          << " (deprecated K version found)." << '\n'
          << '\n' << BYELLOW << "Hint!" << RYELLOW
          << '\n' << "please upgrade to the latest commit; the encountered error may be already fixed at:"
          << '\n' << changelog()
          << '\n' << "If you agree, consider to run \"make latest\" prior further executions."
          << '\n' << '\n';
      };
      static bool unsupported() {
        return FN::output("test -d .git && git rev-parse @") != FN::output("test -d .git && git rev-parse @{u}");
      };
      static string changelog() {
        return FN::output("test -d .git && git --no-pager log --graph --oneline @..@{u}");
      };
  };
}

#endif
