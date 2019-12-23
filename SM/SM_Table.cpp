//
// Created by 赵鋆峰 on 2019/12/21.
//

#include <cmath>
#include "SM_Table.h"
#include "../utils/Utils.h"
#include "../RM/RM_Manager.h"
#include "../RM/RM_FileScan.h"

SM_Table::SM_Table(const TableMeta &tableMeta) : tableMeta(tableMeta) {
    recordSize = 0;
    for (int i = 0; i < tableMeta.columnNum; i++) {
        columnOffset[i] = recordSize;
        recordSize += tableMeta.columns[i].attrLength + 1;//多一位来存储是否是NULL，放在数据的开头
    }
    std::string recordFileName = Utils::getRecordFileName(tableMeta.createName);
    if (!access(recordFileName.c_str(), F_OK)) {
        RM_Manager::Instance().CreateFile(recordFileName.c_str(), recordSize);
    }
    RM_Manager::Instance().OpenFile(recordFileName.c_str(), rmFileHandle);
    std::string stringPoolFileName = Utils::getStringPoolFileName(tableMeta.createName);
    if (!access(stringPoolFileName.c_str(), F_OK)) {
        SP_Manager::CreateStringPool(stringPoolFileName.c_str());
    }
    SP_Manager::OpenStringPool(stringPoolFileName.c_str(), spHandle);
}

char *SM_Table::getColumnData(char *record, ColumnId columnId) {
    int offset = columnOffset[columnId];
    //开头第一位存的是isNull
    if (record[offset]) {
        return nullptr;
    } else {
        return record + offset + 1;
    }
}

RC SM_Table::setRecordData(char *record, std::vector<ColumnId> *columnIdList, std::vector<AttrValue> *constValueList) {
    //列为空则代表是默认的顺序排列
    if (columnIdList == nullptr) {
        //检测给定参数个数是否与行数相同
        if (constValueList->size() != tableMeta.columnNum)return QL_COLUMNS_VALUES_DONT_MATCH;
        for (int i = 0; i < tableMeta.columnNum; i++) {
            TRY(setColumnData(record, i, (*constValueList)[i]));
        }
    } else {
        //检测给定参数个数是否与行数相同
        if (columnIdList->size() != constValueList->size())return QL_COLUMNS_VALUES_DONT_MATCH;
        bool hasValue[tableMeta.columnNum];
        for (int i = 0; i < tableMeta.columnNum; i++)hasValue[i] = false;
        for (int i = 0; i < columnIdList->size(); i++) {
            hasValue[(*columnIdList)[i]] = true;
            TRY(setColumnData(record, (*columnIdList)[i], (*constValueList)[i]));
        }
        for (int i = 0; i < tableMeta.columnNum; i++)
            if (!hasValue[i]) {
                TRY(setColumnNull(record, i));
            }
    }
    return OK_RC;
}

RC SM_Table::insertRecord(const char *record) {
    RM_RID rmRid;
    //TODO 插入之前要进行逻辑判断，主键是否重复、外键是否存在
    TRY(rmFileHandle.InsertRec(record, rmRid));
    return OK_RC;
}

void SM_Table::showRecords(int num) {
    //分割线
    std::string splitLine(tableMeta.columnNum * COLUMN_SHOW_LENGTH, '-');
    std::cout << splitLine << std::endl;
    std::string headerLine;
    for (int i = 0; i < tableMeta.columnNum; i++) {
        std::string columnName = std::string(tableMeta.columns[i].name);
        headerLine.append(columnName);
        if (columnName.length() < COLUMN_SHOW_LENGTH)headerLine.append(COLUMN_SHOW_LENGTH - columnName.length(), ' ');
    }
    std::cout << headerLine << std::endl;
    std::cout << splitLine << std::endl;
    //打开一个无条件遍历
    RM_FileScan rmFileScan;
    rmFileScan.OpenScan(rmFileHandle, INT, 0, 0, NO_OP, nullptr);
    RM_Record rmRecord;
    //如果输出够了条数或者没有了就停止
    for (int i = 0; (i < num || num == -1) && (rmFileScan.GetNextRec(rmRecord) == OK_RC); i++) {
        char *record;
        rmRecord.GetData(record);
        std::cout << formatRecordToString(record) << std::endl;;
    }
    rmFileScan.CloseScan();
    std::cout << splitLine << std::endl;;
}

std::string SM_Table::formatRecordToString(char *record) {
    std::string line;
    for (int i = 0; i < tableMeta.columnNum; i++) {
        std::string column;
        char *data = getColumnData(record, i);
        if (data == nullptr)column = "NULL";
        else {
            switch (tableMeta.columns[i].attrType) {
                case INT: {
                    column = std::to_string(*(int *) data);
                    break;
                }
                case FLOAT: {

                    char temp[100];
                    char format[100];
                    sprintf(format, "%%%d.%df", tableMeta.columns[i].integerLength, tableMeta.columns->decimalLength);
                    sprintf(temp, format, *(float *) data);
                    column = temp;
                    break;
                }
                case DATE: {
                    Date date = *(Date *) data;
                    char temp[100];
                    sprintf(temp, "%04d-%02d-%02d", date.year, date.day, date.day);
                    column = temp;
                    break;
                }
                case STRING: {
                    column = data;
                    break;
                }

                case VARCHAR: {
                    char temp[tableMeta.columns[i].stringMaxLength];
                    ((Varchar *) data)->getData(temp);
                    column = temp;
                    break;
                }
                case ATTRARRAY:
                    //不应该在这儿
                    exit(0);
                    break;
            }
        }
        if (column.length() < COLUMN_SHOW_LENGTH)column.append(COLUMN_SHOW_LENGTH - column.length(), ' ');
        line.append(column);
    }
    return line;
}

RC SM_Table::setColumnData(char *record, ColumnId columnId, AttrValue attrValue) {
    //命令里有传该参数的值，但也有可能是null
    if (attrValue.isNull) {
        TRY(setColumnNull(record, columnId))
    }
    //先设为不是null
    record[columnOffset[columnId]] = 0;
    char *data = record + columnOffset[columnId] + 1;
    switch (tableMeta.columns[columnId].attrType) {
        case INT: {
            char *endPtr;
            int num = strtol(attrValue.charValue, &endPtr, 10);
            if (errno == ERANGE)return QL_INT_OUT_OF_RANGE;
            if (strlen(endPtr) != 0)return QL_INT_CONT_CONVERT_TO_INT;
            *(int *) data = num;
            break;
        }
        case FLOAT: {
            char *endPtr;
            float num = strtof(attrValue.charValue, &endPtr);
            if (errno == ERANGE)return QL_FLOAT_OUT_OF_RANGE;
            if (strlen(endPtr) != 0)return QL_FLOAT_CONT_CONVERT_TO_FLOAT;
            if (fabsf(num) >= powf(10, tableMeta.columns[columnId].decimalLength))return QL_FLOAT_OUT_OF_RANGE;
            *(float *) data = num;
            break;
        }
        case STRING: {
            if (strlen(attrValue.charValue) >= tableMeta.columns[columnId].attrLength)return QL_CHAR_TOO_LONG;
            strcpy(data, attrValue.charValue);
            break;
        }
        case DATE: {
            char *endPtr;
            Date date{};
            date.year = strtol(attrValue.charValue, &endPtr, 10);
            if (endPtr != attrValue.charValue + 4 || endPtr[0] != '-')return QL_DATE_CONT_CONVERT_TO_DATE;
            date.month = strtol(attrValue.charValue + 5, &endPtr, 10);
            if (endPtr != attrValue.charValue + 7 || endPtr[0] != '-')return QL_DATE_CONT_CONVERT_TO_DATE;
            date.day = strtol(attrValue.charValue + 8, &endPtr, 10);
            if (endPtr != attrValue.charValue + 10 || strlen(endPtr) != 0)return QL_DATE_CONT_CONVERT_TO_DATE;
            if (!date.isValid())return QL_DATE_IS_NOT_VALID;
            *(Date *) data = date;
            break;
        }
        case VARCHAR: {
            Varchar varchar{};
            varchar.length = strlen(attrValue.charValue);
            strcpy(varchar.spName, Utils::getStringPoolFileName(tableMeta.createName).c_str());
            TRY(spHandle.InsertString(attrValue.charValue, varchar.length, varchar.offset))
            *(Varchar *) data = varchar;
            break;
        }
        case ATTRARRAY: { //不应该在这儿
            exit(0);
            break;
        }
    }
}

RC SM_Table::setColumnNull(char *record, ColumnId columnId) {
    //命令里没传，所以认为是null，但要先看是不是有defaultValue
    if (tableMeta.columns[columnId].hasDefaultValue) {
        TRY(setColumnData(record, columnId, tableMeta.columns[columnId].defaultValue))
        return OK_RC;
    }
    if (tableMeta.columns[columnId].allowNull) {
        record[columnOffset[columnId]] = 1;
        return OK_RC;
    }
    return QL_COLUMN_NOT_ALLOW_NULL;
}

