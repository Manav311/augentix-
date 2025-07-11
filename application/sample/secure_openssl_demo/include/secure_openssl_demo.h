#ifndef SECURE_OPENSSL_DEMO_H_
#define SECURE_OPENSSL_DEMO_H_

int SEOPENSSL_executeDemo(const int i2c_bus_id, const char *hostname, const int port, const char *cert_file,
                          const char *key_file, const char *ca_file);

#endif /* SECURE_OPENSSL_DEMO_H_ */
