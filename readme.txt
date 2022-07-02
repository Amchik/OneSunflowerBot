One Sunflower Bot 🌻
=================
A simple matrix bot.
Written in C. In future may be supports Lua...

1. Build and run
================
In three lines:
$ suapt i libcjson-dev libcurl4-openssl-dev
$ make
$ bin/onesunflowerbot [...]

1.1 Dependencies
----------------
* libcurl  -- send requests to matrix homeserver
* libcjson -- json parsing

Install on debian/ubuntu:
$ sudo apt install libcjson-dev libcurl4-openssl-dev

1.2 Building
------------
$ make release
Also, you can set TARGET, like that:
$ make TARGET=RELEASE
Default target is DEBUG

TARGETs:
 * RELEASE -- with optimizations
 * DEBUG   -- with debug symbols and wo/ optimizations
 * CLANG   -- like RELEASE, but build and link with clang
See makefile to more information about creating own target

1.3 Usage
---------
$ bin/onesunflowerbot [--homeserver|-h <homeserver>]
                      [--token|-t <token>]
                      [--tokenfile|-f <token file>]
Without arguments bot will use token from .matrix_token
and homeserver "matrix-client.matrix.org"

Token file is just file with bot token:
$ echo "$SECRET_MATRIX_TOKEN" > .matrix_token

2. Adding new commands
======================
Create new file in src/commands/...
and put it:

 #include "../include/matrix-autocmd.h"

 void cmd_ping(MatrixBotContext ctx, MatrixEventMessage *msg) {
   matrixbot_qreply(ctx, "Pong!");
 }

 mxmod {
   mxautocmd_register("!ping", MXAUTOCMD_COMMAND, (void (*)(MatrixBotContext ctx, ...)cmd_ping));
 }

2.1 Sending events/messages
---------------------------
Low-level function:
 #include "include/matrix-client.h"
 
 matrix_request(client, method, path, query?, body?)
   Send request to matrix homeserver
 matrix_sync(client, query?)
   Sync with matrix homeserver (and mutate client)
 matrix_state(client, room_id, event_type, body?)
   Sets state in room_id
 matrix_send(client, room_id, event_type, body?)
   Send event to room_id
 matrix_redact(client, room_id, event_id, reason?)
   Redacts event in room_id

High-level functions:
 #include "include/matrix-bot.h"

 matrixbot_qsync(bot)
   Quiet sync (free result)
 matrixbot_send(ctx, message)
   Send message
 matrixbot_reply(ctx, message)
   Reply to event
 matrixbot_sendf(ctx, fmt, ...)
   Send formatted message
 matrixbot_replyf(ctx, fmt, ...)
   Reply to event with formatted content

 matrixbot_qsend(ctx, message)
   Send message quietly
 matrixbot_qreply(ctx, message)
   Reply to event quietly
 matrixbot_qsendf(ctx, fmt, ...)
   Send formatted message quietly
 matrixbot_qreplyf(ctx, fmt, ...)
   Reply to event with formatted content quietly

2.2 More
--------
For more docs see docs.txt


3. To-Do
========
- [ ] Lua?
- [ ] Rewrite this bot to rust (assigned to @nanoqsh)
      Call it like One Super-Secure-Without-Null-And-Other
        -UB-things--And-One-Thread-Super-Thread-Safe Bot ☕

---
Amchik <you-can-find-my-email-in-commits@example.com>
Sat Jul  2 08:46:05 PM UTC 2022 [date --utc]

