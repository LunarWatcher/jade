#include "SessionMiddleware.hpp"

namespace jade {


void InMemoryStorage::create(const std::string& key, std::shared_ptr<SessionData> init) {
    std::unique_lock l(m);
    data[key] = init;
}

void InMemoryStorage::destroy(const std::string& key) {
    std::unique_lock l(m);
    auto it = data.find(key);
    if (it != data.end()) {
        data.erase(key);
    }
}

void InMemoryStorage::refresh(const std::string& oldKey, const std::string& newKey) {
    // TODO: implement
}

std::shared_ptr<SessionData> InMemoryStorage::getSessionData(const std::string& key) {
    std::shared_lock l(m);
    auto entry = data.find(key);
    if (entry == data.end()) {
        return nullptr;
    } else if (entry->second->validUntil < Clock::now()) {
        // The session is no longer valid
        // Unlock, relock, erase the key without using the iterator - this avoids race conditions if `data` is modified
        // such that the iterator becomes invalid
        l.unlock();
        std::unique_lock l(m);

        data.erase(key);
        return nullptr;
    }
    return entry->second;
}

}
