ERR      = *** Unexpected MAKELEVEL = 0.
HINT     = This Makefile can't be used directly, consider cd ../.. before try again
$(if $(subst 0,,${MAKELEVEL}),,$(warning $(ERR));$(error $(HINT)))

V_ZLIB  = 1.3.1
V_SSL   = 3.3.2
V_CURL  = 8.10.1
V_SQL   = 3460100
V_JSON  = 3.11.3
V_CATCH = 2.13.10
V_UV    = 1.49.0

KBUILD := $(abspath ../../build-$(KHOST))

all lib:
	# my home is your home, if there are houses owned by someone.
	@mkdir -vp $(KBUILD)/var
	$(MAKE) -j4 j0 j1 j2 j3

j0:
	$(MAKE) -j1 zlib openssl curl

j1:
	$(MAKE) -j1 sqlite

j2:
	$(MAKE) -j1 json catch uv

j3:
ifdef GITHUB_ACTIONS
	$(MAKE) -j1 lcov pvs
endif

clean:
	# after some time, the youth goes away, and I will leave a world behind.
	# don't be sad, life is life, and this world will never forget our time.
	@rm -vrf $(KBUILD)

check:
	# zlib    | $(V_ZLIB)   | $(shell curl -s https://www.zlib.net/ChangeLog.txt                    | grep "Changes in " | head -n1 | cut -d' ' -f3)
	# openssl | $(V_SSL)   | $(shell  curl -s https://api.github.com/repos/openssl/openssl/releases | grep "tag_name" | grep $(shell curl -s https://openssl-library.org/source/ | grep "The latest stable version is " | cut -d' ' -f8) | head -n1 | cut -d'o' -f2 | cut -d'-' -f2 | cut -d'"' -f1)
	# curl    | $(V_CURL)  | $(shell  curl -s https://curl.se/changes.html                          | grep "Fixed in " | head -n1 | cut -d' ' -f4)
	# sqlite  | $(V_SQL) | $(shell    curl -s https://www.sqlite.org/download.html                  | grep "sqlite-amalgamation-" | head -n1 | cut -d'-' -f3 | cut -d'.' -f1)
	# json    | $(V_JSON)  | $(shell  curl -s https://api.github.com/repos/nlohmann/json/releases   | grep "tag_name" | head -n1 | cut -d'v' -f2 | cut -d'"' -f1)
	# catch   | $(V_CATCH) | $(shell curl -s https://api.github.com/repos/catchorg/Catch2/releases | grep "tag_name" | grep "v2" | head -n1 | cut -d'v' -f2 | cut -d'"' -f1)
	# uv      | $(V_UV)  | $(shell    curl -s https://api.github.com/repos/libuv/libuv/releases     | grep "tag_name" | head -n1 | cut -d'v' -f2 | cut -d'"' -f1)

zlib:
	test -d $(KBUILD)/var/zlib-$(V_ZLIB) || (                                  \
	curl -L https://zlib.net/zlib-$(V_ZLIB).tar.gz | tar xz -C $(KBUILD)/var   \
	&& cd $(KBUILD)/var/zlib-$(V_ZLIB) && $(if $(findstring mingw32,$(CHOST)), \
	DESTDIR=$(KBUILD)/ BINARY_PATH=bin INCLUDE_PATH=include LIBRARY_PATH=lib   \
	make all install -fwin32/Makefile.gcc PREFIX=$(CHOST)-,                    \
	./configure --static --prefix=$(KBUILD) && make all install)               )

openssl:
	test -d $(KBUILD)/var/openssl-$(V_SSL) || (                                                           \
	curl -L https://github.com/openssl/openssl/releases/download/openssl-$(V_SSL)/openssl-$(V_SSL).tar.gz \
	| tar xz -C $(KBUILD)/var && cd $(KBUILD)/var/openssl-$(V_SSL) && CC=gcc ./Configure                  \
	no-asm no-psk no-srp no-idea no-ssl3 no-weak-ssl-ciphers --openssldir=$(KBUILD)                       \
	$(if $(findstring mingw32,$(CHOST)),mingw64 -static --libdir=lib,gcc)                                 \
	--prefix=$(KBUILD) --cross-compile-prefix=$(CHOST)- && make install_dev                               )

curl:
	test -d $(KBUILD)/var/curl-$(V_CURL) || (                                                       \
	curl -L https://curl.haxx.se/download/curl-$(V_CURL).tar.gz | tar xz -C $(KBUILD)/var           \
	&& cd $(KBUILD)/var/curl-$(V_CURL) && mkdir -p ssl && curl --etag-compare ssl/cacert.etag       \
	--etag-save ssl/cacert.etag https://curl.se/ca/cacert.pem -o ssl/cacert.pem                     \
	&& PKG_CONFIG="pkg-config --static" PKG_CONFIG_PATH=$(KBUILD)/lib/pkgconfig ./configure         \
	--host=$(CHOST) --prefix=$(KBUILD) --with-zlib=$(KBUILD)                                        \
	$(if $(findstring mingw32,$(CHOST)),--with-schannel,--with-openssl=$(KBUILD)                    \
	--with-ca-embed=$(KBUILD)/var/curl-$(V_CURL)/ssl/cacert.pem)                                    \
	--disable-docs --disable-manual  --disable-gopher  --disable-imap   --disable-ftp               \
	--disable-file --without-librtmp --disable-telnet  --disable-pop3   --disable-smb               \
	--disable-rtsp --without-libidn2 --without-zstd    --disable-smtp   --disable-ldap              \
	--disable-tftp --without-libssh2 --without-libpsl  --disable-sspi   --disable-ares              \
	--disable-dict --without-nghttp2 --without-brotli  --disable-shared --enable-static             \
	&& make && (cd lib && make install) && (cd include && make install)                             \
	&& (echo "#define curl_easy_setopt_nowin32(a, b, c) $(if $(findstring mingw32,$(CHOST)),        \
	,curl_easy_setopt(a, b, c))" && echo "`grep extern src/tool_ca_embed.c |                        \
	cut -d';' -f1`;") > $(KBUILD)/include/curl/cacert_embed.h                                       \
	&& cp -v src/tool_ca_embed.o $(KBUILD)/lib/cacert_embed.o                                       )

sqlite:
	test -d $(KBUILD)/var/sqlite-autoconf-$(V_SQL) || (                                       \
	curl -L https://sqlite.org/2024/sqlite-autoconf-$(V_SQL).tar.gz | tar xz -C $(KBUILD)/var \
	&& cd $(KBUILD)/var/sqlite-autoconf-$(V_SQL) && ./configure --prefix=$(KBUILD)            \
	--host=$(CHOST) --enable-static --disable-dynamic-extensions --disable-shared             \
	--enable-threadsafe && make install-libLTLIBRARIES install-includeHEADERS                 )

json:
	test -f $(KBUILD)/include/json.h || (mkdir -p $(KBUILD)/include                   \
	&& curl -L https://github.com/nlohmann/json/releases/download/v$(V_JSON)/json.hpp \
	-o $(KBUILD)/include/json.h &&                                                    \
	sed -i '$$iusing namespace nlohmann;' $(KBUILD)/include/json.h                    )

catch:
	test -f $(KBUILD)/include/catch.h || (mkdir -p $(KBUILD)/include                      \
	&& curl -L https://github.com/catchorg/Catch2/releases/download/v$(V_CATCH)/catch.hpp \
	-o $(KBUILD)/include/catch.h                                                          )

uv:
	$(if $(findstring mingw32,$(CHOST))$(findstring darwin,$(CHOST)),                                 \
	test -d $(KBUILD)/var/libuv-$(V_UV) || (                                                          \
	curl -L https://github.com/libuv/libuv/archive/v$(V_UV).tar.gz | tar xz -C $(KBUILD)/var          \
	&& cd $(KBUILD)/var/libuv-$(V_UV) && sh autogen.sh && CC=$(CHOST)-gcc ./configure --host=$(CHOST) \
	--prefix=$(KBUILD) --disable-shared --enable-static && make all install)                          )

pvs:
ifndef V_PVS
	$(MAKE) $@ V_PVS=$(shell curl -s https://pvs-studio.com/en/pvs-studio/download-all/ | grep x86_64.tgz | cut -d'"' -f2 | cut -d'.' -f-4 | cut -d'-' -f3- | head -n1)
else
	test -d $(KBUILD)/var/pvs-studio-$(V_PVS) || (                    \
	curl -L https://files.pvs-studio.com/pvs-studio-$(V_PVS).tgz      \
	| tar xz -C $(KBUILD)/var && cd $(KBUILD)/var/pvs-studio-$(V_PVS) \
	&& chmod +x install.sh && sudo ./install.sh                       )
endif

lcov:
ifndef V_LCOV
	$(MAKE) $@ V_LCOV=$(shell curl -s https://api.github.com/repos/linux-test-project/lcov/releases/latest | grep name | grep -v 'tag\|lcov' | cut -d '"' -f4 | head -n1)
else
	test -d $(KBUILD)/var/lcov-$(V_LCOV) || (                                                             \
	curl -L https://github.com/linux-test-project/lcov/releases/download/v$(V_LCOV)/lcov-$(V_LCOV).tar.gz \
	| tar xz -C $(KBUILD)/var && cd $(KBUILD)/var/lcov-$(V_LCOV) && sudo make install                     )
endif

.PHONY: all lib clean check zlib openssl curl sqlite json catch uv lcov pvs
