#ifndef __CDC__
#define __CDC__

#define SCI1_BAUDRATE_HI                     (SCI1BDH)
#define SCI1_BAUDRATE_LO                     (SCI1BDL)
#define SCI1_DATA_REG                        (SCI1D)
#define SCI1_CTR1                            (SCI1C1)
#define SCI1_PE                              (SCI1C1_PE)
#define SCI1_PT                              (SCI1C1_PT)
#define SCI1_M                               (SCI1C1_M)
#define SCI1_TXEN                            (SCI1C2_TE)
#define SCI1_TX_COMPLETE_IEN                 (SCI1C2_TCIE)
#define SCI1_RXEN                            (SCI1C2_RE)
#define SCI1_RXIEN                           (SCI1C2_RIE)
#define SCI1_STATUS1                         (SCI1S1)
#define SCI1_TX_INT_VECTOR                   VectorNumber_Vsci1tx
#define SCI1_RX_INT_VECTOR                   VectorNumber_Vsci1rx

#define SCI1_BUS_CLK                         (UINT32)(24000000)
#define SCI1_BAUDRATE(x)                     ((UINT16)((UINT32)((UINT32)(SCI1_BUS_CLK)/(UINT32)(16))/(UINT32)(x)))

extern unsigned char u8CDCState;

/* TypeDefs */
typedef struct
{
    unsigned long  DTERate;
    unsigned char  CharFormat;
    unsigned char  ParityType;
    unsigned char  Databits;
} CDC_Line_Coding;


//extern UINT8 u8RecDataFlag;        
extern CDC_Line_Coding LineCoding; 


/* Prototypes */
void CDC_Init(void);
void CDC_Engine(void);
unsigned long LWordSwap(unsigned long);


#endif /* __CDC__*/