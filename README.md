# Lock Free Ring Buffer

see the Compiler Explorer [Link](https://compiler-explorer.com/z/Yea8f3MaP) output with the thread sanitizer flag(-std=c++20 -fsanitize=thread -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer)


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
            std::thread([&, i] { bool written = ring_bfr.try_write(i, 1); }));
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

    // test the values readen from ring buffer by indexing in map.
    // value of each index must be 1, therefore we can say that there is no
    // duplicated write for same value
    std::map<int, int> map;
    for (auto& val : values) {
        map[val] += 1;
    }

    return 0;
}
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License

[MIT](https://choosealicense.com/licenses/mit/)
