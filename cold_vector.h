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
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
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
	ColdVector(): m_buffer{},
				  m_buffIndex{0},
				  m_buffSize{0},
				  m_vectorSize{0},
				  m_fileName{get_uuid()},
				  m_fileOwner{true},
				  m_fileStream{m_fileName, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc}
	{
		static_assert(BUFF_SIZE != 0, "Cannot have 0 buffer");
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

	class iterator {
		public:
			using value_type = T;
			using difference_type = std::ptrdiff_t;
			using pointer = T*;
			using reference = T&;
			using iterator_category = std::random_access_iterator_tag;

			iterator(): m_internal{nullptr},
						m_idx{0}
			{}

			iterator(ColdVector* pVec): m_internal{pVec},
										m_idx{0}
			{}

			iterator(iterator const& itr): m_internal{itr.pVec},
										   m_idx{itr.m_idx}
			{}

			~iterator()
			{
				m_internal = nullptr;
			}

			iterator& operator++(int) {
				m_idx++;
				return *this;
			}
			
			iterator& operator++() {
				m_idx++;
				return *this;
			}

			reference operator*() {
				return m_internal->operator[](m_idx);
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

		private:
			ColdVector* m_internal;
			std::size_t m_idx;
	};

	iterator begin() { return iterator{this}; }

	iterator end() {
		auto tmp = iterator{this};
		tmp = tmp + m_vectorSize;
		return tmp;
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
	
