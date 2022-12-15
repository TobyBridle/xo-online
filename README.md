Previously, there was a branch which used multi-threading and forks
for handling connections and terminal input simultaneously; however, the code
was not readable and the implementation was not non-blocking.
Thus, the no-multithreading thread has emerged.
This has been merged into main and focuses on using non-blocking calls
to handle connections. Once that has been achieved, the use of multiple
threads for handling seperate games and server control will be implemented.

<b>May be useful..</b>

- [RFC 9110](https://www.rfc-editor.org/rfc/rfc9110)
