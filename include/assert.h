#ifndef __ASSERT_H__
#define __ASSERT_H__

void panic_mode(const char *msg, const char *file, int line, const char*func);

#define assert(ex) ((ex) ? 1 : panic("Assert Failed"))
#define panic(msg) panic_mode (msg, __FILE__, __LINE__, __func__)

#endif
