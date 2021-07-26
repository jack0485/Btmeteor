#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "parse_metafile.h"
#include "bitfield.h"

extern int pieces_length;
extern char *file_name;

Bitmap *bitmap = NULL;          // 指向位图
int download_piece_num = 0;     // 当前已下载的piece数

int create_bitfield() {
    bitmap = (Bitmap *)malloc(sizeof(Bitmap));
    if (bitmap == NULL) {
        printf("allocate memory for bitmap failed\n");
        return -1;
    }

    // pieces_length除以20即为总的piece数
    bitmap->valid_length = pieces_length / 20;
    bitmap->bitfield_length = pieces_length / 20 / 8;
    if ((pieces_length/20)%8 != 0) bitmap->bitfield_length++;

    bitmap->bitfield = (unsigned char *)malloc(bitmap->bitfield_length);
    if (bitmap->bitfield == NULL) {
        printf("allocate memory for bitmap->bitfield failed\n");
        if (bitmap != NULL) free(bitmap);
        return -1;
    }

    char bitmapfile[64];
    sprintf(bitmapfile, "%dbitmap", pieces_length);
    int i;
    FILE *fp = fopen(bitmapfile, "rb");
    if (fp == NULL) {   // 若文件打开失败，说明开始的是一个全新的下载
        memset(bitmap->bitfield, 0, bitmap->bitfield_length);
    } else {
        fseek(fp, 0, SEEK_SET);
        for (i = 0; i < bitmap->bitfield_length; i++)
            (bitmap->bitfield)[i] = fgetc(fp);
        fclose(fp);
        // 给download_piece_num赋新的初值
        download_piece_num = get_download_piece_num();
    }

    return 0;
}

int get_bit_value(Bitmap *bitmap, int index) {
    int ret;
    int byte_index;
    unsigned char byte_value;
    unsigned char inner_byte_index;

    if (bitmap == NULL || bitmap->bitfield == NULL) return -1;
    if (index >= bitmap->valid_length || index < 0) return -1;
    // 获取字节下标
    byte_index = index / 8;
    // 根据下标获取对应字节值
    byte_value = bitmap->bitfield[byte_index];
    // 字节位偏移
    inner_byte_index = index % 8;
    // 字节位右移
    byte_value = byte_value >> (8 - inner_byte_index);
    // 判断是否为偶数，是则最低位必为0，否则为1，也即index对应的位为0或1
    if (byte_value % 2 == 0) ret = 0;
    else ret = 1;

    return ret;
}

int set_bit_value(Bitmap *bitmap, int index, unsigned char value) {
    int byte_index;
    unsigned char inner_byte_index;

    if (bitmap == NULL || bitmap->bitfield == NULL) return -1;
    if (index >= bitmap->valid_length || index < 0) return -1;
    byte_index = index / 8;
    inner_byte_index = index % 8;
    switch (value) {
        case 0:
            bitmap->bitfield[byte_index] &= ~(1U << (8 - inner_byte_index));
            return 0;
        case 1:
            bitmap->bitfield[byte_index] |= (1U << (8 - inner_byte_index));
            return 0;
        default:
            printf("value not 0 or 1\n");
            return -1;
    }
}

int all_zero(Bitmap *bitmap) {
    if (bitmap->bitfield == NULL) return -1;

    memset(bitmap->bitfield, 0, bitmap->bitfield_length);
    return 0;
}

int all_set(Bitmap *bitmap) {
    if (bitmap->bitfield == NULL) return -1;

    memset(bitmap->bitfield, 0xff, bitmap->bitfield_length);
    return 0;
}

void release_memory_in_bitfield() {
    if (bitmap->bitfield != NULL) free(bitmap->bitfield);
    if (bitmap != NULL) free(bitmap);
}

int print_bitfield(Bitmap *bitmap) {
    int num = 0;
    if (bitmap == NULL || bitmap->bitfield == NULL) return -1;

    for (int i = 0; i < bitmap->valid_length; ++i) {
        get_bit_value(bitmap, i) ? printf("* "):printf(" ");
        if (i+1 % 20 == 0) printf("\n");
    }

    return 0;
}

int restore_bitmap() {
    int fd;
    char bitmapfile[64];

    if ( (bitmap == NULL) || (bitmap->bitfield == NULL) || (file_name == NULL)) return -1;
    sprintf(bitmapfile, "%dbitmap", pieces_length);
    fd = open(bitmapfile, O_RDWR|O_CREAT|O_TRUNC, 0666);
    if (fd < 0) { printf("bitmapfile open failed\n"); return -1; }
    write(fd, bitmap->bitfield, bitmap->bitfield_length);
    close(fd);

    return 0;
}

int is_interested(Bitmap *dst, Bitmap *src) {
    unsigned char const_char[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
    unsigned char c1, c2;
    int i, j;

    if (dst == NULL || src == NULL) return -1;
    if (dst->bitfield == NULL || src->bitfield == NULL) return -1;
    if (dst->bitfield_length != src->bitfield_length || dst->valid_length != src->valid_length) return -1;

    // 如果dst中某位为1而src对应为0，则说明src对dst感兴趣
    for (i = 0; i < dst->bitfield_length-1; i++) {
        for (j = 0; j < 8; j++) {   // 比较某个字节的所有位
            c1 = (dst->bitfield)[i] & const_char[j];    // 获取每一位的值
            c2 = (src->bitfield)[i] & const_char[j];
            if (c1 > 0 && c2 == 0) return 1;
        }
    }

    // 位图的valid_length可能小于bitfield_length*8，也就是最后一个字节有效位数可能不等，
    // 需要单独比较
    j = dst->valid_length % 8;
    c1 = dst->bitfield[dst->bitfield_length-1];
    c2 = src->bitfield[src->bitfield_length-1];
    for (i = 0; i < j; i++) {   // 比较位图的最后一个字节
        if ((c1&const_char[i]) > 0 && (c2&const_char[i]) == 0)
            return 1;
    }
    return 0;
}

int get_download_piece_num() {
    unsigned char const_char[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
    int i, j;

    if (bitmap == NULL || bitmap->bitfield == NULL) return -1;
    download_piece_num = 0;

    for (i = 0; i < bitmap->bitfield_length-1; i++) {
        for (j = 0; j < 8; j++) {
            if (((bitmap->bitfield)[i] & const_char[j]) != 0)
                download_piece_num++;
        }
    }

    unsigned char c = (bitmap->bitfield)[i];    // c存放位图最后一个字节的值
    j = bitmap->valid_length % 8;               // j是位图最后一个字节的有效位数
    for (i = 0; i < j; i++) {
        if ( (c & const_char[i]) != 0) download_piece_num++;
    }
    return download_piece_num;
}
