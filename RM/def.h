//
// Created by 赵鋆峰 on 2019/10/28.
//

#ifndef ROBODBMS_DEF_H
#define ROBODBMS_DEF_H

#include "../PF/def.h"

struct RM_FileHeader {
    int recordSize;
    int recordNumPerPage;
    int bitMapOffset;
    int bitMapSize;
    PageNum firstFreePage;
    int pageCount;
};

struct RM_PageHeader {
    int recordNum;
    PageNum nextFreePage;
};

typedef char MultiBits;
#define NO_MORE_FREE_PAGE -1

#define RM_FILE_HEADER_SIZE sizeof(RM_FileHeader)
#define RM_PAGE_HEADER_SIZE sizeof(RM_PageHeader)

#endif //ROBODBMS_DEF_H
