#ifndef BTMETEOR_BITFIELD_H
#define BTMETEOR_BITFIELD_H

typedef struct _Bitmap {
    unsigned char *bitfield;        // 保存位图
    int bitfield_length;            // 位图所占的总字节数
    int valid_length;               // 位图有效的总位数，每一位代表一个piece
} Bitmap;

/*功能：创建位图，分配内存并进行初始化
 * 返回值：成功返回0，出错返回-1*/
int create_bitfield();

/*功能：获取位图中某一位的值
 * 参数：bitmap位图指针 index某一位下标
 * 返回值：成功返回下标值，出错返回-1*/
int get_bit_value(Bitmap *bitmap, int index);

/*功能：设置位中某一位的值
 * 参数：bitmap位图指针 index某一位下标 value为0或者1
 * 返回值：成功返回0，出错返回-1*/
int set_bit_value(Bitmap *bitmap, int index, unsigned char value);

/*功能：将位图所有位清0
 * 参数：@bitmap位图指针
 * 返回值：成功返回0，出错返回-1*/
int all_zero(Bitmap *bitmap);

/*功能：将位图所有位置1
 * 参数：bitmap位图指针
 * 返回值：成功返回0，出错返回-1*/
int all_set(Bitmap *bitmap);

/*功能：释放bitfield.c中动态分配的内存*/
void release_memory_in_bitfield();

/*功能：打印位图值，用于测试
 * 参数：bitmap位图指针
 * 返回值：成功返回0，失败返回-1*/
int print_bitfield(Bitmap *bitmap);

/*功能：位图存储到文件中，在下次下载时，先读取该文件获取已经下载的进度*
 * 返回值：成功返回0，失败返回-1*/
int restore_bitmap();

/*功能：拥有位图src的peer是否对拥有dst位图的peer感兴趣
 * 参数：dst目标位图 src源位图
 * 返回值：感兴趣返回1，否则返回0*/
int is_interested(Bitmap *dst, Bitmap *src);

/*功能：获取当前已下载到的总piece数
 * 返回值：成功返回已下载piece数，出错返回-1*/
int get_download_piece_num();

#endif //BTMETEOR_BITFIELD_H
