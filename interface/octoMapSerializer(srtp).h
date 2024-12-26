#ifndef OCTOMAPSERIALIZER_H
#define OCTOMAPSERIALIZER_H
#include "stdlib.h"
#include "octoMap.h"
#include "octoNodeSet.h"
#include "octoNode.h"
#include "octoTree.h"

#define INF 0x3f3f3f3f 
#define MAX_SERIALIZER_SIZE 4*1024
#define MAX_DICT_SIZE 1024

typedef struct 
{
    coordinate_t center;     // the coordinate of the center --- (x,y,z): tuple
    coordinate_t origin;     // the origin coordinate of this tree --- (x,y,z): tuple
    uint8_t resolution;      // resolution of the tree
    uint8_t maxDepth;        // max depth of the tree
    uint16_t width;
    uint8_t data[MAX_SERIALIZER_SIZE];
}octoMapSerializerResult_t;

typedef struct 
{
    int size; //字典实际使用长度
    uint8_t value[MAX_DICT_SIZE]; // 字典值
    uint16_t times[MAX_DICT_SIZE]; // 字典值出现次数
}dict_t;

// 哈夫曼节点--包含当前字典值出现次数与
typedef struct
{
    int value; // 哈夫曼节点键值
    uint16_t times; // 键值出现次数
    uint16_t left; // 左孩子索引
    uint16_t right; // 右孩子索引
}HuffmanTreeNode;

typedef struct
{
    uint16_t size; // 哈夫曼树节点数
    uint16_t root; // 根节点索引
    uint16_t free; // 指向下一个空闲位置
    HuffmanTreeNode nodes[MAX_DICT_SIZE * 2 - 1]; // 最多 2n-1 个节点
}HuffmanTree;

// 八叉树序列化 @何辰玮
void serializeOctoMap(octoMap_t *octoMap, uint8_t *result);

// 八叉树反序列化 @何辰玮
void deserializeOctoMap(octoMap_t *octoMap, uint8_t *data);

// 压缩数据 = 再编码+哈夫曼编码
void compressData(uint8_t *data, uint8_t *result);

// 解压缩数据 @张玮东
void decompressData(uint8_t *data, uint8_t *result, dict_t *dict);

// 再编码 @董乐天
void encodeData(uint8_t *data, uint8_t *result, dict_t *dict);

// 哈夫曼编码 @李宾
void huffmanEncode(uint8_t* data, dict_t* oldDict, uint8_t* result, HuffmanTree* newDict, int* resultBitSize); // 哈夫曼编码调用函数
#endif