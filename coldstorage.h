/*
 * File:      coldstorage.h
 * Author:    Daniel Hannon
 * 
 * Copyright: 2024 Daniel Hannon
 *
 * Brief: ColdStorage is an array type that stores data
 * on disk as opposed to in memory.
 */

#include <fstream>
#include <ios>
#include <iostream>
#include <string>
#include <cstddef>
#include <utility>
#ifndef COLDSTORAGE_H_EC6C2F54CD8E4BBB8F6479277A057D2C
#define COLDSTORAGE_H_EC6C2F54CD8E4BBB8F6479277A057D2C 1

/*
 * ColdStorage is a vector that is storage backed
 * it allows you to store data that exceeds your RAM size
 * currently assuming you have an ext4 fs, it can hold up to
 * 2TB of data.
 */
struct ColdStorage {
	ColdStorage():  m_currentPosition{0},
					m_fileName{generateUUID()},
					m_file{m_fileName, std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary},
					m_fileSize{0},
					m_fileOwner{true}
	{}

	ColdStorage(ColdStorage const& other):  m_currentPosition{0},
											m_fileName{generateUUID()},
											m_file{m_fileName, std::ios_base::in | std::ios_base::out | std::ios::trunc | std::ios::binary},
											m_fileSize{other.m_fileSize},
											m_fileOwner{true}
	{
		std::ifstream temp{other.m_fileName};
		if(m_file.is_open() && temp.is_open()) {
			while(!temp.eof()) {
				m_file.put(temp.get());
			}
			m_file.seekg(0, std::ios::beg);
		}
		temp.close();
	}

	ColdStorage(ColdStorage&& a): m_currentPosition{a.m_currentPosition},
								  m_fileName{std::move(a.m_fileName)},
								  m_file{std::move(a.m_file)},
								  m_fileSize{a.m_fileSize},
								  m_fileOwner{true}
	{
		// Avoid accidentally deleting the file.
		a.m_fileOwner = false;
	}


	~ColdStorage() {
		// File is no longer needed, destroy it.
		if(m_fileOwner) {
			m_file.close();
			std::remove(m_fileName.c_str());
		}	
	}

	template<typename T>
	T read_at(std::size_t idx, std::size_t size) {
		m_currentPosition = idx * size;
		m_file.seekg(m_currentPosition);
		if(m_file.eof()) {
			return T{};
		}

		char* temp = new char[size];
		m_file.read(temp,size);
		T result = *reinterpret_cast<T*>(temp);
		m_currentPosition += size;
		delete[] temp;
		return result;
	}

	template<typename T>
	void write_to(std::size_t idx, T const& val, std::size_t size) {
		m_currentPosition = idx * size;
		if(m_currentPosition > m_fileSize) {
			m_fileSize += size;
		}
		m_file.seekp(m_currentPosition);

		m_file.write(val,size);
		m_currentPosition += size;
	}

	bool eof() const {
		return m_file.eof();
	}
	
	// filenames are UUIDs as they are the best chance at not having a file name collision
	static std::string generateUUID();
private:
	std::size_t  m_currentPosition; // The Current position of the file pointer.
	std::string  m_fileName;		// The name of the file
	std::fstream m_file;			// The File in which the data is stored.
	std::size_t  m_fileSize;		// The size of the file.
	bool		 m_fileOwner;		// Denotes whether or not the file is owned 
};

#endif // COLDSTORAGE_H_EC6C2F54CD8E4BBB8F6479277A057D2C

