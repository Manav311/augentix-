/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef SQLLIB_H_
#define SQLLIB_H_

#define NULL_TYPE 0
#define INT_TYPE 1
#define STRING_TYPE 2


#define DB_PATH "ini.db"

enum ID_ORDER
{
	FIRST_ID=0,
	LAST_ID,
};


int exec_sql_cmd(const char *db_path,const char *sql);
//int get_sqldata_int_db(const char *db_path,const char *table_name, const char *column_name);
//char* get_sqldata_str_db(const char *db_path,const char *table_name, const char *column_name) ;
//int get_sqldata_by_id_int_db(const char *db_path,const char *table_name, const char* column_name,int id);
char* get_sqldata_by_id_str_path(const char *db_path,const char *table_name, const char* column_name,int id);

int update_sqldata_by_id_str_path(const char *db_path,const char *table_name, int id,const char* column_name, const char* column_value);


int add_sqldata_by_id_str_path(const char *db_path,const char *table_name, int id,const char* column_name, const char* column_value);



int get_sqldata_int(const char *table_name, const char *column_name);
char* get_sqldata_str(const char *table_name, const char *column_name) ;
int get_sqldata_by_id_int(const char *table_name, const char* column_name,int id);
char* get_sqldata_by_id_str(const char *table_name, const char* column_name,int id);

int update_sqldata_by_id_int(const char *table_name, int id,const char* column_name, int column_value);
int update_sqldata_int(const char *table_name, const char* column_name,int column_value);
int update_sqldata_str(const char *table_name, const char* column_name,const char* column_value);
int update_sqldata_by_id_str(const char *table_name, int id,const char* column_name, const char* column_value);

#endif /* SQLLIB_H_ */
