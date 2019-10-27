//
// Created by 赵鋆峰 on 2019/10/27.
//

#include <cstring>
#include "RM_Record.h"

RM_Record::RM_Record() {

}

RM_Record::~RM_Record() {

}

RM_Record::RM_Record(RM_RID &rmRid, char *bufferData, int size) {
    this->rmRid = rmRid;
    data = new char[size];
    memcpy(data, bufferData, size);
    this->size = size;
}

RM_Record::RM_Record(const RM_Record &rmRecord) {
    this->rmRid = rmRecord.rmRid;
    data = new char[rmRecord.size];
    this->size = rmRecord.size;
    memcpy(data, rmRecord.data, size);
}

RM_Record &RM_Record::operator=(const RM_Record &rmRecord) {
    this->rmRid = rmRecord.rmRid;
    delete[] data;
    data = new char[rmRecord.size];
    this->size = rmRecord.size;
    memcpy(data, rmRecord.data, size);
}

RC RM_Record::GetData(char *&pData) const {
    if (data == nullptr) {
        return RM_INVALIDRECORD;
    }
    pData = data;
    return OK_RC;
}

RC RM_Record::GetRid(RM_RID &rid) const {
    rid = this->rmRid;
    return OK_RC;
}
