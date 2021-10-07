FROM debian:buster-slim


RUN apt-get update \
    && apt-get install --no-install-recommends -y git sudo make ca-certificates curl g++

# Feel free to choose the branch you want to build:
RUN git clone -b master https://github.com/ctubio/Krypto-trading-bot.git /K

WORKDIR /K

RUN make docker && rm -rf /var/lib/apt/lists/

# See examples and descriptions of the
# following variables at etc/K.sh.dist.

ENV OPTIONAL_ARGUMENTS --colors --port 3000

ENV K_BINARY_FILE K-trading-bot

#CMD ["./K.sh", "--naked", "--without-ssl"]
