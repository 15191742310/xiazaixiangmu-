#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"

//线程池类保证代码复用
template< typename T >
class threadpool
{
public:
    //线程池的最大数目为8，请求队列最大请求量为10000
    threadpool( int thread_number = 8, int max_requests = 10000 );
    ~threadpool();
    //往请求队列中添加任务
    bool append( T* request );

private:
    //工作线程不断从工作队列中取任务并执行，下面的函数是一起的
    static void* worker( void* arg );
    void run();

private:
    //线程池中的线程数
    int m_thread_number;
    //请求队列中允许的最大请求数目
    int m_max_requests;
    //描述线程池的数组，大小为 m_thread_number
    pthread_t* m_threads; 
   //请求队列  模板
    std::list< T* > m_workqueue;
    //请求队列保护锁
    locker m_queuelocker;
    // 是否有任务需要处理
    sem m_queuestat;
    //是否线程结束
    bool m_stop;
};
//初始化线程的线程数目，请求队列的最大数，以及停止标志，和线程数组
template< typename T >
threadpool< T >::threadpool( int thread_number, int max_requests ) : 
        m_thread_number( thread_number ), m_max_requests( max_requests ), m_stop( false ), m_threads( NULL )
{
    //引入异常机制
    if( ( thread_number <= 0 ) || ( max_requests <= 0 ) )
    {
        throw std::exception();
    }
    //通过线程数组来  保存  这里只是申请数组的大小并没有创建线程  已经创建的线程，可以采用vector代替
    m_threads = new pthread_t[ m_thread_number ];
    //申请失败抛出异常
    if( ! m_threads )
    {
        throw std::exception();
    }

    //创建 thread_number  个线程
    for ( int i = 0; i < thread_number; ++i )
    {
        //提示创建第几个线程
        printf( "create the %dth thread\n", i );
        //创建线程数组失败 删除线程数组并抛出异常  pthread_create成功返回0，失败返回错误码
        if( pthread_create( m_threads + i, NULL, worker, this ) != 0 )
        {
            delete [] m_threads;
            throw std::exception();
        }
        //分离线程，子线程结束时自己回收资源  失败删除线程数组，抛出异常
        if( pthread_detach( m_threads[i] ) )
        {
            delete [] m_threads;
            throw std::exception();
        }
    }
}
//删除线程数组，将线程停止的标志置位true，前面分离线程了因此不必考虑线程的后续工作
template< typename T >
threadpool< T >::~threadpool()
{
    delete [] m_threads;
    m_stop = true;
}

//给请求队列中添加任务
template< typename T >
bool threadpool< T >::append( T* request )
{
    //添加任务的时候需要加锁防止线程安全，因为队列是线程共享的
    m_queuelocker.lock();
    //如果请求队列中的数量大于规定的最大值就解锁返回false
    if ( m_workqueue.size() > m_max_requests )
    {
        m_queuelocker.unlock();
        return false;
    }
    //这里返回添加成功  返回true
    m_workqueue.push_back( request );
    m_queuelocker.unlock();
    m_queuestat.post();
    return true;
}

 //返回的pool目前没多大用后期看有没有需要对返回值进行处理
template< typename T >
void* threadpool< T >::worker( void* arg )
{
   
    threadpool* pool = ( threadpool* )arg;
    pool->run();
    return pool;
}

//执行请求
template< typename T >
void threadpool< T >::run()
{
    //如果线程池没有停止工作那即正常运行
    while ( ! m_stop )
    {
        //等待是否有任务需要处理，如果有看能不能获得请求队列的锁
        m_queuestat.wait();
        m_queuelocker.lock();
        //如果任务队列为空，就不执行
        if ( m_workqueue.empty() )
        {
            m_queuelocker.unlock();
            continue;
        }
        //取得第一个任务，将任务出队
        T* request = m_workqueue.front();
        m_workqueue.pop_front();
        m_queuelocker.unlock();
        //如果取得任务就正常执行
        if ( ! request )
        {
            continue;
        }
        //这是http_conn.cpp中处理客户请求的函数，
        printf("这是http_conn.cpp中处理客户请求的函数，");
        request->process();
    }
}

#endif
