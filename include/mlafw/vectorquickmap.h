#ifndef __MLA_VECTORQUICKMAP__
#define __MLA_VECTORQUICKMAP__

#include <optional>
#include <stdexcept>
#include <vector>

namespace mla
{

// Vector-based implementation with dynamic resizing
template <typename K, typename V>
class VectorQuickMap
{
private:
    struct Entry
    {
        K key;
        V value;
        bool occupied = false;
    };
    std::vector<Entry> data;
    size_t _size = 0;
    float max_load_factor = 0.75f;

    size_t hash(const K& key) const
    {
        return std::hash<K>{}(key) % data.size();
    }

    void rehash(size_t new_capacity)
    {
        std::vector<Entry> old_data = std::move(data);
        data.resize(new_capacity);
        for(auto& entry : data)
        {
            entry.occupied = false;
        }

        _size = 0;
        for(const auto& entry : old_data)
        {
            if(entry.occupied)
            {
                insert(entry.key, entry.value);
            }
        }
    }

public:
    class Iterator
    {
    private:
        VectorQuickMap* map;
        size_t index;
        void find_next_occupied()
        {
            while(index < map->data.size() && !map->data[index].occupied)
            {
                ++index;
            }
        }

    public:
        Iterator(VectorQuickMap* m, size_t i) : map(m), index(i)
        {
            find_next_occupied();
        }
        Iterator& operator++()
        {
            if(index < map->data.size())
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
        bool operator==(const Iterator& other) const
        {
            return index == other.index && map == other.map;
        }
        std::pair<const K&, V&> operator*() const
        {
            return {map->data[index].key, map->data[index].value};
        }
    };

    // Default constructor with a reasonable initial capacity
    VectorQuickMap(size_t initial_capacity = 16)
    {
        data.resize(initial_capacity);
    }

    void insert(const K& key, const V& value)
    {
        if(data.empty())
        {
            data.resize(16);
        }

        if(static_cast<float>(_size + 1) / data.size() > max_load_factor)
        {
            rehash(data.size() * 2);
        }

        size_t index = hash(key);
        size_t start_index = index;
        do
        {
            if(!data[index].occupied || data[index].key == key)
            {
                if(!data[index].occupied)
                {
                    _size++;
                }
                data[index] = {key, value, true};
                return;
            }
            index = (index + 1) % data.size();
        } while(index != start_index);

        // If we get here, the map is full despite rehashing
        // This shouldn't happen with proper rehashing
        throw std::runtime_error("Map is full");
    }

    std::optional<V> get(const K& key) const
    {
        if(data.empty())
        {
            return std::nullopt;
        }

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
            index = (index + 1) % data.size();
        } while(index != start_index);
        return std::nullopt;
    }

    void remove(const K& key)
    {
        if(data.empty())
        {
            return;
        }

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
            index = (index + 1) % data.size();
        } while(index != start_index);
    }

    Iterator find(const K& key)
    {
        if(data.empty())
        {
            return end();
        }

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
            index = (index + 1) % data.size();
        } while(index != start_index);
        return end();
    }

    Iterator begin()
    {
        return Iterator(this, 0);
    }

    Iterator end()
    {
        return Iterator(this, data.size());
    }

    size_t size() const
    {
        return _size;
    }

    bool empty() const
    {
        return _size == 0;
    }

    size_t capacity() const
    {
        return data.size();
    }
};

} // namespace mla

#endif
