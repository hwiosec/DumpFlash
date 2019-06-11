#include <Python.h>
#if 0
int nand_calculate_256_ecc(unsigned char* data_buf, unsigned char* ecc_buf)
{

	unsigned int i;
	unsigned int tmp;
	unsigned int uiparity = 0;
	unsigned int parityCol, ecc = 0;
	unsigned int parityCol4321 = 0, parityCol4343 = 0, parityCol4242 =
	    0, parityColTot = 0;
	unsigned int *Data = (unsigned int *)(data_buf);
	unsigned int Xorbit = 0;

	for (i = 0; i < 8; i++) {
		parityCol = *Data++;
		tmp = *Data++;
		parityCol ^= tmp;
		parityCol4242 ^= tmp;
		tmp = *Data++;
		parityCol ^= tmp;
		parityCol4343 ^= tmp;
		tmp = *Data++;
		parityCol ^= tmp;
		parityCol4343 ^= tmp;
		parityCol4242 ^= tmp;
		tmp = *Data++;
		parityCol ^= tmp;
		parityCol4321 ^= tmp;
		tmp = *Data++;
		parityCol ^= tmp;
		parityCol4242 ^= tmp;
		parityCol4321 ^= tmp;
		tmp = *Data++;
		parityCol ^= tmp;
		parityCol4343 ^= tmp;
		parityCol4321 ^= tmp;
		tmp = *Data++;
		parityCol ^= tmp;
		parityCol4242 ^= tmp;
		parityCol4343 ^= tmp;
		parityCol4321 ^= tmp;

		parityColTot ^= parityCol;

		tmp = (parityCol >> 16) ^ parityCol;
		tmp = (tmp >> 8) ^ tmp;
		tmp = (tmp >> 4) ^ tmp;
		tmp = ((tmp >> 2) ^ tmp) & 0x03;
		if ((tmp == 0x01) || (tmp == 0x02)) {
			uiparity ^= i;
			Xorbit ^= 0x01;
		}
	}

	tmp = (parityCol4321 >> 16) ^ parityCol4321;
	tmp = (tmp << 8) ^ tmp;
	tmp = (tmp >> 4) ^ tmp;
	tmp = (tmp >> 2) ^ tmp;
	ecc |= ((tmp << 1) ^ tmp) & 0x200;	/*  p128 */

	tmp = (parityCol4343 >> 16) ^ parityCol4343;
	tmp = (tmp >> 8) ^ tmp;
	tmp = (tmp << 4) ^ tmp;
	tmp = (tmp << 2) ^ tmp;
	ecc |= ((tmp << 1) ^ tmp) & 0x80;	/*  p64 */

	tmp = (parityCol4242 >> 16) ^ parityCol4242;
	tmp = (tmp >> 8) ^ tmp;
	tmp = (tmp << 4) ^ tmp;
	tmp = (tmp >> 2) ^ tmp;
	ecc |= ((tmp << 1) ^ tmp) & 0x20;	/*  p32 */

	tmp = parityColTot & 0xFFFF0000;
	tmp = tmp >> 16;
	tmp = (tmp >> 8) ^ tmp;
	tmp = (tmp >> 4) ^ tmp;
	tmp = (tmp << 2) ^ tmp;
	ecc |= ((tmp << 1) ^ tmp) & 0x08;	/*  p16 */

	tmp = parityColTot & 0xFF00FF00;
	tmp = (tmp >> 16) ^ tmp;
	tmp = (tmp >> 8);
	tmp = (tmp >> 4) ^ tmp;
	tmp = (tmp >> 2) ^ tmp;
	ecc |= ((tmp << 1) ^ tmp) & 0x02;	/*  p8 */

	tmp = parityColTot & 0xF0F0F0F0;
	tmp = (tmp << 16) ^ tmp;
	tmp = (tmp >> 8) ^ tmp;
	tmp = (tmp << 2) ^ tmp;
	ecc |= ((tmp << 1) ^ tmp) & 0x800000;	/*  p4 */

	tmp = parityColTot & 0xCCCCCCCC;
	tmp = (tmp << 16) ^ tmp;
	tmp = (tmp >> 8) ^ tmp;
	tmp = (tmp << 4) ^ tmp;
	tmp = (tmp >> 2);
	ecc |= ((tmp << 1) ^ tmp) & 0x200000;	/*  p2 */

	tmp = parityColTot & 0xAAAAAAAA;
	tmp = (tmp << 16) ^ tmp;
	tmp = (tmp >> 8) ^ tmp;
	tmp = (tmp >> 4) ^ tmp;
	tmp = (tmp << 2) ^ tmp;
	ecc |= (tmp & 0x80000);	/*  p1 */

	ecc |= (uiparity & 0x01) << 11;	/*  parit256_1 */
	ecc |= (uiparity & 0x02) << 12;	/*  parit512_1 */
	ecc |= (uiparity & 0x04) << 13;	/*  parit1024_1 */

	if (Xorbit) {
		ecc |= (ecc ^ 0x00A8AAAA) >> 1;
	} else {
		ecc |= (ecc >> 1);
	}

	ecc = ~ecc;
	*(ecc_buf + 2) = (unsigned char)(ecc >> 16);
	*(ecc_buf + 1) = (unsigned char)(ecc >> 8);
	*(ecc_buf + 0) = (unsigned char)(ecc);

	return 0;
}

#endif

/**
 * nand_correct_256_data - [NAND Interface] Detect and correct bit error(s)
 * @buf:	raw data read from the chip
 * @read_ecc:	ECC from the chip
 * @calc_ecc:	the ECC calculated from raw data
 *
 * Detect and correct a 1 bit error for 256/512 byte block
 */
int nand_correct_256_data(unsigned char *pPagedata,
		      unsigned char *iEccdata1, unsigned char *iEccdata2)
{

	uint32_t iCompecc = 0, iEccsum = 0;
	uint32_t iFindbyte = 0;
	uint32_t iIndex;
	uint32_t nT1 = 0, nT2 = 0;

	uint8_t iNewvalue;
	uint8_t iFindbit = 0;

	uint8_t *pEcc1 = (uint8_t *) iEccdata1;
	uint8_t *pEcc2 = (uint8_t *) iEccdata2;

	for (iIndex = 0; iIndex < 2; iIndex++) {
		nT1 ^= (((*pEcc1) >> iIndex) & 0x01);
		nT2 ^= (((*pEcc2) >> iIndex) & 0x01);
	}

	for (iIndex = 0; iIndex < 3; iIndex++)
		iCompecc |= ((~(*pEcc1++) ^ ~(*pEcc2++)) << iIndex * 8);

	for (iIndex = 0; iIndex < 24; iIndex++) {
		iEccsum += ((iCompecc >> iIndex) & 0x01);
	}

	switch (iEccsum) {
	case 0:
		printf("RESULT : no error\n"); 
		return 0;	/* ECC_NO_ERROR; */

	case 1:
		printf("RESULT : ECC code 1 bit fail\n"); 
		return 2;	/* ECC_ECC_ERROR; */

	case 11:
		if (nT1 != nT2) {
			iFindbyte =
			    ((iCompecc >> 15 & 1) << 7) +
			    ((iCompecc >> 13 & 1) << 6)
			    + ((iCompecc >> 11 & 1) << 5) +
			    ((iCompecc >> 9 & 1) << 4) +
			    ((iCompecc >> 7 & 1) << 3)
			    + ((iCompecc >> 5 & 1) << 2) +
			    ((iCompecc >> 3 & 1) << 1) + (iCompecc >> 1 & 1);
			iFindbit =
			    (uint8_t) (((iCompecc >> 23 & 1) << 2) +
				       ((iCompecc >> 21 & 1) << 1) +
				       (iCompecc >> 19 & 1));
			iNewvalue =
			    (uint8_t) (pPagedata[iFindbyte] ^ (1 << iFindbit));
			printf("iCompecc = %d\n",iCompecc); 
			printf("RESULT : one bit error\r\n"); 
			printf("byte = %d, bit = %d\r\n", iFindbyte, iFindbit); 
			printf("corrupted = %x, corrected = %x\r\n", pPagedata[iFindbyte], iNewvalue); 
                        pPagedata[iFindbyte] = iNewvalue;
			return 1;	/* ECC_CORRECTABLE_ERROR; */
		} else  {
			printf("nT1 != nT2\n");
			return 3;	/* ECC_UNCORRECTABLE_ERROR; */
		}		

	default:
		printf("RESULT : unrecoverable error %d\n", iEccsum); 
		return 3;	/* ECC_UNCORRECTABLE_ERROR; */
	}
}

void nand_calculate_256_ecc(unsigned char* data_buf, unsigned char* ecc_buf) {
	uint8_t p1, p1_, p2, p2_, p4, p4_, p8, p8_, p16, p16_, p32, p32_, p64, p64_, p128, p128_, p256, p256_, p512, p512_, p1024, p1024_, p2048, p2048_;
	uint8_t ch, bit0, bit1, bit2, bit3, bit4, bit5, bit6, bit7, xor_bit;
	int i;
        uint8_t ecc0, ecc1, ecc2;
    p8 = 0;
    p8_ = 0;
    p16 = 0;
    p16_ = 0;
    p32 = 0;
    p32_ = 0;
    p64 = 0;
    p64_ = 0;
    p128 = 0;
    p128_ = 0;
    p256 = 0;
    p256_ = 0;
    p512 = 0;
    p512_ = 0;
    p1024 = 0;
    p1024_ = 0;
    p2048 = 0;
    p2048_ = 0;

    p1 = 0;
    p1_ = 0;
    p2 = 0;
    p2_ = 0;
    p4 = 0;
    p4_ = 0;

    xor_bit = 0;

    for(i = 0; i < 512; i++) {
    	ch = data_buf[i];
        bit0 = ch & 0x1;
        bit1 = (ch >> 1) & 0x1;
        bit2 = (ch >> 2) & 0x1;
        bit3 = (ch >> 3) & 0x1;
        bit4 = (ch >> 4) & 0x1;
        bit5 = (ch >> 5) & 0x1;
        bit6 = (ch >> 6) & 0x1;
        bit7 = (ch >> 7) & 0x1;

        xor_bit = bit7 ^ bit6 ^ bit5 ^ bit4 ^ bit3 ^ bit2 ^ bit1 ^ bit0;

        if ((i & 0x01) == 0x01)
                p8 = xor_bit ^ p8;
        else
                p8_ = xor_bit ^ p8_;

        if ((i & 0x02) == 0x02)
                p16 = xor_bit ^ p16;
        else
                p16_ = xor_bit ^ p16_;

        if ((i & 0x04) == 0x04)
                p32 = xor_bit ^ p32;
        else
                p32_ = xor_bit ^ p32_;

        if ((i & 0x08) == 0x08)
                p64 = xor_bit ^ p64;
        else
                p64_ = xor_bit ^ p64_;

        if ((i & 0x10) == 0x10)
                p128 = xor_bit ^ p128;
        else
                p128_ = xor_bit ^ p128_;

        if ((i & 0x20) == 0x20)
                p256 = xor_bit ^ p256;
        else
                p256_ = xor_bit ^ p256_;

        if ((i & 0x40) == 0x40)
                p512 = xor_bit ^ p512;
        else
                p512_ = xor_bit ^ p512_;

        if ((i & 0x80) == 0x80)
                p1024 = xor_bit ^ p1024;
        else
                p1024_ = xor_bit ^ p1024_;

        p1 = bit7 ^ bit5 ^ bit3 ^ bit1 ^ p1;
        p1_ = bit6 ^ bit4 ^ bit2 ^ bit0 ^ p1_;
        p2 = bit7 ^ bit6 ^ bit3 ^ bit2 ^ p2;
        p2_ = bit5 ^ bit4 ^ bit1 ^ bit0 ^ p2_;
        p4 = bit7 ^ bit6 ^ bit5 ^ bit4 ^ p4;
        p4_ = bit3 ^ bit2 ^ bit1 ^ bit0 ^ p4_;

    }
    ecc0 = (p64 << 7) + (p64_ << 6) + (p32 << 5) + (p32_ << 4) + (p16 << 3) + (p16_ << 2) + (p8 << 1) + ( p8_ << 0);
    ecc1 = (p1024 << 7) + (p1024_ << 6) + (p512 << 5) + (p512_ << 4) + (p256 << 3) + (p256_ << 2) + (p128 << 1) + (p128_<< 0);
    ecc2 = (p4 << 7) + (p4_ << 6) + (p2 << 5) + (p2_ << 4) + (p1 << 3) + (p1_ << 2) + (p2048 << 1) + (p2048_ << 0);

    ecc0 = ecc0;
    ecc1 = ecc1;
    ecc2 = ecc2;

    ecc_buf[0] = ecc0;
    ecc_buf[1] = ecc1;
    ecc_buf[2] = ecc2;

}

static PyObject* hamming_ecc(PyObject* self, PyObject* args) {
    char *data_buf;
    char *oob_buf;
    int buf_len;
    int oob_len;
    unsigned char ecc[4];

    int i;
    int eccsteps, eccbytes, eccsize;
    eccsteps = 4;
    eccbytes = 3;
    eccsize = 512;
    unsigned char *p;
    unsigned char *ecc_oob;
    int ret = 0;   
      
    PyArg_ParseTuple(args, "s#s#", &data_buf, &buf_len, &oob_buf, &oob_len);
    p = (unsigned char*)data_buf;
    ecc_oob = (unsigned char*)oob_buf;
    for (i = 0 ; eccsteps; eccsteps--, i += eccbytes, p += eccsize, ecc_oob += eccbytes){
        if(ecc_oob[0] == 0xff && ecc_oob[1] == 0xff && ecc_oob[2] == 0xff)
            continue;
       
        if(ecc_oob[0] == 0x00 && ecc_oob[1] == 0x00 && ecc_oob[2] == 0x00)
            continue;
        memset(ecc, 0, sizeof(ecc));
        nand_calculate_256_ecc(p, ecc);
        if(memcmp(ecc, ecc_oob, 3) != 0) {

            if(nand_correct_256_data(p, ecc_oob, ecc) == 3) {
                printf("Unrecoverable error: %02X %02X %02X ==== %02X %02X %02X\n",  ecc[0], ecc[1], ecc[2], ecc_oob[0], ecc_oob[1], ecc_oob[2]);
                ret = -1;
	    }
            else {
                printf("Recoverable error: %02X %02X %02X ==== %02X %02X %02X\n",  ecc[0], ecc[1], ecc[2], ecc_oob[0], ecc_oob[1], ecc_oob[2]);

            }
          
        }
    }
    return Py_BuildValue("i", ret);
}

static PyMethodDef HammingMethods[] = {
    {"hamming_ecc", hamming_ecc, METH_VARARGS, "Hamming ecc calculation"},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC inithamming(void) {
     (void) Py_InitModule("hamming", HammingMethods);
}
