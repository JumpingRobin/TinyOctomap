# include "octoMapSerializer.h"
# include <stdio.h>
# include <stdint.h>
# include <string.h>
# define NOTE 65535
# define INF 0x3f3f3f3f

// 内部记录字符编码
typedef struct
{
    uint8_t code[MAX_DICT_SIZE][MAX_DICT_SIZE]; // 每个字符的编码
    uint8_t length[MAX_DICT_SIZE];             // 每个字符编码的长度
} CodeTable;

// 初始化哈夫曼树--经验证，成功！
void initHuffmanTree(HuffmanTree* tree)
{
    tree->size = 0;
    tree->free = 0;
    tree->root = NOTE;
    // 将指定的哈夫曼树的节点置空
    memset(tree->nodes, 0, sizeof(tree->nodes));
}

// 寻找到当前出现次数times最小的两个索引位置--经验证，成功！
void findTwoSmallest(HuffmanTree* tree, int* min, int* next)
{
    *min = NOTE;
    *next = NOTE;
    for (int i = 0; i < tree->size; i++)
    {
        // 若节点i对应的times为0，则节点已合并
        if (tree->nodes[i].times == 0) continue;
        // *min未初始化或*min已初始化且i索引对应times小于*min索引对应times
        if (*min == NOTE || (*min != NOTE && tree->nodes[*min].times > tree->nodes[i].times))
        {
            *next = *min;
            *min = i;
        }
        // *next未初始化或nodes[i].times小于*min但大于*next
        else if (*next == NOTE || (*next != NOTE && tree->nodes[*next].times > tree->nodes[i].times))
        {
            *next = i;
        }
    }
}

// 构建哈夫曼树--经检验，成功！
void buildHuffmanTree(dict_t* oldDict, HuffmanTree* tree)
{
    // 将旧字典中的数据迁移到HuffmanTree的nodes[]中
    for (int i = 0; i < oldDict->size; i++)
    {
        tree->nodes[tree->free].value = oldDict->value[i];
        tree->nodes[tree->free].times = oldDict->times[i];
        tree->nodes[tree->free].left = NOTE;
        tree->nodes[tree->free].right = NOTE;
        tree->free++; // free++ -> 地址顺序分配
        tree->size++;
    }
    // 原始字典大小--ori_num
    int ori_num = tree->size;
    while (tree->size < 2 * ori_num - 1)
    {
        int min=0, next=0; 
        findTwoSmallest(tree, &min, &next);
        printf("最小索引：%d  次小索引：%d \n", min, next);
        // 合并节点的创建
        tree->nodes[tree->free].value = NOTE;
        tree->nodes[tree->free].times = tree->nodes[min].times + tree->nodes[next].times;
        tree->nodes[tree->free].left = min;
        tree->nodes[tree->free].right = next;
        tree->free++;
        tree->size++;
        // 标记最小节点为已合并(times=0)
        tree->nodes[min].times = 0;
        tree->nodes[next].times = 0;
    }
    // 根节点为当前最后一个有效节点
    tree->root = tree->size - 1;
    printf("当前HuffmanTree的根节点索引为:%d\n", tree->root);
}

// 递归生成哈夫曼编码
void generateCodes(HuffmanTree* tree, CodeTable* table, uint16_t node, uint8_t* code, uint8_t length)
{
    if (tree->nodes[node].left == NOTE && tree->nodes[node].right == NOTE)
    {
        // 叶子节点：保存编码
        int index = tree->nodes[node].value;
        table->length[index] = length;
        memcpy(table->code[index], code, length);
        return;
    }
    // 左子树，添加 '0'
    if (tree->nodes[node].left != NOTE)
    {
        code[length] = 0;
        generateCodes(tree, table, tree->nodes[node].left, code, length + 1);
    }
    // 右子树，添加 '1'
    if (tree->nodes[node].right != NOTE)
    {
        code[length] = 1;
        generateCodes(tree, table, tree->nodes[node].right, code, length + 1);
    }
}

// 哈夫曼树报文编码( int* resultBitSize为当前result报文的长度，从外部传入随意一个int即可 )
void huffmanEncode(uint8_t* data, dict_t* oldDict, uint8_t* result, HuffmanTree* newDict, int* resultBitSize)
{
    CodeTable table = { 0 };
    uint8_t tempCode[MAX_DICT_SIZE] = { 0 };
    // 构建哈夫曼树
    buildHuffmanTree(oldDict, newDict);
    // 生成哈夫曼编码表
    generateCodes(newDict, &table, newDict->root, tempCode, 0);
    // 压缩数据
    int bitPos = 0; // 记录压缩结果中已使用的位数
    memset(result, 0, MAX_DICT_SIZE); // 清空结果数组
    for (int i = 0; data[i] != '\0'; i++) // 遍历输入数据(事实上，这里'\0'的ascii码为0，当data[]中存在0时，就会暂停)
    {
        printf("当前为第%d个字符的编码\n",i+1);
        int index = data[i]; // 获取字典索引
        if (index > MAX_DICT_SIZE) continue;
        for (int j = 0; j < table.length[index]; j++) // 遍历当前符号的编码
        {
            if (table.code[index][j] == 1)
                result[bitPos / 8] |= (1 << (7 - (bitPos % 8))); // 设置结果中的相应位
            bitPos++;
        }
    }
    *resultBitSize = bitPos; // 返回总位数（非字节数）
}

int main()
{
    uint8_t data[] = { 3, 1, 4, 2, 5,'\0'}; // 输入数据
    dict_t oldDict = { 5, {5, 1, 2, 3, 4}, {10, 15, 12, 3, 5} }; //(size,value,times)
    uint8_t result[MAX_DICT_SIZE] = { 0 }; // 存储压缩结果
    HuffmanTree newDict;
    initHuffmanTree(&newDict);
    int result_length = 0;
    huffmanEncode(data, &oldDict, result, &newDict,&result_length);
    // 输出编码结果
    printf("Huffman Encoded Result (in bits): ");
    for (int i = 0; i < result_length; i++)
    {
        // 逐位输出编码结果
        printf("%d", (result[i / 8] >> (7 - (i % 8))) & 1);
    }
    printf("\n");
    printf("Total bits used: %d\n", result_length);
    return 0;
}
