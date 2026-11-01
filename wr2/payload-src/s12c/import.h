#ifndef S12C_IMPORT_H
#define S12C_IMPORT_H
#include "companion.h"
#define S12C_IMPORT 32u
int s12c_pre(unsigned);
void s12c_post(unsigned,unsigned);
void s12c_poll(void);
void s12c_cancel(void);
int s12c_native_close(uint32_t *,uintptr_t);
int s12_fence_mask(s12_io *,uint8_t *,uint64_t);
void s12c_header_begin(const uint32_t *,unsigned);
unsigned s12c_header_read(uint32_t *,const char *,void *,unsigned,unsigned);
unsigned s12c_import_pcm(void *,unsigned,const uint32_t *);
unsigned s12c_pcm_read(uint32_t *,const char *,unsigned,void *,unsigned,unsigned *);
unsigned s12c_workspace_header(void *,unsigned);
unsigned s12c_workspace_write(void *,unsigned);
unsigned s12c_pcm_write(uint32_t *,const void *,unsigned,const char *,unsigned,unsigned);
#endif
