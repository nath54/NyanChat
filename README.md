# NyanChat

![](res/NyanChat_bg_black.png)

**NyanChat** is a multi-user chat with error detection and correction realized by CERISARA Nathan, JACQUET Ysée and JAYAT Adrien.

## Get Started

First, clone the project and its submodules:

```sh
git clone https://github.com/nath54/NyanChat
cd NyanChat
git submodule init && git submodule update
```

### Dependencies

#### Ubuntu

```sh
sudo apt update
sudo apt install make gcc doxygen
```

#### Arch Linux

```sh
sudo pacman -Syu
sudo pacman -S make gcc doxygen
```

### Compilation

Go in `cd chemin/de/NyanChat/` if you're not already in,  then compile with `make`.

### Application launch

- Create a server

```sh
./build/bin/server SERVER_PORT
```

In case you get the error `Error during bind call -> : Address already in use`, change the `SERVER_PORT`.

- Create a proxy with

```sh
./build/bin/proxy IP_SERVEUR SERVER_PORT CLIENTS_LISTENING_PORT
```

If you're testing this application localy, the server IP will be `127.0.0.1` (and will be the proxy one too).
In case you get the error `Error during bind call -> : Address already in use`, change the `CLIENTS_LISTENING_PORT`.

- Launch client interface with

```sh
./build/bin/client_ansi IP_PROXY PORT_PROXY 2> log.txt
```

If you're testing this application localy, the proxy IP will be `127.0.0.1`. Be careful to **not** use the same port for server and proxy.

In case the application does not launch, check `log.txt` (with `cat log.txt` for example).

### Application usage

Once everything is set, you will be first invited to enter your pseudo. Then you can send as many messages as desired!

## Architecture

Only one server, only one proxy (theorically more) and many clients.
The proxy will be able to add errors on messages from clients.
The server will be able to detect and correct errors as best as possible.

For more details, please read the report.
