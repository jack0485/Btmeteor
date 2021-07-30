#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "parse_metafile.h"
#include "bitfield.h"
#include "peer.h"
#include "data.h"
#include "tracker.h"
#include "torrent.h"
#include "signal_handler.h"

extern int download_piece_num;
extern int *fds;
extern int fds_len;
extern Peer *peer_head;

// 程序将退出时，执行一些清理工作
void do_clear_work() {
    // 关闭所有peer的socket
    Peer *p = peer_head;
    while (p != NULL) {
        if (p->state != CLOSING) close(p->socket);
        p = p->next;
    }

    // 保存位图
    if (download_piece_num > 0) {
        restore_bitmap();
    }
    // 关闭文件描述符
    int i;
    for (i = 0; i < fds_len; i++) {
        close(fds[i]);
    }
    // 释放动态分配的内存
    release_memory_in_parse_metafile();
    release_memory_in_bitfield();
    release_memory_in_btcache();
    release_memory_in_peer();
    release_memory_in_torrent();

    exit(0);
}

void process_signal(int signo) {
    printf("Please wait for clear operations\n");
    do_clear_work();
}

// 设置信号处理函数
int set_signal_handler() {
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        perror("can not catch signal:sigpipe\n");
        return -1;
    }

    if (signal(SIGINT, process_signal) == SIG_ERR) {
        perror("can not catch signal:sigint\n");
        return -1;
    }

    if (signal(SIGTERM, process_signal) == SIG_ERR) {
        perror("can not catch signal:sigterm\n");
        return -1;
    }

    return 0;
}
