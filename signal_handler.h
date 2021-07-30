#ifndef BTMETEOR_SIGNAL_HANDLER_H
#define BTMETEOR_SIGNAL_HANDLER_H

// 做一些清理工作，如释放动态分配的内存
void do_clear_work();
// 处理一些信号
void process_signal(int signo);
// 设置信号处理函数
int set_signal_handler();

#endif //BTMETEOR_SIGNAL_HANDLER_H
