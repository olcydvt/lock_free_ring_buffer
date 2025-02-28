#include <atomic>
#include <bitset>
#include <cstring>

template <uint32_t buf_size, typename T = std::byte>
class ring_buffer {
    static constexpr int buffer_size = buf_size;
    T buffer[buffer_size];
    alignas(64) std::atomic<uint32_t> read_cursor;
    alignas(64) std::atomic<uint32_t> write_cursor;
    std::atomic<T*> write_pointer;
    std::atomic<T*> read_pointer;

   public:
    ring_buffer() {
        memset(buffer, 0, buffer_size);
        read_cursor.store(0);
        write_cursor.store(0);
        write_pointer.store(nullptr);
        read_pointer.store(nullptr);
    }

    T* get_buffer() { return buffer; }

    [[nodiscard]] bool try_write(const T& data, uint32_t size) {
        uint32_t old_write_cursor, new_write_cursor;
        do {
            old_write_cursor = write_cursor.load(std::memory_order_acquire);
            uint32_t current_read_cursor =
                read_cursor.load(std::memory_order_acquire);
            new_write_cursor = (old_write_cursor + size) % buffer_size;
            uint32_t available_space =
                buffer_size - (old_write_cursor - current_read_cursor);
            if (new_write_cursor + 1 == current_read_cursor ||
                size > available_space) {
                return false;  // empty buffer
            }
            uint32_t idx = old_write_cursor & (buffer_size - 1);
            buffer[idx] = data;
        } while (!write_cursor.compare_exchange_weak(
            old_write_cursor, new_write_cursor, std::memory_order_release));

        T* current_write_pointer;
        do {
            current_write_pointer = write_pointer.load();
        } while (!write_pointer.compare_exchange_weak(
            current_write_pointer, buffer + new_write_cursor,
            std::memory_order_release));

        return true;
    }

    [[nodiscard]] bool try_read(T& data) {
        uint32_t old_read_cursor, new_read_cursor;
        do {
            old_read_cursor = read_cursor.load(std::memory_order_acquire);
            if (old_read_cursor ==
                write_cursor.load(std::memory_order_acquire)) {
                return false;  // empty buffer
            }
            new_read_cursor = (old_read_cursor + 1) % buffer_size;
            std::memcpy(&data, buffer + old_read_cursor, 1);
        } while (!read_cursor.compare_exchange_weak(
            old_read_cursor, new_read_cursor, std::memory_order_release));

        return true;
    }
};
