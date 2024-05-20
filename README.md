# NyanChat

![](res/NyanChat_bg_black.png)

NyanChat is a multi-user chat with error detection and correction realized by CERISARA Nathan, JACQUET Ysée and JAYAT Adrien.

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

## Architecture

Only one server, only one proxy and many clients.
The proxy will be able to add errors on messages from clients.

[TODO: Faire le Readme.md]
