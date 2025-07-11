TEST_SRCS=$(TEST_PATH)/main.c
TEST_BIN=$(TEST_PATH)/main

TEST_ENV=$(HOME)/local/libs/linux
TFLITE_INC=-I$(TEST_ENV)/tflite/inc
TFLITE_LIBS=-L$(TEST_ENV)/tflite/lib -ltensorflow-lite -Wl,-rpath=$(TEST_ENV)/tflite/lib
INF_LIBS=-L$(LIBINF_PATH)/lib -linf -Wl,-rpath=$(LIBINF_PATH)/lib
TEST_INC := \
	-I$(LIBINF_INC) \
	-I$(MPP_INC) \
	$(TFLITE_INC)

CFLAGS_TEST := -Wall -g
LDFLAGS_TEST := \
	$(TFLITE_LIBS) \
	$(INF_LIBS) \
	-ldl \
	-lm \
	-lrt \
	-pthread

test: $(TEST_BIN)

.PHONY: test-clean
test-clean: 
	$(RM) -rf $(TEST_BIN)

.PHONY: test

$(TEST_BIN): CC=gcc
$(TEST_BIN): $(TEST_SRCS)
	$(Q)$(CC) -o $@ $^ $(CFLAGS_TEST) $(TEST_INC) $(LDFLAGS_TEST)
