
/*klib.asm*/
void out_byte(u16 port, u8 value);
u8 in_byte(u16 port);
void print_str(char *str);
void print_str_color(char *str, int color);
void disable_irq(int irq);
void enable_irq(int irq);
void disable_int();
void enable_int();
/*i8259.c*/
void init_8259A();
void set_irq_handler(int irq, irq_handler handler);
/*protect.c*/
void init_prot();
void spurious_irq(int irq);
/*klib.c*/
void print_int(int num);
void delay(int time);
void itoa_hex(char *str, int num);
/*clock.c*/
void init_clock();
void clock_handler();
void ms_delay(int ms);
/*kernel.asm*/
void restart();
void sys_call();
/*main.c*/
int kernel_main();
void TestA();
void TestB();
void TestC();
/*syscall.asm*/
int get_ticks();
void write(char *buf, int len);
/*proc.c*/
int sys_get_ticks();
/* keyboard.c */
void init_keyboard();
/* tty.c  */
void task_tty();
void in_process(TTY *p_tty, u32 key);
int sys_write(char *buf, int len, PROCESS *p_proc);
/* console.c */
void out_char(CONSOLE *, char);
void init_console(TTY *p_tty);
void scroll_screen(CONSOLE *p_con, int direction);
/* vsprintf.c */
int vsprintf(char *buf, const char*fmt, va_list arg);
/* printf.c */
int printf(const char *fmt, ... );
