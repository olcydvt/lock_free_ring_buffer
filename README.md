# Lock Free Ring Buffer

## Usage

```c++
#include <thread>
#include <vector>
struct test {
    int a;
};

int main()
{
    ring_buffer<10, test> ring_bfr;
    std::vector<std::thread> threads;

    for (int i = 0; i < 5; ++i) {
        test t{};
        t.a = 5;
        threads.emplace_back(std::thread([&] {
            ring_bfr.write(&t, 1);
        }));
    
    }

    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(std::thread([&] {
            test t{};
            t.a = 5;
            ring_bfr.write(&t, 1);
            }));
    }

    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(std::thread([&] {
            test t{};
            ring_bfr.read(t);
         }));
    }
    
    for (auto& th : threads) {
        th.join();
    }

    return 0;
}
```

## License

[MIT](https://choosealicense.com/licenses/mit/)
