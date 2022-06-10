One Sunflower Bot 🌻
=================
A matrix bot that can nothing!
Written in C. In future may be supports Lua...

1. Dependencies
===============
This application require 2 libs:
* libcurl   -- make requests to matrix
* libjson-c -- json...

Note: i use libjson-c 0.16-1 (debian sid, by the way),
      but some distros have libjson-c version 0.15.x.

Install on debian (or ubuntu):
$ sudo apt install libjson-c-dev libcurl4-openssl-dev
Pro Tip: use suapt (see Amchik/suapt)

2. Usage
========
$ make check
shows cool results (no)

For now: noway. See to-do.

By the way, if you need Hello-world fabric, you
can use this bot. It's just puts("Hello, world!")

3. To-Do
========
- [ ] Write matrix-client.h
 - [ ] Impl matrix_{request,sync,send}
- [ ] Write matrix-bot.h -- h-lvl wrapper for matrix-client
 - [ ] struct MatrixBot that contains MatrixClient and unhandled events
 - [ ] struct MatrixBotHandlers that contains handlers for events:
       void (*handle_room_message)(...), etc...
 - [ ] functions like matrix_reply, etc...
...
- [ ] Rewrite this bot to rust (assigned to @nanoqsh)
      Call it like One Super-Secure-Without-Null-And-Other
        -UB-things--And-One-Thread-Super-Thread-Safe Bot ☕

4. GitHub actions
=================
ubuntu-latest have libjson-c-dev version 0.15.x, but this bot
 requires 0.16.x. Solution: do not include <json-c/json_types.h>.
Use <json-c/json.h> instead <json-c/json_*.h>, please.

