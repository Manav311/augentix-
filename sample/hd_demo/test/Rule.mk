TEST_SRCS=../test/main.c ../src/hd_demo.c
TEST_BIN=../test/main

test: $(TEST_BIN)

.PHONY: test-clean
test-clean: 
	$(RM) -rf $(TEST_BIN)

$(TEST_BIN): $(TEST_SRCS)
	$(Q)$(CC) $(CFLAGS) -o $@ $^