### K.sh.dist
Used on install to initialize `./K.sh` file, feel free to add your own hardcoded arguments to your own `./K.sh` file after install.

### ../src/lib/Krypto.ninja-client/www/.bomb.gzip
Used by `--whitelist` argument to attempt to crash UI clients from alien IPs not whitelisted; no need to open.

### Dockerfile
To run K.sh with Docker, please make use of the [Dockerfile](https://raw.githubusercontent.com/ctubio/Krypto-trading-bot/master/etc/Dockerfile):

1. Install [docker](https://www.docker.com/) for your system before proceeding. Requires at least Docker 1.7.1. Mac/Windows only: Ensure boot2docker or docker-machine is set up, depending on Docker version. See [the docs](https://docs.docker.com/installation/mac/) for more help.
2. Set the following environment variables. For further information of the variables, look into K.sh.dist
```
API_EXCHANGE=BINANCE
API_CURRENCY=BTC/EUR
API_KEY=MyAPIKey
API_SECRET=MyAPISecret
API_PASSPHRASE=
```
3. Build and run the docker
```
 $ cd path/to/Dockerfile
 $ docker build --build-arg API_EXCHANGE=$API_EXCHANGE --build-arg API_CURRENCY=$API_CURRENCY --build-arg API_KEY=$API_KEY --build-arg API_SECRET=$API_SECRET --build-arg API_PASSPHRASE=$API_PASSPHRASE --no-cache -t ksh .
 $ docker run -p 3000:3000 --name Ksh -t -d ksh
```
If you want to ensure that your data is persisted, mount a local folder into the container's `/data` folder:
```
$ docker run -p 3000:3000 -v /path/to/data:/data --name Ksh -t -d ksh
```

If you run `docker ps`, you should see K container running.

### Github CI/CD on any *nix Server
Tested on AWS with Ubuntu 20.04, but should work on other *nix server with ssh as well
1. Fork the repository
2. Edit the .github/workflows/deploy.yml by removing 'x' in front of xmaster, which will trigger the deployment on master after push
3. Add following Secrets to Github
```
SSH_HOST=192.168.0.1
SSH_USER=ubuntu
SSH_KEY=id_rsa content
DOT_ENV_BE=the content of .env file with look basically like:
        API_EXCHANGE=BINANCE
        API_CURRENCY=BTC/EUR
        API_KEY=MyAPIKey
        API_SECRET=MyAPISecret
        API_PASSPHRASE=
```

### Vagrantfile
To build your own portable development environment install [VirtualBox](https://www.virtualbox.org/wiki/Downloads) and [vagrant](https://www.vagrantup.com/downloads.html), then:
```
 $ cd path/to/K
 $ cp etc/Vagrantfile Vagrantfile
 $ vagrant up
 $ vagrant ssh
```
See more info at [PR #425](https://github.com/ctubio/Krypto-trading-bot/pull/425).
