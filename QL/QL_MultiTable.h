//
// Created by 赵鋆峰 on 2019/12/31.
//

#ifndef ROBODBMS_QL_MULTITABLE_H
#define ROBODBMS_QL_MULTITABLE_H


#include "../SM/SM_Table.h"

class QL_MultiTable {
public:
    explicit QL_MultiTable(std::vector<TableMeta> *tableMetaList);

    TableMeta
    select(std::vector<PS_Expr> *_valueList, std::vector<PS_Expr> *_conditionList, std::string &_name,
           std::vector<PS_Expr> *_groupByList);

    //从表名和列名获得表以及列id
    std::pair<int, ColumnId> getColumn(std::string &tbName, std::string &columnName);

    RC iterateTables(int n);

    RC eval(PS_Expr &value, std::string group = "NULL");

private:
    int tableNum;
    std::vector<SM_Table> tableList;
    std::vector<RM_Record> recordList{};
    std::vector<std::vector<RM_RID> > ridListList{};
    std::vector<PS_Expr> *valueList{};
    std::vector<PS_Expr> *conditionList{};
    std::vector<PS_Expr> *groupByList{};
    TableMeta targetMeta{};
    SM_Table *smTable;
    bool isFirstIterate{};
    std::string name;
    int totalCount{};
};


#endif //ROBODBMS_QL_MULTITABLE_H
