#ifndef __ASSERT_H__
#define __ASSERT_H__

void panic_mode (const char* msg, const char* file, int line);

#define assert(ex) ((ex) ? 1 : error())
#define panic(msg) panic_mode (msg, __FILE__, __LINE__)

#endif
