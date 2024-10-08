//! \file
//! \brief Data transport/transform helpers.

namespace ₿ {
  class WebSocketFrames {
    protected:
      string frame(string data, const int &opcode, const bool &mask) const {
        const int key = mask ? rand() : 0;
        const int bit = mask ? 0x80   : 0;
        size_t pos = 0,
               len = data.length();
                                data.insert(pos++  , 1,  opcode     | 0x80);
        if      (len <= 0x7D)   data.insert(pos++  , 1,  len        | bit );
        else if (len <= 0xFFFF) data.insert(pos    , 1, (char)(0x7E | bit))
                                    .insert(pos + 1, 1, (len >>  8) & 0xFF)
                                    .insert(pos + 2, 1,  len        & 0xFF), pos += 3;
        else                    data.insert(pos    , 1, (char)(0x7F | bit))
                                    .insert(pos + 1, 4,                  0)
                                    .insert(pos + 5, 1, (len >> 24) & 0xFF)
                                    .insert(pos + 6, 1, (len >> 16) & 0xFF)
                                    .insert(pos + 7, 1, (len >>  8) & 0xFF)
                                    .insert(pos + 8, 1,  len        & 0xFF), pos += 9;
        if (mask) {             data.insert(pos    , 1, (key >> 24) & 0xFF)
                                    .insert(pos + 1, 1, (key >> 16) & 0xFF)
                                    .insert(pos + 2, 1, (key >>  8) & 0xFF)
                                    .insert(pos + 3, 1,  key        & 0xFF), pos += 4;
          for (size_t i = 0; i < len; i++)
            data.at(pos + i) ^= data.at(pos - 4 + (i % 4));
        }
        return data;
      };
      string unframe(string &data, string &pong, bool &drop) const {
        string msg;
        const size_t max = data.length();
        if (max > 1) {
          const bool flat = (data[0] & 0x40) != 0x40,
                     text = (data[0] & 0x80) == 0x80,
                     mask = (data[1] & 0x80) == 0x80;
          const size_t key = mask ? 4 : 0;
          size_t                            len =    data[1] & 0x7F         , pos = key;
          if      (            len <= 0x7D)                                   pos += 2;
          else if (max > 2 and len == 0x7E) len = (((data[2] & 0xFF) <<  8)
                                                |   (data[3] & 0xFF)       ), pos += 4;
          else if (max > 8 and len == 0x7F) len = (((data[6] & 0xFF) << 24)
                                                |  ((data[7] & 0xFF) << 16)
                                                |  ((data[8] & 0xFF) <<  8)
                                                |   (data[9] & 0xFF)       ), pos += 10;
          if (flat and text and pos > key) {
            if (max >= pos + len) {
              if (mask)
                for (size_t i = 0; i < len; i++)
                  data.at(pos + i) ^= data.at(pos - key + (i % key));
              const unsigned char opcode = data[0] & 0x0F;
              if      (opcode == 0x01)
                msg = data.substr(pos, len);
              else if (opcode == 0x09)
                pong += frame(data.substr(pos, len), 0x0A, !mask);
              else if (opcode != 0x0A)
                drop = true;
              data = data.substr(pos + len);
            }
          } else drop = true;
        }
        return msg;
      };
  };

  class FixFrames {
    private_ref:
      const string &target;
      const string &sender;
    public:
      FixFrames(const string &t, const string &s)
        : target(t)
        , sender(s)
      {};
    protected:
      string frame(string data, const string &type, const unsigned long &sequence) const {
        data = "35=" + type                     + "\u0001"
               "34=" + to_string(sequence)      + "\u0001"
               "49=" + sender                   + "\u0001"
               "56=" + target                   + "\u0001"
             + data;
        data = "8=FIX.4.2"                        "\u0001"
               "9="  + to_string(data.length()) + "\u0001"
             + data;
        char ch = 0;
        for (
          size_t i  =  data.length();
                 i --> 0;
          ch += data.at(i)
        );
        stringstream sum;
        sum << setfill('0')
            << setw(3)
            << (ch & 0xFF);
        data += "10=" + sum.str()               + "\u0001";
        return data;
      };
      string unframe(string &data, string &pong, bool &drop) const {
        string msg;
        size_t end = data.find("\u0001" "10=");
        if (end != string::npos and data.length() > end + 7) {
          string raw = data.substr(0, end + 8);
          data = data.substr(raw.length());
          if      (raw.find("\u0001" "35=0" "\u0001") != string::npos
            or     raw.find("\u0001" "35=1" "\u0001") != string::npos
          ) pong = "0";
          else if (raw.find("\u0001" "35=5" "\u0001") != string::npos)
            pong = "5", drop = true;
          else {
            while ((end = raw.find("\u0001")) != string::npos) {
              raw.replace(raw.find("="), 1, "\":\"");
              msg += "\"" + raw.substr(0, end + 2) + "\",";
              raw = raw.substr(end + 3);
            }
            msg.pop_back();
            msg = "{" + msg + "}";
          }
        }
        return msg;
      };
  };

  class Loop {
    public_friend:
      using TimeEvent = function<void(const unsigned int&)>;
      class Timer {
        private:
                  unsigned int tick  = 0;
          mutable unsigned int ticks = 300;
             vector<TimeEvent> jobs;
        public:
          void ticks_factor(const unsigned int &factor) const {
            ticks = 300 * (factor ?: 1);
          };
          void timer_1s() {
            for (const auto &it : jobs) it(tick);
            tick = (tick + 1) % ticks;
          };
          void push_back(const TimeEvent &data) {
            jobs.push_back(data);
          };
      };
      class Async {
        public_friend:
          template<typename T> class Event {
            private_friend:
              class Wakeup {
                private:
                  Async *const event = nullptr;
                public:
                  Wakeup(Async *const e)
                    : event(e)
                  {};
                  ~Wakeup()
                  {
                    event->wakeup();
                  };
              };
            private:
                                 Async *event = nullptr;
                 function<vector<T>()>  job;
                     future<vector<T>>  data;
              function<void(const T&)>  write;
            public:
              bool waiting() const {
                return !!write;
              };
              void callback(const function<void(const T&)> fn) {
                write = fn;
              };
              void try_write(const T &rawdata) const {
                if (write) write(rawdata);
              };
              void wait_for(Loop *const loop, const function<vector<T>()> j) {
                job = j;
                event = loop->async([&]() {
                  if (data.valid())
                    for (const T &it : data.get()) try_write(it);
                });
              };
              void ask_for(const bool &samethread = true) {
                if (data.valid()) return;
                if (samethread) {
                  Wakeup again(event);
                  data = ::async(launch::deferred, [&]() {
                    return job();
                  });
                } else
                  data = ::async(launch::async, [&]() {
                    Wakeup again(event);
                    return job();
                  });
              };
          };
        private:
          function<void()> job;
        public:
          Async(const function<void()> data)
            : job(data)
          {};
          virtual void wakeup() {};
          void ready() {
            job();
          };
          void link(const function<void()> &data) {
            job = data;
          };
      };
      class Poll: public Async {
        protected:
          curl_socket_t sockfd = 0;
        public:
          Poll(const curl_socket_t &s)
            : Async(nullptr)
            , sockfd(s)
          {};
          virtual void start(const curl_socket_t&, const function<void()>&)  = 0;
          virtual void stop()                                                = 0;
        protected:
          virtual void change(const int&, const function<void()>& = nullptr) = 0;
      };
    public:
      virtual          void  timer_ticks_factor(const unsigned int&) const = 0;
      virtual          void  timer_1s(const TimeEvent&)                    = 0;
      virtual         Async *async(const function<void()>&)                = 0;
    protected:
      virtual curl_socket_t  poll()                                        = 0;
      virtual          void  walk()                                        = 0;
      virtual          void  end()                                         = 0;
  };
#if defined _WIN32 or defined __APPLE__
  class Events: public Loop {
    public_friend:
      class Timer: public Loop::Timer {
        public:
          uv_timer_t event;
        public:
          Timer()
            : event()
          {
            event.data = this;
            uv_timer_init(uv_default_loop(), &event);
            uv_timer_start(&event, [](uv_timer_t *event) {
              ((Timer*)event->data)->timer_1s();
            }, 0, 1e+3);
          };
      };
      class Async: public Loop::Async {
        public:
          uv_async_t event;
        public:
          Async(const function<void()> &data)
            : Loop::Async(data)
            , event()
          {
            event.data = this;
            uv_async_init(uv_default_loop(), &event, [](uv_async_t *event) {
              ((Async*)event->data)->ready();
            });
          };
          void wakeup() override {
            uv_async_send(&event);
          };
      };
      class Poll: public Loop::Poll {
        protected:
          const int EPOLLIN  = UV_READABLE;
          const int EPOLLOUT = UV_WRITABLE;
        private:
          uv_poll_t event;
        public:
          Poll(const curl_socket_t &s = 0)
            : Loop::Poll(s)
            , event()
          {};
          void start(const curl_socket_t&, const function<void()> &data) override {
            event.data = this;
            uv_poll_init_socket(uv_default_loop(), &event, sockfd);
            change(EPOLLIN, data);
          };
          void stop() override {
            if (event.data) {
              uv_poll_stop(&event);
              uv_close((uv_handle_t*)&event, [](uv_handle_t*) { });
              if (sockfd) {
                BIO_closesocket(sockfd);
                sockfd = 0;
              }
            }
          };
        protected:
          void change(const int &events, const function<void()> &data = nullptr) override {
            if (event.data) {
              if (data) link(data);
              if (!uv_is_closing((uv_handle_t*)&event))
                uv_poll_start(&event, events, [](uv_poll_t *event, int, int) {
                  ((Poll*)event->data)->ready();
                });
            }
          };
      };
    private:
            Timer timer;
      list<Async> jobs;
    public:
      void timer_ticks_factor(const unsigned int &factor) const override {
        timer.ticks_factor(factor);
      };
      void timer_1s(const TimeEvent &data) override {
        timer.push_back(data);
      };
      Loop::Async *async(const function<void()> &data) override {
        jobs.emplace_back(data);
        return &jobs.back();
      };
    protected:
      curl_socket_t poll() override {
        return 0;
      };
      void walk() override {
        uv_run(uv_default_loop(), UV_RUN_DEFAULT);
      };
      void end() override {
        uv_timer_stop(&timer.event);
        uv_close((uv_handle_t*)&timer.event, [](uv_handle_t*){ });
        for (auto &it : jobs)
          uv_close((uv_handle_t*)&it.event, [](uv_handle_t*){ });
      };
  };
#else
  class Events: public Loop {
    public_friend:
      class Poll: public Loop::Poll {
        private:
          curl_socket_t loopfd = 0;
        public:
          Poll(const curl_socket_t &s = 0)
            : Loop::Poll(s)
          {};
          void start(const curl_socket_t &l, const function<void()> &data) override {
            loopfd = l;
            link(data);
            ctl(EPOLLIN, EPOLL_CTL_ADD);
          };
          void stop() override {
            if (loopfd and sockfd) {
              ctl(0, EPOLL_CTL_DEL);
              ::close(sockfd);
              sockfd = 0;
            }
          };
        protected:
          void change(const int &events, const function<void()> &data = nullptr) override {
            if (loopfd) {
              if (data) link(data);
              ctl(events, EPOLL_CTL_MOD);
            }
          };
        private:
          void ctl(const int &events, const int &opcode) {
            epoll_event event;
            event.events = events;
            event.data.ptr = this;
            epoll_ctl(loopfd, opcode, sockfd, &event);
          };
      };
      class Timer: public Poll,
                   public Loop::Timer {
        private:
          itimerspec ts = {
            {1, 0}, {0, 1}
          };
        public:
          Timer(const curl_socket_t &loopfd)
            : Poll(timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC))
          {
            if (timerfd_settime(sockfd, 0, &ts, nullptr) != -1)
              Poll::start(loopfd, [&]() {
                uint64_t again = 0;
                if (::read(sockfd, &again, 8) == 8)
                  timer_1s();
              });
          };
      };
      class Async: public Poll {
        private:
          const uint64_t again = 1;
        public:
          Async(const curl_socket_t &loopfd, const function<void()> &data)
            : Poll(::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC))
          {
            Poll::start(loopfd, [this, data]() {
              uint64_t again = 0;
              if (::read(sockfd, &again, 8) == 8)
                data();
            });
          };
          void wakeup() override {
            if (::write(sockfd, &again, 8) == 8) {}
          };
      };
    private:
       curl_socket_t sockfd = 0;
               Timer timer;
         list<Async> jobs;
         epoll_event ready[32] = {};
    public:
      Events()
        : sockfd(epoll_create1(EPOLL_CLOEXEC))
        , timer(sockfd)
      {};
      void timer_ticks_factor(const unsigned int &factor) const override {
        timer.ticks_factor(factor);
      };
      void timer_1s(const TimeEvent &data) override {
        timer.push_back(data);
      };
      Loop::Async *async(const function<void()> &data) override {
        jobs.emplace_back(sockfd, data);
        return &jobs.back();
      };
    protected:
      curl_socket_t poll() override {
        return sockfd;
      };
      void walk() override {
        while (sockfd)
          for (
            int i  =  epoll_wait(sockfd, ready, 32, -1);
                i --> 0;
            ((Poll*)ready[i].data.ptr)->ready()
          );
      };
      void end() override {
        timer.stop();
        for (auto &it : jobs)
          it.stop();
        jobs.clear();
        ::close(sockfd);
        sockfd = 0;
      };
  };
#endif

  static struct curl_blob curl_ca_embed_blob = {
    (void*)curl_ca_embed,
    strlen((const char*)curl_ca_embed),
    CURL_BLOB_NOCOPY
  };

  static function<void(CURL*)> args_easy_setopt = [](CURL *curl) {
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "K");
    curl_easy_setopt_nowin32(curl, CURLOPT_CAINFO_BLOB, &curl_ca_embed_blob);
  };

  class Curl {
    private_friend:
      class Easy: public Events::Poll {
        protected:
          string in;
        private:
          string out;
          unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl;
        public:
          Easy()
            : curl(nullptr, curl_easy_cleanup)
          {};
        protected:
          void cleanup() {
            if (curl) {
              if (!out.empty()) send();
              curl.reset();
              stop();
            }
          };
          bool connected() const {
            return sockfd;
          };
          CURLcode connect(const string &url, const string &header, const string &res1, const string &res2) {
            out = header;
            in.clear();
            CURLcode rc;
            if (CURLE_OK == (rc = init())) {
              args_easy_setopt(curl.get());
              curl_easy_setopt(curl.get(), CURLOPT_URL, url.data());
              curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);
              curl_easy_setopt(curl.get(), CURLOPT_CONNECT_ONLY, 1L);
              if ( CURLE_OK != (rc = curl_easy_perform(curl.get()))
                or CURLE_OK != (rc = curl_easy_getinfo(curl.get(), CURLINFO_ACTIVESOCKET, &sockfd))
                or CURLE_OK != (rc = send())
                or CURLE_OK != (rc = recv(5))
                or string::npos == in.find(res1)
                or string::npos == in.find(res2)
              ) {
                if (rc == CURLE_OK)
                  rc = string::npos == in.find("invalid API key")
                     ? CURLE_WEIRD_SERVER_REPLY
                     : CURLE_LOGIN_DENIED;
                cleanup();
              }
              in.clear();
            }
            return rc;
          };
          CURLcode send_recv() {
            CURLcode rc = CURLE_COULDNT_CONNECT;
            if (curl
              and sockfd
              and (out.empty() or CURLE_OK == (rc = send()))
              and CURLE_OPERATION_TIMEDOUT == (rc = recv(0))
            ) rc = CURLE_OK;
            if (rc != CURLE_OK)
              cleanup();
            return rc;
          };
          CURLcode emit(const string &data) {
            CURLcode rc = CURLE_OK;
            if (curl and sockfd) {
              out += data;
              change(EPOLLIN | EPOLLOUT);
              send();
            } else {
              rc = CURLE_COULDNT_CONNECT;
              cleanup();
            }
            return rc;
          };
        private:
          CURLcode init() {
            if (!curl) curl.reset(curl_easy_init());
            else curl_easy_reset(curl.get());
            sockfd = 0;
            return curl
              ? CURLE_OK
              : CURLE_FAILED_INIT;
          };
          CURLcode send() {
            CURLcode rc;
            do {
              do {
                size_t n = 0;
                rc = curl_easy_send(curl.get(), out.data(), out.length(), &n);
                out = out.substr(n);
                if (rc == CURLE_AGAIN and !wait(false, 5))
                  return CURLE_OPERATION_TIMEDOUT;
              } while (rc == CURLE_AGAIN);
              if (rc != CURLE_OK) break;
            } while (!out.empty());
            if (out.empty()) change(EPOLLIN);
            return rc;
          };
          CURLcode recv(const int &timeout) {
            CURLcode rc;
            for(;;) {
              char data[524288];
              size_t n;
              do {
                n = 0;
                rc = curl_easy_recv(curl.get(), data, sizeof(data), &n);
#ifdef _WIN32
                if (rc == CURLE_UNSUPPORTED_PROTOCOL and n == 0)
                  rc = CURLE_OK;
#endif
                in.append(data, n);
                if (rc == CURLE_AGAIN and !wait(true, timeout))
                  return CURLE_OPERATION_TIMEDOUT;
              } while (rc == CURLE_AGAIN);
              if ((timeout and in.find(ANSI_NEW_LINE ANSI_NEW_LINE) != string::npos)
                or rc != CURLE_OK
                or n == 0
              ) break;
            }
            return rc;
          };
          int wait(const bool &io, const int &timeout) const {
            timeval tv = {timeout, 0};
            fd_set infd,
                   outfd;
            FD_ZERO(&infd);
            FD_ZERO(&outfd);
            FD_SET(sockfd, io ? &infd : &outfd);
            return select(sockfd + 1, &infd, &outfd, nullptr, &tv);
          };
      };
    public_friend:
      class Web {
        public:
          static json xfer(
            const string         &url,
            const string         &crud = "GET",
            const string         &post = "",
            const vector<string> &headers = {},
            const string         &auth = ""
          ) {
            string reply;
            CURLcode rc = CURLE_FAILED_INIT;
            unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(
              curl_easy_init(),
              curl_easy_cleanup
            );
            if (curl) {
              args_easy_setopt(curl.get());
              unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> slist(
                nullptr,
                curl_slist_free_all
              );
              for (const auto &it : headers)
                slist.reset(curl_slist_append(slist.release(), it.data()));
              if (slist) curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, slist.get());
              if (!post.empty()) curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, post.data());
              if (!auth.empty()) curl_easy_setopt(curl.get(), CURLOPT_USERPWD, auth.data());
              curl_easy_setopt(curl.get(), CURLOPT_CUSTOMREQUEST, crud.data());
              curl_easy_setopt(curl.get(), CURLOPT_URL, url.data());
              curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, &write);
              curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &reply);
              curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 21L);
              rc = curl_easy_perform(curl.get());
            }
            return rc == CURLE_OK
              ? (json::accept(reply)
                  ? json::parse(reply)
                  : (json){ {"error", "CURL unexpected server reply: " + reply} }
                )
              : (json){ {"error", string("CURL Request Error: ") + curl_easy_strerror(rc)} };
          };
        private:
          static size_t write(void *buf, size_t size, size_t nmemb, void *reply) {
            ((string*)reply)->append((char*)buf, size *= nmemb);
            return size;
          };
      };
      class WebSocket: public Easy,
                       public WebSocketFrames {
        private:
          using Easy::in;
        protected:
          CURLcode connect(const string &uri) {
            CURLcode rc = CURLE_URL_MALFORMAT;
            unique_ptr<CURLU, decltype(&curl_url_cleanup)> url(
              curl_url(),
              curl_url_cleanup
            );
            char *host,
                 *port,
                 *path,
                 *query;
            string header;
            if (!curl_url_set(url.get(), CURLUPART_URL, ("http" + uri.substr(2)).data(), 0)) {
              if (!curl_url_get(url.get(), CURLUPART_HOST, &host, 0)) {
                header = string(host);
                curl_free(host);
                if (!curl_url_get(url.get(), CURLUPART_PORT, &port, CURLU_DEFAULT_PORT)) {
                  header += ":" + string(port);
                  curl_free(port);
                  if (!curl_url_get(url.get(), CURLUPART_PATH, &path, 0)) {
                    header = "GET " + string(path) + (
                                curl_url_get(url.get(), CURLUPART_QUERY, &query, 0)
                                  ? "" : "?" + string(query)
                             ) + " HTTP/1.1"
                             ANSI_NEW_LINE "Host: " + header +
                             ANSI_NEW_LINE "Upgrade: websocket"
                             ANSI_NEW_LINE "Connection: Upgrade"
                             ANSI_NEW_LINE "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw=="
                             ANSI_NEW_LINE "Sec-WebSocket-Version: 13"
                             ANSI_NEW_LINE
                             ANSI_NEW_LINE;
                    curl_free(path);
                    curl_free(query);
                    rc = CURLE_OK;
                  }
                }
              }
            }
            return rc == CURLE_OK
              ? Easy::connect(
                  "http" + uri.substr(2),
                  header,
                  "HTTP/1.1 101 Switching Protocols",
                  "HSmrc0sMlYUkAGmm5OPpG2HaGWk="
                )
              : rc;
          };
          CURLcode emit(const string &data, const int &opcode) {
            return Easy::emit(frame(data, opcode, true));
          };
          string unframe() {
            string pong;
            bool drop = false;
            const string msg = WebSocketFrames::unframe(in, pong, drop);
            if (!pong.empty()) Easy::emit(pong);
            if (drop) cleanup();
            return msg;
          };
      };
      class WebSocketTwin: public WebSocket {
        protected:
          virtual string twin(const string&) const = 0;
      };
      class FixSocket: public Easy,
                       public FixFrames {
        private:
          using Easy::in;
          unsigned long sequence = 0;
        public:
          FixSocket(const string &t, const string &s)
            : FixFrames(t, s)
          {};
        protected:
          CURLcode connect(const string &uri, const string &logon) {
            return Easy::connect(
              "https://" + uri,
              frame(logon, "A", sequence = 1),
              "8=FIX.4.2" "\u0001",
              "\u0001" "35=A" "\u0001"
            );
          };
          CURLcode emit(const string &data, const string &type) {
            return Easy::emit(frame(data, type, ++sequence));
          };
          string unframe() {
            string pong;
            bool drop = false;
            const string msg = FixFrames::unframe(in, pong, drop);
            if (!pong.empty()) emit("", pong);
            if (drop) cleanup();
            return msg;
          };
          unsigned long last() const {
            return sequence;
          };
      };
  };

  class Text {
    public:
      static string strL(string input) {
        transform(input.begin(), input.end(), input.begin(), ::tolower);
        return input;
      };
      static string strU(string input) {
        transform(input.begin(), input.end(), input.begin(), ::toupper);
        return input;
      };
      static string CRC32(const string &input) {
        return to_string(crc32(0, (const Bytef*)input.data(), input.length()));
      };
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wrestrict"
      static string B64(const string &input, const bool &urlsafe = false) {
        unique_ptr<BIO, decltype(&BIO_free_all)> bio(
          BIO_push(BIO_new(BIO_f_base64()), BIO_new(BIO_s_mem())),
          BIO_free_all
        );
        if (BIO_set_close(bio.get(), BIO_CLOSE)) {}
        BIO_set_flags(bio.get(), BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio.get(), input.data(), input.length());
        if (BIO_flush(bio.get())) {}
        BUF_MEM *buf = nullptr;
        BIO_get_mem_ptr(bio.get(), &buf);
        string output(buf->data, buf->length);
        if (urlsafe) {
          size_t n = 0;
          while ((n = output.find("+")) != string::npos)
            output.replace(n, 1, "-");
          while ((n = output.find("/")) != string::npos)
            output.replace(n, 1, "_");
          while (output.back() == '=')
            output.erase(output.length() - 1);
        }
        return output;
      };
      static string B64_decode(const string &input, const bool &urlsafe = false) {
        if (urlsafe) {
          string output = input;
          size_t n = 0;
          while ((n = output.find("-")) != string::npos)
            output.replace(n, 1, "+");
          while ((n = output.find("_")) != string::npos)
            output.replace(n, 1, "/");
          output += string(4 - (output.length() % 4), '=');
          return B64_decode(output + string(4 - (output.length() % 4), '='));
        }
        unique_ptr<BIO, decltype(&BIO_free_all)> bio(
          BIO_push(BIO_new(BIO_f_base64()), BIO_new_mem_buf(input.data(), input.length())),
          BIO_free_all
        );
        char out[input.length()];
        if (BIO_set_close(bio.get(), BIO_CLOSE)) {}
        BIO_set_flags(bio.get(), BIO_FLAGS_BASE64_NO_NL);
        int len = BIO_read(bio.get(), out, input.length());
        return string(out, len);
      };
#pragma GCC diagnostic pop 
      static string SHA1  (const string &input, const bool &rawbin = false) {
        return SHA(input, rawbin, ::SHA1,   SHA_DIGEST_LENGTH);
      };
      static string SHA256(const string &input, const bool &rawbin = false) {
        return SHA(input, rawbin, ::SHA256, SHA256_DIGEST_LENGTH);
      };
      static string SHA512(const string &input, const bool &rawbin = false) {
        return SHA(input, rawbin, ::SHA512, SHA512_DIGEST_LENGTH);
      };
      static string HMAC1  (const string &key, const string &input, const bool &rawbin = false) {
        return HMAC(key, input, rawbin, EVP_sha1,   SHA_DIGEST_LENGTH);
      };
      static string HMAC256(const string &key, const string &input, const bool &rawbin = false) {
        return HMAC(key, input, rawbin, EVP_sha256, SHA256_DIGEST_LENGTH);
      };
      static string HMAC512(const string &key, const string &input, const bool &rawbin = false) {
        return HMAC(key, input, rawbin, EVP_sha512, SHA512_DIGEST_LENGTH);
      };
      static string HMAC384(const string &key, const string &input, const bool &rawbin = false) {
        return HMAC(key, input, rawbin, EVP_sha384, SHA384_DIGEST_LENGTH);
      };
    private:
      static string SHA(
        const string  &input,
        const bool    &rawbin,
        unsigned char *(*md)(const unsigned char*, size_t, unsigned char*),
        const int     &digest_len
      ) {
        unsigned char digest[digest_len];
        md((unsigned char*)input.data(), input.length(), digest);
        return rawbin
             ? string((char*)digest, digest_len)
             :    HEX((char*)digest, digest_len);
      };
      static string HMAC(
        const string &key,
        const string &input,
        const bool   &rawbin,
        const EVP_MD *(evp_md)(),
        const int    &digest_len
      ) {
        unsigned char* digest;
        digest = ::HMAC(
          evp_md(),
          input.data(), input.length(),
          (unsigned char*)key.data(), key.length(),
          nullptr, nullptr
        );
        return rawbin
             ? string((char*)digest, digest_len)
             :    HEX((char*)digest, digest_len);
      };
      static string HEX(const char *digest, const int &digest_len) {
        stringstream output;
        output << hex << setfill('0');
        for (int i = 0; i < digest_len; i++)
          output << setw(2) << (digest[i] & 0xFF);
        return output.str();
      };
  };

  class WebServer {
    public_friend:
      using Response = function<string(string, const string&, const string&)>;
      using Upgrade  = function<int(const int&, const string&)>;
      using Message  = function<string(string, const string&)>;
    private_friend:
      struct Session {
          string auth;
        Response response = nullptr;
         Upgrade upgrade  = nullptr;
         Message message  = nullptr;
      };
      class Socket: public Events::Poll {
        public:
          Socket(const curl_socket_t &s = 0)
            : Poll(s)
          {};
          void shutdown() {
            stop();
          };
          void cork(const int &enable) const {
#ifndef _WIN32
            setsockopt(sockfd, IPPROTO_TCP, TCP_CORK, &enable, sizeof(enable));
#endif
#ifdef __APPLE__
            if (!enable) ::send(sockfd, "", 0, MSG_NOSIGNAL);
#endif
          };
      };
      class Frontend: public Socket,
                      public WebSocketFrames {
        private:
          unique_ptr<
            SSL, function<void(SSL*)>
          >              ssl;
          const Session *session = nullptr;
                  Clock  time    = 0;
                string   addr,
                         out,
                         in;
        public:
          Frontend(const curl_socket_t &s, const curl_socket_t &loopfd, SSL *S, const Session *e)
            : Socket(s)
            , ssl(S, [](SSL *ssl){ 
                SSL_shutdown(ssl);
                SSL_free(ssl);
              })
            , session(e)
            , time(Tstamp)
          {
            Socket::start(loopfd, ioHttp);
          };
          void shutdown() {
            ssl.reset();
            Socket::shutdown();
            if (!time) session->upgrade(-1, addr);
          };
          bool upgraded() const {
            return !time;
          };
          void send(const string &data) {
            out += data;
            change(EPOLLIN | EPOLLOUT);
          };
          bool stale() {
            if (time and sockfd and Tstamp > time + 21e+3)
              shutdown();
            return !sockfd;
          };
        protected:
          string unframe() {
            bool drop = false;
            const string msg = WebSocketFrames::unframe(in, out, drop);
            if (drop) shutdown();
            return msg;
          };
        private:
          function<void()> ioWs = [&]() {
            io();
            if (!sockfd or in.empty()) return;
            const string msg = unframe();
            if (msg.empty()) return;
            const string reply = session->message(msg, addr);
            if (reply.empty()) return;
            out += frame(reply, reply.substr(0, 2) == "PK" ? 0x02 : 0x01, false);
            change(EPOLLIN | EPOLLOUT);
          };
          function<void()> ioHttp = [&]() {
            io();
            if (sockfd
              and out.empty()
              and in.length() > 5
              and in.substr(0, 5) == "GET /"
              and in.find(ANSI_NEW_LINE ANSI_NEW_LINE) != string::npos
            ) {
              if (addr.empty())
                addr = address();
              const string iN     = Text::strU(in);
              const string path   = in.substr(4, in.find(" HTTP/") - 4);
              const size_t papers = iN.find("AUTHORIZATION: BASIC ");
              string auth;
              if (papers != string::npos) {
                auth = in.substr(papers + 21);
                auth = auth.substr(0, auth.find(ANSI_NEW_LINE));
              }
              const size_t key = iN.find("SEC-WEBSOCKET-KEY: ");
              int allowed = 1;
              if (key == string::npos) {
                out = session->response(path, auth, addr);
                if (out.empty())
                  shutdown();
                else change(EPOLLIN | EPOLLOUT);
              } else if ((session->auth.empty() or auth == session->auth)
                and iN.find(ANSI_NEW_LINE "UPGRADE: WEBSOCKET") != string::npos
                and iN.find(ANSI_NEW_LINE "CONNECTION: ")       != string::npos
                and iN.find(" UPGRADE")                         != string::npos
                and (iN.find("SEC-WEBSOCKET-VERSION: 13")       != string::npos)
                and (allowed = session->upgrade(allowed, addr))
              ) {
                time = 0;
                out = "HTTP/1.1 101 Switching Protocols"
                      ANSI_NEW_LINE "Connection: Upgrade"
                      ANSI_NEW_LINE "Upgrade: websocket"
                      ANSI_NEW_LINE "Sec-WebSocket-Version: 13"
                      ANSI_NEW_LINE "Sec-WebSocket-Accept: "
                               + Text::B64(Text::SHA1(
                                 in.substr(key + 19, in.substr(key + 19).find(ANSI_NEW_LINE))
                                   + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11",
                                 true
                               )) +
                      ANSI_NEW_LINE
                      ANSI_NEW_LINE;
                in.clear();
                change(EPOLLIN | EPOLLOUT, ioWs);
              } else {
                if (!allowed) session->upgrade(allowed, addr);
                shutdown();
              }
            }
          };
          string address() const {
            string addr;
            union {
              sockaddr         sa;
              sockaddr_in      s4;
              sockaddr_in6     s6;
              sockaddr_storage ss;
            } u;
            socklen_t len = sizeof(u);
            if (getpeername(sockfd, &u.sa, &len) != -1) {
              char buf[INET6_ADDRSTRLEN];
              if (u.ss.ss_family == AF_INET)
                inet_ntop(AF_INET,  &u.s4.sin_addr,  buf, sizeof(buf));
              else
                inet_ntop(AF_INET6, &u.s6.sin6_addr, buf, sizeof(buf));
              addr = string(buf);
              if (addr.length() > 7 and addr.substr(0, 7) == "::ffff:") addr = addr.substr(7);
              if (addr.length() < 7) addr.clear();
            }
            return addr.empty() ? "unknown" : addr;
          };
          void io() {
            if (ssl) {
              if (!out.empty()) {
                cork(1);
                int n = SSL_write(ssl.get(), out.data(), out.length());
                switch (SSL_get_error(ssl.get(), n)) {
                  case SSL_ERROR_WANT_READ:
                  case SSL_ERROR_WANT_WRITE:  break;
                  case SSL_ERROR_NONE:        out.clear();
                                              change(EPOLLIN);  [[fallthrough]];
                  case SSL_ERROR_ZERO_RETURN: if (!time) break; [[fallthrough]];
                  default:                    shutdown();
                                              return;
                }
                cork(0);
              }
              do {
                char data[1024];
                int n = SSL_read(ssl.get(), data, sizeof(data));
                switch (SSL_get_error(ssl.get(), n)) {
                  case SSL_ERROR_NONE:        in.append(data, n);
                  case SSL_ERROR_WANT_READ:
                  case SSL_ERROR_WANT_WRITE:
                  case SSL_ERROR_ZERO_RETURN: break;
                  default:                    if (!time) break;
                                              shutdown();
                                              return;
                }
              } while (SSL_pending(ssl.get()));
            } else {
              if (!out.empty()) {
                cork(1);
                ssize_t n = ::send(sockfd, out.data(), out.length(), MSG_NOSIGNAL);
                if      (n > 0) out = out.substr(n);
                else if (n < 0) {
                  shutdown();
                  return;
                }
                cork(0);
                if (out.empty()) {
                  if (time) {
                    shutdown();
                    return;
                  }
                  change(EPOLLIN);
                }
              }
              char data[1024];
              ssize_t n = ::recv(sockfd, data, sizeof(data), 0);
              if (n > 0) in.append(data, n);
            }
          };
      };
    public_friend:
      class Backend: public Socket,
                     public WebSocketFrames {
        private:
          unique_ptr<
            SSL_CTX, decltype(&SSL_CTX_free)
          >              ctx;
                 Session session;
          list<Frontend> requests;
        public:
          Backend()
            : ctx(nullptr, SSL_CTX_free)
          {};
          bool idle() const {
            return !any_of(requests.begin(), requests.end(), [](auto &it) {
              return it.upgraded();
            });
          };
          int clients() const {
            return count_if(requests.begin(), requests.end(), [](auto &it) {
              return it.upgraded();
            });
          };
          string protocol() const {
            return "HTTP" + string(!!ctx, 'S');
          };
          void broadcast(const char &portal, const unordered_map<char, string> &queue) {
            string msgs;
            for (const auto &it : queue)
              msgs += frame(portal + (it.first + it.second), 0x01, false);
            for (auto &it : requests)
              if (it.upgraded())
                it.send(msgs);
          };
          void purge() {
            while (!requests.empty()) {
              requests.back().shutdown();
              requests.pop_back();
            }
            ctx.reset();
            shutdown();
          };
          void timeouts() {
            for (auto it = requests.begin(); it != requests.end();)
              if (it->stale())
                it = requests.erase(it);
              else ++it;
          };
          bool listen(const curl_socket_t &loopfd, const string &inet, const int &port, const bool &ipv6, const Session &data) {
            if (sockfd) return false;
            addrinfo  hints  = {AI_PASSIVE, AF_UNSPEC, SOCK_STREAM, 0, 0, nullptr, nullptr, nullptr},
                     *result = nullptr,
                     *rp     = nullptr;
            if (!getaddrinfo(inet.empty() ? nullptr : inet.data(), to_string(port).data(), &hints, &result)) {
              if (ipv6)
                for (rp = result; rp and !sockfd; rp = sockfd ? rp : rp->ai_next)
                  if (rp->ai_family == AF_INET6)
                    socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
              if (!sockfd)
                for (rp = result; rp and !sockfd; rp = sockfd ? rp : rp->ai_next)
                  if (rp->ai_family == AF_INET)
                    socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
              if (rp and sockfd) {
                const
#ifdef _WIN32
                char
#else
                int
#endif
                enabled = 1;
                setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
                if (::bind(sockfd, rp->ai_addr, rp->ai_addrlen) or ::listen(sockfd, 512))
                  shutdown();
                else {
                  session = data;
                  start(loopfd, [this, loopfd]() {
                    accept_request(loopfd);
                  });
                }
              }
              freeaddrinfo(result);
            }
            return sockfd;
          };
          vector<string> ssl_context(const string &crt, const string &key) {
            vector<string> warn;
            ctx.reset(SSL_CTX_new(TLS_server_method()));
            if (ctx) {
              SSL_CTX_set_options(ctx.get(), SSL_OP_NO_SSLv3);
              if (crt.empty() or key.empty()) {
                if (!crt.empty())
                  warn.emplace_back("Ignored .crt file because .key file is missing");
                if (!key.empty())
                  warn.emplace_back("Ignored .key file because .crt file is missing");
                const char tlscrt[] = "-----BEGIN CERTIFICATE-----"                  ANSI_NEW_LINE
                  "MIIDJTCCAg2gAwIBAgIBAjANBgkqhkiG9w0BAQsFADASMRAwDgYDVQQDDAdSb290" ANSI_NEW_LINE
                  "IENBMCAXDTE2MDExNDIyMjk0NloYDzIxMTYwMTE1MjIyOTQ2WjAZMRcwFQYDVQQD" ANSI_NEW_LINE
                  "DA5zZXJ2ZXIuZXhhbXBsZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB" ANSI_NEW_LINE
                  "ANVdYGrf/GHuSKqMEUhDpW22Ul2qmEmxYZI1sfw6BCUMbXn/tNXJ6VwcO+Crs7h9" ANSI_NEW_LINE
                  "o95tveDd11q/FEcRQl6mgtBhwX/dE0bmCYUHDvLU/Bpk0gqtIKsga5bwrczEGVNV" ANSI_NEW_LINE
                  "3AEdpLPvirRJU12KBRzx3OFEv8XX4ncZV1yXC3XuiENxD8pswbSyUKd3RmxYDxG/" ANSI_NEW_LINE
                  "8XYkWq45QrdRZynh0FUwbxfkkeqt+CjCQ2+iZKn7nZiSYkg+6w1PgkqK/z9y7pa1" ANSI_NEW_LINE
                  "rqHBmLrvfZB1bf9aUp6r9cB+0IdD24UHBw99OHr90dPuZR3T6jlqhzfuStPgDW71" ANSI_NEW_LINE
                  "cKzCvfFu85KVXqnwoWWVk40CAwEAAaN9MHswHQYDVR0OBBYEFMDnhL/oWSczELBS" ANSI_NEW_LINE
                  "T1FSLwbWwHrNMB8GA1UdIwQYMBaAFHB/Lq6DaFmYBCMqzes+F80k3QFJMAkGA1Ud" ANSI_NEW_LINE
                  "EwQCMAAwEwYDVR0lBAwwCgYIKwYBBQUHAwEwGQYDVR0RBBIwEIIOc2VydmVyLmV4" ANSI_NEW_LINE
                  "YW1wbGUwDQYJKoZIhvcNAQELBQADggEBAHvTBEN1ig8RrsT716Ginv4gGNX0LzGI" ANSI_NEW_LINE
                  "RrZ1jO7lm5emuaPNYJpGw0iX5Zdo91qGNXPZaZ75X3S55pQTActq3OPEBOll2pyk" ANSI_NEW_LINE
                  "iyjz+Zp/v5cfRZLlBbFW5gv2R94eibYr4U3fSn4B0yPcl4xH/l/HzJhGDsSDW8qK" ANSI_NEW_LINE
                  "8VIJvmvsPwmL0JMCv+FR59F+NFYZdND/KCXet59WUpF9ICmFCoBEX3EyJXEPwhbi" ANSI_NEW_LINE
                  "X2sdPzJbCjx0HLli8e0HUKNttLQxCsBTRGo6iISLLamwN47mGDa9miBADwGSiz2q" ANSI_NEW_LINE
                  "YeeuLO02zToHhnQ6KbPXOrQAqcL1kngO4g+j/ru+4AZThFkdkGnltvk="         ANSI_NEW_LINE
                  "-----END CERTIFICATE-----"                                        ANSI_NEW_LINE;
                const char tlskey[] = "-----BEGIN PRIVATE KEY-----"                  ANSI_NEW_LINE
                  "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDVXWBq3/xh7kiq" ANSI_NEW_LINE
                  "jBFIQ6VttlJdqphJsWGSNbH8OgQlDG15/7TVyelcHDvgq7O4faPebb3g3ddavxRH" ANSI_NEW_LINE
                  "EUJepoLQYcF/3RNG5gmFBw7y1PwaZNIKrSCrIGuW8K3MxBlTVdwBHaSz74q0SVNd" ANSI_NEW_LINE
                  "igUc8dzhRL/F1+J3GVdclwt17ohDcQ/KbMG0slCnd0ZsWA8Rv/F2JFquOUK3UWcp" ANSI_NEW_LINE
                  "4dBVMG8X5JHqrfgowkNvomSp+52YkmJIPusNT4JKiv8/cu6Wta6hwZi6732QdW3/" ANSI_NEW_LINE
                  "WlKeq/XAftCHQ9uFBwcPfTh6/dHT7mUd0+o5aoc37krT4A1u9XCswr3xbvOSlV6p" ANSI_NEW_LINE
                  "8KFllZONAgMBAAECggEADLTt7A+A2Vg2jamf0dztejY0e42QWjstI2b9PZc67fXq" ANSI_NEW_LINE
                  "gyx+WYkX07t+uWegYWliG/oPJ9guXiIpE/5sJHToL37S5kmFP2CtynVcJ4wVo4DD" ANSI_NEW_LINE
                  "nY0n9+kLX0bgIuS+2V6wpoRcbbbjXM9NHrH8kfe5ftT4UtEDlLI2qLX6IcDd7p4u" ANSI_NEW_LINE
                  "OYjILChR8GSGTw96yIy2Ws/1Uq9PMw64JoT4RcK5QqnkcPMDFRH1SeLOL+zXP2c4" ANSI_NEW_LINE
                  "nEl9yOy3HauZKxwl/Ry/XK1s3DdjopIAU29ut+hAuMiTb06kzZnumL9NoplKoZtU" ANSI_NEW_LINE
                  "otw/gVcCKhT+Ep+p6i8InLF0XEME8A0qUR0niWebgQKBgQD6vkxR49B8ZZQrzjw4" ANSI_NEW_LINE
                  "XKs1lI9cP7cgPiuWlDHMNjYou3WbOaGrMeScvbB1Ldh9A8pjAhxlw8AaV/xs4qcA" ANSI_NEW_LINE
                  "trmVmSISVMVyc1wSGlJXWi2nUzTNs9OE3vj22SyStihf8UUZtWwX2b5Y4JrYhA/V" ANSI_NEW_LINE
                  "+ThGGqHR03oLNLShNLtJc2c7YQKBgQDZ1nkibEyrepexw/fnwkw61IJKq9wRIh1G" ANSI_NEW_LINE
                  "PREakhbe9wU5ie0knuf9razt7awzQiwFmlixmWqsM7UEtLuXNnNPciwdrKhhbvrd" ANSI_NEW_LINE
                  "vD/rkbIEHEPllIhFlDtOzn3hRBWTzWmXFjpou/2LvHTSbVis4IYVZymTp2jb1ZLs" ANSI_NEW_LINE
                  "7VbiG9JTrQKBgQDc6n75g1szzpdehQT/r33U5j/syeJBUSU8NPMu9fB/sLHsgjlT" ANSI_NEW_LINE
                  "SNEf2+y1QSBE/Or6kmiMrIv7advn30W+Vj9qc5HWTsPrk4HiHTjA553jl2alebN5" ANSI_NEW_LINE
                  "lK4LZspjtIQcC8mS3goPdXPEgJdM/gWpwzr2YQ6DfOxBJT2j7n64NyoT4QKBgH7/" ANSI_NEW_LINE
                  "yx+GhCx1DHtXBPDZFhg2TL+78lEK0oZgk9gp06up2CHzh44SFq6O0oLkTcCUk5Ww" ANSI_NEW_LINE
                  "poTkLIy4mJBlzfgahp+KsK2cO46SZS9g0ONFzcMXt33hWpE2Gl2XhUwPpYTF/QlY" ANSI_NEW_LINE
                  "rDTjZK5S8Mi9dzVSsNlJi7PJphiEK2R1+nFYRwcBAoGBANWoIG85jpXAOnq/Kcgx" ANSI_NEW_LINE
                  "Rl3YivR0Ke6r1tFlP58rT7X3EkiboXyQl5vLIFCAwUte6RGrLl1dy3Qyh80B9ySL" ANSI_NEW_LINE
                  "Jx6vj42CK7vgv6A96TuVYhnXTnEI6ZvwAQ2VGaw4BizhjALs/kdSE/og9aSCs3ws" ANSI_NEW_LINE
                  "KQypwAFz0tbHxaNag/bSAN0J"                                         ANSI_NEW_LINE
                  "-----END PRIVATE KEY-----"                                        ANSI_NEW_LINE;
                unique_ptr<BIO, decltype(&BIO_free)> cbio(
                  BIO_new_mem_buf(tlscrt, -1),
                  BIO_free
                );
                unique_ptr<BIO, decltype(&BIO_free)> pbio(
                  BIO_new_mem_buf(tlskey, -1),
                  BIO_free
                );
                unique_ptr<X509, decltype(&X509_free)> cert(
                  PEM_read_bio_X509(cbio.get(), nullptr, nullptr, nullptr),
                  X509_free
                );
                unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> pkey(
                  PEM_read_bio_PrivateKey(pbio.get(), nullptr, nullptr, nullptr),
                  EVP_PKEY_free
                );
                if (!SSL_CTX_use_certificate(ctx.get(), cert.get())
                  or !SSL_CTX_use_PrivateKey(ctx.get(), pkey.get())
                ) {
                  ctx.reset();
                  warn.emplace_back("Unable to encrypt web clients using internal cert because it seems to be out of date, will fallback to plain text");
                } else  
                  warn.emplace_back("Connected web clients will enjoy unsecure SSL encryption.." ANSI_NEW_LINE
                    "(because the key is visible in the source) See --help argument to setup your own SSL");
              } else {
                if (access(crt.data(), R_OK) == -1)
                  warn.emplace_back("Unable to read SSL .crt file at " + crt);
                if (access(key.data(), R_OK) == -1)
                  warn.emplace_back("Unable to read SSL .key file at " + key);
                if (!SSL_CTX_use_certificate_file(ctx.get(), crt.data(), SSL_FILETYPE_PEM)
                  or !SSL_CTX_use_PrivateKey_file(ctx.get(), key.data(), SSL_FILETYPE_PEM)
                ) {
                  ctx.reset();
                  warn.emplace_back("Unable to encrypt web clients using the .crt and .key files, will fallback to plain text");
                }
              }
            }
            return warn;
          };
          string document(const string &content, const unsigned int &code, const string &type) const {
            string headers;
            if      (code == 200) headers = "HTTP/1.1 200 OK"
                                            ANSI_NEW_LINE "Connection: keep-alive"
                                            ANSI_NEW_LINE "Accept-Ranges: bytes"
                                            ANSI_NEW_LINE "Vary: Accept-Encoding"
                                            ANSI_NEW_LINE "Cache-Control: public, max-age=0";
            else if (code == 401) headers = "HTTP/1.1 401 Unauthorized"
                                            ANSI_NEW_LINE "Connection: keep-alive"
                                            ANSI_NEW_LINE "Accept-Ranges: bytes"
                                            ANSI_NEW_LINE "Vary: Accept-Encoding"
                                            ANSI_NEW_LINE "WWW-Authenticate: Basic realm=\"Basic Authorization\"";
            else if (code == 403) headers = "HTTP/1.1 403 Forbidden"
                                            ANSI_NEW_LINE "Connection: keep-alive"
                                            ANSI_NEW_LINE "Accept-Ranges: bytes"
                                            ANSI_NEW_LINE "Vary: Accept-Encoding";
            else if (code == 418) headers = "HTTP/1.1 418 I'm a teapot";
            else                  headers = "HTTP/1.1 404 Not Found";
            return headers
                 + string((content.length() > 2 and (content.substr(0, 2) == "PK" or (
                     content.at(0) == '\x1F' and content.at(1) == '\x8B'
                 ))) ? ANSI_NEW_LINE "Content-Encoding: gzip" : "")
                 + ANSI_NEW_LINE "Content-Type: "   + type
                 + ANSI_NEW_LINE "Content-Length: " + to_string(content.length())
                 + ANSI_NEW_LINE
                   ANSI_NEW_LINE
                 + content;
          };
        private:
          void accept_request(const curl_socket_t &loopfd) {
            curl_socket_t clientfd = accept4(sockfd, nullptr, nullptr, SOCK_CLOEXEC | SOCK_NONBLOCK);
            if (clientfd == -1) return;
#ifdef __APPLE__
            const int noSigpipe = 1;
            setsockopt(clientfd, SOL_SOCKET, SO_NOSIGPIPE, &noSigpipe, sizeof(noSigpipe));
#endif
            SSL *ssl = nullptr;
            if (ctx) {
              ssl = SSL_new(ctx.get());
              SSL_set_accept_state(ssl);
              SSL_set_fd(ssl, clientfd);
              SSL_set_mode(ssl, SSL_MODE_RELEASE_BUFFERS);
              SSL_set_mode(ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
            }
            requests.emplace_back(clientfd, loopfd, ssl, &session);
          };
          void socket(const int &domain, const int &type, const int &protocol) {
            sockfd = ::socket(domain, type | SOCK_CLOEXEC | SOCK_NONBLOCK, protocol);
            if (sockfd == -1) sockfd = 0;
#ifdef __APPLE__
            else {
              const int noSigpipe = 1;
              setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &noSigpipe, sizeof(noSigpipe));
            }
#endif
          };
      };
  };

  class Random {
    public:
      static unsigned long long int64() {
        static random_device rd;
        static mt19937_64 gen(rd());
        return uniform_int_distribution<unsigned long long>()(gen);
      };
      static string int45Id() {
        return to_string(int64()).substr(0, 10);
      };
      static string int32Id() {
        return to_string(int64()).substr(0,  8);
      };
      static string char16Id() {
        string id = string(16, ' ');
        for (auto &it : id) {
          const int offset = int64() % (26 + 26 + 10);
          if      (offset < 26)      it = 'a' + offset;
          else if (offset < 26 + 26) it = 'A' + offset - 26;
          else                       it = '0' + offset - 26 - 26;
        }
        return id;
      };
      static string uuid36Id() {
        string uuid = string(36, ' ');
        uuid[8]  =
        uuid[13] =
        uuid[18] =
        uuid[23] = '-';
        uuid[14] = '4';
        unsigned long long rnd = int64();
        for (auto &it : uuid)
          if (it == ' ') {
            if (rnd <= 0x02) rnd = 0x2000000 + (int64() * 0x1000000);
            rnd >>= 4;
            const int offset = (uuid[17] != ' ' and uuid[19] == ' ')
              ? ((rnd & 0xF) & 0x3) | 0x8
              : rnd & 0xf;
            if (offset < 10) it = '0' + offset;
            else             it = 'a' + offset - 10;
          }
        return uuid;
      };
      static string uuid32Id() {
        string uuid = uuid36Id();
        uuid.erase(remove(uuid.begin(), uuid.end(), '-'), uuid.end());
        return uuid;
      }
  };

  class Decimal {
    public:
      stringstream stream;
      double step = 0;
    private:
      double tick = 0;
    public:
      Decimal()
      {
        stream << fixed;
      };
      void precision(const double &t) {
        stream.precision(ceil(abs(log10(tick = t))));
        step = pow(10, -1 * stream.precision());
      };
      double round(const double &input) const {
        return ::round((::round(
          input / step) * step)
                / tick) * tick;
      };
      double floor(const double &input) const {
        return ::floor((::floor(
          input / step) * step)
                / tick) * tick;
      };
      string str(const double &input) {
        stream.str("");
        stream << round(input);
        return stream.str();
      };
  };

  class Files {
    public:
      static bool mkdirs(const string &file) {
        size_t has_dir = file.find_last_of("/\\");
        if (has_dir != string::npos) {
          string dir = file.substr(0, has_dir);
          if (access(dir.data(), R_OK) == -1) {
            mkdir(dir.data()
#ifndef _WIN32
            , 0775
#endif
            );
            if (access(dir.data(), R_OK) == -1)
              return false;
          }
        }
        return true;
      }
  };
}
