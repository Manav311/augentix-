#include <ctype.h>

#include "platforms.h"
#include "inifile.h"

/**
 * Definition
*/
enum { VALID, SECTION_HEAD, BLANK_LINE, INVALID_LINE, COMMENT };

#define _WORKLINESIZE_ (1000)
#define AllocWorkLine() (char *)My_malloc(_WORKLINESIZE_)
#define dbg_msg printf

/**
 * Static Function Prototype
*/
static const char *jumpOverSpace(const char *line);
static int formatKVLine(char *line, char **key, char **val);
static int _ForwardLocateSection(SA_FILE *fp, const char *section, char line[_WORKLINESIZE_]);
static void writeSectionKVPairs(SA_FILE *fp, char *kvs);
static char *readRestOfFile(SA_FILE *fp);

/**
 * Function
*/
SA_BOOL GetProfileString(const char *iniFile, const char *section, const char *szKey, char *value, int len)
{
	SA_FILE *fp = NULL;
	char *line = NULL;
	SA_BOOL gotIt;
	int kLen = 0;

	fp = SA_fopen(iniFile, "r");
	if (!fp)
		return SA_FALSE;

	gotIt = SA_FALSE;
	line = AllocWorkLine();
	if (!line)
		return SA_FALSE;

	if (_ForwardLocateSection(fp, section, line) < 0) {
		My_free(line);
		SA_fclose(fp);
		return SA_FALSE;
	}

	kLen = strlen(szKey);
	SA_DECL_FGETS_BUFFER(line, _WORKLINESIZE_);
	while (SA_fgets(line, _WORKLINESIZE_, fp)) {
		const char *p, *pr;

		if ((p = jumpOverSpace(line)) == NULL)
			continue;
		if (strncasecmp(p, szKey, kLen))
			continue;

		/* compare key. should like
         *    key=xxx
         *      or
         *    key = xxx
         */
		p += kLen;
		while (*p && isspace(*p))
			p++;
		if (*p != '=')
			continue;

		/* value */
		p++; //after '='
		while (*p && isspace(*p))
			p++;
		pr = p + strlen(p) - 1;
		while (pr != p && isspace(*pr))
			pr--;
		pr++;
		if (*p == '"' && pr[-1] == '"') {
			p++;
			pr--;
		}

		//Copy value of key
		len--;
		while (p != pr && len > 0) {
			//*value++ = *p++;
			*value = *p;
			value++;
			p++;
			len--;
		}
		*value = '\0';
		gotIt = SA_TRUE;
		break;
	}
	My_free(line);
	SA_fclose(fp);
	return gotIt;
}

int GetProfileInt(const char *iniFile, const char *section, const char *key, int def)
{
	char strInt[24] = { 0 };
	if (GetProfileString(iniFile, section, key, strInt, sizeof(strInt)))
		return strtol(strInt, NULL, 0);
	else
		return def;
}

//If return is not null, the caller has responsibility to free it
char *GetProfileSection(const char *iniFile, const char *section)
{
	SA_FILE *fp = NULL;
	char *line, *rlt = NULL;
	int size, len = 0;

	fp = SA_fopen(iniFile, "r");
	if (!fp)
		return NULL;

	rlt = NULL;
	line = AllocWorkLine();
	if (_ForwardLocateSection(fp, section, line) >= 0) {
		size = 0;
		len = 0;
		SA_DECL_FGETS_BUFFER(line, _WORKLINESIZE_);
		while (SA_fgets(line, _WORKLINESIZE_, fp)) {
			char *ks, *vs;
			int li;
			if ((li = formatKVLine(line, &ks, &vs)) == VALID) {
				int vlen = strlen(ks) + strlen(vs) + 1;
				if (len + vlen + 1 >= size) {
					size += vlen + 1000;
					rlt = (char *)My_realloc(rlt, size);
				}

				len += sprintf(rlt + len, "%s=%s", ks, vs) + 1;
			}
			if (li == SECTION_HEAD)
				break;
		}
		if (rlt)
			rlt[len] = '\0';
	}

	My_free(line);
	SA_fclose(fp);
	return rlt;
}

SA_BOOL SetProfileSection(const char *iniFile, const char *section, char *sectionValue)
{
	SA_FILE *fp = NULL;
	int offset = 0;
	char *buf, *line = NULL;

	fp = SA_fopen(iniFile, "r+");
	line = AllocWorkLine();
	if (!fp || (offset = _ForwardLocateSection(fp, section, line)) < 0) {
		if (sectionValue) {
			int newf = !fp;
			if (!fp)
				fp = SA_fopen(iniFile, "w");

			if (fp) {
				if (section && *section)
					SA_fprintf(fp, newf ? "[%s]\n" : "\n[%s]\n", section);
				if (*sectionValue)
					writeSectionKVPairs(fp, sectionValue);
				else
					SA_fprintf(fp, "\n");
				goto ok_out;
			}
		}
	}

	if (!fp) {
		My_free(line);
		return SA_FALSE;
	}

	if (sectionValue)
		offset = ftell(fp);
	while (SA_fgets(line, _WORKLINESIZE_, fp)) {
		const char *p;
		if ((p = jumpOverSpace(line)) && (*p == '['))
			break;
	}

	int _eof = feof(fp);
	buf = readRestOfFile(fp);
	SA_fseek(fp, offset, 0);
	if (sectionValue && *sectionValue)
		writeSectionKVPairs(fp, sectionValue);
	if (!_eof)
		fputs(line, fp);
	if (buf) {
		fputs(buf, fp);
		My_free(buf);
	}

ok_out:
	SA_fflush(fp);
	ftruncate(fileno(fp), ftell(fp));
	SA_fclose(fp);
	My_free(line);

	return SA_TRUE;
}

SA_BOOL SetProfileString(const char *iniFile, const char *section, const char *szKey, const char *value)
{
	SA_FILE *fp = NULL;
	int offset, kLen = 0;
	char *buf, *line = NULL;

	fp = SA_fopen(iniFile, "r+");
	line = AllocWorkLine();
	if (!fp || _ForwardLocateSection(fp, section, line) < 0) {
		int newf = 0;
		if (!fp) {
			fp = SA_fopen(iniFile, "w");
			newf = 1;
		}
		if (fp) {
			if (section && *section)
				SA_fprintf(fp, newf ? "[%s]\n" : "\n[%s]\n", section);
			if (value)
				SA_fprintf(fp, "%s = %s\n", szKey, value);
			goto ok_out;
		} else {
			My_free(line);
			return SA_FALSE;
		}
	}

	kLen = strlen(szKey);
	buf = NULL;
	offset = ftell(fp);
	while (SA_fgets(line, _WORKLINESIZE_, fp)) {
		const char *p;
		if ((p = jumpOverSpace(line)) == NULL)
			continue;
		if (*p == '[') {
			SA_fseek(fp, offset, 0);
			buf = readRestOfFile(fp);
			break;
		}
		if (strncasecmp(szKey, p, kLen) == 0) {
			p += kLen;
			while (*p && isspace(*p))
				p++;
			if (*p == '=') {
				buf = readRestOfFile(fp);
				break;
			}
		}
		offset = ftell(fp);
	}
	SA_fseek(fp, offset, 0);
	if (value)
		SA_fprintf(fp, "%s = %s\n", szKey, value);
	if (buf) {
		SA_fprintf(fp, "%s", buf);
		My_free(buf);
	}

ok_out:
	SA_fflush(fp);
	ftruncate(fileno(fp), ftell(fp));
	SA_fclose(fp);
	My_free(line);

	return SA_TRUE;
}

SA_BOOL SetProfileInt(const char *iniFile, const char *section, const char *szKey, int val)
{
	char ss[15] = { 0 };
	sprintf(ss, "%d", val);
	return SetProfileString(iniFile, section, szKey, ss);
}

/**
 * Static Function
*/
static const char *jumpOverSpace(const char *line)
{
	const char *p = line;
	while (*p && isspace(*p))
		p++;
	if (*p == ';' || *p == '#' || !*p)
		return NULL;
	return p;
}

//return: enumeration defined before
static int formatKVLine(char *line, char **key, char **val)
{
	const char *ks = NULL;
	if ((ks = jumpOverSpace(line))) {
		int vlen = 0;
		char *eq, *ke, *vs = NULL;

		if (*ks == '[')
			return SECTION_HEAD;
		if (*ks == ';' || *ks == '#')
			return COMMENT;
		if (!(eq = strchr(ks, '=')))
			return INVALID_LINE;

		vlen = strlen(ks);
		while (isspace(line[vlen - 1]))
			vlen--;
		line[vlen] = '\0';
		vs = eq + 1;
		ke = eq - 1;
		*eq = '\0';
		while (*vs && isspace(*vs))
			vs++;
		while (ke > ks && isspace(*ke))
			*ke-- = '\0';
		if (*vs == '"' && line[vlen - 1] == '"') {
			vs++;
			line[vlen - 1] = '\0';
		}

		*key = (char *)ks;
		*val = vs;

		return VALID;
	}
	return BLANK_LINE;
}

static int _ForwardLocateSection(SA_FILE *fp, const char *section, char line[_WORKLINESIZE_])
{
	int sLen, offset = 0;
	SA_BOOL gotSec = 0;

	if (!line)
		return -1;
	if (!section || !*section)
		return SA_ftell(fp);

	gotSec = SA_FALSE;
	offset = 0;
	sLen = strlen(section);
	SA_DECL_FGETS_BUFFER(line, _WORKLINESIZE_);
	while (SA_fgets(line, _WORKLINESIZE_, fp)) {
		const char *p;
		if ((p = jumpOverSpace(line)) == NULL)
			continue;
		if (*p == '[' && strncasecmp(p + 1, section, sLen) == 0 && p[sLen + 1] == ']') {
			gotSec = SA_TRUE;
			break;
		}
		offset = SA_ftell(fp);
	}
	return gotSec ? offset : -1;
}

static void writeSectionKVPairs(SA_FILE *fp, char *kvs)
{
	char *p = NULL;

	if (!kvs)
		return;
	p = kvs;
	while (*p) {
		char *key, *val, *pp;
		pp = p;
		p += strlen(p) + 1;

		if (formatKVLine(pp, &key, &val) == VALID)
			SA_fprintf(fp, "%s = %s\n", key, val);
	}
	SA_fprintf(fp, "\n");
	return;
}

static char *readRestOfFile(SA_FILE *fp)
{
	char *rlt = NULL;
	int size = 0;
	struct stat _stat = { 0 };
	int fd = fileno(fp);

	fstat(fd, &_stat);
	size = _stat.st_size - SA_ftell(fp);

	if (size) {
		rlt = (char *)My_malloc(size + 1);
		SA_fread(rlt, 1, size, fp);
		rlt[size] = '\0';
	} else
		rlt = NULL;
	return rlt;
}
