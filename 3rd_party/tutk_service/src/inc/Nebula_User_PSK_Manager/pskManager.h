#ifndef __PSK_MANAGER_H__
#define __PSK_MANAGER_H__

#define MAX_IDENTITIES_LIST_SIZE 4096

int GetPskFromFile(const char *identity, const char *file_path, char *psk_buf, int buf_size);
int AppendPskToFile(const char *identity, const char *file_path, char *psk_buf, int buf_size);
int GetAllIdentitiesFromFile(const char *file_path, char **identities_json_str);
int GetCredential(const char *udid, const char *secret_id, const char *identity, const char *mode,
                  const char *file_path, char **credential);
int DeleteCredential(const char *identity, const char *file_path);

#endif