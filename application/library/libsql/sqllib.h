/*
 * sqllib.h
 *
 *  Created on: Nov 23, 2018
 *      Author: im96
 */

#ifndef SQLLIB_H_
#define SQLLIB_H_

#define NULL_TYPE 0
#define INT_TYPE 1
#define STRING_TYPE 2


#define DB_PATH "/tmp/ini.db"

enum ID_ORDER
{
	FIRST_ID=0,
	LAST_ID,
};



//int get_sqldata_int_db(const char *db_path,const char *table_name, const char *column_name);
//char* get_sqldata_str_db(const char *db_path,const char *table_name, const char *column_name) ;
//int get_sqldata_by_id_int_db(const char *db_path,const char *table_name, const char* column_name,int id);
char* get_sqldata_by_id_str_path(const char *db_path,const char *table_name, const char* column_name,int id);

int update_sqldata_by_id_str_path(const char *db_path,const char *table_name, int id,const char* column_name, const char* column_value);


int add_sqldata_by_id_str_path(const char *db_path,const char *table_name, int id,const char* column_name, const char* column_value);

char* get_sqldata_by_protocol_str(const char *table_name, const char* column_name,const char *protocol);


int get_sqldata_int(const char *table_name, const char *column_name);
char* get_sqldata_str(const char *table_name, const char *column_name) ;
int get_sqldata_by_id_int(const char *table_name, const char* column_name,int id);
char* get_sqldata_by_id_str(const char *table_name, const char* column_name,int id);

int update_sqldata_by_id_int(const char *table_name, int id,const char* column_name, int column_value);
int update_sqldata_int(const char *table_name, const char* column_name,int column_value);
int update_sqldata_str(const char *table_name, const char* column_name,const char* column_value);
int update_sqldata_by_id_str(const char *table_name, int id,const char* column_name, const char* column_value);
int update_sqldata_by_protocol_str_path(const char *db_path,const char *table_name, const char *protocol,const char* column_name, const char* column_value);
int update_sqldata_by_id_protocol(const char *table_name, const char *protocol,const char* column_name, const char* column_value);



#endif /* SQLLIB_H_ */
