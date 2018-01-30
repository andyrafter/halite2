#ifndef RAF_STDLIB_UTIL_H_
#define RAF_STDLIB_UTIL_H_

#include <algorithm>
#include <functional>
#include <map>
#include <utility>
#include <vector>

namespace raf {
//std extensions
// namespace stx {

//
// std::vector utils
//
template<typename T, typename Cond>
std::vector<T> select_T(const std::vector<T>& src, Cond predicate)
{
  std::vector<T> result;
  std::copy_if(src.cbegin(), src.cend(), std::back_inserter(result), predicate);
  return result;
}

//
// std::map utils
//

template<typename Key, typename Val>
const Val& value(const std::pair<Key, Val>& keyValue)
{
  return keyValue.second;
}

template<typename Key, typename Val>
std::vector<Val> map_to_vector(const std::map<Key, Val>& container)
{
  std::vector<Val> result;
  std::transform(std::begin(container), std::end(container), std::back_inserter(result), value<Key, Val>);

  return result;
}

template <typename T, typename Cond>
void filter(T& container, Cond cond) {
  container.erase(
    std::remove_if(
      std::begin(container),
      std::end(container),
      cond),
    std::end(container)
  );
}


// Copy all items that meet predicated from a map into a vector
template<typename Key, typename Val>
std::vector<Val> map_select_T(
  const std::map<Key, Val>& src,
  std::function<bool(const Val&)> predicate)
{
  std::vector<Val> result;

  // FIXME. do transform+filter in single pass using std lib functions?

  // transform map into vector
  std::transform(std::begin(src), std::end(src), std::back_inserter(result), value<Key, Val>);
  // remove items that don't meet requirements
  filter(result, predicate);
  return result;
}


// https://stackoverflow.com/questions/8137869/how-to-copy-if-from-map-to-vector
// https://stackoverflow.com/questions/2999537/stl-writing-where-operator-for-a-vector/3000008#3000008

template<class InputIterator, class OutputIterator, class Predicate>
OutputIterator transform_if(
  InputIterator first,
  InputIterator last,
  OutputIterator result,
  Predicate pred) {
  for (; first != last; ++first) {
    if (pred(first->second)) {
      *result++ = first->second;
    }
  }
  return result;
}


template< typename ContainerT, typename PredicateT >
void map_erase_if(ContainerT& items, const PredicateT& predicate) {
  for (auto it = std::begin(items); it != std::end(items); ) {
    if (predicate(it->second)) {
      it = items.erase(it);
    } else {
      ++it;
    }
  }
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
  for (const auto &e : vec) {
    os << e;
  }
  return os;
}

} // namespace raf

#endif // !RAF_STDLIB_UTIL_H_
