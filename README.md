# Matrix-Stream

Stream images over TCP

## What

This repo contains a rudimentary library to implement a server or client.
There is also an example server which draws on a [led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix) (called matrix-stream) and a simple client for test purposes (test-client).
The server example requires [rpi-rgb-led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix) to compile and work.

## Why

I wrote this to be able to write simple games on my main computer and render them on my led-matrix which is bound to a raspberry pi.

## How

There are 4 targets: bin, bin-linked, test-client and lib:
```
make lib
```
