/* UTF-8 string helpers for Rogue 3.6 JEC.
 * Grapheme boundaries and cell widths follow Rogue 5.4.5 JEC 055.
 * Complex emoji rendering is left to the host terminal.
 */
#ifndef ROGUE_UNICODE_TEXT_H
#define ROGUE_UNICODE_TEXT_H

#include <stddef.h>

/* Public Unicode cluster helpers; all positions are UTF-8 byte offsets. */
size_t rogue_text_next(const char *text, size_t pos);
size_t rogue_text_prev(const char *text, size_t pos);
size_t rogue_text_prefix(const char *text, size_t max_bytes);
int rogue_text_width(const char *text);
int rogue_text_width_n(const char *text, size_t max_bytes);

/* Existing Rogue 3.6 interfaces, retained for source compatibility. */
int rogue_utf8_char_bytes(const char *s, size_t remain);
int rogue_utf8_lead_bytes(unsigned char c);
void rogue_utf8_copy_n(char *dst, size_t dstlen, const char *src, size_t srclen);
void rogue_utf8_copy(char *dst, size_t dstlen, const char *src);
int rogue_display_width(const char *s);
int rogue_utf8_width_n(const char *s, int bytes);
int rogue_utf8_prev_char(const char *s, int pos);
int rogue_utf8_next_char(const char *s, int pos, int len);
void rogue_utf8_copy_message_history(char *dst, size_t dstsz, const char *src);
void rogue_utf8_path_display(char *dst, size_t cap, const char *path, int max_width);
int rogue_utf8_center_column(int left, int width, const char *text);

#endif /* ROGUE_UNICODE_TEXT_H */
