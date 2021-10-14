typedef union _BYTE
{
    unsigned char _byte;
    struct{
	  unsigned _a0:1;
        unsigned _a1:1;
        unsigned _a2:1;
        unsigned _a3:1;
        unsigned _a4:1;
        unsigned _a5:1;
        unsigned _a6:1;
        unsigned _a7:1;
	}BITS;
} BYTE;

#define	b0	BITS._a0
#define	b1	BITS._a1 
#define	b2	BITS._a2 
#define	b3	BITS._a3
#define	b4	BITS._a4
#define	b5	BITS._a5 
#define	b6	BITS._a6 
#define	b7	BITS._a7  


typedef enum _COM_STATES
{
    C0 = 0,
    C1,
    C2,
    C3,
    C4,
    C5,
}COM_STATES;   

