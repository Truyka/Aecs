#ifndef __PAGEDVECTOR_H__
#define __PAGEDVECTOR_H__

#include <vector>
#include <memory>
#include <cassert>

namespace aecs
{


template<typename T, size_t pageSize>
class PagedVector
{
public:
    using Page = std::vector<T>;

    class iterator
    {
    public:
        using difference_type = size_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::forward_iterator_tag;

        iterator(size_t i, PagedVector<T, pageSize>& cvec) : idx(i), vec(cvec) {}
        iterator& operator++()                 { idx++; return *this; }
        iterator  operator++(int)              { iterator cpy = *this; ++(*this); return cpy; }
        bool operator==(iterator& other) const { return idx == other.idx; }
        bool operator!=(iterator& other) const { return !(*this == other); }
        T& operator*()                         { return vec[idx]; }
    private:
        size_t idx;
        PagedVector<T, pageSize>& vec;
    };

    iterator begin() { return iterator(0, *this); }
    iterator end()   { return iterator(size_, *this); }

public:
    PagedVector() : size_(0)
    {

    }

    void push_back(T&& elem)
    {
        size_t pageIdx = size_ / pageSize;

        if(pageIdx >= storage_.size())
        {
            storage_.resize(pageIdx + 1);

            storage_[pageIdx] = std::make_unique<Page>();
            storage_[pageIdx]->reserve(pageSize);
        }

        storage_[pageIdx]->push_back(std::forward<T>(elem));

        size_++;
    }

    void pop_back()
    {
        size_t pageIdx = (size_ - 1) / pageSize;
        storage_[pageIdx]->pop_back();

        assert((storage_[pageIdx]->capacity() == pageSize));
        size_--;
    }

    T& operator[](size_t n)
    {
        return storage_[n / pageSize]->operator[](n % pageSize);
    }

    T& back()
    {
        return operator[](size_ - 1);
    }

    size_t size()
    {
        return size_;
    }

private:
    std::vector<std::unique_ptr<Page>> storage_;
    size_t size_;
};


} // namespace aecs
#endif // __PAGEDVECTOR_H__