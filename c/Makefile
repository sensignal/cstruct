CC := gcc
CFLAGS := -Wall -Wextra -std=c99 -O2
TARGET := test_cstruct
OBJS := cstruct.o test_cstruct.o unity/src/unity.o

# 数学ライブラリのリンク
LDFLAGS := -lm

# カバレッジ測定用のフラグ
COVERAGE_FLAGS := -fprofile-arcs -ftest-coverage

.PHONY: all clean coverage coverage-clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# テスト実行用のターゲット
test: $(TARGET)
	./$(TARGET)

# カバレッジ測定用のターゲット
coverage: coverage-clean
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -O0 -g -c cstruct.c -o cstruct.o
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -O0 -g -c test_cstruct.c -o test_cstruct.o
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -O0 -g -c unity/src/unity.c -o unity/src/unity.o
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -O0 -g -o $(TARGET) $(OBJS) $(LDFLAGS)
	./$(TARGET)
	gcov -b cstruct.c

coverage-clean:
	rm -f $(TARGET) $(OBJS)
	rm -f unity/src/unity.o
	rm -f *.gcda *.gcno *.gcov

clean:
	rm -f $(TARGET) $(OBJS)
	rm -f unity/src/unity.o
