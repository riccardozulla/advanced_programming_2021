#ifndef STACK_POOL_H
#define STACK_POOL_H

#include <vector>

template <typename stack_pool, typename N, typename T>
class _iterator {
  stack_pool* pool;
  N current;

 public:
  using value_type = T;
  using reference = value_type&;
  using pointer = value_type*;
  using difference_type = std::ptrdiff_t;
  using iterator_category = std::forward_iterator_tag;

  explicit _iterator(stack_pool* p, N x) : pool{p}, current{x} {}

  reference operator*() const noexcept { return pool->value(current); }
  _iterator& operator++() noexcept {  // pre-inc
    current = pool->next(current);
    return *this;
  }
  _iterator operator++(int) {  // post-inc
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

  friend bool operator==(const _iterator& x, const _iterator& y) noexcept {
    return x.current == y.current;
  }
  friend bool operator!=(const _iterator& x, const _iterator& y) noexcept {
    return !(x == y);
  }
};

/**
 * @brief Pool of blazingly fast stacks
 *
 * This class implements a pool of multiple stacks.
 *
 * Whenever possible, it is advised to use the method stack_pool::reserve
 * to avoid multiple re-allocation and to improve performances.
 *
 * Be careful using stack indexes because no bounds checking is performed.
 * Accessing a nonexistent stack index leads to undefined behavior.
 *
 * @tparam T  The type of the elements.
 * @tparam N  The type of the index used to address the elements
 */
template <typename T, typename N = std::size_t>
class stack_pool {
  struct node_t {
    T value;
    N next;

    template <typename X>
    explicit node_t(X&& val, N n) : value{std::forward<X>(val)}, next{n} {}
  };

  using stack_type = N;
  using value_type = T;
  using size_type = typename std::vector<node_t>::size_type;

  std::vector<node_t> pool;
  stack_type free_nodes;  // at the beginning, it is empty

  node_t& node(stack_type x) noexcept { return pool[x - 1]; }
  const node_t& node(stack_type x) const noexcept { return pool[x - 1]; }

  template <typename X>
  stack_type _push(X&& val, stack_type head) {
    if (empty(free_nodes)) {
      pool.emplace_back(std::forward<X>(val), head);
      return static_cast<stack_type>(pool.size());
    }
    auto tmp = free_nodes;
    free_nodes = next(free_nodes);
    value(tmp) = std::forward<X>(val);
    next(tmp) = head;
    return tmp;
  }

 public:
  /**
   * @brief Construct a new stack pool object
   */
  stack_pool() : free_nodes{end()} {}

  /**
   * @brief Construct a new stack pool object and reserve n node in the pool
   *
   * @param n Number of nodes to reserve
   */
  explicit stack_pool(size_type n) : stack_pool() { reserve(n); }

  using iterator = _iterator<stack_pool, N, T>;
  using const_iterator = _iterator<stack_pool, N, const T>;

  /**
   * @brief Return an iterator to the beginning
   *
   * @param x Head of the stack
   * @return iterator
   */
  iterator begin(stack_type x) { return iterator(this, x); }
  /**
   * @brief Return an iterator to the end
   *
   * @return iterator
   */
  iterator end(stack_type) { return iterator(this, end()); }
  /**
   * @brief Return a const iterator to the beginning
   *
   * @param x Head of the stack
   * @return const_iterator
   */
  const_iterator begin(stack_type x) const { return const_iterator(this, x); }
  /**
   * @brief Return a const iterator to the end
   *
   * @return const_iterator
   */
  const_iterator end(stack_type) const { return const_iterator(this, end()); }
  /**
   * @brief Return a const iterator to the beginning
   *
   * @param x Head of the stack
   * @return const_iterator
   */
  const_iterator cbegin(stack_type x) const { return const_iterator(this, x); }
  /**
   * @brief Return a const iterator to the end
   *
   * @return const_iterator
   */
  const_iterator cend(stack_type) const { return const_iterator(this, end()); }

  /**
   * @brief Return an empty stack
   *
   * @return stack_type
   */
  stack_type new_stack() noexcept { return end(); }

  /**
   * @brief Reserve storage
   * Reserve n nodes in the pool
   *
   * @param n Number of nodes to reserve
   */
  void reserve(size_type n) { pool.reserve(n); }

  /**
   * @brief Capacity of the pool
   *
   * Return the number of elements that can be held in currently allocated
   * storage
   *
   * @return size_type
   */
  size_type capacity() const noexcept { return pool.capacity(); }

  /**
   * @brief Check whether the stack is empty
   *
   * @param x Head of the stack
   * @return true
   * @return false
   */
  bool empty(stack_type x) const noexcept { return x == end(); }

  /**
   * @brief Return the end index
   *
   * @return stack_type
   */
  stack_type end() const noexcept { return stack_type(0); }

  /**
   * @brief Return the value of the node
   *
   * @param x Head of the stack
   * @return T&
   */
  T& value(stack_type x) noexcept { return node(x).value; }
  /**
   * @brief Return the value of the node
   *
   * @param x Head of the stack
   * @return const T&
   */
  const T& value(stack_type x) const noexcept { return node(x).value; }

  /**
   * @brief Return the index of the next node
   *
   * @param x Head of the stack
   * @return stack_type&
   */
  stack_type& next(stack_type x) noexcept { return node(x).next; }

  /**
   * @brief Return the index of the next node
   *
   * @param x Head of the stack
   * @return const stack_type&
   */
  const stack_type& next(stack_type x) const noexcept { return node(x).next; }

  /**
   * @brief Add an element to the front of the stack
   *
   * @param val Value (l-value) of the new element
   * @param head Head of the stack
   * @return stack_type
   */
  stack_type push(const T& val, stack_type head) { return _push(val, head); }

  /**
   * @brief Add an element to the front of the stack
   *
   * @param val Value (r-value) of the new element
   * @param head Head of the stack
   * @return stack_type
   */
  stack_type push(T&& val, stack_type head) {
    return _push(std::move(val), head);
  }

  /**
   * @brief Remove the head of the stack
   *
   * @param x Head of the stack
   * @return stack_type
   */
  stack_type pop(stack_type x) noexcept {
    if (empty(x)) {
      return end();
    }
    auto tmp = next(x);
    next(x) = free_nodes;
    free_nodes = x;
    return tmp;
  }

  /**
   * @brief Free entire stack
   *
   * @param x Head of the stack
   * @return stack_type
   */
  stack_type free_stack(stack_type x) noexcept {
    while (!empty(x)) {
      x = pop(x);
    }
    return x;
  }
};

#endif