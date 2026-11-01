#ifndef WR2_PROJECT_H
#define WR2_PROJECT_H
#include <stddef.h>
#include <stdint.h>

enum { WR2_SLOTS=48, WR2_SLICES=48, WR2_RECORD_BYTES=104,
       WR2_HEADER_BYTES=32, WR2_BLOB_BYTES=5024 };
typedef struct { uint8_t header[32]; uint8_t record[48][104]; } wr2_blob;

/* Project schema, not a dump of the RAM row: see FORMAT.md. */
uint32_t wr2_crc32c(const uint8_t *, size_t);
uint32_t wr2_identity(const volatile uint8_t *instrument);
int wr2_record_valid(const uint8_t *);
int wr2_blob_valid(const wr2_blob *);
void wr2_blob_seal(wr2_blob *);
void wr2_snapshot(wr2_blob *);
void wr2_publish(const wr2_blob *);
void wr2_clear(void);

void wr2_boot(void);
void wr2_new_clear(void);
void *wr2_fileop_step(void *);
int wr2_save_begin(void);
int wr2_save_finish(void);
int wr2_restore(void);
#endif
