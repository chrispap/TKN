#ifndef _TKN_WRAPPER_
#define _TKN_WRAPPER_

/* TKN Functions */
#define TKN_RESET_ADDR	0x7000
#define TKN_RETURN_ADDR 0x7398
#define TKN_POP_ADDR	0x7098
#define TKN_PUSH_ADDR	0x7062
#define BIN1TOHEX2_ADDR	0x734b
#define SETLEDS_ADDR	0x72bc

char TKN_Send(char *buf, char dest_id);
char TKN_Receive(char *buf);

#endif /* _TKN_WRAPPER_ */
