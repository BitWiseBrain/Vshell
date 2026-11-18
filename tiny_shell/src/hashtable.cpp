#include "hashtable.h"

#include <algorithm>

HashTable::HashTable(std::size_t bucketCount)
    : buckets_(std::max<std::size_t>(bucketCount, 1), nullptr) {}

HashTable::~HashTable() {
    for (Entry* head : buckets_) {
        while (head != nullptr) {
            Entry* next = head->next;
            delete head;
            head = next;
        }
    }
}

std::size_t HashTable::hash(const std::string& key) const {
    return hashFor(key, buckets_.size());
}

std::size_t HashTable::hashFor(const std::string& key, std::size_t bucketCount) {
    std::size_t value = 14695981039346656037ull;
    for (unsigned char character : key) {
        value ^= character;
        value *= 1099511628211ull;
    }
    return value % bucketCount;
}

const std::string* HashTable::search(const std::string& key) const {
    for (Entry* entry = buckets_[hash(key)]; entry != nullptr; entry = entry->next) {
        if (entry->key == key) {
            return &entry->value;
        }
    }
    return nullptr;
}

void HashTable::insert(const std::string& key, const std::string& value) {
    if (key.empty() || value.empty()) {
        return;
    }
    const std::size_t index = hash(key);
    for (Entry* entry = buckets_[index]; entry != nullptr; entry = entry->next) {
        if (entry->key == key) {
            entry->value = value;
            return;
        }
    }
    Entry* entry = new Entry(key, value);
    entry->next = buckets_[index];
    buckets_[index] = entry;
    ++size_;

    if (size_ * 4 > buckets_.size() * 3) {
        resize();
    }
}

bool HashTable::erase(const std::string& key) {
    Entry** link = &buckets_[hash(key)];
    while (*link != nullptr) {
        if ((*link)->key == key) {
            Entry* removed = *link;
            *link = removed->next;
            delete removed;
            --size_;
            return true;
        }
        link = &(*link)->next;
    }
    return false;
}

std::vector<std::pair<std::string, std::string>> HashTable::entries() const {
    std::vector<std::pair<std::string, std::string>> result;
    result.reserve(size_);
    for (Entry* head : buckets_) {
        for (Entry* entry = head; entry != nullptr; entry = entry->next) {
            result.emplace_back(entry->key, entry->value);
        }
    }
    std::sort(result.begin(), result.end());
    return result;
}

void HashTable::resize() {
    std::vector<Entry*> larger(buckets_.size() * 2, nullptr);
    for (Entry*& head : buckets_) {
        while (head != nullptr) {
            Entry* entry = head;
            head = head->next;

            const std::size_t index = hashFor(entry->key, larger.size());

            entry->next = larger[index];
            larger[index] = entry;
        }
    }
    buckets_.swap(larger);
}
