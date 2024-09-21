#ifndef __MLA_ARRAYQUICKMAP__
#define __MLA_ARRAYQUICKMAP__

#include <array>
#include <optional>
#include <stdexcept>

namespace mla
{

// Array-based implementation
template <typename K, typename V, size_t Capacity>
class ArrayQuickMap
{
private:
    struct Entry
    {
        K key;
        V value;
        bool occupied = false;
    };
    std::array<Entry, Capacity> data;
    size_t _size = 0;
    float max_load_factor = 0.75f;

    size_t hash(const K& key) const
    {
        return std::hash<K>{}(key) % Capacity;
    }

public:
    class Iterator
    {
    private:
        ArrayQuickMap* map;
        size_t index;
        void find_next_occupied()
        {
            while(index < Capacity && !map->data[index].occupied)
            {
                ++index;
            }
        }

    public:
        Iterator(ArrayQuickMap* m, size_t i) : map(m), index(i)
        {
            find_next_occupied();
        }
        Iterator& operator++()
        {
            if(index < Capacity)
            {
                ++index;
                find_next_occupied();
            }
            return *this;
        }
        bool operator!=(const Iterator& other) const
        {
            return index != other.index || map != other.map;
        }
        std::pair<const K&, V&> operator*() const
        {
            return {map->data[index].key, map->data[index].value};
        }
    };

    ArrayQuickMap() = default;

    void insert(const K& key, const V& value)
    {
        if(static_cast<float>(_size + 1) / Capacity > max_load_factor)
        {
            throw std::runtime_error("Map is full");
        }
        size_t index = hash(key);
        while(data[index].occupied && data[index].key != key)
        {
            index = (index + 1) % Capacity;
        }
        if(!data[index].occupied)
        {
            _size++;
        }
        data[index] = {key, value, true};
    }

    std::optional<V> get(const K& key) const
    {
        size_t index = hash(key);
        size_t start_index = index;
        do
        {
            if(!data[index].occupied)
            {
                return std::nullopt;
            }
            if(data[index].key == key)
            {
                return data[index].value;
            }
            index = (index + 1) % Capacity;
        } while(index != start_index);
        return std::nullopt;
    }

    void remove(const K& key)
    {
        size_t index = hash(key);
        size_t start_index = index;
        do
        {
            if(!data[index].occupied)
            {
                return;
            }
            if(data[index].key == key)
            {
                data[index].occupied = false;
                _size--;
                return;
            }
            index = (index + 1) % Capacity;
        } while(index != start_index);
    }

    Iterator find(const K& key)
    {
        size_t index = hash(key);
        size_t start_index = index;
        do
        {
            if(!data[index].occupied)
            {
                return end();
            }
            if(data[index].key == key)
            {
                return Iterator(this, index);
            }
            index = (index + 1) % Capacity;
        } while(index != start_index);
        return end();
    }

    Iterator begin()
    {
        return Iterator(this, 0);
    }

    Iterator end()
    {
        return Iterator(this, Capacity);
    }

    size_t size() const
    {
        return _size;
    }

    bool empty() const
    {
        return _size == 0;
    }

    static constexpr size_t capacity()
    {
        return Capacity;
    }
};

} // namespace mla

#endif
