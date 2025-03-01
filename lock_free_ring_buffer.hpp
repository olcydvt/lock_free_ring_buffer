#include <atomic>
#include <cstring>

template <uint32_t buf_size, typename T = std::byte>
class ring_buffer {
    static_assert((buf_size & (buf_size - 1)) == 0, "Buffer size must be a power of 2");
    static constexpr uint32_t buffer_size = buf_size;
    T buffer[buffer_size];
    alignas(64) std::atomic<uint32_t> read_cursor;
    alignas(64) std::atomic<uint32_t> write_cursor;

public:
    ring_buffer() noexcept {
        memset(buffer, 0, sizeof(buffer));
        read_cursor.store(0, std::memory_order_relaxed);
        write_cursor.store(0, std::memory_order_relaxed);
    }

    T* get_buffer() noexcept { return buffer; }

    [[nodiscard]] bool try_write(const T& data, uint32_t size) noexcept {
        uint32_t old_write_cursor, new_write_cursor;
        uint32_t current_read_cursor;
        uint32_t available_space;
        
        do {
            old_write_cursor = write_cursor.load(std::memory_order_acquire);
            current_read_cursor = read_cursor.load(std::memory_order_acquire);
            new_write_cursor = (old_write_cursor + size) % buffer_size;
            available_space = buffer_size - (old_write_cursor - current_read_cursor);
            
            if (new_write_cursor + 1 == current_read_cursor || size > available_space) {
                return false;
            }
            
            uint32_t idx = old_write_cursor & (buffer_size - 1);
            buffer[idx] = data;
        } while (!write_cursor.compare_exchange_weak(old_write_cursor, new_write_cursor, std::memory_order_release));
        
        return true;
    }

    [[nodiscard]] bool try_read(T& data) noexcept {
        uint32_t old_read_cursor, new_read_cursor;
        
        do {
            old_read_cursor = read_cursor.load(std::memory_order_acquire);
            
            if (old_read_cursor == write_cursor.load(std::memory_order_acquire)) {
                return false;
            }
            
            new_read_cursor = (old_read_cursor + 1) % buffer_size;
            std::memcpy(&data, buffer + old_read_cursor, sizeof(T));
        } while (!read_cursor.compare_exchange_weak(old_read_cursor, new_read_cursor, std::memory_order_release));
        
        return true;
    }
};
