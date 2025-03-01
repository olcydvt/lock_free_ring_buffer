#include <atomic>
#include <cstring>
#include <stdexcept>

template <uint32_t buf_size, typename T = std::byte>
class ring_buffer {
    static_assert((buf_size & (buf_size - 1)) == 0, "Buffer size must be a power of 2");

    static constexpr uint32_t buffer_size = buf_size;
    T buffer[buffer_size];
    alignas(64) std::atomic<uint32_t> read_cursor{0};
    alignas(64) std::atomic<uint32_t> write_cursor{0};

public:
    ring_buffer() noexcept {
        memset(buffer, 0, sizeof(buffer));
    }

    T* get_buffer() noexcept { return buffer; }

    [[nodiscard]] bool try_write(const T& data) noexcept {
        uint32_t old_write_cursor, new_write_cursor;
        uint32_t current_read_cursor;

        do {
            old_write_cursor = write_cursor.load(std::memory_order_relaxed);
            current_read_cursor = read_cursor.load(std::memory_order_acquire);

            // Check if the buffer is full
            new_write_cursor = (old_write_cursor + 1) % buffer_size;
            if (new_write_cursor == current_read_cursor) {
                return false;  // Buffer is full
            }

            // Try to claim the slot
        } while (!write_cursor.compare_exchange_weak(
            old_write_cursor, new_write_cursor, std::memory_order_acq_rel));

        // Write the data to the claimed slot
        buffer[old_write_cursor] = data;
        return true;
    }

    [[nodiscard]] bool try_read(T& data) noexcept {
        uint32_t old_read_cursor, new_read_cursor;
        uint32_t current_write_cursor;

        do {
            old_read_cursor = read_cursor.load(std::memory_order_relaxed);
            current_write_cursor = write_cursor.load(std::memory_order_acquire);

            // Check if the buffer is empty
            if (old_read_cursor == current_write_cursor) {
                return false;  // Buffer is empty
            }

            // Try to claim the slot
            new_read_cursor = (old_read_cursor + 1) % buffer_size;
        } while (!read_cursor.compare_exchange_weak(
            old_read_cursor, new_read_cursor, std::memory_order_acq_rel));

        // Read the data from the claimed slot
        data = buffer[old_read_cursor];
        return true;
    }
};
