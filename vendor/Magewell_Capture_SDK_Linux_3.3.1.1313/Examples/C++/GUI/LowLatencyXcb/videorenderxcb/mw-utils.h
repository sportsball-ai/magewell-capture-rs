////////////////////////////////////////////////////////////////////////////////
// CONFIDENTIAL and PROPRIETARY software of Magewell Electronics Co., Ltd.
// Copyright (c) 2011-2014 Magewell Electronics Co., Ltd. (Nanjing)
// All rights reserved.
// This copyright notice MUST be reproduced on all authorized copies.
////////////////////////////////////////////////////////////////////////////////
#ifndef MW_UTILS_H
#define MW_UTILS_H

extern int debug_level;

enum {
    MW_DEBUG_ERR = 1,
    MW_DEBUG_WARNING,
    MW_DEBUG_INFO,
};

#define MW_DEBUG_ENABLE
#ifdef 	MW_DEBUG_ENABLE
    #define mw_debug(level, fmt, ...) { 		      \
        if (debug_level > level) 					  \
            printf("[%d:%24s]" fmt, 		  \
                __LINE__ ,__func__, ##__VA_ARGS__);  \
        }
    #else
#define mw_debug(level, fmt, ...) do {} while(0)
#endif

#include <queue>
#include <pthread.h>

template <typename T>
class MWQueue
{
public:
    MWQueue()
    {
        pthread_mutex_init(&m_mutex, NULL);
        pthread_cond_init(&m_cond, NULL);
    }

    ~MWQueue()
    {
        pthread_cond_destroy(&m_cond);
        pthread_mutex_destroy(&m_mutex);
    }

    T pop()
    {
        pthread_mutex_lock(&m_mutex);

        while (m_queue.empty()) {
            pthread_cond_wait(&m_cond, &m_mutex);
        }

        T item = m_queue.front();
        m_queue.pop();

        pthread_mutex_unlock(&m_mutex);

        return item;
    }

    bool trypop(T &item) {
        pthread_mutex_lock(&m_mutex);

        if (m_queue.empty()) {
            pthread_mutex_unlock(&m_mutex);
            return false;
        }

        item = m_queue.front();
        m_queue.pop();
        pthread_mutex_unlock(&m_mutex);

        return true;
    }

    void push(const T& item)
    {
        pthread_mutex_lock(&m_mutex);
        m_queue.push(item);
        pthread_mutex_unlock(&m_mutex);

        pthread_cond_signal(&m_cond);
    }

    bool empty() { return m_queue.empty(); }

    int size() { return m_queue.size(); }

    void clear() {
        pthread_mutex_lock(&m_mutex);
        while (!m_queue.empty()) {
            m_queue.pop();
        }
        pthread_mutex_unlock(&m_mutex);
    }

private:
    std::queue<T> m_queue;

    pthread_mutex_t m_mutex;
    pthread_cond_t  m_cond;
};

template <class T, size_t N>
class MWArray
{
public:
    MWArray() :
        m_Size(0)
    {
        m_Data = new T[N];
    }

    ~MWArray()
    {
        delete m_Data;
    }

    void clear() {
        m_Size = 0;
    }

    size_t size() {
        return m_Size;
    }

    T & operator[](int i) {
        //assert(0 <= i && i < m_MaxSize); TODO
        return m_Data[i];
    }

    T *data() {
        return m_Data;
    }

    bool resize(size_t size, bool keepOrig) {
        T *newData = new T[size];
        if (newData == NULL)
            return false;

        if (keepOrig) {
            memcpy(newData, m_Data, sizeof(T) * m_Size);
        } else
            m_Size = 0;
        delete m_Data;
        m_Data = newData;
        m_MaxSize = size;

        return true;
    }

    bool append(T &item) {
        if (m_Size >= m_MaxSize) {
            if (!resize(m_MaxSize + 8, true))
                return false;
        }

        m_Data[m_Size] = item;
        m_Size++;
        return true;
    }

    bool has(T &item, bool is_equel(T &item1, T &item2)) {
        for (size_t i = 0; i < m_Size; i++) {
            if (is_equel(m_Data[i], item))
                return true;
        }
        return false;
    }

private:
    size_t  m_MaxSize;
    size_t  m_Size;
    T *     m_Data;
};

#endif // MW_UTILS_H
