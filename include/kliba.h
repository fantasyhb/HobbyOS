
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
void panic(const char* fmt, ...);
int get_ticks();

/*syscall.asm*/
void write(char *buf, int len);
/*proc.c*/
int sys_get_ticks();
void* va2la(int pid, void *va);
int ldt_seg_linear(PROCESS *p, int idx);
int proc2pid(PROCESS *p);
int sys_sendrec(int function, int src_dest, MESSAGE *m, struct s_proc *p);
int send_recv(int function, int src_dest, MESSAGE *msg);

/* keyboard.c */
void init_keyboard();
/* tty.c  */
void task_tty();
void in_process(TTY *p_tty, u32 key);
/* int sys_write(char *buf, int len, PROCESS *p_proc); */
int sys_printx(int _unused1, int _unused2, char *s, PROCESS *p_proc);
/* console.c */
void out_char(CONSOLE *, char);
void init_console(TTY *p_tty);
void scroll_screen(CONSOLE *p_con, int direction);
/* vsprintf.c */
int vsprintf(char *buf, const char*fmt, va_list arg);
/* printf.c */
int printf(const char *fmt, ... );
#define printl printf
/* misc.c */
void spin( char *info);
/* systask.c */
void task_sys();
