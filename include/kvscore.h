#ifndef KVSCORE_H
#define KVSCORE_H

#include <variant>
#include <vector>
#include <cstdint>
#include <string>
#include <memory>
#include <type_traits>
#include <stdexcept>
#include <utility>
#include <initializer_list>
#include <map>
#include <iostream>
#include <stack>
#include <unordered_set>
#include <functional>

#include "kvsutils.h"

struct KVS
{

  /**
   * The Type enum defines the possible types of values that can be stored in the KVS. It includes:
   * - Undefined: Represents an uninitialized or undefined value.
   * - Array: Represents an array of elements.
   * - Map: Represents a key-value map.
   * - Int32: Represents a 32-bit integer value.
   * - Int64: Represents a 64-bit integer value.
   * - Float: Represents a single-precision floating-point value.
   * - Double: Represents a double-precision floating-point value.
   * - String: Represents a string value.
   */
  enum class Type
  {
    None,
    Array,
    Map,
    Ordered,
    Int32,
    Int64,
    Float,
    Double,
    Int32Range,
    Int64Range,
    FloatRange,
    DoubleRange,
    String
  };

  using None = std::monostate;

  struct Array;
  struct Map;
  struct Ordered;

  /**
   * Class representing a ranged value - a value that contains a value and a range that it must be within
   * @tparam T The type of the value and range (e.g. int, float, etc.)
   */
  template <typename T>
  struct ranged
  {
    T begin;
    T value;
    T end;

    /**
     * Set the value of the ranged object, ensuring that it is within the specified range. If the value is out of range, an std::out_of_range exception is thrown.
     */
    void set(T newValue)
    {
      if (newValue < begin || newValue > end)
        throw std::out_of_range("Value out of range");
      value = newValue;
    }

    /**
     * Compare two ranged objects for equality. They are equal if only their value match, regardless of their range.
     */
    bool operator==(const ranged<T> &other) const
    {
      return value == other.value;
    }

    ranged() = default;

    /**
     * Constructor for the ranged object. It takes a value and a range (begin and end) and initializes the object. If the value is out of the specified range, an std::out_of_range exception is thrown.
     * @param v The value to be stored in the ranged object.
     * @param b The beginning of the range (inclusive).
     * @param e The end of the range (inclusive).
     * @throws std::out_of_range If the value is outside the specified range.
     */
    ranged(T v, T b, T e) : begin(b), value(v), end(e)
    {
      if (v < b || v > e)
        throw std::out_of_range("Value out of range");
    }
  };

  // Type aliases for specific ranged types
  using int32Range = ranged<int32_t>;
  using int64Range = ranged<int64_t>;
  using floatRange = ranged<float>;
  using doubleRange = ranged<double>;

  /**
   * Basic type erased element class that can hold any of the supported types.
   */
  struct Element
  {

    std::variant<std::monostate, int32_t, int64_t, float, double, ranged<int32_t>, ranged<int64_t>, ranged<float>, ranged<double>, std::string, std::shared_ptr<Array>, std::shared_ptr<Map>, std::shared_ptr<Ordered>> value;
    std::map<std::string, std::string> metadata; // Optional metadata associated with the element
    bool directive = false; // Flag to indicate if the element is a directive
    Type type = Type::None;

    Element() = default;
    Element(int32_t v) : value(v), type(Type::Int32) {}
    Element(int64_t v) : value(v), type(Type::Int64) {}
    Element(float v) : value(v), type(Type::Float) {}
    Element(double v) : value(v), type(Type::Double) {}
    Element(ranged<int32_t> v) : value(v), type(Type::Int32Range) {}
    Element(ranged<int64_t> v) : value(v), type(Type::Int64Range) {}
    Element(ranged<float> v) : value(v), type(Type::FloatRange) {}
    Element(ranged<double> v) : value(v), type(Type::DoubleRange) {}
    Element(const std::string &v) : value(v), type(Type::String) {}
    Element(std::string &&v) : value(std::move(v)), type(Type::String) {}
    Element(const char *s) : value(std::string(s)), type(Type::String) {}
    Element(const Array &a) : value(std::make_shared<Array>(a)), type(Type::Array) {}
    Element(std::shared_ptr<Array> a) : value(std::move(a)), type(Type::Array) {}
    Element(const Map &m) : value(std::make_shared<Map>(m)), type(Type::Map) {}
    Element(std::shared_ptr<Map> m) : value(std::move(m)), type(Type::Map) {}
    Element(const Ordered &n) : value(std::make_shared<Ordered>(n)), type(Type::Ordered) {}
    Element(std::shared_ptr<Ordered> n) : value(std::move(n)), type(Type::Ordered) {}

    /**
     * Set the optional comment for the element
     */
    void setComment(const std::string &c)
    {
      setMetadata("comment", c);
    }

    /**
     * Get the optional comment for the element. If no comment is set, an empty string is returned.
     */
    std::string getComment() const
    {
      auto it = metadata.find("comment");
      if (it != metadata.end())
        return it->second;
      return "";
    }

    /**
     * Check if this element contains a given key. This is only applicable if the element is a Map or an Ordered collection. If the element is not a Map or Ordered collection, an std::runtime_error is thrown.
     * @param key The key to check for in the element.
     * @return true if the key exists in the element, false otherwise.
     * @throws std::runtime_error If the element is not a Map or Ordered collection.
     */
		bool hasElement(const std::string &key) const {
      if (type == Type::Map){
        auto &map = get<Map>();
        for (const auto &pair : map.data){
          if (pair.first == key)
            return true;
        }
        return false;
      }
      else if (type == Type::Ordered){
        auto &ordered = get<Ordered>();
        for (const auto &pair : ordered.data){
          if (pair.first == key)
            return true;
        }
        return false;
      }
      else
        throw std::runtime_error("Element is not a map or ordered collection");
		}

    /**
     * Get the size of the element. This is only applicable if the element is an Array, Map, or Ordered collection. If the element is not one of these types, an std::runtime_error is thrown.
     * @return The size of the element.
     * @throws std::runtime_error If the element is not an array, map, or ordered collection.
     */
    size_t size() const {
      if (type == Type::Array)
        return get<Array>().size();
      else if (type == Type::Map)
        return get<Map>().data.size();
      else if (type == Type::Ordered)
        return get<Ordered>().data.size();
      else
        throw std::runtime_error("Element is not an array, map, or ordered collection");
    }

    template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    Element &operator[](T index);
    Element &operator[](const std::string key);
		Element getElement(const std::string key);
		template<typename T>
		Element getElement(const std::string key, const T&value);
    Element &operator[](const char *key);
    Element &operator[](const Element &key);

    Element &upsert(const std::string &key);
    Element &upsert(const char *key);

    bool operator==(const Element &other) const;
    template <typename T>
    T &get();

    template<typename T>
    const T &get() const;

    void setMetadata(const std::string &key, const std::string &value)
    {
      metadata[key] = value;
    }

    bool hasMetadata(const std::string &key) const
    {
      return metadata.find(key) != metadata.end();
    }

    std::string getMetadata(const std::string &key) const
    {
      auto it = metadata.find(key);
      if (it != metadata.end())
        return it->second;
      throw std::out_of_range("Metadata key not found: " + key);
    }

    /**
     * Convert a Map element to an Ordered collection. This is useful when you want to maintain the order of key-value pairs while still allowing access by key. If the element is not a Map, an std::runtime_error is thrown.
     * @throws std::runtime_error If the element is not a Map and cannot be converted to an ordered collection.
     */
    void toOrdered()
    {
      if (type == Type::Map)
      {
        auto newOrdered = std::make_shared<Ordered>();
        auto map = std::get<std::shared_ptr<Map>>(value);
        for (const auto &pair : map->data)
        {
          newOrdered->data.emplace_back(pair.first, pair.second);
        }
        value = newOrdered;
        type = Type::Ordered;
      }
      else if (type == Type::Ordered)
      {
        // Already an ordered collection, do nothing
      }
      else
      {
        throw std::runtime_error("Element is not a map and cannot be converted to an ordered collection");
      }
    }

   void toMap()
    {
      if (type == Type::Ordered)
      {
        auto newMap = std::make_shared<Map>();
        auto ordered = std::get<std::shared_ptr<Ordered>>(value);
        for (const auto &pair : ordered->data)
        {
          newMap->data.emplace_back(pair.first, pair.second);
        }
        value = newMap;
        type = Type::Map;
      }
      else if (type == Type::Map)
      {
        // Already a map, do nothing
      }
      else
      {
        throw std::runtime_error("Element is not an ordered collection and cannot be converted to a map");
      }
    }


      int64_t asInt(){
        if (type == Type::Int32)
          return std::get<int32_t>(value);
        else if (type == Type::Int64)
          return std::get<int64_t>(value);
        else
          throw std::runtime_error("Element is not an integer type");
      }

      double asReal(){
        if (type == Type::Float)
          return std::get<float>(value);
        else if (type == Type::Double)
          return std::get<double>(value);
        else if (type == Type::Int32)
          return static_cast<double>(std::get<int32_t>(value));
        else if (type == Type::Int64)
          return static_cast<double>(std::get<int64_t>(value));
        else
          throw std::runtime_error("Element is not a real number type");
      }

      std::string asString(){
        if (type == Type::String)
          return std::get<std::string>(value);
        if (type == Type::Int32)
          return std::to_string(std::get<int32_t>(value));
        else if (type == Type::Int64)
          return std::to_string(std::get<int64_t>(value));
        else if (type == Type::Float)
          return KVSConversion::floatToString(std::get<float>(value));
        else if (type == Type::Double)
          return KVSConversion::doubleToString(std::get<double>(value));
        else
          throw std::runtime_error("Element cannot be converted to string");
      }

    void traverse(std::function<void(const std::string&, const Element&)> blockStart, std::function<void(const std::string&)> blockEnd, std::function<void(const std::string&, const Element&)> elementCallback) const
    {
      if (type == Type::Map)
      {
        auto map = std::get<std::shared_ptr<Map>>(value);
        for (const auto &pair : map->data)
        {
          if (pair.second.type != Type::Map && pair.second.type != Type::Ordered)
            elementCallback(pair.first, pair.second);
          else
          {
            blockStart(pair.first, pair.second); 
            pair.second.traverse(blockStart, blockEnd, elementCallback);
            blockEnd(pair.first);
          }
        }
      }
      else if (type == Type::Ordered)
      {
        auto ordered = std::get<std::shared_ptr<Ordered>>(value);
        for (const auto &pair : ordered->data)
        {
          if (pair.second.type != Type::Map && pair.second.type != Type::Ordered)
            elementCallback(pair.first, pair.second);
          else
          {
            blockStart(pair.first, pair.second);
            pair.second.traverse(blockStart, blockEnd, elementCallback);
            blockEnd(pair.first);
          }
        }
      }
      else
      {
        elementCallback("", *this);
      }
    }

  };

  /**
   * Custom hash function for Element
   */
  struct ElementHash
  {
    size_t operator()(Element const &e) const noexcept
    {
      using K = KVS;
      size_t seed = std::hash<int>()(static_cast<int>(e.type));
      // Simple hash function based on a TEA inspired mix function
      auto mix = [](size_t a, size_t b)
      {
        return a ^ (b + 0x9e3779b97f4a7c15ULL + (a << 6) + (a >> 2));
      };

      switch (e.type)
      {
      case K::Type::Int32:
        return mix(seed, std::hash<int32_t>{}(std::get<int32_t>(e.value)));
      case K::Type::Int64:
        return mix(seed, std::hash<int64_t>{}(std::get<int64_t>(e.value)));
      case K::Type::Float:
        return mix(seed, std::hash<float>{}(std::get<float>(e.value)));
      case K::Type::Double:
        return mix(seed, std::hash<double>{}(std::get<double>(e.value)));
      case K::Type::Int32Range:
        return mix(seed, std::hash<int32_t>{}(std::get<ranged<int32_t>>(e.value).value));
      case K::Type::Int64Range:
        return mix(seed, std::hash<int64_t>{}(std::get<ranged<int64_t>>(e.value).value));
      case K::Type::FloatRange:
        return mix(seed, std::hash<float>{}(std::get<ranged<float>>(e.value).value));
      case K::Type::DoubleRange:
        return mix(seed, std::hash<double>{}(std::get<ranged<double>>(e.value).value));
      case K::Type::String:
        return mix(seed, std::hash<std::string>{}(std::get<std::string>(e.value)));
      case K::Type::Array:
        return mix(seed, std::hash<const void *>{}(std::get<std::shared_ptr<K::Array>>(e.value).get()));
      case K::Type::Map:
        return mix(seed, std::hash<const void *>{}(std::get<std::shared_ptr<K::Map>>(e.value).get()));
      case K::Type::Ordered:
        return mix(seed, std::hash<const void *>{}(std::get<std::shared_ptr<K::Ordered>>(e.value).get()));
      default:
        return seed;
      }
    }
  };

  // Comparison functor for std::map (must provide a strict weak ordering)
  struct ElementLess
  {
    bool operator()(Element const &a, Element const &b) const noexcept
    {
      if (a.type != b.type)
        return static_cast<int>(a.type) < static_cast<int>(b.type);

      switch (a.type)
      {
      case Type::Int32:
        return std::get<int32_t>(a.value) < std::get<int32_t>(b.value);
      case Type::Int64:
        return std::get<int64_t>(a.value) < std::get<int64_t>(b.value);
      case Type::Float:
        return std::get<float>(a.value) < std::get<float>(b.value);
      case Type::Double:
        return std::get<double>(a.value) < std::get<double>(b.value);
      case Type::Int32Range:
        return std::get<int32Range>(a.value).value < std::get<int32Range>(b.value).value;
      case Type::Int64Range:
        return std::get<int64Range>(a.value).value < std::get<int64Range>(b.value).value;
      case Type::FloatRange:
        return std::get<floatRange>(a.value).value < std::get<floatRange>(b.value).value;
      case Type::DoubleRange:
        return std::get<doubleRange>(a.value).value < std::get<doubleRange>(b.value).value;
      case Type::String:
        return std::get<std::string>(a.value) < std::get<std::string>(b.value);
      case Type::Array:
        return std::get<std::shared_ptr<Array>>(a.value).get() < std::get<std::shared_ptr<Array>>(b.value).get();
      case Type::Map:
        return std::get<std::shared_ptr<Map>>(a.value).get() < std::get<std::shared_ptr<Map>>(b.value).get();
      case Type::Ordered:
        return std::get<std::shared_ptr<Ordered>>(a.value).get() < std::get<std::shared_ptr<Ordered>>(b.value).get();
      default:
        return false;
      }
    }
  };


  // Output operator for Element
  friend std::ostream &operator<<(std::ostream &os, const Element &e)
  {
    switch (e.type)
    {
    case Type::Int32:
      os << std::get<int32_t>(e.value);
      break;
    case Type::Int64:
      os << std::get<int64_t>(e.value);
      break;
    case Type::Float:
      os << std::get<float>(e.value);
      break;
    case Type::Double:
      os << std::get<double>(e.value);
      break;

    case Type::Int32Range:
      os << std::get<ranged<int32_t>>(e.value).value << " (range: " << std::get<ranged<int32_t>>(e.value).begin << " - " << std::get<ranged<int32_t>>(e.value).end << ")";
      break;
    case Type::Int64Range:
      os << std::get<ranged<int64_t>>(e.value).value << " (range: " << std::get<ranged<int64_t>>(e.value).begin << " - " << std::get<ranged<int64_t>>(e.value).end << ")";
      break;
    case Type::FloatRange:
      os << std::get<ranged<float>>(e.value).value << " (range: " << std::get<ranged<float>>(e.value).begin << " - " << std::get<ranged<float>>(e.value).end << ")";
      break;
    case Type::DoubleRange:
      os << std::get<ranged<double>>(e.value).value << " (range: " << std::get<ranged<double>>(e.value).begin << " - " << std::get<ranged<double>>(e.value).end << ")";
      break;
    case Type::String:
      os << std::get<std::string>(e.value);
      break;
    case Type::Array:
      // Array has its own output operator, so we can just delegate to it
      os << *std::get<std::shared_ptr<Array>>(e.value);
      break;
    case Type::Map:
      // Map has its own output operator, so we can just delegate to it
      os << *std::get<std::shared_ptr<Map>>(e.value);
      break;
    case Type::Ordered:
      // Ordered has its own output operator, so we can just delegate to it
      os << *std::get<std::shared_ptr<Ordered>>(e.value);
      break;
    default:
      os << "None";
      break;
    }
    return os;
  }

  /**
   * Class representing an array of elements
   */
  struct Array
  {
    std::vector<Element> elements;
    Array() = default;
    Array(std::initializer_list<Element> il) : elements(il) {}
    /**
     * Access an element of the array by index. Throws std::out_of_range if the index is out of bounds.
     * @param index The index of the element to access.
     * @return A reference to the element at the specified index.
     * @throws std::out_of_range If the index is out of bounds.
     */
    Element &operator[](size_t index)
    {
      return elements[index];
    }
    /**
     * Add an element to the end of the array.
     * @param element The element to be added to the array.
     */
    void push_back(const Element &element)
    {
      elements.push_back(element);
    }
    /**
     * Set the size of the array
     * @param newSize The new size of the array
     */
    void resize(size_t newSize)
    {
      elements.resize(newSize);
    }
    /**
     * Reserve space for a certain number of elements in the array to optimize memory allocations when the number of elements is known in advance.
     * @param newCapacity The number of elements to reserve space for.
     */
    void reserve(size_t newCapacity)
    {
      elements.reserve(newCapacity);
    }
    /**
     * Get the number of elements in the array
     * @return The number of elements currently stored in the array.
     */
    size_t size() const
    {
      return elements.size();
    }

    auto begin() { return elements.begin(); }
    auto end() { return elements.end(); }
    auto begin() const { return elements.begin(); }
    auto end() const { return elements.end(); }

    void traverse(std::function<void(const Element&)> elementCallback){
      for (const auto &element : elements)
      {
        elementCallback(element);
      }
    }

  };

  // Output operator for array
  friend std::ostream &operator<<(std::ostream &os, const Array &a)
  {
    os << "[";
    for (size_t i = 0; i < a.elements.size(); ++i)
    {
      os << a.elements[i];
      if (i < a.elements.size() - 1)
        os << ", ";
    }
    os << "]";
    return os;
  }

  /**
   * Class representing a key value pair. The Key
   */
  struct Map
  {
    std::vector<std::pair<std::string, Element>> data;
    Map() = default;
    Map(std::initializer_list<std::pair<std::string, Element>> il) : data(il) {}

    Element &operator[](const std::string &key)
    {
      //Check for more than one key
      int count = 0;
      std::pair<std::string, Element>* lastMatch = nullptr;
      for (auto &pair : data)
      {
        if (pair.first == key){
          count++;
          lastMatch = &pair;
        }
        if (count > 1)
          break;
      }
      if (count == 1)
        return lastMatch->second;
      else if (count == 0){
        throw std::runtime_error("Key not found: " + key);
      }
      else
        throw std::runtime_error("Multiple elements with key: " + key);
    }


    Element &upsert(const std::string &key)
    {
      //Check for more than one key
      int count = 0;
      std::pair<std::string, Element>* lastMatch = nullptr;
      for (auto &pair : data)
      {
        if (pair.first == key){
          count++;
          lastMatch = &pair;
        }
        if (count > 1)
          break;
      }
      if (count == 1)
        return lastMatch->second;
      else if (count == 0){
        data.emplace_back(key, Element());
        return data.back().second;
      }
      else
        throw std::runtime_error("Multiple elements with key: " + key);
    }


    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }

    void traverse(std::function<void(const std::string&, const Element&)> elementCallback){
      for (const auto &pair : data)
      {
        elementCallback(pair.first, pair.second);
      }
    }
  };

  // Output operator for map
  friend std::ostream &operator<<(std::ostream &os, const Map &m)
  {
    os << "{";
    size_t count = 0;
    for (const auto &pair : m.data)
    {
      os << pair.first << ": " << pair.second;
      if (count < m.data.size() - 1)
        os << ", ";
      ++count;
    }
    os << "}\n";
    return os;
  }

  /**
   * Class representing an ordered collection of elements. It can be accessed by index
   * or by key, and maintains the order of insertion.
   */
  struct Ordered
  {
    std::vector<std::pair<std::string, Element>> data;
    Ordered() = default;
    Ordered(std::initializer_list<std::pair<std::string, Element>> il) : data(il) {}

    Element &operator[](const std::string &key)
    {
      //Check for more than one key
      int count = 0;
      std::pair<std::string, Element>* lastMatch = nullptr;
      for (auto &pair : data)
      {
        if (pair.first == key){
          count++;
          lastMatch = &pair;
        }
        if (count > 1)
          break;
      }
      if (count == 1)
        return lastMatch->second;
      else if (count == 0){
        throw std::runtime_error("Key not found: " + key);
      }
      else
        throw std::runtime_error("Multiple elements with key: " + key);
    }

    Element &upsert(const std::string &key)
    {
      //Check for more than one key
      int count = 0;
      std::pair<std::string, Element>* lastMatch = nullptr;
      for (auto &pair : data)
      {
        if (pair.first == key){
          count++;
          lastMatch = &pair;
        }
        if (count > 1)
          break;
      }
      if (count == 1)
        return lastMatch->second;
      else if (count == 0){
        data.emplace_back(key, Element());
        return data.back().second;
      }
      else
        throw std::runtime_error("Multiple elements with key: " + key);
    } 

    Element &operator[](size_t index)
    {
      return data[index].second;
    }

    std::string getKey(size_t index) const
    {
      return data[index].first;
    }

    std::vector<std::reference_wrapper<Element>> getAllByKey(const std::string &key)
    {
      std::vector<std::reference_wrapper<Element>> matches;
      for (auto &pair : data)
      {
        if (pair.first == key)
          matches.push_back(std::ref(pair.second));
      }
      return matches;
    }

    Element& push_back(const std::string &key, const Element &element)
    {
      data.emplace_back(key, element);
      return data.back().second;
    }

    bool hasDuplicateKeys() const
    {
      std::unordered_set<std::string> keys;
      for (const auto &pair : data)
      {
        if (keys.count(pair.first) > 0)
          return true;
        keys.insert(pair.first);
      }
      return false;
    }

    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }

    void traverse(std::function<void(const std::string&, const Element&)> elementCallback){
      for (const auto &pair : data)
      {
        elementCallback(pair.first, pair.second);
      }
    }
  };

  // Output operator for Ordered
  friend std::ostream &operator<<(std::ostream &os, const Ordered &n)
  {
    os << "{";
    for (size_t i = 0; i < n.data.size(); ++i)
    {
      os << n.data[i].first << ": " << n.data[i].second;
      if (i < n.data.size() - 1)
        os << ", ";
    }
    os << "}\n";
    return os;
  }

  Element root;

  KVS() = default;
  KVS(const Element &element) : root(element) {}

  template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
  Element &operator[](T index)
  {
    return root[index];
  }

  Element &operator[](std::string key)
  {
    return root[key];
  }

  Element &operator[](const char *key)
  {
    return root[std::string(key)];
  }

  Element &upsert(const std::string &key)
  {
    return root[key];
  }

  Element &upsert(const char *key)
  {
    return root[std::string(key)];
  }

  void traverse(std::function<void(const std::string&, const Element&)> blockStart, std::function<void(const std::string&)> blockEnd, std::function<void(const std::string&, const Element&)> elementCallback) const
  {
    blockStart("{root}",  root);
    root.traverse(blockStart, blockEnd, elementCallback);
    blockEnd("{root}");
  }

};

template <typename T, std::enable_if_t<std::is_integral_v<T>, int>>
KVS::Element &KVS::Element::operator[](T index)
{
  if (type == Type::Array)
    return get<Array>()[index];
  if (type == Type::Ordered)
    return get<Ordered>()[index];
  throw std::runtime_error("Root element is not an array or ordered collection");
}

KVS::Element &KVS::Element::operator[](const std::string key)
{
  if (type == Type::Map)
    return get<Map>()[key];
  if (type == Type::Ordered)
    return get<Ordered>()[key];
  throw std::runtime_error("Root element is not a map or ordered collection");
}

/**
 * Get an element by key, returning a copy of the element. This means that if the element is a map or ordered collection then modifying the members of that element WILL modify the original element, but reassigning the element itself will not modify the original element. If the key does not exist, an std::runtime_error is thrown.
 * @param key The key of the element to retrieve.
 * @return A copy of the element associated with the specified key.
 * @throws std::runtime_error If the key does not exist or if the root element is not a map or ordered collection.
 */
KVS::Element KVS::Element::getElement(const std::string key)
{
  return (*this)[key];
}

/**
 * Get an element by key, returning a copy of the element. If the key does not exist then an element with the provided value is returned. This element is NOT added to the collection and modifying it will not modify the original collection. This means that if the element is a map, array or ordered collection then modifying the members of that element WILL modify the original element, but reassigning the element itself will not modify the original element. If the root element is not a map or ordered collection, an std::runtime_error is thrown.
 * @param key The key of the element to retrieve.
 * @param value The default value to return if the key does not exist.
 * @return A copy of the element associated with the specified key, or an element with the provided value if the key does not exist.
 * @throws std::runtime_error If the root element is not a map or ordered collection.
 */
template<typename T>
KVS::Element KVS::Element::getElement(const std::string key, const T&value){
  if (!hasElement(key))
    return Element(value);
  return (*this)[key];
}

KVS::Element &KVS::Element::operator[](const char *key)
{
  return (*this)[std::string(key)];
}

KVS::Element &KVS::Element::upsert(const std::string &key)
{
  if (type == Type::Map)
    return get<Map>().upsert(key);
  if (type == Type::Ordered)
    return get<Ordered>().upsert(key);
  throw std::runtime_error("Root element is not a map or ordered collection");
}

KVS::Element &KVS::Element::upsert(const char *key)
{
  return upsert(std::string(key));
}

bool KVS::Element::operator==(const KVS::Element &other) const
{
  if (type != other.type)
    return false;
  switch (type)
  {
  case Type::Int32:
    return std::get<int32_t>(value) == std::get<int32_t>(other.value);
  case Type::Int64:
    return std::get<int64_t>(value) == std::get<int64_t>(other.value);
  case Type::Float:
    return std::get<float>(value) == std::get<float>(other.value);
  case Type::Double:
    return std::get<double>(value) == std::get<double>(other.value);
  case Type::Int32Range:
    return std::get<ranged<int32_t>>(value) == std::get<ranged<int32_t>>(other.value);
  case Type::Int64Range:
    return std::get<ranged<int64_t>>(value) == std::get<ranged<int64_t>>(other.value);
  case Type::FloatRange:
    return std::get<ranged<float>>(value) == std::get<ranged<float>>(other.value);
  case Type::DoubleRange:
    return std::get<ranged<double>>(value) == std::get<ranged<double>>(other.value);
  case Type::String:
    return std::get<std::string>(value) == std::get<std::string>(other.value);
  case Type::Array:
    return std::get<std::shared_ptr<Array>>(value)->elements == std::get<std::shared_ptr<Array>>(other.value)->elements;
  case Type::Map:
    return std::get<std::shared_ptr<Map>>(value)->data == std::get<std::shared_ptr<Map>>(other.value)->data;
  default: // Undefined / monostate
    return true;
  }
}

template <typename T>
T &KVS::Element::get()
{
  if constexpr (std::is_same_v<T, Array>)
  {
    return *std::get<std::shared_ptr<Array>>(value);
  }
  else if constexpr (std::is_same_v<T, Map>)
  {
    return *std::get<std::shared_ptr<Map>>(value);
  }
  else if constexpr (std::is_same_v<T, Ordered>)
  {
    return *std::get<std::shared_ptr<Ordered>>(value);
  }
  else
  {
    return std::get<T>(value);
  }
}

template<typename T>
const T &KVS::Element::get() const
{
  if constexpr (std::is_same_v<T, Array>)
  {
    return *std::get<std::shared_ptr<Array>>(value);
  }
  else if constexpr (std::is_same_v<T, Map>)
  {
    return *std::get<std::shared_ptr<Map>>(value);
  }
  else if constexpr (std::is_same_v<T, Ordered>)
  {
    return *std::get<std::shared_ptr<Ordered>>(value);
  }
  else
  {
    return std::get<T>(value);
  }
}

// Output operator for KVS
inline std::ostream &operator<<(std::ostream &os, const KVS &kvs)
{
  os << kvs.root;
  return os;
}

#endif
