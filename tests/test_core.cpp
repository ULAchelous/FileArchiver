#include <cassert>
#include <cstring>

#include "mathf.h"
#include "renamer.h"

int main() {
    // 全部为同一字符时熵为 0
    int counts[256] = {0};
    counts['a'] = 256;
    assert(information_entropy(counts, 256) == 0.0);

    // 两字符等概率出现时熵恰为 1 bit
    int half[256] = {0};
    half['a'] = 50;
    half['b'] = 50;
    assert(information_entropy(half, 100) == 1.0);

    // total 为 0 时不崩溃，返回 0
    int empty[256] = {0};
    assert(information_entropy(empty, 0) == 0.0);

    assert(r_get_name("test.txt") == "test.txt");
    assert(!r_is_eng("abc123"));

    return 0;
}
