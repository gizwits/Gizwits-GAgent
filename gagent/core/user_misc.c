#include <stdarg.h>
#include "gagent.h"

extern void DRV_ConAuxPrint(char *buffer, int len, int level);
u16 calc_sum(void *data, u32 len)
{
    u32 cksum=0;
    //__packed u16 *p=data;
    u16 *p=data;

    while (len > 1)
    {
        cksum += *p++;
        len -=2;
    }
    if (len)
    {
        cksum += *(u8 *)p;
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >>16);

    return ~cksum;
}

int check_sum(void *data, u32 len)  
{
    u16 *sum;
    u8 *p = (u8 *)data;

    return 1; // TODO: real cksum
    p += len - 2;

    sum = (u16 *)p;

    if (calc_sum(data, len - 2) != *sum) {  // check sum error    
        return 0;
    }
    return 1;
}

#if 1
#define UCHAR unsigned char
#define VOID void
#define CHAR char
#define ULONG unsigned int
#define FRMWRI_BUFSIZE  256
#define LONG int

static VOID
comio(UCHAR output, VOID *number,UCHAR * strbuf)
{
    *(strbuf+*((LONG *)number))=output;
    (*((LONG *)number))++;
    return;
}

/* -------------------------------------------------------------- */
/*                                                                */
/*                      - _C_formatter -                          */
/*                                                                */
/* This routine forms the core and entry of the formatter.
   The conversion
 */
/* performed conforms rather well to the ANSI specification for "printf".
 */
/* -------------------------------------------------------------- */
static LONG _C_formatter(const CHAR *format,
                         VOID put_one_char(UCHAR, VOID *,UCHAR *),
                         VOID *secret_pointer,
                         va_list ap, UCHAR * str)
{
    CHAR format_flag;
    LONG precision;
    LONG length, mask, nr_of_bits, n;
    LONG field_width, nr_of_chars;
    CHAR flag_char, left_adjust;
    ULONG ulong;

    LONG num;
    CHAR *buf_pointer;
    CHAR *ptr, *hex;
    CHAR zeropad;
    CHAR buf[FRMWRI_BUFSIZE];

    ULONG ulDate;/* added by  2k/09/20 for "%t"*/
    ULONG ulTime;/* added by  2k/09/20 for "%t"*/

    nr_of_chars = 0;
    for (;;)    /* Until full format string read */
    {
        if (nr_of_chars >= 200)
        {
            return nr_of_chars;
        }
        while ((format_flag = *format++) != '%')        /* Until '%' or '\0' */
        {
            if (!format_flag)
            {
                *(str+*((LONG *)secret_pointer))='\0';
                return (nr_of_chars);
            }
            put_one_char ((UCHAR)format_flag, secret_pointer,str);
            nr_of_chars++;
        }   /* while () */

        if (*format == '%')             /* %% prints as % */
        {
            format++;
            put_one_char('%',secret_pointer,str);
            nr_of_chars++;
            continue;
        }   /* if () */

        flag_char = 0;
        left_adjust = 0;
        /*=====================================*/
        /* check for leading -, + or ' 'flags  */
        /*=====================================*/
        for (;;)
        {
            if (*format == '+' || *format == ' ')
            {
                flag_char = *format++;
            }
            else if (*format == '-')
            {
                left_adjust++;
                format++;
            }
            else
            {
                break;
            }
        }   /* for (;;) */

        /*======================================*/
        /* a '0' may follow the flag character  */
        /* this is the requested fill character */
        /*======================================*/
        if (*format == '0')
        {
            zeropad = 1;
            format++;
        }
        else
        {
            zeropad = 0;
        }

        /*===================================*/
        /* Optional field width (may be '*') */
        /*===================================*/
        if (*format == '*')
        {
            field_width = va_arg(ap, LONG);
            if (field_width < 0)
            {
                field_width = (CHAR)(-field_width);
                left_adjust = (CHAR)(!left_adjust);
            }   /* if () */
            format++;
        }
        else
        {
            field_width = 0;
            while (*format >= '0' && *format <= '9')
            {
                field_width = field_width * 10 + (*format++ - '0');
            }
        }   /* if () */

        /*=============================*/
        /* Optional precision (or '*') */
        /*=============================*/
        if (*format=='.')
        {
            if (*++format == '*')
            {
                precision = va_arg(ap, LONG);
                format++;
            }
            else
            {
                precision = 0;
                while (*format >= '0' && *format <= '9')
                {
                    precision = precision * 10 + (*format++ - '0');
                }
            }   /* if () */
        }
        else
        {
            precision = -1;
        }

        /*======================================================*/
        /* At this point, "left_adjust" is nonzero if there was */
        /* a sign, "zeropad" is 1 if there was a leading zero   */
        /* and 0 otherwise, "field_width" and "precision"       */
        /* contain numbers corresponding to the digit strings   */
        /* before and after the decimal point, respectively,    */
        /* and "flag_char" is either 0 (no flag) or contains    */
        /* a plus or space character. If there was no decimal   */
        /* point, "precision" will be -1.                       */
        /*======================================================*/

        /*========================*/
        /* Optional "l" modifier? */
        /*========================*/
        if (*format == 'l')
        {
            length = 1;
            format++;
        }
        else
        {
            if (*format == 'F')
            {
                length = 1;
                format++;
            }
            else
            {
                length = 0;
            }
        }

        /*===================================================*/
        /* At exit from the following switch, we will emit   */
        /* the characters starting at "buf_pointer" and      */
        /* ending at "ptr"-1                                 */
        /*===================================================*/
        switch (format_flag = *format++)
        {
        case 'c':
        {
            buf[0] = (CHAR)va_arg(ap, LONG);
            ptr = buf_pointer = &buf[0];
            if (buf[0]){ ptr++; }
            break;
        }
        case 's':
        {
            if ((buf_pointer = va_arg(ap,CHAR *)) == NULL)
                buf_pointer = "(null pointer)";
            if (precision < 0)
                precision = 10000;
            for (n=0; *buf_pointer++ && n < precision; n++)
                ;
            ptr = --buf_pointer;
            buf_pointer -= n;
            break;
        }
        case 'o':
        case 'p':
        case 'X':
        case 'x':
        {
            if (format_flag == 'p')
            {
                if (length)
                    ulong = (LONG)va_arg(ap,CHAR *);
                else
                    ulong = (LONG)va_arg(ap,CHAR *);
            }
            else if (length)
                ulong = va_arg(ap,ULONG);
            else
                ulong = (ULONG)va_arg(ap,LONG);

            ptr = buf_pointer = &buf[FRMWRI_BUFSIZE - 1];
            hex = "0123456789ABCDEF";
            if (format_flag == 'o')
            {
                mask = 0x7;
                nr_of_bits = 3;
            }
            else
            {
                if (format_flag == 'x')
                    hex = "0123456789abcdef";
                mask = 0xf;
                nr_of_bits = 4;
            }   /* if () */

            do
                *--buf_pointer = *(hex + ((LONG) ulong & mask));
            while ( ( ulong >>= nr_of_bits ) != 0 );

            if (precision < 0)          /* "precision" takes precedence */
                if (zeropad)
                    precision = field_width;
            while (precision > ptr - buf_pointer)
                *--buf_pointer = '0';
            break;
        }
        case 'd':
        case 'i':
        case 'u':
        {
            if (length) { num = va_arg(ap,LONG); }
            else
            {
                n = va_arg(ap,LONG);
                if (format_flag == 'u')
                    num = (ULONG) n;
                else
                    num = (LONG) n;
            }   /* if () */
            if ( ( n = (format_flag != 'u' && num < 0) ) != 0 )
            {
                flag_char = '-';
                ulong = -num;
            }
            else
            {
                n = flag_char != 0;
                ulong = num;
            }   /* if () */

            /*=======================*/
            /* now convert to digits */
            /*=======================*/
            ptr = buf_pointer = &buf[FRMWRI_BUFSIZE - 1];
            do
            {
                *--buf_pointer = (CHAR)((ulong % 10) + '0');
            }
            while ( (ulong /= 10) != 0 );

            if (precision < 0)        /* "precision" takes precedence */
            {
                if (zeropad)
                {
                    precision = field_width - n;
                }
            }

            while (precision > ptr - buf_pointer)
            {
                *--buf_pointer = '0';
            }
            break;
        }
        case 'f':
        case 'e':
        case 'E':
        {
            ptr = buf_pointer = "FLOATS? wrong formatter installed!";
            while (*ptr)
                ptr++;
            break;
        }
        case '\0':              /* Really bad place to find NUL in */
            format--;

        default:
            /*=======================*/
            /* Undefined conversion! */
            /*=======================*/
            ptr = buf_pointer = "???";
            ptr += 4;
            break;
        }   /* switch () */


        /*===========================================================*/
        /* This part emittes the formatted string to "put_one_char". */
        /*===========================================================*/
        if ((length = ptr - buf_pointer) > field_width)
        {
            n = 0;
        }
        else
        {
            n = field_width - length - (flag_char != 0);
        }

        /*=================================*/
        /* emit any leading pad characters */
        /*=================================*/
        if (!left_adjust)
        {
            while (--n >= 0)
            {
                put_one_char(' ', secret_pointer,str);
                nr_of_chars++;
            }   /* while () */
        }

        /*===============================*/
        /* emit flag characters (if any) */
        /*===============================*/
        if (flag_char)
        {
            put_one_char((UCHAR)flag_char, secret_pointer,str);
            nr_of_chars++;
        }   /* if () */

        /*========================*/
        /* emit the string itself */
        /*========================*/
        while (--length >= 0)
        {
            put_one_char((UCHAR)(*buf_pointer++), secret_pointer,str);
            nr_of_chars++;
        }   /* while () */

        /*================================*/
        /* emit trailing space characters */
        /*================================*/
        if (left_adjust) 
        {
            while (--n >= 0)
            {
                put_one_char(' ', secret_pointer,str);
                nr_of_chars++;
            }   /* while () */
        }
    }   /* for (;;) */

    *(str+*((LONG *)secret_pointer))='\0';

    return nr_of_chars;
}   /* _C_formatter(,,,) */
#endif


void GAgent_Printf(unsigned int level, char *fmt, ...)
{
    char str[256];
    char *buffer;
    LONG i;
    LONG number=0;
    va_list vl;

    /* only show enable log */
    if(GAgent_loglevelenable( level )!=0 )
        return ;

    memset(str, 0x0, sizeof(str));
    switch(level & 0xFF)
    {
    case GAGENT_ERROR:
        i = 8;
        memcpy(str,"\r\nERROR ", i);
        buffer = str + i;
        //Log2Cloud(level);
        break;

    case GAGENT_WARNING:
        i = 10;
        memcpy(str,"\r\nWARNING ", i);
        buffer = str + i;
        break;

    case GAGENT_INFO:
        i = 7;
        memcpy(str,"\r\nINFO ", i);
        buffer = str + i;
        break;

    case GAGENT_CRITICAL:
        i = 11;
        memcpy(str,"\r\nCRITICAL ", i);
        buffer = str + i;
        break;

    case GAGENT_DEBUG:
        i = 8;
        memcpy(str,"\r\nDEBUG ", i);
        buffer = str + i;
        break;
    case GAGENT_DUMP:
        i = 0;
        buffer = str;
        break;
    default:
        i = 0;
        buffer = str + i;
        break;
    }

    va_start(vl,fmt);
    i += _C_formatter(fmt, comio, &number, vl, buffer);
    va_end(vl);

    DRV_ConAuxPrint(str, i, level );
    return;
}


void GAgent_DebugPacket(unsigned char *pData, int len)
{
    int i;
    GAgent_Printf(GAGENT_DUMP, "\r\nDump Packet, len:%d", len);
    for (i = 0; i < (len+7); 1)
    {
        GAgent_Printf(GAGENT_DUMP, "\r\nPacket %4d %02x%02x %02x%02x %02x%02x %02x%02x",
                      i,
                      pData[i],pData[i+1],pData[i+2],pData[i+3],
                      pData[i+4],pData[i+5],pData[i+6],pData[i+7]);

        i+=8;
    }
    GAgent_Printf(GAGENT_DUMP, "\r\n");

    return;
}

