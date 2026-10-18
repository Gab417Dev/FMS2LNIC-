# 01 - POSIX echo server
A blocking TCP echo server written with raw POSIX sockets.
It accepts one client at a time and sends back whatever it receives.

## Build and run
```
cmake -S . -B build
cmake --build build
./build/bin/posix_echo_server
```

## How to test
```
nc 127.0.0.1 12345
```

Write something and it should come back.

## How it works
The first thing that it does is creating a listening socket, whose job is to accept new connections, then it gives
the listening socket an option "**SO_REUSEADDR**", which allows restarting the server without errors, after that we 
set up the server address at 127.0.0.1:12345, bind it to the socket and then listen for a connection, after all of this
we wait for a client with accept and start the echo loop of reading the client input, checking errors and sending
the same buffer to the client, after the client disconnects or gets an error the server waits for another until its
forcefully closed

## Known limitations
- Only one client at a time.

## Resources
- `man 7 tcp`
- `man 2 socket`
- `man 7 socket`
- `man 2 accept`
- `man 2 listen`
- `man 2 send`
- `man 2 write`
- `man 2 read`
- `man 2 bind`