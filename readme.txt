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

2.1. matrix-client.h
--------------------
matrix-client.h is a middle-level (because it not with strings)
API that can sends basic requests to homeserver.

2.1.1. Init client
------------------
#include "include/matrix-client.h"

MatrixClient client;
memset(&client, 0, sizeof(client)); /* or set only .batch */
strncpy(client->homeserver, "example.com", sizeof(client->homeserver));
strncpy(client->access_token, "super-secret-information", sizeof(client->access_token));

2.1.2. First sync
-----------------
First, we need to get batch. matrix_sync function also sets it on
request:

json_object *json;
json = matrix_sync(&client, 0 /* do not pass query params */);
json_object_put(json); /* if you don't need it */

To get events call it again. matrix_sync will pass state after first call.

2.1.3. Sending events
---------------------
Simple m.room.message event looks like that:

srand(time(0)); /* only one time, see 2.1.5 */
char *s;
s = matrixnode_stringify(matrix_newnode("msgtype", L"m.notice", 0 /* flags, just ignore */),
    &matrix_newnode("body", L"Hello, world!", 0, 0 /* NULL */));
json_object *json;
json = matrix_send(&client, "!myroomid:example.com", "m.room.message", s);
free(s);
/* work with json object... */
json_object_put(json); /* free it */

Hard-way is using json-c library or hardcode event body:
...
json = matrix_send(&client, "!myroomid:example.com", "m.cool.event",
    "{ \"temp\": { \"number\": 160, \"formatted\": \"160 K\" } }");
...

Also you can redact event using matrix_redact().
To set room state use matrix_state().

2.1.4. Working with MatrixNode
------------------------------
Three functions (one macro):
 * MatrixNode matrix_newnode(key, value, flags, next)
    (macro) creates new MatrixNode
 * char* matrixnode_stringify(node)
    Returns JSON string of node. (string need to be free)
 * void matrixnode_free(node)
    Free matrix node by flags

Node flags:
 * MATRIXNODE_FREENODE
    Node object will be free (without key & value)
 * MATRIXNODE_FREEKEY
    Node key will be free
 * MATRIXNODE_FREEVALUE
    Node value will be free
You can combine it. If node haven't any flags it will be
 skipped in matrixnode_free.

2.1.5. Known problems
---------------------
In matrix_{state, send, redact}() function txnId sets by 2 rand() output.
To send messages correctly application needs srand(time(0)).
Also, it may be can be not unique and events can be not delivered.

3. To-Do
========
- [x] Write matrix-client.h
 - [x] Impl matrix_{request,sync,state,send,redact}
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

