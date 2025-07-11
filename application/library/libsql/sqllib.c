/*
 * sqllib.c
 *
 *  Created on: Nov 23, 2018
 *      Author: shihhung.tsai
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include "shared_mutex.h"
#include "sqlite3.h"
#include "sqllib.h"

#define SQL_DB_STR_BUF_SIZE    4096


shared_mutex_t mutex;

int get_sqldata_count(const char *table_name);
int get_sqldata(const char *category, char *query_columns, char **return_data);
int add_sqldata(const char *table_name, char *add_data);
int update_sqldata(const char *table_name, int id, char *input_data);
int update_sqldata_by_condition(const char *table_name, char *condition_data,
		char *input_data);
int delete_sqldata_by_id(const char *table_name, int id);
int delete_sqldata_by_condition(const char *table_name, char *query_columns_ptr);
int get_sql_id(const char *table_name, int id_order);
int get_sqldatas(const char *db_path,const char *table_name, char *query_columns,char **return_data);


int db_lock() {
	return 0;
}

int db_unlock() {
	return 0;
}

static sqlite3 *open_sql_file(const char *db_path) {
	sqlite3 *db;
	int res;
	

	res = sqlite3_open(db_path, &db);

	if (res) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
	}

	return db;
}

int exec_sql_cmd(const char *db_path,const char *sql) {
	sqlite3 *db;
	char *zErrMsg = 0;
	int retry_count = 777;
	int rc;
	db = open_sql_file(db_path);

	if (db == NULL) {
		return -1;
	}

	retry:

	rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
	if (rc != SQLITE_OK) {

		if (--retry_count > 0) {
			sleep(1);
			goto retry;
		}

		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		fprintf(stderr, "SQL Cmd:%s\n", sql);
		sqlite3_free(zErrMsg);

		rc = -1;
	} else {
		rc = 0;
	}

	sqlite3_close(db);
	return rc;
}

int get_sqldata_count(const char *table_name) {
	sqlite3 *db;
	char sql_cmd[512];
	sqlite3_stmt *stmt;
	int count = 0;
	db = open_sql_file(DB_PATH);

	if (!db) {
		goto end;
	}

	sprintf(sql_cmd, "select count(*), * from %s", table_name);

	sqlite3_prepare_v2(db, sql_cmd, -1, &stmt, NULL);

	if (sqlite3_step(stmt) != SQLITE_DONE) {
		count = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);

	end:

	return count;

}

//----------------GET DATA API------------------------------

int get_sqldata_int_db(const char *db_path,const char *table_name, const char *column_name) {

	char*ptr;
	char *column_ptr = NULL;
	int res = 0;
	int column_count = 1;
	int data_type = 0;

	int length = strlen(column_name) + 1 + 4;
	column_ptr = (char*) malloc(length);
	memcpy(column_ptr, &column_count, 4);
	memcpy(column_ptr + 4, column_name, strlen(column_name) + 1);

	if (get_sqldatas(db_path,table_name, column_ptr, &ptr) >= 0) {

		memcpy(&data_type, ptr + 8, 4);

		if (data_type == NULL_TYPE) {
			res = -1;
		} else if (data_type == INT_TYPE) {
			memcpy(&res, ptr + 12, 4);
		} else {
			res = -1;
		}

	} else {
		res = -1;
	}

	free(column_ptr);
	free(ptr);

	return res;

}


int get_sqldata_int(const char *table_name, const char *column_name)
{
    return get_sqldata_int_db(DB_PATH,table_name,column_name);
}



char* get_sqldata_str(const char *table_name, const char *column_name) {
	char*ptr;
	char *column_ptr = NULL;
	char* res = NULL;
	int column_count = 1;
	int data_type = 0;

	int length = strlen(column_name) + 1 + 4;
	column_ptr = (char*) malloc(length);
	memcpy(column_ptr, &column_count, 4);
	memcpy(column_ptr + 4, column_name, strlen(column_name) + 1);

	if (get_sqldatas(DB_PATH,table_name, column_ptr, &ptr) >= 0) {

		memcpy(&data_type, ptr + 8, 4);

		if (data_type == NULL_TYPE) {
			res = NULL;
		} else if (data_type == STRING_TYPE) {
			int length = strlen(ptr + 12) + 1;
			res = (char*) malloc(length);
			memcpy(res, ptr + 12, length);
		} else {
			res = NULL;
		}

	} else {
		res = NULL;
	}

	free(column_ptr);
	free(ptr);

	return res;

}

int get_sqldata_by_id_int(const char *table_name, const char* column_name,
		int id) {
	sqlite3 *db=NULL;
	sqlite3_stmt *stmt;
	char sql_cmd[512] = { 0 };
	int res = -1;

	if (table_name == NULL || column_name == NULL) {
		goto end;
	}

	db = open_sql_file(DB_PATH);

	if (!db) {
		goto end;
	}

	sprintf(sql_cmd, "select %s from %s where id=%d;", column_name, table_name,
			id);

	//printf("sql cmd:%s\n",sql_cmd);

	if (sqlite3_prepare_v2(db, sql_cmd, -1, &stmt, NULL) != SQLITE_OK) {
		if (stmt) {
			printf("can not generate stm!!!\n");
			res = -1;
			goto end;
		}
	}

	if (sqlite3_step(stmt) != SQLITE_DONE) {


		switch (sqlite3_column_type(stmt, 0)) {

		case (SQLITE_INTEGER):

			res = sqlite3_column_int(stmt, 0);
			break;

		default:
			res = -1;
			break;
		}

	}

	end:

        if(db)
        {
	   sqlite3_finalize(stmt);
	   sqlite3_close(db);
        }
	return res;

}

char* get_sqldata_by_id_str_path(const char *db_path,const char *table_name, const char* column_name,
		int id) {
	sqlite3 *db = NULL;;
	sqlite3_stmt *stmt;
	char sql_cmd[512] = { 0 };
	char* res = NULL;
	size_t data_length;

	if (table_name == NULL || column_name == NULL) {
		goto end;
	}

	db = open_sql_file(db_path);

	if (!db) {
		goto end;
	}

	sprintf(sql_cmd, "select %s from %s where id=%d;", column_name, table_name,
			id);



	if (sqlite3_prepare_v2(db, sql_cmd, -1, &stmt, NULL) != SQLITE_OK) {
		if (stmt) {
			printf("can not generate stm!!!\n");
			res = NULL;
			goto end;
		}
	}

	while (sqlite3_step(stmt) != SQLITE_DONE) {


		switch (sqlite3_column_type(stmt, 0)) {

		case (SQLITE_TEXT):
			data_length = strlen((const char*)sqlite3_column_text(stmt, 0)) + 1;

			res = (char*)malloc(data_length);
			if (res != NULL) {
				memcpy(res, sqlite3_column_text(stmt, 0),data_length);
			}
			break;

		default:
			res = NULL;
			break;
		}

	}

	end:

	if(db)
        {
	   sqlite3_finalize(stmt);
	   sqlite3_close(db);
        }
	return res;

}

char* get_sqldata_by_protocol_str(const char *table_name, const char* column_name,const char *protocol)
{
	sqlite3 *db = NULL;;
	sqlite3_stmt *stmt;
	char sql_cmd[512] = { 0 };
	char* res = NULL;
	size_t data_length;

	if (table_name == NULL || column_name == NULL) {
		goto end;
	}

	db = open_sql_file(DB_PATH);

	if (!db) {
		goto end;
	}

	sprintf(sql_cmd, "select %s from %s where protocol='%s';", column_name, table_name,
			protocol);


	if (sqlite3_prepare_v2(db, sql_cmd, -1, &stmt, NULL) != SQLITE_OK) {
		if (stmt) {
			printf("can not generate stm!!!\n");
			res = NULL;
			goto end;
		}
	}

	while (sqlite3_step(stmt) != SQLITE_DONE) {


		switch (sqlite3_column_type(stmt, 0)) {

		case (SQLITE_TEXT):
			data_length = strlen((const char*)sqlite3_column_text(stmt, 0)) + 1;

			res = (char*)malloc(data_length);
			if (res != NULL)
				memcpy(res, sqlite3_column_text(stmt, 0),data_length);
			break;

		default:
			res = NULL;
			break;
		}

	}

	end:

	if(db)
       {
	   sqlite3_finalize(stmt);
	   sqlite3_close(db);
       }
	return res;

}


char* get_sqldata_by_id_str(const char *table_name, const char* column_name,
		int id) {

	return get_sqldata_by_id_str_path(DB_PATH,table_name,column_name,id);

}

int get_sqldatas(const char *db_path ,const char *table_name, char *query_columns,
		char **return_data) {
	sqlite3 *db;
	int res;
	sqlite3_stmt *stmt;
	char sql_cmd[512] = { 0 };
	char sql_query_buf[512] = { 0 };
	char *sql_ptr = sql_cmd;
	int ptr_off = 4;
	size_t data_type;
	size_t data_length;
	int column_count = 0;
	int element_count = 0;
	char *ptr;
	int current_size = 0;
	int query_column_count = 0;
	int i;

	db = open_sql_file(db_path);

	if (!db) {
		res = -1;
		goto end2;
	}

	*return_data = (char*) malloc(4);

	if (*return_data == NULL) {
		res = -1;
		sqlite3_close(db);
		goto end2;
	}

	memset(*return_data, 0, 4);

	ptr = *return_data;
	ptr += 4;
	current_size = 4;

	if (query_columns != NULL) {
		memcpy(&query_column_count, query_columns, 4);
		query_columns += 4;

		if (query_column_count == 0) {
			sprintf(sql_query_buf, " * ");
		} else {
			for (i = 0; i < query_column_count; i++) {
				int length = strlen(query_columns) + 1;

				sprintf(sql_query_buf, "%s %s ", sql_query_buf, query_columns);

				if (i != query_column_count - 1) {
					sprintf(sql_query_buf, "%s,", sql_query_buf);
				}

				query_columns += length;
			}
		}

	} else {
		sprintf(sql_query_buf, " * ");
	}

	//printf("query column=>%s\n",sql_query_buf);

	sprintf(sql_cmd, "select %s from %s", sql_query_buf, table_name);
	//printf("\nsql cmd:%s\n", sql_cmd);

	//db_lock();

	if (sqlite3_prepare_v2(db, sql_cmd, -1, &stmt, NULL) != SQLITE_OK) {
		if (stmt != SQLITE_OK) {
			printf("can not generate stmt!!!\n");
			res = -1;
			goto end1;
		}
	}

	while (sqlite3_step(stmt) != SQLITE_DONE) {
		int i;

		int num_cols = sqlite3_column_count(stmt);
		memcpy(sql_ptr + ptr_off, &num_cols, 4);
		ptr_off += 4;
		current_size += 4;

		for (i = 0; i < num_cols; i++) {
			switch (sqlite3_column_type(stmt, i)) {

			case (SQLITE_NULL):

				/*Data type*/
				data_type = NULL_TYPE;
				memcpy(sql_ptr + ptr_off, &data_type, 4);
				ptr_off += 4;
				current_size += 4;

				/*Data Value*/
				memset(sql_ptr + ptr_off, 0, 4);
				ptr_off += 4;
				current_size += 4;

				++column_count;

				break;

			case (SQLITE3_TEXT):

				/*Data type*/

				data_length = strlen((const char*)sqlite3_column_text(stmt, i)) + 1;
				data_type = STRING_TYPE;
				memcpy(sql_ptr + ptr_off, &data_type, 4);
				ptr_off += 4;
				current_size += 4;

				/*Data value*/
				memcpy(sql_ptr + ptr_off, sqlite3_column_text(stmt, i),
						data_length);

				ptr_off += data_length;
				current_size += data_length;

				++column_count;

				break;

			case (SQLITE_INTEGER):

				/*Data type*/
				data_length = 4;
				data_type = INT_TYPE;
				memcpy(sql_ptr + ptr_off, &data_type, 4);
				ptr_off += 4;
				current_size += 4;

				/*Data value*/
				int temp_data = sqlite3_column_int(stmt, i);
				memcpy(sql_ptr + ptr_off, &temp_data, 4);

				ptr_off += data_length;
				current_size += data_length;

				++column_count;

				break;

			default:

				break;
			}

		} //column end

		memcpy(sql_ptr, &column_count, 4);

		//copy data
		*return_data = (char*) realloc(*return_data, current_size);

		if (*return_data == NULL) {
			printf("error:malloc memory fail!!!\n");
			res = -1;
			goto end1;
		}

		ptr = *return_data;

		memcpy(ptr + current_size - ptr_off, sql_cmd, ptr_off);

		//clear
		ptr_off = 0;
		memset(sql_cmd, 0, 512);
		column_count = 0;

		//increament
		++element_count;

	}

	ptr = *return_data;
	memcpy(ptr, &element_count, 4);

	res = get_sql_id(table_name,LAST_ID);

	end1: sqlite3_finalize(stmt);
	sqlite3_close(db);
	//db_unlock();

	end2: return res;
}

//-----ADD DATA API----------------------------------------

int add_sqldata(const char *table_name, char *add_data) {

	int res;
	char *sql_cmd1 = NULL;
	char *sql_cmd2 = NULL;
	int element_count = 0;
	int i;
	int off = 0;
	int column_name_length;
	int column_type;


	sql_cmd1 = (char*)malloc((size_t)8192);

	if(sql_cmd1 == NULL)
	{
		res = -1;
		goto end;
	}

	sql_cmd2 = (char*)malloc((size_t)8192);

	if(sql_cmd2 == NULL)
	{
		res = -1;
		goto end;
	}

	sprintf(sql_cmd1, "INSERT INTO %s (", table_name);
	sprintf(sql_cmd2, "VALUES (");

	memcpy(&element_count, add_data, 4);
	printf("element count=%d\n", element_count);
	off += 4;

	for (i = 0; i < element_count; i++) {

		/*COLUMN NAME*/
		sprintf(sql_cmd1, "%s%s", sql_cmd1, add_data + off);
		column_name_length = strlen(add_data + off) + 1;
		off += column_name_length;

		if (i != (element_count - 1)) {
			sprintf(sql_cmd1, "%s ,", sql_cmd1);
		} else {
			sprintf(sql_cmd1, "%s)", sql_cmd1);
		}

		/*TYPE*/
		//column_type = (int) (*(add_data_ptr + off));
		memcpy(&column_type, add_data + off, 4);
		off += 4;

		/*VALUE*/
		if (column_type == INT_TYPE) {
			//printf("column type=%d\n", column_type);
			int data;
			memcpy(&data, add_data + off, 4);
			sprintf(sql_cmd2, "%s%d", sql_cmd2, data);
			off += 4;

		} else if (column_type == STRING_TYPE) {
			//printf("column type=%d\n", column_type);
			sprintf(sql_cmd2, "%s'%s'", sql_cmd2, add_data + off);
			off += strlen(add_data + off) + 1;
		} else {
			res = -1;
			goto end;
		}

		if (i != (element_count - 1)) {
			sprintf(sql_cmd2, "%s ,", sql_cmd2);
		} else {
			sprintf(sql_cmd2, "%s);", sql_cmd2);
		}

	}

	sprintf(sql_cmd1, "%s %s", sql_cmd1, sql_cmd2);

	//printf("final cmd:%s\n", sql_cmd1);

	db_lock();
	res = exec_sql_cmd(DB_PATH,sql_cmd1);
	db_unlock();

	end:

	if(sql_cmd1 != NULL)
	{
		free(sql_cmd1);
	}

	if(sql_cmd2 != NULL)
	{
		free(sql_cmd2);
	}

	return res;
}

//----UPDATE DATA API----------------------------------

int update_sqldata_by_id_int(const char *table_name, int id,
		const char* column_name, int column_value) {

	char *sql_cmd1 = NULL;
	int res = 0;

	sql_cmd1 = (char*)malloc((size_t)8192);

	if(sql_cmd1 == NULL)
	{
		res = -1;
		goto end;
	}

	sprintf(sql_cmd1, "UPDATE %s set %s=%d where id=%d", table_name,
			column_name, column_value, id);

	db_lock();
	res = exec_sql_cmd(DB_PATH,sql_cmd1);
	db_unlock();

end:

    if(sql_cmd1 != NULL)
    {
    	free(sql_cmd1);
    }

	return res;
}

int update_sqldata_int(const char *table_name, const char* column_name,
		int column_value) {

	char *sql_cmd1 = NULL;
	int res = 0;

	int id = get_sql_id(table_name, FIRST_ID);

	if (id == -1) {
		res = -1;
		goto end;
	}

	sql_cmd1 = (char*)malloc((size_t)8192);

	if(sql_cmd1 == NULL)
	{
		res = -1;
		goto end;
	}

	sprintf(sql_cmd1, "UPDATE %s set %s=%d where id=%d", table_name,
			column_name, column_value, id);

	db_lock();
	res = exec_sql_cmd(DB_PATH,sql_cmd1);
	db_unlock();


	end:

	if(sql_cmd1 != NULL)
	{
		free(sql_cmd1);
	}

	return res;
}

int update_sqldata_str(const char *table_name, const char* column_name,
		const char* column_value) {

	char *sql_cmd1 = NULL;
	int res = 0;
	size_t length = 0;

	int id = get_sql_id(table_name, FIRST_ID);

	if (id == -1) {
		res = -1;
		goto end;
	}


	if(column_value == NULL)
	{
		res = -1;
		goto end;
	}

	length = strlen(column_value) + 1 + 1024;

	sql_cmd1 = (char*)malloc(length);

	if(sql_cmd1 == NULL)
	{
		res = -1;
		goto end;
	}


	sprintf(sql_cmd1, "UPDATE %s set %s='%s' where id=%d", table_name,
			column_name, column_value, id);

	db_lock();
	res = exec_sql_cmd(DB_PATH,sql_cmd1);
	db_unlock();

	end:

	if(sql_cmd1 != NULL)
	{
		free(sql_cmd1);
	}

	return res;
}

int update_sqldata_by_id_str_path(const char *db_path,const char *table_name, int id,
		const char* column_name, const char* column_value) {

	char *sql_cmd1 = NULL;
	int res = 0;
	size_t length = 0;

	if(column_value == NULL)
	{
		res = -1;
		goto end;
	}

    length = strlen(column_value) + 1 + 1024;



    sql_cmd1 = (char*)malloc(length);

    if(sql_cmd1 == NULL)
    {

    	res = -1;
    	goto end;
    }



	sprintf(sql_cmd1, "UPDATE %s set %s='%s' where id=%d", table_name,
			column_name, column_value, id);
	db_lock();
	res = exec_sql_cmd(db_path,sql_cmd1);
	db_unlock();

end:

    if(sql_cmd1 != NULL)
    {
    	free(sql_cmd1);
    }

	return res;
}


int update_sqldata_by_protocol_str_path(const char *db_path,const char *table_name, const char *protocol,const char* column_name, const char* column_value)
{

	char *sql_cmd1 = NULL;
	int res = 0;
	size_t length = 0;

	if(column_value == NULL)
	{
		res = -1;
		goto end;
	}

    length = strlen(column_value) + 1 + 1024;



    sql_cmd1 = (char*)malloc(length);

    if(sql_cmd1 == NULL)
    {

    	res = -1;
    	goto end;
    }



	sprintf(sql_cmd1, "UPDATE %s set %s='%s' where protocol='%s'", table_name,
			column_name, column_value, protocol);
	//printf("sql cmd:%s\n",sql_cmd1);
	db_lock();
	res = exec_sql_cmd(db_path,sql_cmd1);
	db_unlock();

end:

    if(sql_cmd1 != NULL)
    {
    	free(sql_cmd1);
    }

	return res;
}


int add_sqldata_by_id_str_path(const char *db_path,const char *table_name, int id,const char* column_name, const char* column_value)
{

     size_t length = 8192;
     int res = -1;
     char *sql_cmd1 = (char*)malloc(length);

     if(!sql_cmd1)
     {
         
         return res;
     }
    
     sprintf(sql_cmd1, "INSERT INTO %s (id, %s) VALUES(%d, '%s');" , table_name,
			column_name,id ,column_value);


    db_lock();
    res = exec_sql_cmd(db_path,sql_cmd1);
    db_unlock();



    if(sql_cmd1 != NULL)
    {
    	free(sql_cmd1);
    }

    return res;
     

}


int update_sqldata_by_id_str(const char *table_name, int id,const char* column_name, const char* column_value)
{
       return update_sqldata_by_id_str_path(DB_PATH,table_name,id,column_name,column_value);
}

int update_sqldata_by_id_protocol(const char *table_name, const char *protocol,const char* column_name, const char* column_value)
{
	return update_sqldata_by_protocol_str_path(DB_PATH,table_name,protocol,column_name,column_value);
}

int update_sqldata(const char *table_name, int id __attribute__((unused)), char *input_data)
{
	char *sql_cmd1 = NULL;
	char *ptr = input_data;
	int off = 0;
	int column_count = 0;
	int i;
	int column_name_length;
	int column_type;
	int res = 0;

	sql_cmd1 = (char*)malloc((size_t)8192);

	if(sql_cmd1 == NULL)
	{
		res = -1;
		goto end;
	}

	column_count = (int) (*(ptr));
	off += 4;

	/*"UPDATE NETWORK set %s='%s' where IPADDR='%s'*/
	sprintf(sql_cmd1, "UPDATE %s set ", table_name);

	for (i = 0; i < column_count; i++) {
		/*NAME*/
		sprintf(sql_cmd1, "%s%s=", sql_cmd1, ptr + off);
		column_name_length = strlen(ptr + off) + 1;
		off += column_name_length;

		/*TYPE*/
		column_type = (int) (*(ptr + off));
		off += 4;

		/*VALUE*/
		if (column_type == INT_TYPE) {

			int data;
			memcpy(&data, ptr + off, 4);
			sprintf(sql_cmd1, "%s%d", sql_cmd1, data);
			off += 4;

		} else if (column_type == STRING_TYPE) {

			sprintf(sql_cmd1, "%s'%s'", sql_cmd1, ptr + off);
			off += strlen(ptr + off) + 1;
		} else {
			res = -1;
			goto end;
		}

		if (i != (column_count - 1)) {
			sprintf(sql_cmd1, "%s ,", sql_cmd1);
		}

	}

	db_lock();
	res = exec_sql_cmd(DB_PATH,sql_cmd1);
	db_unlock();

	end:

	if(sql_cmd1 != NULL)
	{
		free(sql_cmd1);
	}

	return res;
        }

int update_sqldata_by_condition(const char *table_name, char *condition_data,
		char *input_data) {

	char *sql_cmd1 = NULL;
	char *ptr = input_data;
	int off = 0;
	int column_count = 0;
	int i;
	int column_name_length;
	int column_type;
	int res = 0;
	char *cond_ptr = condition_data;
	int count = 0;
	int data_type;

	sql_cmd1 = (char*)malloc((size_t)8192);

	if(sql_cmd1 == NULL)
	{
		res = -1;
		goto end;
	}

	column_count = (int) (*(ptr));
	off += 4;

	/*"UPDATE NETWORK set %s='%s' where IPADDR='%s'*/
	sprintf(sql_cmd1, "UPDATE %s set ", table_name);

	for (i = 0; i < column_count; i++) {
		/*NAME*/
		sprintf(sql_cmd1, "%s%s=", sql_cmd1, ptr + off);
		column_name_length = strlen(ptr + off) + 1;
		off += column_name_length;

		/*TYPE*/
		column_type = (int) (*(ptr + off));
		off += 4;

		/*VALUE*/
		if (column_type == INT_TYPE) {

			int data;
			memcpy(&data, ptr + off, 4);
			sprintf(sql_cmd1, "%s%d", sql_cmd1, data);
			off += 4;

		} else if (column_type == STRING_TYPE) {

			sprintf(sql_cmd1, "%s'%s'", sql_cmd1, ptr + off);
			off += strlen(ptr + off) + 1;
		}

		if (i != (column_count - 1)) {
			sprintf(sql_cmd1, "%s ,", sql_cmd1);
		}

	}

//---------condition--------------------------

	if (cond_ptr != NULL) {

		memcpy(&count, condition_data, 4);

		if (count == 0) {
			res = -1;
			goto end;
		}

		condition_data += 4;

		sprintf(sql_cmd1, "%s where ", sql_cmd1);

		for (i = 0; i < count; i++) {
			/*COLUMN NAME*/
			int length = strlen(condition_data) + 1;
			sprintf(sql_cmd1, "%s %s=", sql_cmd1, condition_data);
			printf("hhh=%s\n", condition_data);
			condition_data += length;

			/*DATA TYPE*/
			memcpy(&data_type, condition_data, 4);
			condition_data += 4;

			/*DATA VALUE*/
			if (data_type == INT_TYPE) {
				int value = 0;
				memcpy(&value, condition_data, 4);
				condition_data += 4;
				sprintf(sql_cmd1, "%s%d", sql_cmd1, value);

			} else if (data_type == STRING_TYPE) {

				sprintf(sql_cmd1, "%s'%s'", sql_cmd1, condition_data);

			} else {

				res = -1;
				goto end;
			}

			if (i != count - 1) {
				sprintf(sql_cmd1, "%s and ", sql_cmd1);
			}

		}

	}

	db_lock();
	res = exec_sql_cmd(DB_PATH,sql_cmd1);
	db_unlock();

	end:

	if(sql_cmd1 != NULL)
	{
		free(sql_cmd1);
	}

	return res;

}

//----Delete API--------------

int delete_sqldata_by_condition(const char *table_name, char *query_columns_ptr) {
	int res = 0;
	char *sql_cmd = { 0 };
	char *ptr = query_columns_ptr;
	int count = 0;
	int i;
	int length = 0;
	int data_type = 0;


	sql_cmd = (char*)malloc((size_t)8192);

	if(sql_cmd == NULL)
	{
		res = -1;
		goto end;
	}

	if (query_columns_ptr == NULL) {
		res = -1;
		goto end;
	}

	memcpy(&count, ptr, 4);

	if (count == 0) {
		res = -1;
		goto end;
	}

	query_columns_ptr += 4;

	sprintf(sql_cmd, "delete from %s where ", table_name);

	for (i = 0; i < count; i++) {
		/*COLUMN NAME*/
		length = strlen(query_columns_ptr) + 1;
		sprintf(sql_cmd, "%s %s=", sql_cmd, query_columns_ptr);
		query_columns_ptr += length;

		/*DATA TYPE*/
		memcpy(&data_type, query_columns_ptr, 4);
		query_columns_ptr += 4;

		/*DATA VALUE*/
		if (data_type == INT_TYPE) {
			int value = 0;
			memcpy(&value, query_columns_ptr, 4);
			query_columns_ptr += 4;
			sprintf(sql_cmd, "%s%d", sql_cmd, value);

		} else if (data_type == STRING_TYPE) {

			sprintf(sql_cmd, "%s'%s'", sql_cmd, query_columns_ptr);

		} else {
			res = -1;
			goto end;
		}

		if (i != count - 1) {
			sprintf(sql_cmd, "%s and ", sql_cmd);
		}

	}

	db_lock();

	res = exec_sql_cmd(DB_PATH,sql_cmd);

	db_unlock();

	end:

	if(sql_cmd != NULL)
	{
		free(sql_cmd);
	}

	return res;
}

int delete_sqldata_by_id(const char *table_name, int id) {
	int res = 0;
	char sql_cmd[512] = { 0 };

	sprintf(sql_cmd, "delete from %s where id=%d", table_name, id);

	db_lock();

	res = exec_sql_cmd(DB_PATH,sql_cmd);

	db_unlock();

	return res;
}

//-------------------
int get_sql_id(const char *table_name, int id_order) {

	char sql_cmd[128]={0};
	sqlite3 *db;
	int res = -1;
	sqlite3_stmt *stmt;
	int i;

	if (table_name == NULL)
		return res;

	db = open_sql_file(DB_PATH);

	if (!db) {
		printf("error:can not open db:%s\n", DB_PATH);
		res = -1;
		goto end;
	}

	if (id_order == FIRST_ID) {
		sprintf(sql_cmd, "select MIN(id) from %s", table_name);
	} else if (LAST_ID) {
		sprintf(sql_cmd, "select MAX(id) from %s", table_name);
	} else {
		sprintf(sql_cmd, "select MIN(id) from %s", table_name);
	}

	sqlite3_prepare_v2(db, sql_cmd, -1, &stmt, NULL);

	while (sqlite3_step(stmt) != SQLITE_DONE) {

		int num_cols = sqlite3_column_count(stmt);

		for (i = 0; i < num_cols; i++) {

			switch (sqlite3_column_type(stmt, i)) {

			case (SQLITE_NULL):
				printf("null value\n");
				res = -1;
				break;

			case (SQLITE_INTEGER):

				/*Data type*/
				printf("get value\n");
				res = sqlite3_column_int(stmt, i);

				break;

			default:
				printf("w9\n");
				res = -1;
				break;
			}

		}

	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
	end:

	return res;

}
