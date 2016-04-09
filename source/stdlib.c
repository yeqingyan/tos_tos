#include <kernel.h>

/* 
 * charToBin()
 * -----------
 *  Convert a ASCII character into decimal value, then put it into binary array
 *  For example, 'a' = 97, binary number is 01100001, the content of binary array will be [0,1,1,0,0,0,0,1]
 *
 *  Parameters:
 *  c: input char
 *  buf: output binary array
 *  length: the binary length
 *
 */
void charToBin(unsigned char c, int* buf, const int length) {
        int i, j;
        for (i=0, j=length-1; i<length; i++, j--) {
                *(buf+j) = (c >> i) & 1; 
        }
}

/*
 * printnum()
 * ----------
 *  Translate a number into string.
 *
 *  Parameters:
 *  b:          buff store the string.
 *  u:          input number
 *  base:       indicate the base of numerical notation. 
 *  negflag:    indicate the number is negative or not
 *  length:     the string length
 *  ladjust:    indicate whether it need to pad the space
 *  padc:       pad character
 *  upcase: the hexadecimal using upcase or lowercase.
 */
#define MAXBUF (sizeof(long int) * 8) /* MAX length of the string */
char *printnum(char *b, unsigned int u, int base, 
                BOOL negflag, int length, BOOL ladjust, 
                char padc, BOOL upcase)
{
        char buf[MAXBUF];       /* build number here */
        char *p = &buf[MAXBUF-1];
        int size;
        char *digs;
        static char up_digs[]   = "0123456789ABCDEF";
        static char low_digs[]  = "0123456789abcdef";
        
        digs = upcase ? up_digs : low_digs;
        do {
                *p-- = digs[u % base];
                u /= base;
        } while (u != 0);

        if (negflag) {
                *b++ = '-';
        }

        size = &buf[MAXBUF - 1] - p;
        
        if (size < length && !ladjust) {
                while (length > size) {
                        *b++ = padc;
                        length--;
                }
        }

        while (++p != &buf[MAXBUF]) {
                *b++ = *p;
        }

        if (size < length) {
                while (length > size) {
                        *b++ = padc;
                        length--;
                }
        }
        return b;
}

/*
 * vs_printf()
 * ----------
 *  We use vs_printf() to avoid conflict with the built-in function vsprintf()
 *  This version implements therefore follwing printf features:
 *    
 *    %d: decimal conversion
 *    %u: unsigned conversion
 *    %x: hexadecimal conversion
 *    %X: hexadecimal conversion with capital letters
 *    %o: octal conversion
 *    %s: string
 *    %m.n:     field width, precision
 *    %-m.n:    left adjustment
 *    %0m.n:    zero-padding
 *    %*.*      width and precision taken from arguments
 *
 *    This version does not implement %f, %e, or %g. It accepts, but 
 *    ignores, an 'l' as in %ld, %lo, %lx, and %lu, and therefore will 
 *    not work correctly on machines for which sizeof(long) != sizeof(int).
 *    It does not even parse %D, %O or %U; you should be using %ld, %o and 
 *    %lu if you mean long conversion
 *
 *    This versino implements the following nonstandard features:
 *
 *    %b: binary conversion
 *
 *    Parameters:
 *    buf:      buf to store string
 *    fmt:      format used by printf
 *    argp:     parameters for fmt
 */
#define isdigit(d) ((d) >= '0' && (d) <= '9')
#define ctod(c) ((c) - '0')
void vs_printf(char *buf, const char *fmt, va_list argp){
        char            *p;
        char            *p2;
        int             length;
        int             prec;
        int             ladjust;
        char            padc;
        int             n;
        unsigned int    u;
        int             negflag;
        char            c;

        while (*fmt != '\0') {
                if (*fmt != '%') {
                        *buf++ = *fmt++;
                        continue;
                }
                fmt++;
                if (*fmt == 'l'){
                        fmt++;          /* need to use it if sizeof(int) < sizeof(long) */
                }

                length = 0;
                prec = -1;
                ladjust = FALSE;
                padc = ' ';

                if (*fmt == '-') {
                        ladjust = TRUE;
                        fmt ++;
                }

                if (*fmt == '0') {
                        padc = '0';
                        fmt++;
                }

                if (isdigit(*fmt)) {
                        while(isdigit (*fmt)) {
                                length = 10 * length + ctod(*fmt++);
                        }
                } else if (*fmt == '*') {
                        length = va_arg(argp, int);
                        fmt++;
                        if (length < 0) {
                                ladjust = !ladjust;
                                length = -length;
                        }
                }

                if (*fmt == '.') {
                        fmt++;
                        if (isdigit(*fmt)) {
                                prec = 0;
                                while(isdigit (*fmt)) {
                                        prec = 10 * prec +ctod(*fmt++);

                                }
                        } else if (*fmt == '*') {
                                prec = va_arg(argp, int);
                                fmt ++;

                        }
                }

                negflag = FALSE;

                switch(*fmt) {
                        case 'b':
                        case 'B':
                                u = va_arg(argp, unsigned int);
                                buf = printnum(buf, u, 2, FALSE, length, ladjust, padc, 0);
                                break;

                        case 'c':
                                c = va_arg(argp, int);
                                *buf++ = c;
                                break;

                        case 'd':
                        case 'D':
                                n = va_arg(argp, int);
                                if (n >= 0) {
                                        u = n;
                                } else {
                                        u = -n;
                                        negflag = TRUE;
                                }
                                buf = printnum(buf, u, 10, negflag, length, ladjust, padc, 0);
                                break;

                        case 'o':
                        case 'O':
                                u = va_arg(argp, unsigned int);
                                buf = printnum(buf, u, 8, FALSE, length, ladjust, padc, 0);
                                break;

                        case 's':
                                p = va_arg(argp, char *);
                                if (p == (char *)0) {
                                        p = "(NULL)";
                                }
                                if (length > 0 && !ladjust) {
                                        n = 0;
                                        p2 = p;
                                        for (; *p != '\0' && (prec == -1 || n < prec); p++) {
                                                n++;
                                        }
                                        p = p2;
                                        while (n < length) {
                                                *buf++ = ' ';
                                                n++;
                                        }
                                }
                                n = 0;
                                while (*p != '\0') {
                                        if (++n > prec && prec != -1) {
                                                break;
                                        }
                                        *buf++ = *p++;
                                }
                                if (n < length && ladjust) {
                                        while (n < length) {
                                                *buf++ = ' ';
                                                n++;
                                        }
                                }
                                break;
                        
                        case 'u':
                        case 'U':
                                u = va_arg(argp, unsigned int);
                                buf = printnum(buf, u, 10, FALSE, length, ladjust, padc, 0);
                                break;

                        case 'x':
                                u = va_arg(argp, unsigned int);
                                buf = printnum(buf, u, 16, FALSE, length, ladjust, padc, 0);
                                break;

                        case 'X':
                                u = va_arg(argp, unsigned int);
                                buf = printnum(buf, u, 16, FALSE, length, ladjust, padc, 1);
                                break;

                        case '\0':
                                fmt--;
                                break;

                        default:
                                *buf++ = *fmt;
                }
                fmt++;
        }
        *buf = '\0';
}
