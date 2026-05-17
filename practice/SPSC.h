#include <atomic>
#include <stdexcept>
#include <memory>
#include <vector>

template<typename T>
class SPSC{
public:

    SPSC(size_t capacity) noexcept:
    m_capacity{capacity}
    {
        if (m_capacity&(m_capacity-1) !=0){
            throw(std::runtime_error("the capacity must be the power of 2"));
        }
        data.reserve(m_capacity);
    };

bool try_push(T const& data){
    temp_tail = tail.load(std::memory_order_relaxed);
    temp_head = head.load(std::memory_order_acquire);
    if (temp_head > temp_tail){
    return false;
    }
    m_slot.push_back[temp_tail];
    ++temp_tail
    if (temp_tail > m_capacity){
        temp_tail = wrap(temp_tail)
    }
    tail.store(temp_tail,std::memory_order_release);

}
bool try_pop(T &data){
    temp_tail = tail.load(std::memory_order_acquire);
    temp_head = head.load(std::memory_order_relaxed);
    if (temp_head = temp_tail){
        return false;
    }
    data = m_slot.[temp_head];
    ++temp_head;
    head.store(temp_head,std::memory_order_release);
}
    

private:
    int wrap(int idx){
        return idx & (m_capacity-1)
    } 
    size_t m_capacity{0};
    int temp_tail {0};
    int temp_head {0};
    alignas(64) std::atomic<int> tail {0};
    alignas(64) std::atomic<int> head {0};
    std::vector<T> m_slot;



};