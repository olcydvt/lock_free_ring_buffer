#include <atomic>

template<int size>
class ring_buffer {
    static constexpr int buffer_size = size;
    char buffer[buffer_size];
    std::atomic<size_t> read_cursor;
    std::atomic<size_t> write_cursor;

public:

    ring_buffer() {
        memset(buffer, 0, buffer_size);
        read_cursor.store(0);
        write_cursor.store(0);
    }

  

    bool write(void* data, size_t size) {
        size_t old_write_cursor, new_write_cursor;
        do {
            old_write_cursor = write_cursor.load(std::memory_order_acquire);
            new_write_cursor = (old_write_cursor + size) % buffer_size;
            if (new_write_cursor == read_cursor.load(std::memory_order_acquire)) {
                return false; // empty buffer
            }
        } while (!write_cursor.compare_exchange_weak(old_write_cursor, new_write_cursor, std::memory_order_release));
        std::memcpy(buffer + old_write_cursor, data, size);
        std::atomic_thread_fence(std::memory_order_release);  // Ensure memcpy completes before reading
        return true;
    }

    bool read(void* data, size_t size) {
        size_t old_read_cursor, new_read_cursor;
        do {
            old_read_cursor = read_cursor.load(std::memory_order_acquire);
            if (old_read_cursor == write_cursor.load(std::memory_order_acquire)) {
                return false; // empty buffer
            }
            new_read_cursor = (old_read_cursor + size) % buffer_size;
        } while (!read_cursor.compare_exchange_weak(old_read_cursor, new_read_cursor, std::memory_order_release));

        std::atomic_thread_fence(std::memory_order_acquire);  // Ensure memcpy completes before reading
        std::memcpy(data, buffer + old_read_cursor, size);
        return true;
    }
};
