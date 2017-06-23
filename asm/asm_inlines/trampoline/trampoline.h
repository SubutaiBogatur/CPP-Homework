#ifndef TRAMPOLINE_H
#define TRAMPOLINE_H

#include <string>
#include <iostream>
#include "argument_types.h"
#include "allocator.h"

template <typename T>
struct trampoline;

template<typename T, typename ... Args>
void swap(trampoline<T (Args...)>& a, trampoline<T (Args...)>& b);

template <typename T, typename ... Args>
struct trampoline<T (Args ...)>
{
private:
    void *page;
    char *ip; //current instruction pointer

    void* func_obj;
    void (*deleter)(void*);

    struct imm
    {
        union
        {
            void *addr;
            int32_t i;
        };
        size_t size;

        imm(void *addr, size_t size):size(size), addr(addr){}
        imm(int32_t i, size_t size):size(size), i(i){}
    };

    const char* shifts[6] = {
        "\x48\x89\xfe" /*mov rsi rdi*/,
        "\x48\x89\xf2" /*mov rdx rsi*/,
        "\x48\x89\xd1" /*mov rcx rdx*/,
        "\x49\x89\xc8" /*mov r8 rcx;*/,
        "\x4d\x89\xc1" /*mov r9 r8;*/,
        "\x41\x51" /*push %%r9;*/
    };

    //function to add instructions
    void add() {}

    void add(std::string command)
    {
        for (const char *i = command.c_str(); *i; i++) *(ip++) = *i;
    }

    void add(imm im)
    {
        *(void**)ip = im.addr;
        ip += im.size;
    }

    template<typename ... A>
    void add(std::string str, A... args)
    {
        add(str);
        add(args...);
    }

    template<typename ... A>
    void add(imm im, A... args)
    {
        add(im);
        add(args...);
    }

public:
    template <typename F>
    trampoline(F func) : func_obj(new F(std::move(func))), deleter(my_deleter<F>)
    {
        page = allocator::get_instance().malloc();
        ip = (char*)page;

        if (args_types<Args ...>::INTEGER < 6)
        {
            // shift all the arguments
            for (int i = args_types<Args ...>::INTEGER - 1; i >=0 ; i--) add(shifts[i]);

            add("\x48\xbf", imm(func_obj, 8),                   // mov  rdi, ptr_to_func_obj
                "\x48\xb8", imm((void *)&do_call<F>, 8),        // mov  rax, address_of_do_call
                "\xff\xe0");                                    // jmp  rax
        }
        else
        {
            // save return address, which is in on top of stack
            add("\x4c\x8b\x1c\x24");                            // mov  r11 [rsp]

            // shift all the args and push last to the stack
            for (int i = 5 ; i >= 0; i--) add(shifts[i]);

            // maximum possible size of arguments on stack
            int stack_size = 8 * (args_types<Args ...>::INTEGER - 5 + std::max(args_types<Args ...>::SSE - 8, 0));

            // now let's get stack ready for new call
            add("\x48\x89\xe0",                                 // mov  rax, rsp
                "\x48\x05", imm(stack_size, 4),                 // add  rax, stack_size
                "\x48\x81\xc4", imm(8, 4));                     // add  rsp, 8

            char* label_1 = ip; //cycle for shifting

            add("\x48\x39\xe0",                                 // cmp  rax, rsp
                "\x74\x12",                                     // je   12            (*sorry very much, it does jumping 12 lines below *)
                "\x48\x81\xc4", imm(8, 4),                      // add  rsp, 0x00000008
                "\x48\x8b\x3c\x24",                             // mov  rdi, [rsp]
                "\x48\x89\x7c\x24\xf8",                         // mov  [rsp - 8], rdi
                "\xeb");                                        // jmp  label_1
            *ip = label_1 - ip++ - 1; //placing label wher to jump

            //now all args in stack are shifted:
            add("\x4c\x89\x1c\x24",                             // mov  [rsp], r11
                "\x48\x81\xec", imm(stack_size, 4),             // sub  rsp, stack_size
                "\x48\xbf", imm(func_obj, 8),                   // mov  rdi, func_obj
                "\x48\xb8", imm((void *)&do_call<F>, 8),        // mov  rax, address_of_do_call
                "\xff\xd0");                                    // call rax             (*let's do what this all was for*)

            //function called, mission completed. Now let's restore previous state of stack, here hack is used
            add("\x41\x59",                                     // pop   r9
                "\x4c\x8b\x9c\x24", imm(stack_size - 8, 4),     // mov   r11, [rsp + stack_size - 8]
                "\x4c\x89\x1c\x24",                             // mov   [rsp], r11
                "\xc3");                                        // ret           (* returning to base, comanches *)
        }
    }

    template <typename F>
    static T do_call(void* obj, Args ...args) {
        return (*(F*)obj)(args...);
    }

    T (*get() const)(Args ... args) {
        return (T(*)(Args ... args))page;
    }

    trampoline& operator=(const trampoline& other) = delete;
    trampoline& operator=(trampoline&& other)
    {
        std::cout << "in = constructor\n";
        trampoline tmp(std::move(other));
        swap(*this, tmp);
        return *this;
    }

    trampoline(const trampoline& other) = delete;
    trampoline(trampoline&& other)
    {
        func_obj = other.func_obj;
        page = other.page;
        deleter = other.deleter;
        other.func_obj = nullptr;
    }

    friend void swap<>(trampoline& a, trampoline& b);

    ~trampoline()
    {
        if (func_obj) deleter(func_obj);
        allocator::get_instance().free(page);
    }

private:


    template <typename F>
    static void my_deleter(void* func_obj) {
        delete static_cast<F*>(func_obj);
    }
};

template<typename T, typename... Args>
void swap(trampoline<T (Args...)>& a, trampoline<T (Args...)>& b)
{
    std::swap(a.func_object, b.func_object);
    std::swap(a.code, b.code);
    std::swap(a.deleter, b.deleter);
}

#endif
