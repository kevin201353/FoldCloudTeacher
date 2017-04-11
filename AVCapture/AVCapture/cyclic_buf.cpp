#include <stdint.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "cyclic_buf.h"
#include "global.h"

#define CYCLIC_INTERNAL_BUF_SIZE	0x1400000

static uint8_t cyclic_internal_buf[CYCLIC_INTERNAL_BUF_SIZE];
static int cyclic_alloc_size = 0;

void cyclic_buf_init(struct cyclic_buf *cb, int num, int data_size)
{
	int i;
	assert(num<256);
	cb->p_index = -1;
	cb->c_index = -1;
	cb->num = num;
	cb->data_size = data_size;
	for( i=0; i<num; i++ ){
		cb->data[i] = cyclic_internal_buf + cyclic_alloc_size;
		cb->valid[i] = 0;
		cyclic_alloc_size += data_size;
		if( cyclic_alloc_size > CYCLIC_INTERNAL_BUF_SIZE ){
			fprintf(stderr, "cyclic_buf_init: out of memory: %d > %d",
				cyclic_alloc_size, CYCLIC_INTERNAL_BUF_SIZE);
			exit(1);
		}
	}
	fprintf(stderr, "cyclic_buf_init: ------------- cyclic_alloc_size=0x%x\n", cyclic_alloc_size);
}

void cyclic_buf_reset(struct cyclic_buf *cb)
{
	int i;
	cb->p_index = -1;
	cb->c_index = -1;
	for( i=0; i<cb->num; i++ )
	{
		cb->valid[i] = 0;
	}
}

void cyclic_buf_free(struct cyclic_buf *cb)
{
	int i;

	for( i=0; i<cb->num; i++ )
		free(cb->data[i]);
}

int cyclic_buf_produce_data(struct cyclic_buf *cb, void* data, int data_size)
{
	int valid=0;

	if( cb->data_size < data_size ){
		printf("cyclic_buf_produce_data: data_size=%d too big\n", data_size);
		return -1;
	}
	if(cb->p_index == -1){
		valid = 1;
		cb->p_index = 0;
	}else if( (cb->p_index >= 0) && (cb->valid[(cb->p_index + 1) % cb->num] == 0 ) ){
		valid = 1;
		cb->p_index = (cb->p_index + 1) % cb->num;
	}
	if( !valid )
		return -1;
	memcpy(cb->data[cb->p_index], data, data_size);
	cb->valid[cb->p_index] = 1;
	return cb->p_index;
}

void* cyclic_buf_produce_get(struct cyclic_buf *cb)
{
	int valid=0;

	if(cb->p_index == -1){
		valid = 1;
		cb->p_index = 0;
	}else if( (cb->p_index >= 0) && (cb->valid[(cb->p_index + 1) % cb->num] == 0 ) ){
		valid = 1;
		cb->p_index = (cb->p_index + 1) % cb->num;
	}

	if( !valid )
		return NULL;

	return cb->data[cb->p_index];
}

void cyclic_buf_produce(struct cyclic_buf *cb)
{
	cb->valid[cb->p_index] = 1;
}

void *cyclic_buf_consume_get(struct cyclic_buf *cb)
{
	int valid=0;

	if( (cb->c_index == -1) && (cb->p_index >= 0) ){
		cb->c_index = 0;
		valid = 1;
	}else if( (cb->p_index >= 0) && (cb->c_index >= 0) && cb->valid[(cb->c_index + 1)%cb->num]){
		cb->c_index = (cb->c_index + 1) % cb->num;
		valid = 1;
	}

	if( ! valid ) 
		return NULL;

	return cb->data[cb->c_index];
}

void cyclic_buf_consume(struct cyclic_buf *cb)
{
	cb->valid[cb->c_index] = 0;
}

void dump_cyclic_buf(struct cyclic_buf *cb, const char *msg)
{
	int i;

	printf("==== Dump of cypclic buf %p: %s ====\n", cb, msg);
	printf("p_index=%d, c_index=%d, num=%d, data_size=%d\n", 
		cb->p_index, cb->c_index, cb->num, cb->data_size);
	for( i=0; i<cb->num; i++ ){
		printf("valid[%d]=%d\n", i, cb->valid[i]);
	}
	printf("\n");
}
