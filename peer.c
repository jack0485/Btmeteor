#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "peer.h"
#include "message.h"
#include "bitfield.h"

extern Bitmap *bitmap;

// 指向当前与之进行通信的peer链表
Peer *peer_head = NULL;

void initialize_peer_failed(Peer *peer) {
    if (peer->in_buff != NULL) free(peer->in_buff);
    if (peer->out_msg != NULL) free(peer->out_msg);
    if (peer->out_msg_copy != NULL) free(peer->out_msg_copy);
}

int initialize_peer(Peer *peer) {
    if (peer == NULL) return -1;
    peer->socket = -1;
    memset(peer->ip, 0, 16);
    peer->port = 0;
    memset(peer->id, 0, 21);
    peer->state = INITIAL;

    peer->in_buff = NULL;
    peer->out_msg = NULL;
    peer->out_msg_copy = NULL;

    peer->in_buff = (char *)malloc(MSG_SIZE);
    if (peer->in_buff == NULL) { initialize_peer_failed(peer); return -1;}
    memset(peer->in_buff, 0, MSG_SIZE);
    peer->buff_len = 0;

    peer->out_msg = (char *)malloc(MSG_SIZE);
    if (peer->out_msg == NULL) { initialize_peer_failed(peer); return -1;}
    memset(peer->out_msg, 0, MSG_SIZE);
    peer->msg_len = 0;

    peer->out_msg_copy = (char *)malloc(MSG_SIZE);
    if (peer->out_msg_copy == NULL) { initialize_peer_failed(peer); return -1;}
    memset(peer->out_msg_copy, 0, MSG_SIZE);
    peer->msg_copy_len = 0;
    peer->msg_copy_index = 0;

    peer->am_choking = 1;
    peer->am_interested = 0;
    peer->peer_choking = 1;
    peer->peer_interested = 0;

    peer->bitmap.bitfield = NULL;
    peer->bitmap.bitfield_length = 0;
    peer->bitmap.valid_length = 0;

    peer->Request_piece_head = NULL;
    peer->Requested_piece_head = NULL;

    peer->down_total = 0;
    peer->up_total = 0;

    peer->start_timestamp = 0;
    peer->recet_timestamp = 0;

    peer->last_down_timestamp = 0;
    peer->last_up_timestamp = 0;
    peer->down_count = 0;
    peer->up_count = 0;
    peer->down_rate = 0.0f;
    peer->up_rate = 0.0f;

    peer->next = NULL;

    return 0;
}

Peer *add_peer_node() {
    int ret;
    Peer *node, *p;

    // 分配内存空间
    node = (Peer *)malloc(sizeof(Peer));
    if (node == NULL) {
        fprintf(stderr, "%s:%d error\n", __FILE__, __LINE__);
        return NULL;
    }
    // 进行初始化
    ret = initialize_peer(node);
    if (ret < 0) {
        fprintf(stderr, "%s:%d error\n", __FILE__, __LINE__);
        free(node);
        return NULL;
    }
    // 将node加入到peer链表中
    if (peer_head == NULL) { peer_head = node; }    // node为peer链表的第一个节点
    else {
        p = peer_head;  // 使p指针指向peer链表的最后一个节点
        while (p->next != NULL) p = p->next;
        p->next = node;
    }

    return node;
}

int del_peer_node(Peer *peer) {
    Peer *p = peer_head, *q;
    if (peer == NULL) return -1;

    while (p != NULL) {
        if (p == peer) {
            if (p == peer_head) {peer_head = p->next;}
            else q->next = p->next;     // 找到peer但不为头，则q一定指向前一个节点
            free_peer_node(p);
            return 0;
        } else {
            q = p;
            p = p->next;
        }
    }

    return -1;
}

void free_peer_node(Peer *node) {

}

int cancel_request_list(Peer *node) {
    Request_piece *p = node->Request_piece_head;
    while (p != NULL) {
        node->Request_piece_head = node->Request_piece_head->next;
        free(p);
        p = node->Request_piece_head;
    }

    return 0;
}

int cancel_requested_list(Peer *node) {
    Request_piece *p = node->Requested_piece_head;
    while (p != NULL) {
        node->Requested_piece_head = node->Requested_piece_head->next;
        free(p);
        p = node->Requested_piece_head;
    }

    return 0;
}

void release_memory_in_peer() {

}

void print_peers_data() {

}
