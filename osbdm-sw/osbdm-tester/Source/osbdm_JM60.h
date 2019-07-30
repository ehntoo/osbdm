
void osbdm_mem_header(unsigned char cmd, unsigned char type, unsigned long addr,
					  unsigned char width, unsigned long len);
int osbdm_write8(unsigned char type, unsigned long address, unsigned char data);
int osbdm_write32(unsigned char type, unsigned long address, unsigned long data);

unsigned long osbdm_read32(unsigned char type, unsigned long address);

int osbdm_connect();
int osbdm_open(unsigned char device_no);
int osbdm_init();
void osbdm_close();

int osbdm_write_block(unsigned char type, unsigned char width, unsigned long addr, unsigned char *data, unsigned long size);
int osbdm_read_block(unsigned char type, unsigned char width, unsigned long addr, unsigned char *data, unsigned long size);
int osbdm_write_fill(unsigned char type, unsigned char width, unsigned long addr, unsigned char *data, unsigned long count);

int osbdm_send1(unsigned char cmd);
int osbdm_send(unsigned char count);

int osbdm_status(char *data);
int osbdm_target_reset(unsigned char mode);
int osbdm_target_halt();
int osbdm_target_go();
int osbdm_target_step();
int osbdm_target_set_speed(float speed);
int osbdm_target_get_speed(unsigned char *data);

int osbdm_flash_unsecure(unsigned char unsec_instruction, unsigned char unsec_instruction_len,
						unsigned char flash_clock_divisor);

int osbdm_flash_dlstart(void);
int osbdm_flash_dlend(void);
int osbdm_flash_prog(unsigned int faddr, unsigned char size, unsigned char *data);

int osbdm_sci_read(unsigned char *data);
int osbdm_sci_write(char count, char *data);
int osbdm_sci_write1(unsigned char data);

unsigned char packet_data_size(unsigned char hsize, unsigned char width, unsigned long size);


extern unsigned char usb_data[];
extern unsigned char usb_data2[];

