//! \file
//! \brief Minimal user application framework.

namespace ₿ {
  static string epilogue, epitaph;

  //! \brief     Call all endingFn once and print a last log msg.
  //! \param[in] reason Allows any (colorful?) string.
  static void exit(const string &reason = "") {
    epilogue = reason + string(!(reason.empty() or reason.back() == '.'), '.');
    raise(SIGQUIT);
  };

  //! \brief     Call all endingFn once and print a last error log msg.
  //! \param[in] prefix Allows any string, if possible with a length of 2.
  //! \param[in] reason Allows any (colorful?) string.
  static void error(const string &prefix, const string &reason) {
    exit(prefix + ANSI_PUKE_RED + " Errrror: " + ANSI_HIGH_RED + reason);
  };

  static mutex lock;

  class Rollout {
    public:
      Rollout() {
        static once_flag rollout;
        call_once(rollout, version);
      };
    protected:
      static string changelog() {
        string mods;
        const json diff =
#ifndef NDEBUG
          json::object();
#else
          Curl::Web::xfer(lock, "https://api.github.com/repos/ctubio/"
            "Krypto-trading-bot/compare/" K_HEAD "...HEAD");
#endif
        if (diff.value("ahead_by", 0)
          and diff.contains("commits")
          and diff.at("commits").is_array()
        ) for (const json &it : diff.at("commits"))
          mods += it.value("/commit/author/date"_json_pointer, "").substr(0, 10) + " "
                + it.value("/commit/author/date"_json_pointer, "").substr(11, 8)
                + " (" + it.value("sha", "").substr(0, 7) + ") "
                + it.value("/commit/message"_json_pointer, "").substr(0,
                  it.value("/commit/message"_json_pointer, "").find("\n\n") + 1
                );
        return mods;
      };
    private:
      static void version() {
        clog << ANSI_HIGH_GREEN << K_SOURCE " " K_BUILD
             << ANSI_PUKE_GREEN << " (build on " K_CHOST " at " K_STAMP ")"
#ifndef NDEBUG
             << ANSI_HIGH_GREEN << " with DEBUG MODE enabled"
             << ANSI_PUKE_GREEN
#endif
             << '.' << ANSI_RESET ANSI_NEW_LINE;
      };
  };

  static vector<function<void()>> endingFn;

  static volatile sig_atomic_t sigscr = SIGWINCH;

  class Ending: public Rollout {
    public_friend:
      using QuitEvent = function<void()>;
    public:
      Ending() {
        ::signal(SIGPIPE,  SIG_IGN);
        ::signal(SIGINT, [](const int) {
          clog << ANSI_NEW_LINE;
          raise(SIGQUIT);
        });
        ::signal(SIGQUIT,  die);
        ::signal(SIGTERM,  err);
        ::signal(SIGABRT,  wtf);
        ::signal(SIGSEGV,  wtf);
        ::signal(SIGUSR1,  wtf);
        ::signal(SIGWINCH, meh);
      };
      void ending(const QuitEvent &fn) {
        endingFn.push_back(fn);
      };
    private:
      static void meh(const int sig) {
        sigscr = sig;
      };
      static void halt(const int code) {
        vector<function<void()>> happyEndingFn;
        endingFn.swap(happyEndingFn);
        for (const auto &it : happyEndingFn) it();
        colorful = true;
        clog << ANSI_HIGH_GREEN << 'K'
             << ANSI_PUKE_GREEN << " exit code "
             << ANSI_HIGH_GREEN << code
             << ANSI_PUKE_GREEN << '.'
             << ANSI_RESET ANSI_NEW_LINE;
        EXIT(code);
      };
      static void die(const int) {
        if (epilogue.empty())
          epilogue = "Excellent decision! "
                   + Curl::Web::xfer(lock, "https://api.icndb.com/jokes/random?escape=javascript&limitTo=[nerdy]")
                       .value("/value/joke"_json_pointer, "let's plant a tree instead..");
        halt(
          epilogue.find("Errrror") == string::npos
            ? EXIT_SUCCESS
            : EXIT_FAILURE
        );
      };
      static void err(const int) {
        if (epilogue.empty()) epilogue = "Unknown exit reason, no joke.";
        halt(EXIT_FAILURE);
      };
      static void wtf(const int sig) {
        epilogue = ANSI_PUKE_CYAN + "Errrror: " + strsignal(sig) + ' ';
        const string mods = changelog();
        if (mods.empty()) {
          epilogue += "(Three-Headed Monkey found):" ANSI_NEW_LINE + epitaph
            + "- binbuild: " K_SOURCE " " K_CHOST   ANSI_NEW_LINE
              "- lastbeat: " + to_string(Tspent)  + ANSI_NEW_LINE
#ifndef _WIN32
            + "- tracelog: " ANSI_NEW_LINE;
          void *k[69];
          size_t jumps = backtrace(k, 69);
          char **trace = backtrace_symbols(k, jumps);
          for (;
            jumps --> 0;
            epilogue += "  " + to_string(jumps) + ": " + string(trace[jumps]) + ANSI_NEW_LINE
          );
          free(trace)
#endif
          ;
          epilogue += ANSI_NEW_LINE
            + ANSI_HIGH_RED + "Yikes!" + ANSI_PUKE_RED
            + ANSI_NEW_LINE   "please copy and paste the error above into a new github issue (noworry for duplicates)."
              ANSI_NEW_LINE   "If you agree, go to https://github.com/ctubio/Krypto-trading-bot/issues/new"
              ANSI_NEW_LINE;
        } else
          epilogue += string("(deprecated K version found).") + ANSI_NEW_LINE ANSI_NEW_LINE
            + ANSI_HIGH_YELLOW + "Hint!" + ANSI_PUKE_YELLOW
            + ANSI_NEW_LINE      "please upgrade to the latest commit; the encountered error may be already fixed at:"
              ANSI_NEW_LINE    + mods
            + ANSI_NEW_LINE      "If you agree, consider to run \"make upgrade\" prior further executions."
              ANSI_NEW_LINE;
        halt(EXIT_FAILURE);
      };
  };

  class Terminal {
    public:
      struct {
        string (*terminal)() = nullptr;
        mutable unsigned int width  = 80,
                             height = 0;
      } display;
    protected:
      bool gobeep = false;
#ifndef  _WIN32
      struct termios original = {};
#endif
    private:
      mutable Clock warned = 0;
      mutable vector<string> clogs;
      mutable unsigned int frame = 0;
      const vector<string> spinner = { "∙∙∙", "∙∙●", "∙●∙", "●∙∙" };
    public:
      void resize() const {
        sigscr = 0;
#ifdef _WIN32
        CONSOLE_SCREEN_BUFFER_INFO ws;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ws);
        display.width  = ws.srWindow.Right  - ws.srWindow.Left + 1;
        display.height = ws.srWindow.Bottom - ws.srWindow.Top  + 1;
#else
        struct winsize ws;
        ioctl(1, TIOCGWINSZ, &ws);
        display.width  = ws.ws_col;
        display.height = ws.ws_row;
#endif
      };
      void repaint(const bool &spin = false) const {
        if (display.height) {
          if (spin) frame = (frame + 1) % 4;
          if (sigscr) resize();
          clog << ANSI_HIDE_CURSOR
               << ANSI_TOP_RIGHT
               << display.terminal()
               << ANSI_RESET
               << ANSI_SHOW_CURSOR;
        }
      };
      void log(const string &prefix, const string &reason, const string &highlight = "") const {
        const string space = highlight.empty()
          ? ""
          : string(reason.back() != '=', ' ');
        puke_rainbow(stamp() +
          prefix + ' '       + ANSI_PUKE_WHITE  +
          reason + space     + ANSI_HIGH_YELLOW +
          highlight
        );
      };
      void warn(const string &prefix, const string &reason, const Clock &ratelimit = 0) const {
        if (ratelimit) {
          const Clock now = Tstamp;
          if (warned + ratelimit > now) return;
          warned = now;
        }
        puke_rainbow(stamp() +
          prefix             + ANSI_PUKE_RED +
          " Warrrrning: "    + ANSI_HIGH_RED +
          reason
        );
      };
      void beep() const {
        if (gobeep)
          clog << ANSI_BELL;
      };
      string spin() {
        return ANSI_HIGH_YELLOW
             + spinner.at(frame)
             + ANSI_PUKE_WHITE;
      };
      string logs(const unsigned int &rows, const string &prefix) {
        string lines;
        const unsigned int empty_rows = fmax(0, display.height - rows - 1);
        if (clogs.size() > empty_rows)
          clogs.erase(clogs.begin(), clogs.end() - empty_rows);
        else while (clogs.size() < empty_rows)
          clogs.insert(clogs.begin(), " ");
        for (auto &it : clogs) {
          if (it.length() < 6 or it.substr(it.length() - 6) != ANSI_END_LINE)
            it.append(ANSI_END_LINE)
              .insert(0, ANSI_PUKE_WHITE + prefix);
          lines += it;
        }
        return lines + "│ " + ANSI_END_LINE;
      };
    protected:
      bool windowed() {
        if (display.terminal) {
#ifndef _WIN32
          struct termios t;
          tcgetattr(1, &t);
          original = t;
          cfmakeraw(&t);
          tcsetattr(1, TCSANOW, &t);
#endif
          clog << ANSI_ALTERNATIVE
                  ANSI_CURSOR;
          resize();
          return true;
        }
        return false;
      };
      void with_goodbye() {
        if (display.height) {
          display = {};
#ifndef _WIN32
          tcsetattr(1, TCSANOW, &original);
#endif
          clog << ANSI_ORIGINAL;
        }
        clog << stamp()
             << epilogue
             << (epilogue.empty() ? "" : ANSI_NEW_LINE);
      };
    private:
      void puke_rainbow(const string &rain) const {
        string puke = rain + ANSI_PUKE_WHITE
                    + '.'  + ANSI_RESET
                           + ANSI_NEW_LINE;
#ifdef NDEBUG
        if (!display.height)
          clog << puke;
#endif
        if (display.terminal) {
          string::size_type n = 0;
          while ((n = puke.find(ANSI_NEW_LINE)) != string::npos) {
            clogs.emplace_back(puke.begin(), puke.begin() + n);
            puke.erase(0, n + 2);
          }
          repaint();
        }
      };
      string stamp() const {
        chrono::system_clock::time_point clock = chrono::system_clock::now();
        chrono::system_clock::duration t = clock.time_since_epoch();
        t -= chrono::duration_cast<chrono::seconds>(t);
        auto milliseconds = chrono::duration_cast<chrono::milliseconds>(t);
        t -= milliseconds;
        auto microseconds = chrono::duration_cast<chrono::microseconds>(t);
        stringstream microtime;
        microtime << setfill('0') << '.'
          << setw(3) << milliseconds.count()
          << setw(3) << microseconds.count();
        time_t tt = chrono::system_clock::to_time_t(clock);
        char datetime[15];
        strftime(datetime, 15, "%m/%d %H:%M:%S", localtime(&tt));
        return                  ANSI_HIGH_GREEN +
              datetime        + ANSI_PUKE_GREEN +
              microtime.str() + ANSI_HIGH_WHITE +
              ' ';
      };
  };

  class Option: public Terminal {
    private_friend:
      struct Argument {
       const string  name;
       const string  defined_value;
       const char   *default_value;
       const string  help;
      };
    protected:
      using MutableUserArguments = unordered_map<string, variant<string, int, double>>;
      pair<vector<Argument>, function<void(MutableUserArguments&)>> arguments;
    private:
      MutableUserArguments args;
    public:
      template <typename T> const T arg(const string &name) const {
#ifndef NDEBUG
        if (!args.contains(name)) return T();
#endif
        return get<T>(args.at(name));
      };
    protected:
      void optional_setup(int argc, char** argv,
        const vector<variant<
          Loop::TimeEvent,
          Ending::QuitEvent,
          Gw::DataEvent
        >> &events,
        const bool &blackhole,
        const bool &headless
      ) {
        args["autobot"]  =
        args["headless"] = headless;
        args["naked"]    = !display.terminal;
        bool order_ev = false;
        for (const auto &it : events)
          if (holds_alternative<Gw::DataEvent>(it)
            and holds_alternative<function<void(const Order&)>>(get<Gw::DataEvent>(it))
          ) order_ev = true;
        vector<Argument> long_options = {
          {"INFORMATION",  "",       nullptr,  ""},
          {"help",         "h",      nullptr,  "print this help and quit"},
          {"version",      "v",      nullptr,  "print current build version and quit"},
          {"latency",      "1",      nullptr,  "print current HTTP latency (not from WS) and quit"},
          {"list",         "1",      nullptr,  "print current available currency pairs and quit"},
          {"CREDENTIALS",  "",       nullptr,  ""},
          {"exchange",     "NAME",   "",       "set exchange NAME for trading, mandatory"},
          {"currency",     "PAIR",   "",       "set currency PAIR for trading, use format ISO 4217-A3"
                                               ANSI_NEW_LINE "with '/' separator, like 'BTC/EUR', mandatory"},
          {"apikey",       "WORD",   "",       "set (never share!) WORD as api key for trading, mandatory"},
          {"secret",       "WORD",   "",       "set (never share!) WORD as api secret for trading, mandatory"},
          {"passphrase",   "WORD",   "",       "set (never share!) WORD as api passphrase for trading"},
          {"ENDPOINTS",    "",       nullptr,  ""},
          {"http",         "URL",    "",       "set URL of alernative HTTPS api endpoint for trading"},
          {"wss",          "URL",    "",       "set URL of alernative WSS api endpoint for trading"},
          {"fix",          "URL",    "",       "set URL of alernative FIX api endpoint for trading"},
          {"NETWORK",      "",       nullptr,  ""},
          {"nocache",      "1",      nullptr,  "do not cache handshakes 7 hours at " K_HOME "/cache"},
          {"interface",    "IP",     "",       "set IP to bind as outgoing network interface"},
          {"ipv6",         "1",      nullptr,  "use IPv6 when possible"}
        };
        if (!arg<int>("headless")) for (const Argument &it : (vector<Argument>){
          {"ADMIN",        "",       nullptr,  ""},
          {"headless",     "1",      nullptr,  "do not listen for UI connections,"
                                               ANSI_NEW_LINE "all other UI related arguments will be ignored"},
          {"without-ssl",  "1",      nullptr,  "do not use HTTPS for UI connections (use HTTP only)"},
          {"whitelist",    "IP",     "",       "set IP or csv of IPs to allow UI connections,"
                                               ANSI_NEW_LINE "alien IPs will get a zip-bomb instead"},
          {"client-limit", "NUMBER", "7",      "set NUMBER of maximum concurrent UI connections"},
          {"port",         "NUMBER", "3000",   "set NUMBER of an open port to listen for UI connections"
                                               ANSI_NEW_LINE "default NUMBER is '3000'"},
          {"user",         "WORD",   "",       "set allowed WORD as username for UI basic authentication"},
          {"pass",         "WORD",   "",       "set allowed WORD as password for UI basic authentication"},
          {"ssl-crt",      "FILE",   "",       "set FILE to custom SSL .crt file for HTTPS UI connections"
                                               ANSI_NEW_LINE "(see www.akadia.com/services/ssh_test_certificate.html)"},
          {"ssl-key",      "FILE",   "",       "set FILE to custom SSL .key file for HTTPS UI connections"
                                               ANSI_NEW_LINE "(the passphrase MUST be removed from the .key file!)"},
          {"matryoshka",   "URL",    "https://example.com/", "set Matryoshka link URL of the next UI"},
          {"ignore-sun",   "2",      nullptr,  "do not switch UI to light theme on daylight"},
          {"ignore-moon",  "1",      nullptr,  "do not switch UI to dark theme on moonlight"}
        }) long_options.push_back(it);
        for (const Argument &it : (vector<Argument>){
          {"title",        "NAME",   K_SOURCE, "set NAME to allow admins to identify different bots"},
          {"BOT",          "",       nullptr,  ""}
        }) long_options.push_back(it);
        if (!arg<int>("autobot")) long_options.push_back(
          {"autobot",      "1",      nullptr,  "automatically start trading on boot"}
        );
        for (const Argument &it : arguments.first)
          long_options.push_back(it);
        arguments.first.clear();
        for (const Argument &it : (vector<Argument>){
          {"heartbeat",    "1",      nullptr,  "print detailed output about most important data"},
          {"debug-orders", "1",      nullptr,  "print detailed output about order states"},
          {"debug-quotes", "1",      nullptr,  "print detailed output about quoting engine"},
          {"debug-secret", "1",      nullptr,  "print (never share!) secret exchange messages"},
          {"debug",        "1",      nullptr,  "print detailed output about all the (previous) things!"},
          {"colors",       "1",      nullptr,  "print highlighted output"},
        }) long_options.push_back(it);
        if (!arg<int>("naked")) long_options.push_back(
          {"naked",        "1",      nullptr,  "do not display CLI, print output to stdout instead"}
        );
        if (order_ev) long_options.push_back(
          {"beep",         "1",      nullptr,  "make computer go beep on filled orders"}
        );
        if (!blackhole) long_options.push_back(
          {"database",     "FILE",   "",       "set alternative PATH to database filename,"
                                               ANSI_NEW_LINE "default PATH is '" K_HOME "/db/K-*.db',"
                                               ANSI_NEW_LINE "or use ':memory:' (see sqlite.org/inmemorydb.html)"}
        );
        long_options.push_back(
          {"free-version", "1",      nullptr,  "slowdown market levels 121 seconds"}
        );
        vector<Argument> io_options(find_if(
          long_options.begin(), long_options.end(),
          [&](const Argument &it) {
            return "heartbeat" == it.name.substr(0, 9);
          }
        ), long_options.end());
        long_options = vector<Argument>(
          long_options.begin(),
          long_options.end() - io_options.size()
        );
        long_options.push_back(
          {"market-limit", "NUMBER", "321",    "set NUMBER of maximum market levels saved in memory,"
                                               ANSI_NEW_LINE "default NUMBER is '321' and the minimum is '10'"}
        );
        if (order_ev) long_options.push_back(
          {"lifetime",     "NUMBER", "0",      "set NUMBER of minimum milliseconds to keep orders open,"
                                               ANSI_NEW_LINE "otherwise open orders can be replaced anytime required"}
        );
        long_options.push_back(
          {"I/O",          "",       nullptr,  ""}
        );
        for (const Argument &it : io_options)
          long_options.push_back(it);
        int index = ANY_NUM;
        vector<option> opt_long = { {nullptr, 0, nullptr, 0} };
        for (const Argument &it : long_options) {
          if (it.help.empty()) continue;
          else if (!it.default_value)             args[it.name] = 0;
          else if ( it.defined_value == "NUMBER") args[it.name] = stoi(it.default_value);
          else if ( it.defined_value == "AMOUNT") args[it.name] = stod(it.default_value);
          else                                    args[it.name] =      it.default_value;
          opt_long.insert(opt_long.end()-1, {
            it.name.data(),
            it.default_value
              ? required_argument
              : no_argument,
            it.default_value or it.defined_value.at(0) > '>'
              ? nullptr
              : get_if<int>(&args.at(it.name)),
            it.default_value
              ? index++
              : (it.defined_value.at(0) > '>'
                ? (int)it.defined_value.at(0)
                : stoi(it.defined_value)
              )
          });
        }
        int k = 0;
        while (++k)
          switch (k = getopt_long(argc, argv, "hv", (option*)&opt_long[0], &index)) {
            case -1 :
            case  0 : break;
            case 'h': help(long_options, order_ev, headless); [[fallthrough]];
            case '?':
            case 'v': EXIT(EXIT_SUCCESS);                     [[fallthrough]];
            default : {
              const string name(opt_long.at(index).name);
              if      (holds_alternative<int>(args[name]))    args[name] =   stoi(optarg);
              else if (holds_alternative<double>(args[name])) args[name] =   stod(optarg);
              else if (holds_alternative<string>(args[name])) args[name] = string(optarg);
            }
          }
        if (optind < argc) {
          string argerr = "Unhandled argument option(s):";
          while(optind < argc) argerr += string(" ") + argv[optind++];
          error("CF", argerr);
        }
        tidy();
        if (order_ev)
          gobeep = arg<int>("beep");
        colorful = arg<int>("colors");
        if (arguments.second) {
          arguments.second(args);
          arguments.second = nullptr;
        }
        if (arg<int>("naked"))
          display = {};

      };
    private:
      void tidy() {
        if (arg<string>("currency").find("/") == string::npos or arg<string>("currency").length() < 3)
          error("CF", "Invalid --currency value; must be in the format of BASE/QUOTE, like BTC/EUR");
        if (arg<string>("exchange").empty())
          error("CF", "Invalid --exchange value; the config file may have errors (there are extra spaces or double defined variables?)");
        args["exchange"] = Text::strU(arg<string>("exchange"));
        args["currency"] = Text::strU(arg<string>("currency"));
        args["base"]  = arg<string>("currency").substr(0, arg<string>("currency").find("/"));
        args["quote"] = arg<string>("currency").substr(1+ arg<string>("currency").find("/"));
        if (!args.contains("leverage"))  args["leverage"]  = 1.0;
        if (!args.contains("min-size"))  args["min-size"]  = 0.0;
        if (!args.contains("maker-fee")) args["maker-fee"] = 0.0;
        if (!args.contains("taker-fee")) args["taker-fee"] = 0.0;
        args["market-limit"] = max(10, arg<int>("market-limit"));
        args["leverage"] = fmax(0, fmin(100, arg<double>("leverage")));
        if (arg<int>("debug"))
          args["heartbeat"] =
          args["debug-orders"] =
          args["debug-quotes"] =
          args["debug-secret"] = 1;
#ifdef NDEBUG
        if (arg<int>("latency")
          or arg<int>("list")
          or arg<int>("debug-orders")
          or arg<int>("debug-quotes")
          or arg<int>("debug-secret")
        )
#endif
          args["naked"] = 1;
        if (args.contains("database")) {
          args["diskdata"] = "";
          if (arg<string>("database").empty() or arg<string>("database") == ":memory:")
            (arg<string>("database") == ":memory:"
              ? args["diskdata"]
              : args["database"]
            ) = (K_HOME "/db/" K_SOURCE)
              + ('.' + arg<string>("exchange"))
              +  '.' + arg<string>("base")
              +  '.' + arg<string>("quote")
              +  '.' + "db";
        }
        if (!arg<int>("headless")) {
          if (arg<int>("ignore-moon") and arg<int>("ignore-sun"))
            error("CF", "Invalid use of --ignore-moon and --ignore-sun together");
          if (arg<int>("latency") or !arg<int>("port") or !arg<int>("client-limit"))
            args["headless"] = 1;
          args["B64auth"] = (!arg<int>("headless")
            and !arg<string>("user").empty() and !arg<string>("pass").empty()
          ) ? Text::B64(arg<string>("user") + ':' + arg<string>("pass"))
            : "";
        }
        if (!arg<string>("interface").empty() and !arg<int>("ipv6"))
          args_easy_setopt = [inet = arg<string>("interface")](CURL *curl) {
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "K");
            curl_easy_setopt(curl, CURLOPT_INTERFACE, inet.data());
            curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
          };
        else if (!arg<string>("interface").empty())
          args_easy_setopt = [inet = arg<string>("interface")](CURL *curl) {
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "K");
            curl_easy_setopt(curl, CURLOPT_INTERFACE, inet.data());
          };
        else if (!arg<int>("ipv6"))
          args_easy_setopt = [](CURL *curl) {
            curl_easy_setopt(curl, CURLOPT_USERAGENT, "K");
            curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
          };
      };
      void help(const vector<Argument> &long_options, const bool &order_ev, const bool &headless) {
        const vector<string> stamp = {
          " \\__/  \\__/ ", " | (   .    ", "  __   \\__/ ",
          " /  \\__/  \\ ", " |  `.  `.  ", " /  \\       ",
          " \\__/  \\__/ ", " |    )   ) ", " \\__/   __  ",
          " /  \\__/  \\ ", " |  ,'  ,'  ", "       /  \\ "
        };
              unsigned int y = Tstamp;
        const unsigned int x = !(y % 2)
                             + !(y % 21);
        clog
          << ANSI_PUKE_GREEN << ANSI_NEW_LINE PERMISSIVE_analpaper_SOFTWARE_LICENSE ANSI_NEW_LINE ANSI_NEW_LINE
          << ANSI_PUKE_GREEN << "   installer: " << ANSI_PUKE_YELLOW << "https://krypto.ninja/Makefile" ANSI_NEW_LINE
          << ANSI_HIGH_GREEN << " K" << ANSI_PUKE_GREEN << " questions: " << ANSI_PUKE_YELLOW << "https://github.com/ctubio/Krypto-trading-bot/discussions" ANSI_NEW_LINE
          << ANSI_PUKE_GREEN  << "   bugkiller: " << ANSI_PUKE_YELLOW << "https://github.com/ctubio/Krypto-trading-bot/issues" ANSI_NEW_LINE
          << ANSI_HIGH_WHITE << stamp.at(((++y%4)*3)+x) << "Usage:" << ANSI_HIGH_YELLOW << " " << K_SOURCE " [arguments]";
        clog
          << ANSI_NEW_LINE << ANSI_HIGH_WHITE << stamp.at(((++y%4)*3)+x) << "[arguments]:";
        for (const Argument &it : long_options) {
          if ( ( headless and  it.name == "title")
            or (!order_ev and (it.name == "debug-orders"
                            or it.name == "debug-quotes"))
          ) continue;
          string usage = it.help;
          if (usage.empty())
            clog
              << ANSI_NEW_LINE << ANSI_HIGH_WHITE << stamp.at(((++y%4)*3)+x) << string(16, ' ')
              << "░▒▓█ " << string((11 - it.name.length()) / 2, ' ')
              << it.name
              << string((11 - it.name.length()) / 2, ' ') << " █▓▒░";
          else {
            string::size_type n = 0;
            while ((n = usage.find(ANSI_NEW_LINE, n + 2)) != string::npos)
              usage.insert(n + 2, 28, ' ');
            const string example = "--" + it.name + (it.default_value ? "=" + it.defined_value : "");
            usage = ANSI_NEW_LINE + (
              (!it.default_value and it.defined_value.at(0) > '>')
                ? "-" + it.defined_value + ", "
                : "    "
            ) + example + string(22 - example.length(), ' ')
              + "- " + usage;
            n = 0;
            do usage.insert(n + 2, ANSI_HIGH_WHITE + stamp.at(((++y%4)*3)+x) + ANSI_PUKE_WHITE);
            while ((n = usage.find(ANSI_NEW_LINE, n + 2)) != string::npos);
            clog << usage << '.';
          }
        }
        clog << ANSI_NEW_LINE
          << ANSI_PUKE_GREEN << "   more help: " << ANSI_PUKE_YELLOW << "https://github.com/ctubio/Krypto-trading-bot/blob/master/doc/MANUAL.md" ANSI_NEW_LINE
          << ANSI_HIGH_GREEN << " K" << ANSI_PUKE_GREEN << " questions: " << ANSI_PUKE_YELLOW << "irc://irc.libera.chat:6697/#krypto.ninja" ANSI_NEW_LINE
          << ANSI_PUKE_GREEN << "   home page: " << ANSI_PUKE_YELLOW << "https://ca.rles-tub.io./trades" ANSI_NEW_LINE
          << ANSI_RESET;
      };
  };

  class Hotkey {
    public_friend:
      class Keymap {
        public:
          Keymap(const Hotkey &hotkey, const vector<pair<const char, const function<void()>>> &hotkeys)
          {
            for (const auto &it : hotkeys)
              hotkey.keymap(it.first, it.second);
          };
      };
    private:
      Loop::Async::Event<char> keylogger;
      mutable unordered_map<char, function<void()>> maps = {
        { (char)SIGQUIT, []() { raise(SIGQUIT); } }
      };
    protected:
      void wait_for_keylog(Loop *const loop) {
        if (keylogger.waiting())
          error("SH", string("Unable to launch another \"keylogger\" thread"));
        keylogger.callback([&](const char &ch) { keylog(ch); });
        keylogger.wait_for(loop, [&]() { return sync_keylogger(); });
        keylogger.ask_for();
      };
    private:
      void keymap(const char &ch, function<void()> fn) const {
        if (maps.contains(ch))
          error("SH", string("Too many handlers for \"") + ch + "\" hotkey event");
        maps[ch] = fn;
      };
      void keylog(const char &ch) {
        if (maps.contains(ch))
          maps.at(ch)();
        keylogger.ask_for();
      };
      vector<char> sync_keylogger() {
        int ch =
#ifdef  _WIN32
        getch
#else
        getchar
#endif
        ();
        return {
          (char)ch
        };
      };
  };

  class About {
    public:
      enum class mMatter: char {
        FairValue            = 'a',                                                       Connectivity       = 'd',
        MarketData           = 'e', QuotingParameters    = 'f',
        OrderStatusReports   = 'i', ProductAdvertisement = 'j', ApplicationState   = 'k', EWMAStats          = 'l',
        STDEVStats           = 'm', Position             = 'n', Profit             = 'o', SubmitNewOrder     = 'p',
        CancelOrder          = 'q', MarketTrade          = 'r', Trades             = 's',
        QuoteStatus          = 'u', TargetBasePosition   = 'v', TradeSafetyValue   = 'w', CancelAllOrders    = 'x',
        CleanAllClosedTrades = 'y', CleanAllTrades       = 'z', CleanTrade         = 'A',
                                    MarketChart          = 'D', Notepad            = 'E',
                                    MarketDataLongTerm   = 'H'
      };
    public:
      virtual mMatter about() const = 0;
      bool persist() const {
        return about() == mMatter::QuotingParameters;
      };
  };

  class Blob: virtual public About {
    public:
      virtual json blob() const = 0;
  };

  class Sqlite {
    public_friend:
      class Backup: public Blob {
        public:
          using Report = pair<bool, string>;
          function<void()> push;
        public:
          Backup(const Sqlite &sqlite)
          {
            sqlite.tables.push_back(this);
          };
          void backup() const {
            if (push) push();
          };
          virtual Report pull(const json &j) = 0;
          virtual string increment() const { return "NULL"; };
          virtual double limit()     const { return 0; };
          virtual Clock  lifetime()  const { return 0; };
        protected:
          Report report(const bool &empty) const {
            string msg = empty
              ? explainKO()
              : explainOK();
            const size_t token = msg.find("%");
            if (token != string::npos)
              msg.replace(token, 1, explain());
            return {empty, msg};
          };
        private:
          virtual string explain()   const = 0;
          virtual string explainOK() const = 0;
          virtual string explainKO() const { return ""; };
      };
      template <typename T> class StructBackup: public Backup {
        public:
          StructBackup(const Sqlite &sqlite)
            : Backup(sqlite)
          {};
          json blob() const override {
            return *(T*)this;
          };
          Report pull(const json &j) override {
            from_json(j.empty() ? blob() : not_null(j.at(0)), *(T*)this);
            return report(j.empty());
          };
        private:
          string explainOK() const override {
            return "loaded last % OK";
          };
          static json not_null(json j) {
            for (auto it = j.begin(); it != j.end();)
              if (it.value().is_null()) it = j.erase(it); else ++it;
            return j;
          };
      };
      template <typename T> class VectorBackup: public Backup {
        public:
          VectorBackup(const Sqlite &sqlite)
            : Backup(sqlite)
          {};
          vector<T> rows;
          using reference              = typename vector<T>::reference;
          using const_reference        = typename vector<T>::const_reference;
          using iterator               = typename vector<T>::iterator;
          using const_iterator         = typename vector<T>::const_iterator;
          using reverse_iterator       = typename vector<T>::reverse_iterator;
          using const_reverse_iterator = typename vector<T>::const_reverse_iterator;
          iterator                 begin()       noexcept { return rows.begin();   };
          const_iterator          cbegin() const noexcept { return rows.cbegin();  };
          iterator                   end()       noexcept { return rows.end();     };
          const_iterator            cend() const noexcept { return rows.cend();    };
          reverse_iterator        rbegin()       noexcept { return rows.rbegin();  };
          const_reverse_iterator crbegin() const noexcept { return rows.crbegin(); };
          bool                     empty() const noexcept { return rows.empty();   };
          size_t                    size() const noexcept { return rows.size();    };
          reference                   at(size_t n)        { return rows.at(n);     };
          const_reference             at(size_t n) const  { return rows.at(n);     };
          virtual void erase() {
            if (size() > limit())
              rows.erase(begin(), end() - limit());
          };
          virtual void push_back(const T &row) {
            rows.push_back(row);
            backup();
            erase();
          };
          Report pull(const json &j) override {
            for (const json &it : j)
              rows.push_back(it);
            return report(empty());
          };
          json blob() const override {
            return rows.back();
          };
        private:
          string explain() const override {
            return to_string(size());
          };
      };
    public:
      function<unsigned int()> dbSize = [](){ return 0; };
    private:
      sqlite3 *db = nullptr;
      string disk = "main";
      mutable vector<Backup*> tables;
    protected:
      void backups(const Option *const K) {
        if (blackhole()) return;
        if (K->arg<string>("database") != ":memory:")
          dbSize = [K](){
            struct stat st;
            return stat(K->arg<string>("database").data(), &st) ? 0 : st.st_size;
          };
        if (sqlite3_open(K->arg<string>("database").data(), &db))
          error("DB", sqlite3_errmsg(db));
        K->log("DB", "loaded OK from", K->arg<string>("database"));
        if (!K->arg<string>("diskdata").empty()) {
          exec("ATTACH '" + K->arg<string>("diskdata") + "' AS " + (disk = "disk") + ";");
            K->log("DB", "loaded OK from", K->arg<string>("diskdata"));
        }
        exec("PRAGMA " + disk + ".journal_mode = WAL;"
             "PRAGMA " + disk + ".synchronous = NORMAL;");
        for (auto &it : tables) {
          const Backup::Report note = it->pull(select(it));
          if (!note.second.empty()) {
            if (note.first)
              K->warn("DB", note.second);
            else K->log("DB", note.second);
          }
          it->push = [this, it]() {
            insert(it);
          };
        }
        tables.clear();
      };
      bool blackhole() const {
        return tables.empty() and !db;
      };
    private:
      json select(const Backup *const data) {
        const string table = schema(data);
        json result = json::array();
        exec(
          create(table)
          + truncate(table, data->lifetime())
          + "SELECT json FROM " + table + " ORDER BY time ASC;",
          &result
        );
        return result;
      };
      void insert(const Backup *const data) {
        const string table    = schema(data);
        const json   blob     = data->blob();
        const double limit    = data->limit();
        const Clock  lifetime = data->lifetime();
        const string incr     = data->increment();
        const string sql      = (
          (incr != "NULL" or !limit or lifetime)
            ? "DELETE FROM " + table + (
              incr != "NULL"
                ? " WHERE id = " + incr
                : (limit ? " WHERE time < " + to_string(Tstamp - lifetime) : "")
            ) + ";" : ""
        ) + (
          blob.is_null()
            ? ""
            : "INSERT INTO " + table
              + " (id,json) VALUES(" + incr + ",'" + blob.dump() + "');"
        );
        exec(sql);
      };
      string schema(const Backup *const data) const {
        return (
          data->persist()
            ? disk
            : "main"
        ) + "." + (char)data->about();
      };
      string create(const string &table) const {
        return "CREATE TABLE IF NOT EXISTS " + table + "("
          + "id    INTEGER   PRIMARY KEY AUTOINCREMENT                                           NOT NULL,"
          + "json  BLOB                                                                          NOT NULL,"
          + "time  TIMESTAMP DEFAULT (CAST((julianday('now') - 2440587.5)*86400000 AS INTEGER))  NOT NULL);";
      };
      string truncate(const string &table, const Clock &lifetime) const {
        return lifetime
          ? "DELETE FROM " + table + " WHERE time < " + to_string(Tstamp - lifetime) + ";"
          : "";
      };
      void exec(const string &sql, json *const result = nullptr) {
        char* zErrMsg = nullptr;
        sqlite3_exec(db, sql.data(), result ? write : nullptr, (void*)result, &zErrMsg);
        if (zErrMsg) error("DB", "SQLite error: " + (zErrMsg + (" at " + sql)));
        sqlite3_free(zErrMsg);
      };
      static int write(void *result, int argc, char **argv, char**) {
        for (int i = 0; i < argc; ++i)
          ((json*)result)->push_back(json::parse(argv[i]));
        return 0;
      };
  };

  class Client {
    public_friend:
      class Readable: public Blob {
        public:
          function<void()> read;
        public:
          Readable(const Client &client)
          {
            client.readable.push_back(this);
          };
          virtual json hello() {
            return { blob() };
          };
          virtual bool realtime() const {
            return true;
          };
      };
      template <typename T> class Broadcast: public Readable {
        public:
          Broadcast(const Client &client)
            : Readable(client)
          {};
          bool broadcast() {
            if ((read_asap() or read_soon())
              and (read_same_blob() or diff_blob())
            ) {
              if (read) read();
              return true;
            }
            return false;
          };
          json blob() const override {
            return *(T*)this;
          };
        protected:
          Clock last_Tstamp = 0;
          string last_blob;
          virtual bool read_same_blob() const {
            return true;
          };
          bool diff_blob() {
            const string last = last_blob;
            return (last_blob = blob().dump()) != last;
          };
          virtual bool read_asap() const {
            return true;
          };
          bool read_soon(const int &delay = 0) {
            const Clock now = Tstamp;
            if (last_Tstamp + max(369, delay) > now)
              return false;
            last_Tstamp = now;
            return true;
          };
      };
      class Clickable: virtual public About {
        public:
          Clickable(const Client &client)
          {
            client.clickable.push_back(this);
          };
          virtual void click(const json&) = 0;
      };
      class Clicked {
        public:
          Clicked(const Client &client, const vector<pair<const Clickable*, variant<
            const function<void()>,
            const function<void(const json&)>
          >>> &clicked)
          {
            for (const auto &it : clicked)
              client.clicked(
                it.first,
                holds_alternative<const function<void()>>(it.second)
                  ? [it](const json&) { get<const function<void()>>(it.second)(); }
                  : get<const function<void(const json&)>>(it.second)
              );
          };
      };
    protected:
      string wtfismyip = "localhost";
      unordered_map<string, pair<const char*, const int>> documents;
    private:
      string protocol = "HTTP";
      WebServer::Backend server;
      const Option *option = nullptr;
      mutable unsigned int delay = 0;
      mutable vector<Readable*> readable;
      mutable vector<Clickable*> clickable;
      mutable unordered_map<const Clickable*, vector<function<void(const json&)>>> clickFn;
      const pair<char, char> portal = {'=', '-'};
      unordered_map<char, function<json()>> hello;
      unordered_map<char, function<void(const json&)>> kisses;
      unordered_map<char, string> queue;
    public:
      void listen(const Option *const K, const curl_socket_t &loopfd) {
        option = K;
        if (!server.listen(
          loopfd,
          K->arg<string>("interface"),
          K->arg<int>("port"),
          K->arg<int>("ipv6"),
          {
            K->arg<string>("B64auth"),
            response,
            upgrade,
            message
          }
        )) error("UI", "Unable to listen at port number " + to_string(K->arg<int>("port"))
             + " (may be already in use by another program)");
        if (!K->arg<int>("without-ssl"))
          for (const auto &it : server.ssl_context(
            K->arg<string>("ssl-crt"),
            K->arg<string>("ssl-key")
          )) K->warn("UI", it);
        protocol  = server.protocol();
        K->log("UI", "ready at", location());
      };
      string location() const {
        return option
          ? Text::strL(protocol) + "://" + wtfismyip + ":" + to_string(option->arg<int>("port"))
          : "loading..";
      };
      void clicked(const Clickable *data, const json &j = nullptr) const {
        if (clickFn.contains(data))
          for (const auto &it : clickFn.at(data)) it(j);
      };
      void client_queue_delay(const unsigned int &d) const {
        delay = d;
      };
      void broadcast(const unsigned int &tick) {
        if (delay and !(tick % delay))
          broadcast();
        server.timeouts();
      };
      void welcome() {
        for (auto &it : readable) {
          it->read = [this, it]() {
            if (server.idle()) return;
            queue[(char)it->about()] = it->blob().dump();
            if (it->realtime() or !delay) broadcast();
          };
          hello[(char)it->about()] = [it]() {
            return it->hello();
          };
        }
        readable.clear();
        for (auto &it : clickable)
          kisses[(char)it->about()] = [it](const json &butterfly) {
            it->click(butterfly);
          };
        clickable.clear();
      };
      void headless() {
        for (auto &it : readable)
          it->read = nullptr;
        readable.clear();
        clickable.clear();
        documents.clear();
      };
      void without_goodbye() {
        server.purge();
      };
    private:
      void clicked(const Clickable *data, const function<void(const json&)> &fn) const {
        clickFn[data].push_back(fn);
      };
      void broadcast() {
        if (queue.empty()) return;
        if (!server.idle())
          server.broadcast(portal.second, queue);
        queue.clear();
      };
      bool alien(const string &addr) {
        if (addr != "unknown"
          and !option->arg<string>("whitelist").empty()
          and option->arg<string>("whitelist").find(addr) == string::npos
        ) {
          option->log("UI", "dropping gzip bomb on", addr);
          return true;
        }
        return false;
      };
      WebServer::Response response = [&](string path, const string &auth, const string &addr) {
        if (alien(addr))
          path.clear();
        const bool papersplease = !(path.empty() or option->arg<string>("B64auth").empty());
        string content,
               type = "text/html; charset=UTF-8";
        unsigned int code = 200;
        const string leaf = path.substr(path.find_last_of('.') + 1);
        if (papersplease and auth.empty()) {
          option->log("UI", "authorization attempt from", addr);
          code = 401;
        } else if (papersplease and auth != option->arg<string>("B64auth")) {
          option->log("UI", "authorization failed from", addr);
          code = 403;
        } else if (leaf != "/" or server.clients() < option->arg<int>("client-limit")) {
          if (!documents.contains(path))
            path = path.substr(path.find_last_of("/", path.find_last_of("/") - 1));
          if (!documents.contains(path))
            path = path.substr(path.find_last_of("/"));
          if (documents.contains(path)) {
            content = string(documents.at(path).first,
                             documents.at(path).second);
            if (leaf == "/") option->log("UI", "authorization success from", addr);
            else if (leaf == "js")    type = "application/javascript; charset=UTF-8";
            else if (leaf == "css")   type = "text/css; charset=UTF-8";
            else if (leaf == "ico")   type = "image/x-icon";
            else if (leaf == "mp3")   type = "audio/mpeg";
            else if (leaf == "woff2") type = "font/woff2";
          } else {
            if (Random::int64() % 21)
              code = 404, content = "Today, is a beautiful day.";
            else // Humans! go to any random path to check your luck.
              code = 418, content = "Today, is your lucky day!";
          }
        } else {
          option->log("UI", "--client-limit=" + to_string(option->arg<int>("client-limit"))
            + " reached by", addr);
          content = "Thank you! but our princess is already in this castle!"
                    "<br/>" "Refresh the page anytime to retry.";
        }
        return server.document(content, code, type);
      };
      WebServer::Upgrade upgrade = [&](const int &sum, const string &addr) {
        const int tentative = server.clients() + sum;
        option->log("UI", to_string(tentative) + " client" + string(tentative != 1, 's')
          + (sum > 0 ? "" : " remain") + " connected, last connection was from", addr);
        if (tentative > option->arg<int>("client-limit")) {
          option->log("UI", "--client-limit=" + to_string(option->arg<int>("client-limit"))
            + " reached by", addr);
          return 0;
        }
        return sum;
      };
      WebServer::Message message = [&](string msg, const string &addr) {
        if (alien(addr))
          return string(documents.at("").first, documents.at("").second);
        const char matter = msg.at(1);
        if (portal.first == msg.at(0)) {
          if (hello.contains(matter)) {
            const json reply = hello.at(matter)();
            if (!reply.is_null())
              return portal.first + (matter + reply.dump());
          }
        } else if (portal.second == msg.at(0) and kisses.contains(matter)) {
          msg = msg.substr(2);
          json butterfly = json::accept(msg)
            ? json::parse(msg)
            : json::object();
          for (auto it = butterfly.begin(); it != butterfly.end();)
            if (it.value().is_null()) it = butterfly.erase(it); else ++it;
          kisses.at(matter)(butterfly);
        }
        return string();
      };
  };

  enum class QuoteState: unsigned int {
    Disconnected,  Live,             Crossed,
    MissingData,   UnknownHeld,      WidthTooHigh,
    DepletedFunds, DisabledQuotes,   WaitingFunds,
    UpTrendHeld,   DownTrendHeld,
    TBPHeld,       MaxTradesSeconds, WaitingPing,
    ScaleSided,    ScalationLimit,   DeviationLimit
  };

  class System {
    public_friend:
      class Quote: public Level {
        public:
          const Side       side   = (Side)0;
                QuoteState state  = QuoteState::MissingData;
                bool       isPong = false;
        public:
          Quote(const Side &s)
            : side(s)
          {};
          bool empty() const {
            return !size or !price;
          };
          void skip() {
            size = 0;
          };
          void skip(const QuoteState &reason) {
            price = size = 0;
            state = reason;
          };
          bool deprecates(const Price &otherPrice) const {
            return side == Side::Bid
                     ? price < otherPrice
                     : price > otherPrice;
          };
          bool checkCrossed(const Quote &otherSide) {
            if (empty()) return false;
            if (otherSide.empty() or deprecates(otherSide.price)) {
              state = QuoteState::Live;
              return false;
            }
            state = QuoteState::Crossed;
            return true;
          };
      };
      class Quotes {
        public:
          Quote bid,
                ask;
        private:
          QuoteState prevBidState = QuoteState::MissingData,
                     prevAskState = QuoteState::MissingData;
        private_ref:
          const Option &K;
        public:
          Quotes(const Option &bot)
            : bid(Side::Bid)
            , ask(Side::Ask)
            , K(bot)
          {};
          void calcQuotes() {
            reset();
            calcRawQuotes();
            unset();
            applyQuotingParameters();
            upset();
            log();
          };
          void pending() {
            states(QuoteState::WaitingFunds);
            log();
          };
          void paused() {
            states(QuoteState::DisabledQuotes);
            log();
          };
          void offline() {
            states(QuoteState::Disconnected);
            log();
          };
        protected:
          void debug(const string &step) const {
            if (K.arg<int>("debug-quotes"))
              K.log("DEBUG QE", "[" + step + "] "
                + to_string((int)ask.state)  + ":"
                + to_string((int)bid.state)  + " "
                + to_string((int)ask.isPong) + ":"
                + to_string((int)bid.isPong) + " "
                + ((json){
                    {"ask", ask},
                    {"bid", bid}
                  }).dump()
              );
          };
        private:
          virtual string explainState(const Quote&) const = 0;
          virtual void calcRawQuotes() = 0;
          virtual void applyQuotingParameters() = 0;
          void states(const QuoteState &state) {
            bid.state =
            ask.state = state;
          };
          void reset() {
            bid.isPong =
            ask.isPong = false;
            states(QuoteState::UnknownHeld);
          };
          void unset() {
            if (bid.price <= 0 or ask.price <= 0) {
              bid.skip(QuoteState::WidthTooHigh);
              ask.skip(QuoteState::WidthTooHigh);
              K.warn("QP", "Negative price detected, width must be lower", 3e+3);
            }
          };
          void upset() {
            if (bid.checkCrossed(ask) or ask.checkCrossed(bid))
              K.warn("QE", "Crossed bid/ask quotes detected, that is.. unexpected", 3e+3);
          };
          void log() {
            logState(bid, &prevBidState);
            logState(ask, &prevAskState);
          };
          void logState(const Quote &quote, QuoteState *const prevState) {
            if (quote.state != *prevState) {
              *prevState = quote.state;
              const string reason = explainState(quote);
              if (!reason.empty())
                K.log("QP", (quote.side == Side::Bid
                  ? ANSI_PUKE_CYAN    + "BID"
                  : ANSI_PUKE_MAGENTA + "ASK"
                ) + ANSI_PUKE_WHITE + " quoting", reason);
            }
          };
      };
      class Orderbook {
        public:
          class Zombies {
            public:
              unsigned int countZombies = 0,
                           countWaiting = 0,
                           countWorking = 0;
            private:
              vector<const Order*> zombies;
            private_ref:
              Orderbook *const &orders;
            public:
              Zombies(Orderbook *const &o)
                : orders(o)
              {};
              void purge() {
                countZombies =
                countWaiting =
                countWorking = 0;
                for (const Order *const it : zombies)
                  orders->purge(it);
                zombies.clear();
              };
              bool stillAlive(const Order &order) {
                if (order.status == Status::Waiting) {
                  if (Tstamp - 10e+3 > order.time) {
                    zombies.push_back(&order);
                    ++countZombies;
                    return false;
                  }
                  ++countWaiting;
                } else ++countWorking;
                return !order.manual;
              };
          } zombies;
          Order *last = nullptr;
        private:
          unordered_map<string, Order> orders;
        private_ref:
          const bool &debug;
          Gw *const &gateway;
        public:
          Orderbook(const System &system)
            : zombies(system.orders)
            , debug(system.debug)
            , gateway(system.gateway)
          {
            system.orders = this;
          };
          virtual bool purgeable(const Order &order) const {
            return order.status == Status::Terminated;
          };
          Order *find(const string &orderId) {
            return (orderId.empty()
              or !orders.contains(orderId)
            ) ? nullptr
              : &orders.at(orderId);
          };
          Order *update(const Order &raw, const string &reason = "  place") {
            Order *const order = Order::update(raw, findsert(raw));
            if (debug) {
              gateway->print(reason + "(" + to_string(size()) + "): " + ((json)raw).dump());
              gateway->print("  saved(" + to_string(size()) + "): " + (order ? ((json)*order).dump() : "not found (external)"));
            }
            return order;
          };
          void purge(const Order *const order) {
            if (debug) gateway->print("  purge(" + to_string(size() - 1) + "): " + order->orderId);
            orders.erase(order->orderId);
          };
          unsigned int size() const {
            return orders.size();
          };
          vector<Order*> at(const Side &side) {
            vector<Order*> sideOrders;
            for (auto &it : orders)
              if (side == it.second.side)
                sideOrders.push_back(&it.second);
            return sideOrders;
          };
          vector<Order*> open() {
            vector<Order*> autoOrders;
            for (auto &it : orders)
              if (!it.second.manual)
                autoOrders.push_back(&it.second);
            return autoOrders;
          };
          vector<Order> working(const bool &sorted) const {
            vector<Order> workingOrders;
            for (const auto &it : orders)
              if (Status::Working == it.second.status)
                workingOrders.push_back(it.second);
            if (sorted)
              sort(workingOrders.begin(), workingOrders.end(),
                [](const Order &a, const Order &b) {
                  return a.price > b.price;
                }
              );
            return workingOrders;
          };
          Amount held(const Side &side) const {
            Amount held = 0;
            for (const auto &it : orders)
              if (it.second.side == side)
                held += (
                  it.second.side == Side::Ask
                    ? it.second.quantity
                    : it.second.quantity * it.second.price
                );
            return held;
          };
          void resetFilters(
            unordered_map<Price, Amount> *const filterBidOrders,
            unordered_map<Price, Amount> *const filterAskOrders
          ) const {
            filterBidOrders->clear();
            filterAskOrders->clear();
            for (const auto &it : orders)
              (it.second.side == Side::Bid
                ? *filterBidOrders
                : *filterAskOrders
              )[it.second.price] += it.second.quantity;
          };
        private:
          Order *findsert(const Order &raw) {
            if (raw.status == Status::Waiting and !raw.orderId.empty())
              return &(orders[raw.orderId] = raw);
            if (raw.orderId.empty() and !raw.exchangeId.empty()) {
              auto it = find_if(
                orders.begin(), orders.end(),
                [&](const pair<string, Order> &it) {
                  return raw.exchangeId == it.second.exchangeId;
                }
              );
              if (it != orders.end())
                return &it->second;
            }
            return find(raw.orderId);
          };
      };
    public:
      Gw *gateway = nullptr;
    protected:
      vector<variant<
          Loop::TimeEvent,
        Ending::QuitEvent,
            Gw::DataEvent
      >> events;
    private:
      mutable Orderbook *orders = nullptr;
      bool debug = false;
      Order external;
    public:
      void place(const Order &raw) const {
        gateway->place(orders->update(raw));
      };
      void replace(const Price &price, const bool &isPong, Order *const order) const {
        if (Order::replace(price, isPong, order)) {
          gateway->replace(order);
          if (debug) gateway->print("replace(" + to_string(orders->size()) + "): " + ((json)*order).dump());
        }
      };
      void cancel() const {
        for (Order *const it : orders->open())
          cancel(it);
      };
      void cancel(const string &orderId) const {
        cancel(orders->find(orderId));
      };
      void cancel(Order *const order) const {
        if (Order::cancel(order)) {
          gateway->cancel(order);
          if (debug) gateway->print(" cancel(" + to_string(orders->size()) + "): " + order->orderId);
        }
      };
      unsigned int memSize() const {
#ifdef _WIN32
        return 0;
#else
        struct rusage ru;
        return getrusage(RUSAGE_SELF, &ru) ? 0 : ru.ru_maxrss * 1e+3;
#endif
      };
    protected:
      void required_setup(const Option *const K, mutex &lock, const curl_socket_t &loopfd) {
        if (!(gateway = Gw::new_Gw(K->arg<string>("exchange"))))
          error("CF",
            "Unable to configure a valid gateway using --exchange="
              + K->arg<string>("exchange") + " argument"
          );
        epitaph = "- exchange: " + (gateway->exchange = K->arg<string>("exchange")) + ANSI_NEW_LINE
                + "- currency: " + (gateway->base     = K->arg<string>("base"))     + "/"
                                 + (gateway->quote    = K->arg<string>("quote"))    + ANSI_NEW_LINE;
        if (!gateway->http.empty() and !K->arg<string>("http").empty())
          gateway->http    = K->arg<string>("http");
        if (!gateway->ws.empty() and !K->arg<string>("wss").empty())
          gateway->ws      = K->arg<string>("wss");
        if (!gateway->fix.empty() and !K->arg<string>("fix").empty())
          gateway->fix     = K->arg<string>("fix");
        if (K->arg<double>("taker-fee"))
          gateway->takeFee = K->arg<double>("taker-fee") / 1e+2;
        if (K->arg<double>("maker-fee"))
          gateway->makeFee = K->arg<double>("maker-fee") / 1e+2;
        if (K->arg<double>("min-size"))
          gateway->minSize = K->arg<double>("min-size");
        gateway->leverage  = K->arg<double>("leverage");
        gateway->apikey    = K->arg<string>("apikey");
        gateway->secret    = K->arg<string>("secret");
        gateway->pass      = K->arg<string>("passphrase");
        gateway->maxLevel  = K->arg<int>("market-limit");
        gateway->debug     = K->arg<int>("debug-secret");
        gateway->guard     = &lock;
        gateway->loopfd    = loopfd;
        gateway->printer   = [K](const string &prefix, const string &reason, const string &highlight) {
          if (reason.find("Error") != string::npos)
            K->warn(prefix, reason);
          else K->log(prefix, reason, highlight);
        };
        gateway->adminAgreement = (Connectivity)K->arg<int>("autobot");
        wrap_events(K);
        if (CURLE_OK != curl_global_init(CURL_GLOBAL_ALL))
          error("CF", string("CURL ") + curl_easy_strerror(CURLE_FAILED_INIT));
      };
      void handshake(const GwExchange::Report &notes, const bool &nocache) {
        const json reply = gateway->handshake(nocache);
        if (!gateway->tickPrice or !gateway->tickSize or !gateway->minSize)
          error("GW", "Unable to fetch data from " + gateway->exchange
            + " for symbols " + gateway->base + "/" + gateway->quote
            + ", response was: " + reply.dump());
        gateway->report(notes, nocache);
      };
    private:
      Order *alien(const Order &raw) {
        external = raw;
        external.orderId.clear();
        return &external;
      };
      void wrap_events(const Option *const K) {
        debug = K->arg<int>("debug-orders");
        if (!orders) orders = new Orderbook(*this);
        for (auto &it : events)
          if (holds_alternative<Gw::DataEvent>(it)
            and holds_alternative<function<void(const Order&)>>(get<Gw::DataEvent>(it))
          ) it = [&, fn = get<function<void(const Order&)>>(get<Gw::DataEvent>(it)),
       make_computer_go = K
            ](const Order &raw) {
              orders->last = orders->update(raw, "  reply");
              fn(*(orders->last ?: alien(raw)));
              if (orders->last) {
                if (orders->purgeable(*orders->last))
                  orders->purge(orders->last);
                else orders->last->justFilled = 0;
              }
              if (raw.justFilled) {
                gateway->askForBalance = true;
                make_computer_go->beep();
              }
            };
      };
  };

  class KryptoNinja: public Events,
                     public Ending,
                     public Option,
                     public Hotkey,
                     public Sqlite,
                     public Client,
                     public System {
    public:
      KryptoNinja *main(int argc, char** argv) {
        {
          ending([&]() { with_goodbye(); });
          optional_setup(argc, argv, events, blackhole(), documents.empty());
          required_setup(this, lock, poll());
        } {
          if (windowed())
            wait_for_keylog(this);
        } {
          log("CF", "Outbound IP address is",
            wtfismyip = Curl::Web::xfer(lock, "https://wtfismyip.com/json")
                          .value("YourFuckingIPAddress", wtfismyip)
          );
        } {
          if (arg<int>("list"))
            exit("CF " + ANSI_PUKE_WHITE + gateway->pairs());
          if (arg<int>("latency"))
            exit("CF " + ANSI_PUKE_WHITE + gateway->latency([&]() {
              handshake({
                {"gateway", gateway->http}
              }, true);
            }));
        } {
          gateway->wait_for_data(this);
          timer_1s([&](const unsigned int &tick) {
            gateway->ask_for_data(tick);
          });
          for (const auto &it : events)
            if (holds_alternative<TimeEvent>(it))
              timer_1s(get<TimeEvent>(it));
            else if (holds_alternative<QuitEvent>(it))
              ending(get<QuitEvent>(it));
            else if (holds_alternative<Gw::DataEvent>(it))
              gateway->data(get<Gw::DataEvent>(it));
          events.clear();
          ending([&]() {
            gateway->end();
            end();
          });
        } {
          handshake({
            {"gateway", gateway->http      },
            {"gateway", gateway->ws        },
            {"gateway", gateway->fix       },
            {"autoBot", arg<int>("autobot")
                          ? "yes"
                          : "no"           }
          }, arg<int>("nocache"));
        } {
          backups(this);
        } {
          if (arg<int>("headless")) headless();
          else {
            listen(this, poll());
            timer_1s([&](const unsigned int &tick) {
              broadcast(tick);
            });
            ending([&]() { without_goodbye(); });
            welcome();
          }
        } {
          ending([&]() {
            curl_global_cleanup();
            if (!arg<int>("free-version"))
              gateway->disclaimer();
          });
        }
        return this;
      };
      void wait() {
        walk();
      };
  };
}
