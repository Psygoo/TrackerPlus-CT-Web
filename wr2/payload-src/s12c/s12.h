#ifndef S12_H
#define S12_H
#include "wr2_project.h"
#define S12_FM 0x2001e488u
#define S12_FN(a,t) ((t)(uintptr_t)((a)|1u))
typedef struct {uint32_t file[65];_Alignas(8) uint32_t fil[0x250/4];} s12_io;
_Static_assert(sizeof(s12_io)==856,"bounded embedded IO");
int s12_open(s12_io *,const char *,unsigned);
int s12_close(s12_io *);
int s12_read(s12_io *,void *,unsigned);
int s12_write(s12_io *,const void *,unsigned);
unsigned s12_size(s12_io *);
void s12_init(void);
void s12_new(void);
void s12_poll(void);
int s12_blocked(void);
int s12_pre(unsigned,unsigned);
void s12_post(unsigned,unsigned);
void s12_release(void *,s12_io *);
void s12_filter(wr2_blob *,s12_io *,uint8_t *);
void s12_saved(void);
void wr2_slot_snapshot(uint8_t *,unsigned);
void wr2_slot_clear(unsigned);
int wr2_slot_publish(const uint8_t *,unsigned);
#endif
