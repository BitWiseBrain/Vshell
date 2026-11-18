#ifndef TINY_SHELL_HASHTABLE_H
#define TINY_SHELL_HASHTABLE_H

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

class HashTable {
public:
    explicit HashTable(std::size_t bucketCount = 8);
    ~HashTable();

    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    void insert(const std::string& key, const std::string& value);
    const std::string* search(const std::string& key) const;
    bool erase(const std::string& key);
    std::vector<std::pair<std::string, std::string>> entries() const;
    std::size_t size() const { return size_; }
    std::size_t bucketCount() const { return buckets_.size(); }

private:
    struct Entry {
        Entry(std::string entryKey, std::string entryValue)
            : key(std::move(entryKey)), value(std::move(entryValue)) {}
        std::string key;
        std::string value;
        Entry* next = nullptr;
    };

    std::size_t hash(const std::string& key) const;
    static std::size_t hashFor(const std::string& key, std::size_t bucketCount);
    void resize();

    std::vector<Entry*> buckets_;
    std::size_t size_ = 0;
};

#endif
