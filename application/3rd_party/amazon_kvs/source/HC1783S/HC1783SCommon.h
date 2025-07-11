#ifndef HC1783S_COMMON_H_
#define HC1783S_COMMON_H_

#define LOG(msg, ...) printf(msg "\n", ##__VA_ARGS__)

#define HANDLE_NULL_CHECK(x)                                                                                                                    \
    if (!(x)) {                                                                                                                                      \
        return -EINVAL;                                                                                                                              \
    }

#define HANDLE_STATUS_CHECK(expectedStatus)                                                                                         \
    if (self->status != (expectedStatus)) {                                                                                                  \
        return -EAGAIN;                                                                                                                              \
    }

#endif //HC1783S_COMMON_H_
