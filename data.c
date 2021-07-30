#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>
#include "parse_metafile.h"
#include "bitfield.h"
#include "message.h"
#include "sha1.h"
#include "data.h"

extern char *file_name;     // 待下载文件的文件名
extern Files *files_head;   // 对于多文种子有效，存放各个文件的路径和长度
extern int file_length;     // 待下载文件的总长度
extern int piece_length;    // 每个piece的长度
extern char *pieces;        // 缓冲区pieces的长度
extern int pieces_length;   // 缓冲区pieces的长度

extern Bitmap *bitmap;      // 指向己方的位图
extern int download_piece_num;  // 记录已经下载了多少个piece
extern Peer *peer_head;     // 指向peer链表

#define btcache_len 1024    // 缓冲区中共有多少个Btcache节点
Btcache *btcache_head = NULL;   // 指向一个大小为16MB的缓冲区
Btcache *last_piece = NULL;     // 存放待下载文件的最后一个piece;
int last_piece_index = 0;       // 最后一个piece的索引，它的值为总piece数减1
int last_piece_count = 0;       // 针对最后一个piece，记录已下载了多少个slice
int last_slice_len = 0;         // 最后一个piece的最后一个slice的长度

int *fds = NULL;                // 存放文件描述符
int fds_len = 0;                // 指针fds所指向的数组的长度
int have_piece_index[64];        // 存放刚刚下载到的piece的索引
int end_mode = 0;               // 是否进入了终端模式，终端模式的含义参考BT协议


Btcache *initialize_btcache_node() {
    Btcache *node;
    node = (Btcache *)malloc(sizeof(Btcache));
    if (node == NULL) return NULL;
    node->buff = (unsigned char *)malloc(16*1024);
    if (node->buff == NULL) { if (node != NULL) free(node); return NULL; }

    node->index = -1;
    node->begin = -1;
    node->length = -1;

    node->in_use = 0;
    node->read_write = -1;
    node->is_full = 0;
    node->is_writed = 0;
    node->access_count = 0;
    node->next = NULL;

    return node;
}

int create_btcache() {
    int i;
    Btcache *node, *last;   // node指向刚刚创建的节点、last指向缓冲区中最后一个节点

    for (i = 0; i < btcache_len; i++) {
        node = initialize_btcache_node();
        if (node == NULL) {
            printf("%s:%d create_btcache error\n", __FILE__, __LINE__);
            release_memory_in_btcache();
            return -1;;
        }
        if (btcache_head == NULL) { btcache_head = node; last = node;}
        else { last->next = node; last = node; }
    }
    // 为存储最后一个piece申请空间
    int count = file_length % piece_length / (16*1024);
    if (file_length % piece_length % (16*1024) != 0) count++;
    last_piece_count = count;   // count为最后一个piece所含有的slice数
    last_slice_len = file_length % piece_length % (16*1024);
    if (last_slice_len == 0) last_slice_len = 16*1024;
    last_piece_index = pieces_length / 20 - 1;  // 最后一个piece的index值
    while (count > 0) {
        node = initialize_btcache_node();
        if (node == NULL) {
            printf("%s:%d create_btcache error\n", __FILE__, __LINE__);
            release_memory_in_btcache();
            return -1;
        }
        if (last_piece == NULL) { last_piece = node; last = node; }
        else { last->next = node; last = node; }

        count--;
    }

    for (i = 0; i < 64; i++) {
        have_piece_index[i] = -1;
    }
    return 0;
}

void release_memory_in_btcache() {
    Btcache *p = btcache_head;
    while (p != NULL) {
        btcache_head = p->next;
        if (p->buff != NULL) free(p->buff);
        free(p);
        p = btcache_head;
    }

    release_last_piece();
    if (fds != NULL) free(fds);
}

void release_last_piece() {
    Btcache *p = last_piece;
    while (p != NULL) {
        last_piece = p->next;
        if (p->buff != NULL) free(p->buff);
        free(p);
        p = last_piece;
    }
}

int get_files_count() {
    int count = 0;

    if (is_multi_files() == 0) return 1;
    Files *p = files_head;
    while (p != NULL) {
        count++;
        p = p->next;
    }

    return count;
}

int create_files() {
    int ret, i;
    char buff[1] = { 0x0 };

    fds_len = get_files_count();
    if (fds_len < 0) return -1;
    fds = (int *)malloc(fds_len * sizeof(int));
    if (fds == NULL) return -1;

    if (is_multi_files() == 0) {    // 待下载的为单文件
        *fds = open(file_name, O_RDWR|O_CREAT, 0777);
        if (*fds < 0) { printf("%s:%d error", __FILE__, __LINE__); return -1; }
        ret = lseek(*fds, file_length-1, SEEK_SET);
        if (ret < 0) { printf("%s:%d error", __FILE__, __LINE__); return -1; }
        ret = write(*fds, buff, 1);
        if (ret != 1) { printf("%s:%d error", __FILE__, __LINE__); return -1; }
    } else {    // 待下载的是多文件
        ret = chdir(file_name);
        if (ret < 0) {      // 改变目录失败，说明该目录还未创建
            ret = mkdir(file_name, 0777);
            if (ret < 0) { printf("%s:%d error", __FILE__, __LINE__); return -1; }
            ret = chdir(file_name);
            if (ret < 0) { printf("%s:%d error", __FILE__, __LINE__); return -1; }
        }
        Files *p = files_head;
        i  = 0;
        while (p != NULL) {
            fds[i] = open(p->path, O_RDWR|O_CREAT, 0777);
            if (fds[i] < 0) { printf("%s:%d error", __FILE__, __LINE__); return -1; }
            ret = lseek(fds[i], p->length-1, SEEK_SET);
            if (ret < 0) { printf("%s:%d error", __FILE__, __LINE__); return -1; }
            ret = write(fds[i], buff, 1);
            if (ret != 1) { printf("%s:%d error", __FILE__, __LINE__); return -1; }

            p = p->next;
            i++;
        }   // while循环结束
    }   // end else

    return 0;
}

int write_btcache_node_to_harddisk(Btcache *node) {
    return 0;
}
