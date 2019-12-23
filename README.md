# RoboDBMS
## NOTICE
 - 注意区分有些RC，有些返回非0是正常的，这样数据库还应该继续工作，所以要注意处理后事
## TODO
 - RM
    - 支持变长字符串 √
    - Bitmap查找效率问题 × (不是大问题，优先级往后放)
    - 析构函数的bug ×
 - IX
    - 索引建立 √
    - 变长字符串做索引 √ (~~不打算支持~~ 支持了_(:з」∠)_)
    - 变长字符串做索引速度过慢问题（频繁的打开、关闭、读文件）×
    - 支持日期类型并支持日期类型做索引 √
    - 支持联合索引 √
 - SM
    - 系统管理 √
    - 列增加和删除 ×
        - 这样一改整个记录得重改，然后整个索引也得重建？好像只要遍历一遍，一条一条加就可以了
    - 支持联合主键 √
    - 支持联合外键 √
 - QL
    - 插入 ×
        - 需要保证主键的唯一性，也是实现判断 这个主键是否存在
        - 同时记得更新索引
        - 常量转记录类型（记录转常量类型？也是需要的，因为有时候从表里拿出来的数据需要parse或者比较）
    - 删除 ×
        - 如果删除一条主键，它存在依赖，那么我们可以直接禁止，而不是级联
        - 需要一个判断 某个表里是否存在reference这个主键的外键 的函数
        - 记得删除索引
    - 更新 ×
        - setClause
            - 这个比较容易，因为只有赋值操作
        - whereClause
            - 删除和更新的应该是一样的 op 右边只能是常量(不对，是可以是列名的)
                - 这部分需要一个  遍历整个表，进一步还能够根据索引来优化直接解决和常量比较的条件
        - 更新要做成先删除要插入嘛？
    - 查询 ×
        - whereClause
            - 右边还可以是列名
            - 只和常量比较的分到表内部做 然后多表之间的再暴力比较？
        - 查询出来的感觉是个表，然后有个record内容的vector？所以他也需要有列的定义 它也有tableMeta就可以了？
    - 查询优化 ×
        - 用索引就算？所谓的DAG是什么？
    - 多表连接 ×
        - 和双表可能差别不大？
    - 聚集查询 ×
        - 怎么做？MIN/MAX/AVG/SUM/COUNT还可以  GROUP BY/HAVING 怎么办？
    - 模糊查询 ×
        - like好像也不难
    - 嵌套查询 ×
        - 怎么做？临时放到一个记录文件里？好像可以
 - PS
    - 命令解析 部分完成
 - 其他
    - 修改比较器的获取方式 √
    - 在比较器中方便地加入范围查找 × (不打算做)
    - 支持字符串的like表达式 ×
    - 修改项目结构 ×
    - 可能存在的内存泄漏问题（lex） ×
## TIPS
 - 序列化的是Varchar，但是内存中可以存成String
 - char类型存进去的时候后面多拷贝个\0方便格式化
 - RM_FileScan是否可以去除比较器，挪到Table来做？
## Reference
 - https://github.com/huzecong/rebaseDB/tree/initial (Initial Redbase)
 - https://github.com/yifeih/redbase
 - https://github.com/parachvte/B-Plus-Tree
 
