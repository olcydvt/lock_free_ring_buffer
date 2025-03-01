# Lock Free MPMC Ring Buffer

see the Compiler Explorer [Link](https://compiler-explorer.com/z/Yea8f3MaP) that shows you output along with thread sanitizer check(-std=c++20 -fsanitize=thread -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer)


## Usage
```c++
#include <thread>
#include <vector>
#include <map>

int main() {
    ring_buffer<256, int> ring_bfr;
    std::vector<std::thread> threads;

    for (int i = 0; i < 128; ++i) {
        threads.emplace_back(
            std::thread([&, i] { bool written = ring_bfr.try_write(i); }));
    }

    for (int i = 0; i < 128; ++i) {
        threads.emplace_back(std::thread([&, i] {
            int val;
            bool written = ring_bfr.try_read(val);
        }));
    }

    for (auto& th : threads) {
        th.join();
    }

    // Examine the values retrieved from the ring buffer by indexing in MAP. Each index value should equal 1, which indicates that there are no duplicate writes for the same value.
    std::map<int, int> map;
    for (auto& val : values) {
        map[val] += 1;
    }

    return 0;
}
```

## License
[MIT](https://choosealicense.com/licenses/mit/)
