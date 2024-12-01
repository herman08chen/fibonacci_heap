#include <algorithm>
#include <bit>
#include <functional>
#include <list>
#include <memory>

template<class T, class Compare = std::less<T>>
class fibonacci_heap {
    struct node {
        T value;
        typename std::list<node>::iterator parent;
        std::list<node> children;
        bool marked;

        explicit node(auto&&... args) : value(std::forward<decltype(args)>(args)...), parent{null_it}, children{}, marked{false} {}
        node(const node& other) =default;
        node(node&& other) =default;
        node& operator=(const node& other) =default;
        node& operator=(node&& other) =default;

        std::size_t degree() { return children.size(); }
    };
    typedef typename std::list<node>::iterator list_it;
    const static list_it null_it;
    Compare comp{};

    struct iterator {
        list_it it;
        iterator& operator++() {
            if(it->degree()) {
                it = it->children.begin();
            }
            else {
                while(it->parent != null_it && it->parent->children.end() == std::next(it)) {
                    it = it->parent;
                }
                ++it;
            }
            return *this;
        }
        iterator operator++(int) {
            auto old = *this;
            operator++();
            return old;
        }
        T& operator*() {
            return it->value;
        }
        bool operator==(const iterator& other) const = default;
    };
    typedef iterator const_iterator;

    void update_min(list_it other) noexcept(noexcept(comp(other->value, _min->value))) {
        if(_size == 1) [[unlikely]] _min = other;
        else {
            if(comp(other->value, _min->value)) {
                _min = other;
            }
            roots.splice(roots.begin(), roots, _min);
        }
    }
    list_it recalc_min() {
        return std::ranges::min_element(roots, comp, [&](auto&& i) -> const T& {
            return i.value;
        });
    }

    std::list<node> roots;
    list_it _min;
    std::size_t _size;

    public:
    fibonacci_heap() : roots{}, _min{}, _size{} {}
    fibonacci_heap(const fibonacci_heap& other) = default;
    fibonacci_heap(fibonacci_heap&& other) = default;
    template<class InputIt>
    fibonacci_heap(InputIt begin, InputIt end) : roots{}, _min{}, _size{} {
        while(begin != end) {
            insert(*begin);
            ++begin;
        }
        _min = recalc_min();
    }
    template<class R>
    explicit fibonacci_heap(const R& rg) : fibonacci_heap(std::ranges::begin(rg), std::ranges::end(rg)) {}

    fibonacci_heap& operator=(const fibonacci_heap& other) = default;
    fibonacci_heap& operator=(fibonacci_heap&& other) = default;

    ~fibonacci_heap() = default;

    auto begin() {
        return const_iterator{_min};
    }
    auto end() {
        roots.splice(roots.begin(), roots, _min);
        return const_iterator{roots.end()};
    }

    const T& find_min() {
        if(_size == 0) [[unlikely]] throw std::exception{};
        return _min->value;
    }
    auto emplace(auto&&... args) {
        auto ref = roots.emplace(roots.begin(), std::forward<decltype(args)>(args)...);
        _size++;
        update_min(roots.begin());
        return const_iterator{ref};
    }
    auto insert(auto&& i) {
        return emplace(std::forward<decltype(i)>(i));
    }
    void merge(fibonacci_heap& other) {
        _size += other._size;
        other._size = 0;
        roots.splice(roots.begin(), other.roots);
        update_min(other._min);
        other._min = null_it;
    }
    void delete_min() {
        if(_size == 0) [[unlikely]] throw std::exception{};
        for(auto&& i: _min->children) { i.parent = null_it; }
        roots.splice(roots.begin(), _min->children);
        roots.erase(_min);
        _size--;

        if(_size == 0) [[unlikely]]{
            return;
        }

        std::vector<list_it> table(std::bit_ceil(size()) + 1, null_it);

        std::function<void(list_it)> process =
        [&](list_it it) {
            if(table[it->degree()] == null_it) {
                table[it->degree()] = it;
            }
            else {
                auto&& other = table[it->degree()];
                list_it parent = other, child = it;
                if(comp(it->value, other->value)) {
                    std::swap(parent, child);
                }
                //std::cout << "moving " << child->value << " to " << parent->value << std::endl;
                child->parent = parent;
                parent->children.splice(parent->children.begin(), roots, child);
                other = null_it;
                process(parent);
            }
        };

        for(auto it = roots.begin(); it != roots.end();) {
            auto next = std::next(it);
            process(it);
            it = next;
        }

        _min = recalc_min();
    }
    void decrease_key(const_iterator key, auto&& value) {
        list_it it = key.it;
        it->value = std::forward<decltype(value)>(value);
        if(key.it->parent == null_it) {
            update_min(it);
            return;
        }
        iterator parent = iterator{it->parent};
        if(parent.it != null_it && comp(*key, *parent)) {
            cut(key);
            cascading_cut(parent);
        }
        update_min(it);
    }
    void cut(const_iterator key) {
        roots.splice(roots.end(), key.it->parent->children, key.it);
    }
    void cascading_cut(const_iterator key) {
        auto parent = iterator{key.it->parent};
        if(parent.it != null_it) {
            if(key.it->marked == false) {
                key.it->marked = true;
            }
            else {
                cut(key);
                cascading_cut(parent);
            }
        }
    }

    [[nodiscard]] const T& top() { return _min->value; }
    [[nodiscard]] bool empty() const { return _size == 0; }
    [[nodiscard]] std::size_t size() const { return _size; }
    void push(const T& value) { emplace(value); }
    void push(T&& value) { emplace(std::move(value)); }
    void pop() { delete_min(); }
    void swap(fibonacci_heap& other) noexcept(noexcept(swap(roots, other.roots)) && noexcept(swap(comp, other.comp))) {
        std::swap(_min, other._min);
        std::swap(_size, other._size);
        std::swap(roots, other.roots);
        std::swap(comp, other.comp);
   }
    void printRoots() {
        std::cout << "roots: [";
        for(auto&& i: roots) {
            std::cout << i.value << ' ';
        }
        std::cout << "]" <<std::endl;
    }
};

template<class T, class Compare>
const typename fibonacci_heap<T, Compare>::list_it fibonacci_heap<T, Compare>::null_it = typename fibonacci_heap<T>::list_it{};