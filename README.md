# CPP-Homework
The repository contains homeworks for C++ course in ITMO university.

`echo_server` directory contains echo server in C++ implemented using extremely low primitives: system calls, epoll, file descriptors. Clients can connect to the server and send messages to it, while it answers with same text. Server is implemented in a nice manner with library interface, so it's done in cool Object-Oriented approach with inheritance & polymorphism. Moreover, attention was paid to handling exceptions and saving system resources. For example, client, which does not send messages for specified amount of time is being disconnected. Or client, who only does writing to the server, but not reading will also be soon disconnected, because size of buffer, which stores data, written by client, is limited. Project was created in Spring 2017.    

ASM directory contains code written in assembler. There are a few programs there in pure assembler, they do long arithmetics operations. Moreover, there are C++ programs there with assembler inlines. They do counting number of words in string using vectorization and memory copying. Programs were written during a **C++ language** course in Spring 2016 and also during an **Assembler programming** course in Spring 2017. See [README](asm/) file in asm directory for detailed information.

CPP directory contains code in C++. There are data structures, implementations of standart library functions and class for working with big integers there. Directory contains code written for an exam and just as homework also during **C++ language** course in Spring 2016, Autumn 2016. See specific [README](cpp/) if interested.



