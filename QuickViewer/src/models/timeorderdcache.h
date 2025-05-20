#ifndef TIMEORDERDCACHE_H
#define TIMEORDERDCACHE_H

#include <QtCore>
#include <QtConcurrent>

template <typename Key, typename T>
class TimeOrderdCache
{
public:
    TimeOrderdCache(int maxsize=30) {m_maxSize = maxsize;}

    void insert(const Key &key, T object) {
//        qDebug("add cached: %d", key);
        checkShouldBeInserted(key);
        insertNoChecked(key, object);
    }
    void insertNoChecked(const Key &key, T object) {
        m_cache.insert(key, object);
        while(size() > m_maxSize) {
            Key last = m_newerOrders.takeLast();
    //            qDebug("remove cached: %d", last);
            trash(m_cache.take(last));
        }
    }
    void remove(const Key& key) {
        if(m_newerOrders.contains(key)) {
            m_newerOrders.removeOne(key);
            trash(m_cache.take(key));
        }
    }

    bool checkShouldBeInserted(const Key& key) {
        bool contains = m_newerOrders.contains(key);
        if(contains)
            m_newerOrders.removeOne(key);
        m_newerOrders.push_front(key);
        return !contains;
    }
    void retain(const Key& key) {
        if(m_newerOrders.contains(key)) {
            m_newerOrders.removeOne(key);
            m_newerOrders.push_front(key);
        }
    }

    int size() const { return m_cache.size(); }
    int count() const { return m_cache.size(); }
    bool empty() const { return m_cache.empty(); }
    bool contains(Key& key) const {return m_newerOrders.contains(key); }
    T& object(Key &key) { return m_cache[key]; }
    void clear() {
        foreach(const Key& key, m_cache.keys()) {
            trash(m_cache.take(key));
        }
        m_cache.clear();
        m_newerOrders.clear();
    }
    virtual void trash(T ) {
    }


protected:
    QMap<Key, T> m_cache;
    QList<Key> m_newerOrders;
    int m_maxSize;
};

template <typename Key, class T>
class TimeOrderdCachePtr : public TimeOrderdCache<Key, T*>
{
public:
     TimeOrderdCachePtr(int maxsize=30)
         : TimeOrderdCache<Key, T*>(maxsize){}
    void trash(T* oldest) {
        delete oldest;
    }
};

template <typename Key, class T>
class TimeOrderdCacheFuture : public TimeOrderdCache<Key, QFuture<T> >
{
public:
     TimeOrderdCacheFuture(int maxsize=30)
         : TimeOrderdCache<Key, QFuture<T> >(maxsize){}
    void trash(QFuture<T> oldest) {
        oldest.result();
    }
};


template <typename Key, class T>
class TimeOrderdCacheFuturePtr : public TimeOrderdCache<Key, QFuture<T*> >
{
public:
     TimeOrderdCacheFuturePtr(int maxsize=30)
         : TimeOrderdCache<Key, QFuture<T*> >(maxsize){}
    void trash(QFuture<T*> oldest) {
        QtConcurrent::run([=]{
            delete oldest.result();
        });
    }
};

#endif // TIMEORDERDCACHE_H
