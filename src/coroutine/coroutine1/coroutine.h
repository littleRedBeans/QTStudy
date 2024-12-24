#ifndef C_COROUTINE_H
#define C_COROUTINE_H
/*
The MIT License (MIT)
    
    Copyright (c) 2013 codingnow.com Inc.
    
    Permission is hereby granted, free of charge, to any person obtaining a copy of
    this software and associated documentation files (the "Software"), to deal in
    the Software without restriction, including without limitation the rights to
    use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
                                                              the Software, and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:
                                          
                                          The above copyright notice and this permission notice shall be included in all
                                          copies or substantial portions of the Software.
                                             
                                             THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
                                                                    FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
                                                                    COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
    IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
                                     CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
//#ifdef __cplusplus
//extern "C" {
//#endif
#include <functional>
#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3

struct schedule;

//typedef void (*coroutine_func)(struct schedule *, void *ud);
using coroutine_func = std::function<void(struct schedule *, void *ud)>;

struct schedule *coroutine_open(void);
void coroutine_close(struct schedule *);

int coroutine_new(struct schedule *, const coroutine_func &, void *ud);
void coroutine_resume(struct schedule *, int id);
int coroutine_status(struct schedule *, int id);
int coroutine_running(struct schedule *);
void coroutine_yield(struct schedule *);
//#ifdef __cplusplus
//}
//#endif

#endif
