/*
 * File:      cold_vector.h
 * Author:    Daniel Hannon
 * 
 * Copyright: 2024 Daniel Hannon
 *
 * Brief: ColdVector is a storage backed vector type.
 */

#include "util.h"

#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <utility>
#include <iostream>
#include <iterator>
#ifndef COLD_VECTOR_H_6C4A5FE951FC4491A740E7D8F71E2E6B
#define COLD_VECTOR_H_6C4A5FE951FC4491A740E7D8F71E2E6B 1

/*
 * ColdVector is a storage-backed vector with an internal
 * buffer to reduce the need for read/write operations.
 */
template<typename T, std::size_t BUFF_SIZE = 100>
struct ColdVector {
	ColdVector(std::initializer_list<T> inp): m_buffer{},
				  m_buffIndex{0},
				  m_buffSize{0},
				  m_vectorSize{0},
				  m_fileName{get_uuid()},
				  m_fileOwner{true},
				  m_fileStream{m_fileName, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc}
	{
		static_assert(BUFF_SIZE != 0, "Cannot have 0 buffer");
		for(auto const& v : inp) {
			emplace_back(v);
		}
	}

	ColdVector(ColdVector const& other): m_buffer{other.m_buffer},
										 m_buffIndex{other.m_buffIndex},
										 m_buffSize{other.m_buffSize},
										 m_vectorSize{0},
										 m_fileName{get_uuid()},
										 m_fileOwner{true},
										 m_fileStream{m_fileName, std::ios::in | std::ios::out | std::ios::binary}
	{
		std::ifstream temp{other.m_fileName};
		if(m_fileStream.is_open() && temp.is_open()) {
			while(!temp.eof()) {
				m_fileStream.put(temp.get());
			}
			m_fileStream.seekp(0, std::ios::beg);
			m_fileStream.seekg(0, std::ios::beg);
		}
		temp.close();
	}

	ColdVector(ColdVector&& a): m_buffer{std::move(a.m_buffer)},
								m_buffIndex{a.m_buffIndex},
								m_buffSize{a.m_buffSize},
								m_vectorSize{a.m_vectorSize},
								m_fileName{std::move(a.m_fileName)},
								m_fileOwner{true},
								m_fileStream{std::move(a.m_fileStream)}
	{
		a.m_fileOwner = false;
	}


	~ColdVector() {
		m_fileStream.close();
		// Cleanup
		if(m_fileOwner) {
			std::remove(m_fileName.c_str());
		}
	}

	ColdVector& operator=(ColdVector const& other) {
		if(this != &other) {
			auto val = ColdVector{other};
			swap(val);
		}
		return *this;
	}

	// Amount of elements currently stored in the vector
	std::size_t size() const noexcept {
		return m_vectorSize;
	}
	
	// Maximum allowed number of elements
	std::size_t max_size() const noexcept {
		constexpr std::size_t maxSize = UINT64_MAX / sizeof(T);
		return maxSize;
	}
	
	// Returns true if ColdVector is empty
	bool empty() const {
		return (m_buffSize == 0 && m_vectorSize == 0);
	}

	T& at(std::size_t idx) {
		if(idx >= m_vectorSize) {
			throw std::out_of_range("Element is out of bounds!");
		}

		if((idx < m_buffIndex)
	       || (idx >= (m_buffIndex + m_buffSize))) {
			dump_buffer();
			load_buffer_at(idx);
		}

		return m_buffer[idx - m_buffIndex];
	}

	template<class... Args>
	void emplace_back(Args&&... args) {
		if(m_vectorSize >= (m_buffIndex + BUFF_SIZE)) {
			dump_buffer();
			load_buffer_at(m_vectorSize);
		}
		m_buffer[m_vectorSize - m_buffIndex] = T(std::forward<Args>(args)...);
		m_buffSize++;
		m_vectorSize++;
	}

	void pop_back() {
		if(m_vectorSize == 0) return;
		m_vectorSize--;
	}

	T& operator[](std::size_t idx) {
		if((idx < m_buffIndex) || (idx >= (m_buffIndex + m_buffSize))) {
			dump_buffer();
			load_buffer_at(idx);
		}

		return m_buffer[idx - m_buffIndex];
	}

	T operator[](std::size_t idx) const {
		if((idx >= m_buffIndex) && (idx < (m_buffIndex + m_buffSize))) {
			return m_buffer[idx - m_buffIndex];
		}
		return peekAt(idx);
	}

	void swap(ColdVector & other) noexcept {
		std::swap(m_buffer,other.m_buffer);
		std::swap(m_buffIndex, other.m_buffIndex);
		std::swap(m_buffSize,other.m_buffSize);
		std::swap(m_vectorSize, other.m_vectorSize);
		std::swap(m_fileName, other.m_fileName);
		std::swap(m_fileOwner, other.m_fileOwner);
		std::swap(m_fileStream, other.m_fileStream);
	}

	class iterator {
		public:
			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using pointer = T*;
			using reference = T&;
			using iterator_category = std::random_access_iterator_tag;

			iterator(ColdVector const* pVec): m_internal{pVec},
										      m_idx{0}
			{}

			iterator(iterator const& itr): m_internal{itr.m_internal},
										   m_idx{itr.m_idx}
			{}

			iterator& operator++(int) {
				m_idx++;
				return *this;
			}
			
			iterator& operator++() {
				m_idx++;
				return *this;
			}

			iterator operator+(std::size_t val) {
				auto tmp{*this};
				tmp.m_idx += val;
				return tmp;
			}

			iterator operator-(std::size_t val) {
				auto tmp{*this};
				tmp.m_idx -= val;
				return tmp;
			}
			
			iterator& operator--(int) {
				m_idx--;
				return *this;
			}

			iterator& operator--() {
				m_idx--;
				return *this;
			}

			T& operator*() const {
				return const_cast<ColdVector *>(m_internal)->operator[](m_idx);
			}

			pointer operator->() {
				return &m_internal->operator[](m_idx);
			}

			iterator operator+(difference_type v) const {
				iterator temp{*this};
				temp.m_idx += v;
				return temp;
			}

			bool operator==(iterator const& rhs) const {return (m_idx == rhs.m_idx) && (m_internal == rhs.m_internal);}
			bool operator!=(iterator const& rhs) const {return !(*this == rhs);}
			
			friend std::weak_ordering operator<=>(iterator const& lhs, iterator const& rhs) {
				if(lhs.m_internal != rhs.m_internal) return std::weak_ordering::greater;
				return lhs.m_idx <=> rhs.m_idx;
			}

		std::int64_t get_index() const {
			return m_idx;
		}

		private:
			ColdVector const* m_internal;
			std::int64_t m_idx;
	};

	class reverse_iterator: public iterator {
	public:
		reverse_iterator(ColdVector const* pVec): iterator{pVec}
		{}

		reverse_iterator(iterator const& itr): iterator{itr}
		{}

		reverse_iterator& operator++(int) {
			iterator::operator--();
			return *this;
		}

		reverse_iterator& operator++() {
			iterator::operator--();
			return *this;
		}

		reverse_iterator operator+(iterator::difference_type v) const {
			return iterator::operator-(v);	
		}

		reverse_iterator operator-(iterator::difference_type v) const {
			return iterator::operator+(v);
		}

		reverse_iterator& operator+=(iterator::difference_type v) {
			return iterator::operator-=(v);
		}

		reverse_iterator& operator-=(iterator::difference_type v) {
			return iterator::operator+=(v);
		}

		std::weak_ordering operator<=>(reverse_iterator const& rhs) {
			return rhs.iterator <=> this->iterator;
		}
	};

	iterator begin() const { return iterator{this}; }

	iterator end() const {
		auto tmp = iterator{this};
		tmp = tmp + m_vectorSize;
		return tmp;
	}

	reverse_iterator rbegin() const {
		return end() - 1;
	}

	reverse_iterator rend() const {
		return begin() -1;
	}

	ColdVector(iterator const& begin,
			   iterator const& end) : m_buffer{},
		                              m_buffIndex{0},
									  m_buffSize{0},
									  m_vectorSize{0},
									  m_fileName{get_uuid()},
									  m_fileOwner{true},
									  m_fileStream{m_fileName, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc}
	{
		for(auto it = begin; it < end; it++) {
			emplace_back(*it);
		}
	}

	void insert(iterator const& at, iterator const& begin, iterator const& end) {
		auto insertedData = begin;
		auto atPosition = at.get_index();
		while(insertedData != end) {
			if(atPosition == m_vectorSize) {
				emplace_back(*insertedData);
			} else {
				this->operator[](atPosition) = *insertedData;
			}
			atPosition++;
			insertedData++;
		}
	}

private:
	// dump_buffer is an internal thing to help deal with random file access.
	void dump_buffer() {
		if(m_buffSize == 0) return;
		m_fileStream.seekp(m_buffIndex * sizeof(T), std::ios::beg);
		m_fileStream.write(reinterpret_cast<const char *>(&m_buffer), m_buffSize * sizeof(T));
	}

	// Another helper for dealing with random file access.
	void load_buffer_at(std::size_t idx) {
		// if the index is the same as the size of the file, load in nothing.
		if(idx >= m_vectorSize) {
			m_buffSize = 0;
			m_buffIndex = m_vectorSize;
			return;
		}
		m_fileStream.seekg(idx * sizeof(T), std::ios::beg);
		m_fileStream.read(reinterpret_cast<char*>(&m_buffer), std::min(BUFF_SIZE, m_vectorSize) * sizeof(T));
		m_buffIndex = idx;
		m_buffSize = std::min((m_vectorSize - idx),BUFF_SIZE);
	}

	// Horrible hack job
	T const peekAt(std::size_t idx) const {
		if(m_buffSize == 0) return T{};
		std::ifstream tmp{m_fileName};
		if(!tmp.is_open()) return T{};
		tmp.seekg(idx * sizeof(T), std::ios::beg);
		T out{};
		tmp.read(reinterpret_cast<char*>(&out), sizeof(T));
		tmp.close();
		return out;
	}

private:
	std::array<T, BUFF_SIZE> m_buffer;		// The Data buffer to reduce read/writes
	std::size_t				 m_buffIndex;	// Index of m_buffer[0]
	std::size_t				 m_buffSize;	// The amount of indexed data currently stored in the vector
	std::size_t				 m_vectorSize;	// The size of the vector
	std::string				 m_fileName;	// The name of the file in which the data is stored
	bool					 m_fileOwner;	// Check if this instance owns the file or not.
	std::fstream			 m_fileStream;	// The filestream associated with the vector
};
#endif // COLD_VECTOR_H_6C4A5FE951FC4491A740E7D8F71E2E6B
	
