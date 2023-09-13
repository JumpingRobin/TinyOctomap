/* octoMap.c: Do the mapping task */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "octoMap.h"
#include "octoTree.h"
#include "octoNodeSet.h"
#include "rrtConnect.h"

#define FILE_LENGTH 1800
#define FILE_NAME_START "assets/start_points.csv"
#define FILE_NAME_END "assets/end_points.csv"

void octoMapInit(octoMap_t *octoMap)
{
    // init node set
    octoNodeSet_t* nodeSet;
    nodeSet = (octoNodeSet_t*)malloc(sizeof(octoNodeSet_t));
    // print nodeSet size
    printf("sizeof(octoNode) = %lu\n", sizeof(octoNode_t));
    octoNodeSetInit(nodeSet);
    printf("sizeof(octoNode) = %lu\n", sizeof(octoNode_t));

    // init octoMap
    octoMap->octoTree = octoTreeInit();
    octoMap->octoNodeSet = nodeSet;
    // avoid index 0 is used (octoNodeHasChildren will fail)
    octoMap->octoTree->root->children = octoNodeSetMalloc(octoMap->octoNodeSet);
    // print octoMap
    printf("octoMap.octoTree->center = (%d, %d, %d)\n", octoMap->octoTree->center.x, octoMap->octoTree->center.y, octoMap->octoTree->center.z);
    printf("octoMap.octoTree->origin = (%d, %d, %d)\n", octoMap->octoTree->origin.x, octoMap->octoTree->origin.y, octoMap->octoTree->origin.z);
    printf("octoMap.octoTree->resolution = %d\n", octoMap->octoTree->resolution);
    printf("octoMap.octoTree->maxDepth = %d\n", octoMap->octoTree->maxDepth);
    printf("octoMap.octoTree->width = %d\n", octoMap->octoTree->width);
    // print octoMap.octoTree->root
    printf("octoMap.octoTree->root->children = %d\n", octoMap->octoTree->root->children);
    printf("octoMap.octoTree->root->logOdds = %d\n", octoMap->octoTree->root->logOdds);
    printf("octoMap.octoTree->root->isLeaf = %d\n", octoMap->octoTree->root->isLeaf);
    // print octoMap.octoNodeSet
    printf("octoMap.octoNodeSet->freeQueueEntry = %d, octoMap.octoNodeSet->fullQueueEntry = %d\n\n", octoMap->octoNodeSet->freeQueueEntry, octoMap->octoNodeSet->fullQueueEntry);
    //print the length and numFree and numOccupied
    printf("octoMap.octoNodeSet->length = %d, octoMap.octoNodeSet->numFree = %d, octoMap.octoNodeSet->numOccupied = %d\n\n", octoMap->octoNodeSet->length, octoMap->octoNodeSet->numFree, octoMap->octoNodeSet->numOccupied);
    printf("octoMap.octoNodeSet->volumeFree = %d, octoMap.octoNodeSet->volumeOccupied = %d\n\n", octoMap->octoNodeSet->volumeFree, octoMap->octoNodeSet->volumeOccupied);
    return;
}
void testFromFile(coordinate_t *(start_points[FILE_LENGTH]), coordinate_t *(end_points[FILE_LENGTH]))
{
    char *start_file = FILE_NAME_START;
    char *end_file = FILE_NAME_END;
    FILE *fp1 = fopen(start_file, "r"); // 打开起点文件
    FILE *fp2 = fopen(end_file, "r"); // 打开终点文件
    if (fp1 == NULL || fp2 == NULL) {
        printf("无法打开文件 %s 或 %s\n", start_file, end_file);
    }
    char buffer1[1024]; // 缓冲区1
    char buffer2[1024]; // 缓冲区2

    int row = 0; // 行数
    int col = 0; // 列数
    int size = 0; // 数组大小

    while (row < FILE_LENGTH && fgets(buffer1, sizeof(buffer1), fp1) && fgets(buffer2, sizeof(buffer2), fp2)) { // 同时读取两个文件的一行
        row++;

        char *token1 = strtok(buffer1, ","); // 按逗号分割字符串
        coordinate_t p_start;
        col = 0;
        while (token1) {
            col++;
            if (col == 1) {
                token1 = strtok(NULL, ","); // 继续分割
                continue; // 跳过第一列
            }
            switch(col) { // 根据列数赋值给Point结构体
                case 2:
                    p_start.x = atoi(token1);
                    break;
                case 3:
                    p_start.y = atoi(token1);
                    break;
                case 4:
                    p_start.z = atoi(token1);
                    break;
                default:
                    break;
            }

            token1 = strtok(NULL, ","); // 继续分割
        }

        char *token2 = strtok(buffer2, ","); // 按逗号分割字符串
        coordinate_t p_end;
        col = 0;
        while (token2) {
            col++;
            if (col == 1) {
                token2 = strtok(NULL, ","); // 继续分割
                continue; // 跳过第一列
            }
            switch(col) { // 根据列数赋值给Point结构体
                case 2:
                    p_end.x = atoi(token2);
                    break;
                case 3:
                    p_end.y = atoi(token2);
                    break;
                case 4:
                    p_end.z = atoi(token2);
                    break;
                default:
                    break;
            }

            token2 = strtok(NULL, ","); // 继续分割
        }

        size++; // 数组大小加一
        start_points[size - 1]->x = p_start.x;
        start_points[size - 1]->y = p_start.y;
        start_points[size - 1]->z = p_start.z;
        end_points[size - 1]->x = p_end.x;
        end_points[size - 1]->y = p_end.y;
        end_points[size - 1]->z = p_end.z;

        // print start_points and end_points
//        printf("row = %d\n", row);
//        printf("start_points[%d] = (%d, %d, %d)\n", size - 1, start_points[row - 1]->x, start_points[row - 1]->y, start_points[row - 1]->z);
//        printf("end_points[%d] = (%d, %d, %d)\n", size - 1, end_points[row - 1]->x, end_points[row - 1]->y, end_points[row - 1]->z);
    }
}

int leafCountRecursive = 0;
void recursiveExportOctoMap(octoMap_t* octoMap, octoNode_t* node, coordinate_t origin, uint16_t width, FILE* f_octoMap){
    if (node->isLeaf) {
        if(LOG_ODDS_FREE == node->logOdds ){
            fprintf(f_octoMap,",FN,,%d,%d,%d,%d\n",origin.x,origin.y,origin.z,width);   
        }
        else if(LOG_ODDS_OCCUPIED == node->logOdds){
            fprintf(f_octoMap,",ON,,%d,%d,%d,%d\n",origin.x,origin.y,origin.z,width);
        }
        // DEBUG_PRINT("node->x = %d, node->y = %d, node->z = %d, node->width = %d, node->logOdds = %d\n", node->origin.x, node->origin.y, node->origin.z, width, node->logOdds);
        // fprintf(fp, "%d, %d, %d, %d, %d\n", node->origin.x, node->origin.y, node->origin.z, width, node->logOdds);
    } else {
        for (int i = 0; i < 8; i++) {
            if (octoNodeHasChildren(node) && width > octoMap->octoTree->resolution) {
                coordinate_t newOrigin = calOrigin(i,origin,width);
                recursiveExportOctoMap(octoMap, &octoMap->octoNodeSet->setData[node->children].data[i], newOrigin, width / 2, f_octoMap);
            }
        }
    }
}

void iterativeExportOctoMap(octoMap_t* octoMap) {
    octoNode_t* cur;
    int leafCount = 0;
    for (int i = 0; i < NODE_SET_SIZE; i++) {
        for (int j = 0; j < 8; j++) {
            cur = &octoMap->octoNodeSet->setData[i].data[j];
            if (cur->isLeaf) {
                if (octoNodeLogOddsIsOccupiedOrFree(cur)) {
                    leafCount++;
                }
            }
        }
    }
    printf("[iterativeExportOctoMap]leafCount = %d\n", leafCount);
}

void exportOctoMap(octoMap_t* octoMap) {
    FILE *fp = fopen("assets/octoMap.csv", "w");
    if (fp == NULL) {
        printf("无法打开文件 octoMap.csv\n");
    }
    // octoNode_t* node = octoMap->octoTree->root;
    // recursiveExportOctoMap(octoMap, node, fp, octoMap->octoTree->width);
    iterativeExportOctoMap(octoMap);
    printf("[recursiveExportOctoMap]leafCount = %d\n", leafCountRecursive);
}

void printOctoMapNodeDistribution(octoMap_t* octoMap, int times, FILE *fp) {
    int nodeCount = 0;
    int occupiedCount = 0;
    int freeCount = 0;
    int unknownCount = 0;
    for (int i = 0; i < NODE_SET_SIZE; i++) {
        for (int j = 0; j < 8; j++) {
            nodeCount++;
            octoNode_t* node = &octoMap->octoNodeSet->setData[i].data[j];
            if (node->logOdds == LOG_ODDS_OCCUPIED) {
                occupiedCount++;
            } else if (node->logOdds == LOG_ODDS_FREE) {
                freeCount++;
            } else {
                unknownCount++;
            }
        }
    }
    fprintf(fp, "%d, %d, %d, %d, %d\n", times, nodeCount, occupiedCount, freeCount, unknownCount);
}

// int main()
// {
//     // malloc start_points and end_points
//     coordinate_t *(start_points[FILE_LENGTH]);
//     for (int i = 0; i < FILE_LENGTH; i++) {
//         start_points[i] = malloc(sizeof(coordinate_t));
//     }
//     coordinate_t *(end_points[FILE_LENGTH]);
//     for (int i = 0; i < FILE_LENGTH; i++) {
//         end_points[i] = malloc(sizeof(coordinate_t));
//     }
//     testFromFile(start_points, end_points);

//     octoMap_t* octoMap;
//     octoMap = malloc(sizeof(octoMap_t));
//     octoMapInit(octoMap);

//     // test from file
//     for (int i = 0; i < FILE_LENGTH; i++) {
//         octoTreeRayCasting(octoMap->octoTree, octoMap, start_points[i], end_points[i]);
//     }
//     exportOctoMap(octoMap);
//     return 0;

//     // test rrt
//     array_t res;
//     res.len = 0;
//     int i = 0;
//     do
//     {
//         ++i;
//         printf("--------------------------i = %d---------------------------\n", i);
//         planning(start_points[0], end_points[180], octoMap->octoTree, octoMap, &res);
//     } while (0 && res.len == 0);
//     printf("res.len = %d\n", res.len);
//     for (int i = 0; i < res.len; i++) {
//         printf("res[%d] = (%d, %d, %d)\n", i, res.arr[i].loc.x, res.arr[i].loc.y, res.arr[i].loc.z);
//     }

//     // free
//     for (int i = 0; i < FILE_LENGTH; i++) {
//         free(start_points[i]);
//         free(end_points[i]);
//     }
//     free(octoMap->octoNodeSet);
//     free(octoMap);
//     return 0;
// }