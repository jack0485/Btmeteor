#ifndef BTMETEOR_DATA_H
#define BTMETEOR_DATA_H
#include "peer.h"

// 每个Btcache节点维护一个长度为16KB的缓冲区，该缓冲区保存一个slice的数据
typedef struct _Btcache {
    unsigned char *buff;        // 指向缓冲区的指针
    int index;                  // 数据所在的piece块的索引
    int begin;                  // 数据所在piece块中的起始位置
    int length;                 // 数据的长度

    unsigned char in_use;       // 该缓冲区是否在使用中
    unsigned read_write;        // 是发送给

    unsigned char is_full;
    unsigned char is_writed;
    int access_count;
    struct _Btcache *next;
} Btcache;

Btcache *initialize_btcache_node(); // 为Btcache节点分配内存空间并进行初始化
int create_btcache();               // 创建总大小为16K*1024即16MB的缓冲区
void release_memory_in_btcache();   // 释放data.c中动态分配的内存

int get_files_count();              // 获取种子文件中待下载的文件个数
int create_files();                 // 根据种子文件中的信息创建保存下载数据的文件

// 判断一个Btcache节点中的数据要写到哪个文件以及具体位置，并写入
int write_btcache_node_to_harddisk(Btcache *node);
// 从硬盘读出一个slice的数据存放到缓冲区中，在peer需要时发送给peer
// 要读入的slice的索引，index、begin、length已存到node所指向的节点中
int read_slice_from_harddisk(Btcache *node);
// 检查一个piece的数据是否正确，若正确则写入硬盘上的文件
int write_piece_to_harddisk(int sequence, Peer *peer);
// 从硬盘上的文件中读取一个piece存放到p指针所指向的缓冲区中
int read_piece_from_harddisk(Btcache *p, int index);

// 将整个缓冲区中已下载的数据写入到硬盘上的文件中
int write_btcache_to_harddisk(Peer *peer);
// 当缓冲区不够用时，释放那些从硬盘上读取的piece
int release_read_btcache_node(int base_count);
// 从btcache缓冲区中清除那些未完成下载的piece
void clear_btcache_before_peer_close(Peer *peer);
// 将刚刚从peer处获取的一个slice存放到缓冲区中
int write_slice_to_btcache(int index, int begin, int length, unsigned char *buff,
                           int len, Peer *peer);
// 从缓冲区获取一个slice，读取的slice存放到peer的发送缓冲区中
int read_slice_for_send(int index, int begin, int length, Peer *peer);

// 以下是为下载和上传最后一个piece而增加的函数
// 最后一个piece较为特殊，因为它是一个不完整的piece
int write_last_piece_to_btcache(Peer *peer);
int write_slice_to_last_piece(int index, int begin, int length, unsigned char *buff,
                              int len, Peer *peer);
int read_last_piece_from_harddisk(Btcache *p, int index);
int read_slice_for_send_last_piece(int index, int begin, int length, Peer *peer);
void release_last_piece();

#endif //BTMETEOR_DATA_H
