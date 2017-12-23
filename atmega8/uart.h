#define P(format, ...) printf_P(PSTR(format), __VA_ARGS__)
#define L(str) puts_P(PSTR(str))

void	uartInit(void);
