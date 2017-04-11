#ifndef CYCLIC_BUF_H
#define CYCLIC_BUF_H

void cyclic_buf_init(struct cyclic_buf *cb, int num, int data_size);
int cyclic_buf_produce_data(struct cyclic_buf *cb, void* data, int data_size);
void *cyclic_buf_produce_get(struct cyclic_buf *cb);
void cyclic_buf_produce(struct cyclic_buf *cb);
void *cyclic_buf_consume_get(struct cyclic_buf *cb);
void cyclic_buf_consume(struct cyclic_buf *cb);
void dump_cyclic_buf(struct cyclic_buf *cb, const char *msg);
void cyclic_buf_reset(struct cyclic_buf *cb);
void cyclic_buf_free(struct cyclic_buf *cb);
#endif