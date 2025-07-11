#include <list>
#include <unordered_map>
#include <utility>
#include <algorithm>

template <typename Key, typename Value> class LRUCache {
    public:
	LRUCache(size_t capacity, Value absent)
	        : _capacity(capacity)
	        , _absent(absent)
	{
	}

	Value get(Key key)
	{
		auto it = _hash_list.find(key);
		if (it == _hash_list.end()) {
			return _absent;
		}

		_lru_list.splice(_lru_list.begin(), _lru_list, it->second);
		_hash_list[key] = _lru_list.begin();
		return _hash_list[key]->second;
	}

	std::pair<Key, Value> put(Key key, Value value)
	{
		auto it = _hash_list.find(key);
		// key found
		if (it != _hash_list.end()) {
			auto &old_value = it->second->second;
			it->second->second = value;
			_lru_list.splice(_lru_list.begin(), _lru_list, it->second);
			_hash_list[key] = _lru_list.begin();
			return std::make_pair(key, old_value);
		}
		// key missing
		std::pair<Key, Value> purged = std::make_pair(key, _absent);
		if (_lru_list.size() == _capacity) {
			purged = _lru_list.back();
			_hash_list.erase(purged.first);
			_lru_list.pop_back();
		}
		_lru_list.push_front(std::make_pair(key, value));
		_hash_list[key] = _lru_list.begin();
		return purged;
	}

	size_t size()
	{
		return _lru_list.size();
	}

	template <typename F> void clear(F cleaner)
	{
		visit_cache(cleaner);
		clear();
	}

	void clear()
	{
		_hash_list.clear();
		_lru_list.clear();
	}

	template <typename Visitor> void visit_cache(Visitor visitor)
	{
		std::for_each(_lru_list.begin(), _lru_list.end(),
		              [&visitor](const std::pair<Key, Value> &item) { visitor(item.first, item.second); });
	}

    private:
	size_t _capacity;
	Value _absent;
	std::list<std::pair<Key, Value> > _lru_list{};
	std::unordered_map<Key, typename std::list<std::pair<Key, Value> >::iterator> _hash_list{};
};
