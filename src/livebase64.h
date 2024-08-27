#ifndef __LIVE_BASE64_H__
#define __LIVE_BASE64_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void live_base64_decode(char *src, int src_len, char *dst, int *dstlen);

long live_base64_encode (char *to, char *from, unsigned int len);

#endif
