import sys
import json
import pprint
import os


indent = '\t'
top_level_struct_name = ''
existing_enum = {}


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


def schemaToStruct(schema):
    global existing_enum

    header = []
    body = []
    type = ''
    # If size = 0, means it's an scaler.
    # If size = n, where n > 0, means it's an array of size n.
    size = 0
    comment = ''

    checkRequiredKeys(['type'], schema)

    if schema['type'] == 'object':
        checkRequiredKeys(['properties'], schema)

        if 'title' in schema:
            struct_name = schema['title']
        else:
            struct_name = '%s_%X_S' % (
                top_level_struct_name, abs(hash(pprint.pformat(schema))))

        body.append('typedef struct {\n')
        for var_name, var_schema in sorted(schema['properties'].items()):
            header[0:0], body[0:0], var_type, var_size, var_comment = schemaToStruct(
                var_schema)
            if(var_size == 0):
                body.append('%s%s %s;%s\n' %
                            (indent, var_type, var_name, var_comment))
            else:
                var_size_define = 'MAX_%s_%s_SIZE' % (
                    struct_name.upper(), var_name.upper())
                body.append(
                    '%s%s %s[%s];%s\n' %
                    (indent,
                     var_type,
                     var_name,
                     var_size_define,
                     var_comment))
                header.append('#define %s %d\n' % (var_size_define, var_size))
        body.append('} %s;\n\n' % struct_name)
        type = struct_name
    elif schema['type'] == 'array':
        checkRequiredKeys(['items', 'maxItems'], schema)
        header, body, var_type, var_size, var_comment = schemaToStruct(
            schema['items'])
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
                    exit()
                else:
                    print('Info: enum %s already declared using identical schema.' % enum_name)
                    type = enum_name
            else:
                existing_enum[enum_name] = schema
                enum_name_without_postfix = enum_name[0:-2]			
                enum_list = schema['enum']
                header.append('typedef enum {\n')
                for enum_item in enum_list[:-1]:
                    header.append(
						  '%s%s_%s,\n' %
						  (indent, enum_name_without_postfix, enum_item))
                else:
                    header.append('%s%s_%s\n' %
								  (indent, enum_name_without_postfix, enum_list[-1]))
                header.append('} %s;\n\n' % (enum_name))
                type = enum_name
        else:
            dumpSchema('Missing "maxLength" or "enum" in schema', schema)
    else:
        dumpSchema('Un-supported "type" in schema.', schema)

    if 'description' in schema:
        comment = makeComment(schema['description'])

    return(header, body, type, size, comment)


def help():
    print('Usage:')
    print('\t%s <input file> <output file>' % sys.argv[0])


def main():
    global top_level_struct_name

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
	
	# process schema
    header, body, _, _, _ = schemaToStruct(schema)
	
	# add header and footer
    define_guard = '%s_H_' % top_level_struct_name.upper()		
    header.insert(0, '#ifndef %s\n' % define_guard)
    header.insert(1, '#define %s\n\n' % define_guard)
    header.insert(2, '#include "agtx_types.h"\n')
    header.insert(3, 'struct json_object;\n\n')
    body.insert(0, '\n')
    #body.append('void parse_%s_s(%s_S *data, struct json_object *cmd_obj);\n' %(top_level_struct_name.lower(), top_level_struct_name))
    body.append('\n')
    body.append('#endif /* %s */\n' % define_guard)

	# write output file
    if os.path.isfile(output_file):
        os.unlink(output_file)	
    writeFile(output_file, header + body, 'wt')


if __name__ == "__main__":
    main()
