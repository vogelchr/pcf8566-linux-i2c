#include "pcf8566.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static const unsigned char patterns[] = {
	0xff, /* 8 */
	0x63, /* o top */
	0x5c, /* o bot */
	0x3f, /* 0 */
	0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80
};

static const unsigned char around[] = {
	0x07, 0x0e, 0x1c, 0x38, 0x31, 0x23
};

int
main(int argc, char **argv)
{
	int fd;
	unsigned int i,j;
	struct pcf8566 *p;
	unsigned char buf[12]; /* 2BP: 6, 3BP: 9, 4BP: 12 */

	(void)argc;
	(void)argv;

	if ((fd = open("/dev/i2c-0",O_RDWR|O_NOCTTY|O_CLOEXEC)) == -1) {
		perror("/dev/i2c-0");
		exit(1);
	}

	if ((p = pcf8566_new(fd, 0x3e)) == NULL)
		exit(1); /* will have written error message already... */


	for (j=0; j<sizeof(patterns); j++) {
		for (i=0; i<7; i++) {
			pcf8566_clear_disp(p);
			pcf8566_set_char(p, i, patterns[j]);
			pcf8566_update(p);
			usleep(20000);
		}
	}

	for (j=0; j<8*sizeof(around); j++) {
		pcf8566_clear_disp(p);
		for (i=0; i<6; i++)
			pcf8566_set_char(p, i, around[j%sizeof(around)]);
		pcf8566_update(p);
		usleep(50000);
	}

	pcf8566_clear_disp(p);
	pcf8566_set_char(p, 5, 0x76); /* H */
	pcf8566_set_char(p, 4, 0x79); /* E */
	pcf8566_set_char(p, 3, 0x38); /* L */
	pcf8566_set_char(p, 2, 0x38); /* L */
	pcf8566_set_char(p, 1, 0x3f); /* O */
	pcf8566_update(p);

	return 0;
}
