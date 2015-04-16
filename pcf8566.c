#include "pcf8566.h"

#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define NUM_CHARS    PCF8566_NUM_CHARS
#define NUM_SEGS     8

static const signed char seg_to_ram_map[NUM_SEGS][NUM_CHARS] = {
	{  0, 24, 36, 48, 60, 76, 90 }, /* a  0x01 */
	{ 18,  4, 28, 40, 72, 84, -1 }, /* b  0x02 */
	{ 14,  5, 29, 41, 73, 85, -1 }, /* c  0x04*/
	{  2, 26, 38, 50, 62, 78, 74 }, /* d  0x08 */
	{ 13, 17, 57, 53, 65, 89, -1 }, /* e  0x10 */
	{ 12, 16, 56, 52, 64, 88, -1 }, /* f  0x20 */
	{  1, 25, 37, 49, 61, 77, 86 }, /* g  0x40 */
	{ -1,  6, 30, 42, 54, 66, -1 }  /* DP 0x80 */
};

/* valid for all commands: continue command mode after this byte */
#define PCF8566_C		(1U<<7) /* continuation */

/* mode command */
#define PCF8566_MODE		(1U<<6)
#define PCF8566_LP		(1U<<4)	/* power saving */
#define PCF8566_E		(1U<<3)	/* enable */
#define PCF8566_B		(1U<<2) /* 1: 1/2 bias, 0: 1/3 bias */

/* number of active backplanes: "mode" bits M0, M1 */
#define PCF8566_Mx(v)		((v) & 0x03) /* # of backplanes */
#define PCF8566_1BP		PCF8566_Mx(1)	/* use BP0 */
#define PCF8566_2BP		PCF8566_Mx(2)	/* use BP0 & 1 */
#define PCF8566_3BP		PCF8566_Mx(3)	/* use BP0-2 */
#define PCF8566_4BP		PCF8566_Mx(0)	/* use BP0-3 */

/* command: set write pointer */
#define PCF8566_Px(v)		((v) & 0x1f)          /* write pointer */

/* command: set subaddress */
#define PCF8566_Ax(v)		(0x60 | ((v) & 0x1f)) /* sub-address */

struct pcf8566 {
	int		fd;
	unsigned char	addr;
	unsigned char   buf[14]; /* 12+2, includes buffer for cmds */
};

static int
pcf8566_write_i2c (
	struct pcf8566 *p,
	const unsigned char *txbuf,
	unsigned int length)
{
	struct i2c_msg msgs[1];
	struct i2c_rdwr_ioctl_data i2c_ioctl;
	unsigned int i;

	bzero(&msgs, sizeof(msgs));
	bzero(&i2c_ioctl, sizeof(i2c_ioctl));

	msgs[0].addr  = p->addr;
	msgs[0].flags = 0; /* write */
	msgs[0].buf   = (unsigned char*)txbuf; /* intentionally dropping const! */
	msgs[0].len   = length;

#if 0	/* ==== debugging ==== */
	fprintf(stderr,"%s: addr=%d: [", __FUNCTION__, p->addr);
	for (i=0; i<length; i++) {
		if (txbuf[i] == 0x00)
			fprintf(stderr,"\033[0;34m__\033[0m");
		else
			fprintf(stderr,"\033[0;36;1m%02x\033[0m", txbuf[i]);

		if (i < (length-1))
			fprintf(stderr,", ");
		else
			fprintf(stderr,"]\n");
	}
#endif

	i2c_ioctl.msgs = msgs;
	i2c_ioctl.nmsgs=1;

	if (ioctl(p->fd, I2C_RDWR, &i2c_ioctl) != (int)i2c_ioctl.nmsgs)
		return -1;

	return 0;
}

int
pcf8566_update(struct pcf8566 *p)
{

	p->buf[0] = PCF8566_Ax(0)|PCF8566_C;
	p->buf[1] = PCF8566_Px(0);

	if (pcf8566_write_i2c(p, p->buf, sizeof(p->buf))) {
		perror("pcf8566_write_i2c(buf)");
		exit(1);
	}
}

void
pcf8566_set_char(struct pcf8566 *p, unsigned int ch, unsigned char segbits)
{
	unsigned int seg;
	unsigned int rampos;
	unsigned int rambit;

	if (ch >= NUM_CHARS)
		return;

	for (seg=0; seg<NUM_SEGS; seg++) {
		/* buffer include two prepended command bytes! */
		/* bits are numbered from MSB to LSB */

		rampos =    2  +  seg_to_ram_map[seg][ch] / 8;
		rambit = 0x80 >> (seg_to_ram_map[seg][ch] % 8);

		if (segbits & (1<<seg))
			p->buf[rampos] |=  rambit;  /* segment on */
		else
			p->buf[rampos] &= ~rambit;  /* segment off */
	}
}

void
pcf8566_clear_disp(struct pcf8566 *p)
{
	bzero(p->buf, sizeof(p->buf));
}

struct pcf8566 *
pcf8566_new(int fd, unsigned char addr)
{
	struct pcf8566 *p;

	assert(p = malloc(sizeof(*p)));
	bzero(p, sizeof(*p));

	p->fd = fd;
	p->addr = addr;

	bzero(p->buf, sizeof(p->buf));

	/* initialize lcd controller */
	p->buf[0] = PCF8566_MODE | PCF8566_E | PCF8566_4BP;
	if (pcf8566_write_i2c(p, p->buf, 1)) {
		perror("pcf8566_write_i2c(cmd)");
		free(p);
		return NULL;
	}

	return p;
}

void
pcf8566_delete(struct pcf8566 *p)
{
	/* disable LCD */
	p->buf[0] = PCF8566_MODE;
	pcf8566_write_i2c(p, p->buf, 1);

	free(p);
}
