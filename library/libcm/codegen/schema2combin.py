import sys
import json
import pprint
import os
import re
#from schema2csource import schemaToCSource
#from schema2jsource import schemaToJSource

top_level_struct_name = ''
existing_enum = {}

indent = []
level = []
tmp_flag = []
str_flag = 0
cnt_flag = 0


def genIndent(num):
    indent = ['']
    level = []
    for i in range(num):
        indent.append(indent[i] + "\t")
        level.append(i)

    level[0] = ''
    return indent, level

def writeFile(filename, list, mode):
    if len(list) > 0:
        with open(filename, mode) as f:
            for line in list:
                f.write("%s" % line)


def dumpSchema(error_message, schema):
    print('Error: %s' % error_message)
    print('-------- BEGIN SCHEMA DUMP --------')
    pprint.pprint(schema)
    print('-------- END SCHEMA DUMP --------')
    exit()


def checkRequiredKeys(required_keys, schema):
    missing = []
    for k in required_keys:
        if k not in schema:
            missing.append(k)

    if len(missing) > 0:
        error_message = 'Missing'
        for m in missing[:-1]:
            error_message = '%s "%s",' % (error_message, m)
        else:
            error_message = '%s "%s" in schema' % (error_message, missing[-1])
        dumpSchema(error_message, schema)


def makeComment(comment):
    if len(comment) > 0:
        return ' /* %s */' % comment
    else:
        return ''


def schemaToCSource(schema):
    global existing_enum
    global cnt_flag, str_flag
    indent, level = genIndent(10)

    header = []
    body = []
    func = []
    type = ''
    # If size = 0, means it's an scaler.
    # If size = n, where n > 0, means it's an array of size n.
    size = 0
    comment = ''

    checkRequiredKeys(['type'], schema)

    if schema['type'] == 'object':
        checkRequiredKeys(['properties'], schema)

        cnt_flag_ini = 0
        str_flag_ini = 0
        if 'title' in schema:
            struct_name = schema['title']
        else:
            struct_name = '%s_%X_S' % (top_level_struct_name, abs(hash(pprint.pformat(schema))))

        func_name = struct_name.lower()[5:-2]
        if (func_name == "event_conf"):
            func_name = "evt_conf"
        elif (func_name == "stitching_conf"):
            func_name = "stitch_conf"
        body.append('void parse_%s(%s *data, struct json_object *cmd_obj)\n' % (func_name, struct_name))
        func.append('void parse_%s(%s *data, struct json_object *cmd_obj);' % (func_name, struct_name))
        body.append('{\n')

        body.append('%sstruct json_object *tmp_obj;\n' % (indent[1]))
#        body.append('%sint i;\n\n' % (indent[1]))
        for var_name, var_schema in sorted(schema['properties'].items()):
            header[0:0], body[0:0], func[0:0], var_type, var_size, var_comment = schemaToCSource(var_schema)
            if(var_size == 0):
                if(var_type == 'AGTX_BOOL'):
                    body.append('%sif (json_object_object_get_ex(cmd_obj, "%s", &tmp_obj)) {\n' % (indent[1], var_name))
                    body.append('%sdata->%s = json_object_get_bool(tmp_obj);\n' % (indent[2],var_name))
                    body.append('%s}\n' % (indent[1]))
                elif(var_type == 'AGTX_INT32'):
                    body.append('%sif (json_object_object_get_ex(cmd_obj, "%s", &tmp_obj)) {\n' % (indent[1], var_name))
                    body.append('%sdata->%s = json_object_get_int(tmp_obj);\n' % (indent[2],var_name))
                    body.append('%s}\n' % (indent[1]))
                elif(var_type == 'AGTX_FLOAT'):
                    body.append('%sif (json_object_object_get_ex(cmd_obj, "%s", &tmp_obj)) {\n' % (indent[1], var_name))
                    body.append('%sdata->%s = json_object_get_double(tmp_obj);\n' % (indent[2],var_name))
                    body.append('%s}\n' % (indent[1]))
                elif(re.search('^[A-Z0-9_]+_S$',var_type)):
                    body.append('%sif (json_object_object_get_ex(cmd_obj, "%s", &tmp_obj)) {\n' % (indent[1], var_name))
                    body.append('%sparse_%s(&(data->%s), tmp_obj);\n' % (indent[2], var_type.lower()[5:-2], var_name))
                    body.append('%s}\n' % (indent[1]))
                elif(re.search('^[A-Z0-9_]+_E$',var_type)):
                    if cnt_flag == 0 or cnt_flag_ini == 0:
                        body.append('%sint i;\n' % (indent[1]))
                        cnt_flag = 1
                        cnt_flag_ini = 1

                    if str_flag == 0 or str_flag_ini == 0:
                        body.append('%sconst char *str;\n\n' % (indent[1]))
                        str_flag = 1
                        str_flag_ini = 1

                    body.append('%sif (json_object_object_get_ex(cmd_obj, "%s", &tmp_obj)) {\n' % (indent[1], var_name))
                    body.append('%sstr = json_object_get_string(tmp_obj);\n' % (indent[2]))
#                    body.append('%sfor(i=0;i<10;i++) {\n' % (indent[2]))
                    body.append('%sfor (i = 0; (unsigned long)i < sizeof(%s_map)/sizeof(char *); i++) {\n' % (indent[2], var_schema['title'].lower()))
                    body.append('%sif (strcmp(%s_map[i], str) == 0) {\n' % (indent[3], var_type.lower()))
                    body.append('%sdata->%s = (%s) i;\n' % (indent[4], var_name, var_type))
                    body.append('%sbreak;\n' % (indent[4]))
                    body.append('%s}\n' % (indent[3]))
                    body.append('%s}\n' % (indent[2]))
                    body.append('%s}\n' % (indent[1]))
            else:
                if(var_type == 'AGTX_UINT8'):
                    if cnt_flag == 0 or cnt_flag_ini == 0:
                        body.append('%sint i;\n\n' % (indent[1]))
                        cnt_flag = 1
                        cnt_flag_ini = 1

                    body.append('%sif (json_object_object_get_ex(cmd_obj, "%s", &tmp_obj)) {\n' % (indent[1], var_name))
                    body.append('%si = min(MAX_%s_%s_SIZE, json_object_get_string_len(tmp_obj));\n' % (indent[2], struct_name.upper(),var_name.upper()))
                    body.append('%sstrncpy((char *)data->%s, json_object_get_string(tmp_obj), i);\n' % (indent[2],var_name))
                    body.append('%sdata->%s[i] = \'\\0\';\n' % (indent[2],var_name))
                    body.append('%s}\n' % (indent[1]))
                else:
                    if cnt_flag == 0 or cnt_flag_ini == 0:
                        body.append('%sint i;\n' % (indent[1]))
                        cnt_flag = 1
                        cnt_flag_ini = 1

                    body.append('%sif (json_object_object_get_ex(cmd_obj, "%s", &tmp_obj)) {\n' % (indent[1], var_name))
                    #body.append('%sarr_length = json_object_array_length(tmp_obj);\n' % (indent[2]))
                    #body.append('%sfor (i = 0; i < arr_length; i++) {\n' % (indent[2]))
                    body.append('%sfor (i = 0; i < MAX_%s_%s_SIZE; i++) {\n' % (indent[2], struct_name.upper(),var_name.upper()))
                    if(var_type == 'AGTX_BOOL'):
                        body.append('%sdata->%s[i] = json_object_get_bool(json_object_array_get_idx(tmp_obj, i));\n' % (indent[3],var_name))
                    elif(var_type == 'AGTX_INT32'):
                        body.append('%sdata->%s[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));\n' % (indent[3],var_name))
                    elif(var_type == 'AGTX_FLOAT'):
                        body.append('%sdata->%s[i] = json_object_get_double(json_object_array_get_idx(tmp_obj, i));\n' % (indent[3],var_name))
                    elif(re.search('^AGTX_[A-Z0-9_]+_S$',var_type)):
                        body.append('%sparse_%s(&(data->%s[i]), json_object_array_get_idx(tmp_obj, i));\n' % (indent[3], var_type.lower()[5:-2], var_name))
                    body.append('%s}\n' % (indent[2]))
                    body.append('%s}\n' % (indent[1]))
        body.append('}\n\n')
        type = struct_name
    elif schema['type'] == 'array':
        checkRequiredKeys(['items', 'maxItems'], schema)
        header, body, func, var_type, var_size, var_comment = schemaToCSource(schema['items'])
        type = var_type
        size = schema['maxItems']
    elif schema['type'] == 'integer':
        type = 'AGTX_INT32'
    elif schema['type'] == 'number':
        type = 'AGTX_FLOAT'
    elif schema['type'] == 'boolean':
        #type = 'AGTX_BOOL'
        type = 'AGTX_INT32'
    elif schema['type'] == 'string':
        if 'maxLength' in schema:
            type = 'AGTX_UINT8'
            size = schema['maxLength'] + 1  # plus one for string terminator
        elif 'enum' in schema:
            checkRequiredKeys(['title'], schema)
            enum_name = schema['title']
            if enum_name in existing_enum.keys():
                original_schema = pprint.pformat(existing_enum[enum_name])
                current_schema = pprint.pformat(schema)
                if(hash(original_schema) != hash(current_schema)):
                    print('Error: enum %s already declared using another schema.' % enum_name)
                    print('\t---- original schema ----')
                    print('%s' % original_schema)
                    print('\t---- current schema ----')
                    print('%s\n' % current_schema)
                    exit();
                else:
                    print('Info: enum %s already declared using identical schema.' % enum_name)
                    type = enum_name
            else:
                existing_enum[enum_name] = schema
                enum_name_without_postfix = enum_name[0:-2]
                enum_list = schema['enum']
                header.append('const char * %s_map[] = {\n' % (enum_name.lower()))
                for enum_item in enum_list[:-1]:
                    header.append('%s"%s",\n' % (indent[1], enum_item))
                else:
                    header.append('%s"%s"\n' % (indent[1], enum_list[-1]))
                header.append('};\n\n')
                type = enum_name
        else:
            dumpSchema('Missing "maxLength" or "enum" in schema', schema)
    else:
        dumpSchema('Un-supported "type" in schema.', schema)

    #if 'description' in schema:
    #    comment = makeComment(schema['description'])

    return(header, body, func, type, size, comment)

### --------------------------------------  end C source --------------------------------------------------  ###

def schemaToJSource(schema_key, schema):
    global existing_enum
    global str_flag
    global cnt_flag
    indent, level = genIndent(10)

    header = []
    body = []
    func = []
    type = ''
    # If size = 0, means it's an scaler.
    # If size = n, where n > 0, means it's an array of size n.
    size = 0
    comment = ''
    level_idx = 0
    indent_idx = level_idx + 1

    checkRequiredKeys(['type'], schema)
#    print("schema['type']",schema['type'])
    if schema['type'] == 'object':
        checkRequiredKeys(['properties'], schema)

        if 'title' in schema:
            struct_name = schema['title']
        else:
            struct_name = '%s_%X_S' % (top_level_struct_name, abs(hash(pprint.pformat(schema))))

        str_flag_ini = 0
        cnt_flag_ini = 0
        func_name = struct_name.lower()[5:-2]
        if schema_key == 'array':
            body.append('%svoid comp_%s(struct json_object *array_obj, %s *data)\n' % (indent[indent_idx-1], func_name, struct_name))
            func.append('%svoid comp_%s(struct json_object *array_obj, %s *data);' % (indent[indent_idx-1], func_name, struct_name))
            body.append('%s{\n' % (indent[indent_idx-1]))
            body.append('%sstruct json_object *tmp%s_obj = NULL;\n\n' % (indent[indent_idx], level[level_idx]))
            body.append('%stmp%s_obj = json_object_new_object();\n\n' % (indent[indent_idx], level[level_idx]))
            body.append('%sif (tmp%s_obj) {\n' % (indent[indent_idx], level[level_idx]))
            level_idx = level_idx + 1
            indent_idx = level_idx + 1
        else:
            if (func_name == "event_conf"):
                func_name = "evt_conf"
            elif (func_name == "stitching_conf"):
                func_name = "stitch_conf"
            body.append('%svoid comp_%s(struct json_object *ret_obj, %s *data)\n' % (indent[indent_idx-1], func_name, struct_name))
            func.append('%svoid comp_%s(struct json_object *ret_obj, %s *data);' % (indent[indent_idx-1], func_name, struct_name))
            body.append('%s{\n' % (indent[indent_idx-1]))

        ###  init ###
        body.append('%sstruct json_object *tmp%s_obj = NULL;\n\n' % (indent[indent_idx], level[level_idx]))

        for var_name, var_schema in sorted(schema['properties'].items()):
            header[0:0], body[0:0], func[0:0], var_type, var_size, var_comment = schemaToJSource(var_name, var_schema)
#            print(var_name, var_schema, header[0:0], body[0:0], var_type, var_size, var_comment)
#            print("\n\n")
            if(var_size == 0):
                if(var_type == 'AGTX_BOOL') or (var_type == 'AGTX_INT32'):
                    body.append('%stmp%s_obj = json_object_new_int(data->%s);\n' % (indent[indent_idx], level[level_idx], var_name))
                    body.append('%sif (tmp%s_obj) {\n' % (indent[indent_idx], level[level_idx]))

                    if (level_idx == 0):
                        body.append('%sjson_object_object_add(ret_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], var_name, level[level_idx]))
                    else:
                        body.append('%sjson_object_object_add(tmp%s_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], level[level_idx - 1], var_name, level[level_idx]))

                    body.append('%s} else {\n' % (indent[indent_idx]))
                    body.append('%sprintf("Cannot create %%s object\\n", "%s");\n' % (indent[indent_idx+1],var_name))
                    body.append('%s}\n\n' % (indent[indent_idx]))
                elif (var_type == 'AGTX_FLOAT'):
                    body.append('%stmp%s_obj = json_object_new_double(data->%s);\n' % (indent[indent_idx], level[level_idx], var_name))
                    body.append('%sif (tmp%s_obj) {\n' % (indent[indent_idx], level[level_idx]))

                    if (level_idx == 0):
                        body.append('%sjson_object_object_add(ret_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], var_name, level[level_idx]))
                    else:
                        body.append('%sjson_object_object_add(tmp%s_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], level[level_idx - 1], var_name, level[level_idx]))

                    body.append('%s} else {\n' % (indent[indent_idx]))
                    body.append('%sprintf("Cannot create %%s object\\n", "%s");\n' % (indent[indent_idx+1],var_name))
                    body.append('%s}\n\n' % (indent[indent_idx]))

                elif(re.search('^[A-Z0-9_]+_S$',var_type)):
                    body.append('%stmp%s_obj = json_object_new_object();\n' % (indent[indent_idx], level[level_idx]))
                    body.append('%sif (tmp%s_obj) {\n' % (indent[indent_idx], level[level_idx]))
                    body.append('%scomp_%s(tmp%s_obj, &(data->%s));\n' % (indent[indent_idx+1], var_type.lower()[5:-2], level[level_idx], var_name))

                    if (level_idx == 0):
                        body.append('%sjson_object_object_add(ret_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], var_name, level[level_idx]))
                    else:
                        body.append('%sjson_object_object_add(tmp%s_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], level[level_idx - 1], var_name, level[level_idx]))

                    body.append('%s} else {\n' % (indent[indent_idx]))
                    body.append('%sprintf("Cannot create %%s object\\n", "%s");\n' % (indent[indent_idx+1],var_name))
                    body.append('%s}\n\n' % (indent[indent_idx]))
                elif(re.search('^[A-Z0-9_]+_E$',var_type)):
                    if str_flag != 1 or str_flag_ini == 0:
                        body.append('%sconst char *str;\n' % (indent[indent_idx]))
                        str_flag = 1
                        str_flag_ini = 1

                    body.append('%sstr = %s_map[data->%s];\n' % (indent[indent_idx], var_schema['title'].lower(), var_name))
                    body.append('%stmp%s_obj = json_object_new_string(str);\n' % (indent[indent_idx], level[level_idx]))
                    body.append('%sif (tmp%s_obj) {\n' % (indent[indent_idx], level[level_idx]))

                    if (level_idx == 0):
                        body.append('%sjson_object_object_add(ret_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], var_name, level[level_idx]))
                    else:
                        body.append('%sjson_object_object_add(tmp%s_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], level[level_idx - 1], var_name, level[level_idx]))

                    body.append('%s} else {\n' % (indent[indent_idx]))
                    body.append('%sprintf("Cannot create %%s object\\n", "%s");\n' % (indent[indent_idx+1],var_name))
                    body.append('%s}\n\n' % (indent[indent_idx]))
                else: ## string
                    if str_flag != 1 or str_flag_ini == 0:
                        body.append('%sconst char *str;\n' % (indent[indent_idx]))
                        str_flag = 1
                        str_flag_ini = 1

                    body.append('%sstr = %s_map[data->%s];\n' % (indent[indent_idx], var_schema['title'].lower(), var_name))
                    body.append('%stmp%s_obj = json_object_new_string(str);\n' % (indent[indent_idx], level[level_idx]))
                    body.append('%sif (tmp%s_obj) {\n' % (indent[indent_idx], level[level_idx]))

                    if (level_idx == 0):
                        body.append('%sjson_object_object_add(ret_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], var_name, level[level_idx]))
                    else:
                        body.append('%sjson_object_object_add(tmp%s_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], level[level_idx - 1], var_name, level[level_idx]))

                    body.append('%s} else {\n' % (indent[indent_idx]))
                    body.append('%sprintf("Cannot create %%s object\\n", "%s");\n' % (indent[indent_idx+1],var_name))
                    body.append('%s}\n\n' % (indent[indent_idx]))
            else: # var_size != 0
                if (var_type == 'AGTX_UINT8'): # "type": "array"
                   if str_flag != 1 or str_flag_ini == 0:
                       body.append('%sconst char *str;\n' % (indent[indent_idx]))
                       str_flag = 1
                       str_flag_ini = 0

                   body.append('%sstr = (const char *)data->%s;\n' % (indent[indent_idx], var_name))
                   body.append('%stmp%s_obj = json_object_new_string(str);\n' % (indent[indent_idx], level[level_idx]))
                   body.append('%sif (tmp%s_obj) {\n' % (indent[indent_idx], level[level_idx]))

                   if (level_idx == 0):
                       body.append('%sjson_object_object_add(ret_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], var_name, level[level_idx]))
                   else:
                       body.append('%sjson_object_object_add(tmp%s_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], level[level_idx - 1], var_name, level[level_idx]))

                   body.append('%s} else {\n' % (indent[indent_idx]))
                   body.append('%sprintf("Cannot create %%s object\\n", "%s");\n' % (indent[indent_idx+1],var_name))
                   body.append('%s}\n\n' % (indent[indent_idx]))

                else:
                    if cnt_flag == 0 or cnt_flag_ini == 0:
                        body.append('%sint i;\n' % (indent[indent_idx]))
                        cnt_flag = 1
                        cnt_flag_ini = 1

                    body.append('%stmp%s_obj = json_object_new_array();\n' % (indent[indent_idx], level[level_idx]))
                    body.append('%sif (tmp%s_obj) {\n' % (indent[indent_idx], level[level_idx]))
                    body.append('%sfor (i = 0; i < MAX_%s_%s_SIZE; i++) {\n' % (indent[indent_idx+1], struct_name.upper(),var_name.upper()))
                    if(var_type == 'AGTX_BOOL'):
                       body.append('%sjson_object_array_add(tmp%s_obj, json_object_new_bool(data->%s[i]));\n' % (indent[indent_idx+2], level[level_idx], var_name))
                    elif(var_type == 'AGTX_INT32'):
                        body.append('%sjson_object_array_add(tmp%s_obj, json_object_new_int(data->%s[i]));\n' % (indent[indent_idx+2], level[level_idx], var_name))
                    elif(var_type == 'AGTX_FLOAT'):
                        body.append('%sjson_object_array_add(tmp%s_obj, json_object_new_double(data->%s[i]));\n' % (indent[indent_idx+2], level[level_idx], var_name))
                    elif(re.search('^AGTX_[A-Z0-9_]+_S$',var_type)):
                        body.append('%scomp_%s(tmp%s_obj, &(data->%s[i]));\n' % (indent[indent_idx+2], var_type.lower()[5:-2], level[level_idx], var_name))
                    body.append('%s}\n' % (indent[indent_idx+1])) # end for loop

                    if (level_idx == 0):
                        body.append('%sjson_object_object_add(ret_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], var_name, level[level_idx]))
                    else:
                        body.append('%sjson_object_object_add(tmp%s_obj, "%s", tmp%s_obj);\n' % (indent[indent_idx+1], level[level_idx - 1], var_name, level[level_idx]))

                    body.append('%s} else {\n' % (indent[indent_idx]))
                    body.append('%sprintf("Cannot create array %%s object\\n", "%s");\n' % (indent[indent_idx+1],var_name))
                    body.append('%s}\n\n' % (indent[indent_idx]))

        if schema_key == 'array':
            level_idx = level_idx - 1
            indent_idx = level_idx + 1
            body.append('%sjson_object_array_add(array_obj, tmp%s_obj);\n' % (indent[indent_idx+1], level[level_idx]))

            body.append('%s} else {\n' % (indent[indent_idx]))
            body.append('%sprintf("Cannot create object in array\\n");\n' % (indent[indent_idx+1]))
            body.append('%s}\n' % (indent[indent_idx]))

        body.append('%s}\n\n' % (indent[indent_idx-1]))
        type = struct_name
    elif schema['type'] == 'array':
        checkRequiredKeys(['items', 'maxItems'], schema)
        header_sub, body, func, var_type, var_size, var_comment = schemaToJSource(schema['type'], schema['items'])

        type = var_type
        size = schema['maxItems']
    elif schema['type'] == 'integer':
        type = 'AGTX_INT32'
    elif schema['type'] == 'number':
        type = 'AGTX_FLOAT'
    elif schema['type'] == 'boolean':
        #type = 'AGTX_BOOL'
        type = 'AGTX_INT32'
    elif schema['type'] == 'string':
        if 'maxLength' in schema:
            type = 'AGTX_UINT8'
            size = schema['maxLength'] + 1  # plus one for string terminator
        elif 'enum' in schema:
            checkRequiredKeys(['title'], schema)
            enum_name = schema['title']
            if enum_name in existing_enum.keys():
                original_schema = pprint.pformat(existing_enum[enum_name])
                current_schema = pprint.pformat(schema)
                if(hash(original_schema) != hash(current_schema)):
                    print('Error: enum %s already declared using another schema.' % enum_name)
                    print('\t---- original schema ----')
                    print('%s' % original_schema)
                    print('\t---- current schema ----')
                    print('%s\n' % current_schema)
                    exit();
                else:
                    print('Info: enum %s already declared using identical schema.' % enum_name)
                    type = enum_name
            else:
                existing_enum[enum_name] = schema
                enum_name_without_postfix = enum_name[0:-2]
                enum_list = schema['enum']
                header.append('const char * %s_map[] = {\n' % (enum_name.lower()))
                for enum_item in enum_list[:-1]:
                    header.append('%s"%s",\n' % (indent[indent_idx], enum_item))
                else:
                    header.append('%s"%s"\n' % (indent[indent_idx], enum_list[-1]))
                header.append('};\n\n')
                type = enum_name
        else:
            dumpSchema('Missing "maxLength" or "enum" in schema', schema)
    else:
        dumpSchema('Un-supported "type" in schema.', schema)

    #if 'description' in schema:
    #    comment = makeComment(schema['description'])


    return(header, body, func, type, size, comment)


### --------------------------------------  end C source --------------------------------------------------  ###

def getHeaderFun(schema):
    header = []
    end = []

    name = schema['title'].lower()[5:-2]
    if (name == "event_conf"):
        name = "evt"
    elif (name == "osd_conf"):
        name = "osd"

    header.append('#ifdef __cplusplus\n')
    header.append('extern "C" {\n')
    header.append('#endif /* __cplusplus */\n\n\n')


    header.append('#include <stdio.h>\n')
    header.append('#include <string.h>\n')
    header.append('#include "json.h"\n')
    header.append('#include "cm_%s.h"\n\n\n' % (name))

    end.append('#ifdef __cplusplus\n')
    end.append('}\n')
    end.append('#endif /* __cplusplus */\n')

    return header, end

def getHeader(schema, func, func2):
    header = []

    name = schema['title'][5:-2]
    if (name == "EVENT_CONF"):
        name = "EVT"
    elif (name == "OSD_CONF"):
        name = "OSD"


    name_lower = name.lower()

    header.append('/******************************************************************************\n')
    header.append('*\n')
    header.append('* copyright (c) Augentix Inc. - All Rights Reserved\n')
    header.append('*\n')
    header.append('* Unauthorized copying of this file, via any medium is strictly prohibited.\n')
    header.append('*\n')
    header.append('* Proprietary and confidential.\n')
    header.append('*\n')
    header.append('******************************************************************************/\n\n')

    header.append('#ifndef CM_%s_H_\n' %(name))
    header.append('#define CM_%s_H_\n\n' %(name))

    header.append('#ifdef __cplusplus\n')
    header.append('extern "C" {\n')
    header.append('#endif /* __cplusplus */\n\n\n')


    header.append('#include "agtx_types.h"\n')
    header.append('#include "agtx_common.h"\n')
    header.append('#include "agtx_%s.h"\n\n\n' % (name_lower))

    header.append('#define min(X,Y) (((X) < (Y)) ? (X) : (Y))\n\n')
    header.append('struct json_object;\n\n\n')


    header.append('%s\n' % (func[len(func) - 1]))
    header.append('%s\n' % (func2[len(func2) - 1]))

    header.append('\n\n')
    header.append('#ifdef __cplusplus\n')
    header.append('}\n')
    header.append('#endif /* __cplusplus */\n\n')

    header.append('#endif /* !CM_%s_H_ */\n' %(name))

    return header


def help():
    print('Usage:')
    print('\t%s <input file> <output file>' % sys.argv[0])


def main():
    global top_level_struct_name
    global str_flag,cnt_flag
	# get input and output filenames
    if(len(sys.argv) < 3):
        help()
        exit()

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    if(input_file == output_file):
        print('Error: input and output are the same file.')
        exit()

	# load schema
    with open(input_file, 'r') as f:
        schema = json.load(f)

	# top-level schema must have a title because it converts to a C struct
	# also use it for any sub-level schema that converts to a C struct but without a title
    checkRequiredKeys(['type', 'title', 'properties'], schema)
    top_level_struct_name = schema['title'][:-2]


	# add header and footer
    header = []
    body = []
    header, end = getHeaderFun(schema)

	# process schema
    header1, body1, func1, _, _, _ = schemaToCSource(schema)
    str_flag = 0
    cnt_flag = 0
    header2, body2, func2, _, _, _ = schemaToJSource("",schema)
    header.extend(header1)
    header.extend(header2)
    body.extend(body1)
    body.extend(body2)
    body.extend(end)

	# write output file
    if os.path.isfile(output_file):
        os.unlink(output_file)
    writeFile(output_file, header + body, 'wt')

    # add cm header
    header = []
    header = getHeader(schema, func1, func2)

    # write output file
    cm_header_file = output_file[:-1] + "h"
    print("cm_header_file",cm_header_file)
    if os.path.isfile(cm_header_file):
        os.unlink(cm_header_file)
    writeFile(cm_header_file, header, 'wt')


if __name__ == "__main__":
    main()
