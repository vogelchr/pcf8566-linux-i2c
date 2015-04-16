#ifndef PCF8566_H
#define PCF8566_H

#define PCF8566_NUM_CHARS 7

/* Numbering of individual characters' segments:
 * =============================================
 *
 *    a      segment bits: a=1<<0 ... g=1<<6, dp=1<<7
 *   ==
 * f| g|b
 *   ==      for the leftmost A/D/- symbol:
 * e| d|c     'A'=a, 'D'=d, '-'=g
 *   ==
 *     dp
 *
 * Numbering of characters on the LCD
 * ==================================
 *
 * A/D/- at character index 6
 * |
 * v       v- character index 0
 * -88888888
 *  ^-------- character index 5
 *
 * Mapping of character segments to controller RAM
 * bit numbers:
 *
 *   digit_map[char][segment] = RAM bit number
 *
 * A bit number of -1 indicates a missing segment.
 *
 */

struct pcf8566 ;

/* connect to pcf8566 LCD controller on i2c-bus opened as fd, on i2c
   address addr, return struct pcf8566 or NULL on error */
extern struct pcf8566 *
pcf8566_new(int fd, unsigned char addr);

/* free pcf8566 object, tries to disable LCD before */
extern void
pcf8566_delete(struct pcf8566 *p);

/* send our local buffer to LCD, return 0 if ok, -1 on error */
extern int pcf8566_update(struct pcf8566 *p);

/* set segments of character number 'ch' to segbits */
extern void pcf8566_set_char(struct pcf8566 *p, unsigned int ch,
		unsigned char segbits);

/* clear all characters and segments */
extern void pcf8566_clear_disp(struct pcf8566 *p);

#endif