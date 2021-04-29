#ifndef LOCKER_H
#define LOCKER_H

#include<exception>
#include<pthread.h>
#include<semaphore.h>
//信号量的类
class sem
{
    public:
    //创建信号量并初始化
    sem()
    {    
        //通过异常来报告错误
        if(sem_init(&m_sem,0,0) != 0)
        {
            throw std::exception();
        }
    }
    //析构信号量
    ~sem()
    {
        sem_destroy(&m_sem);
    }
    //等待信号量
    bool wait()
    {
        return sem_wait(&m_sem) == 0;
    }
    //增加信号量
    bool post()
    {
        return sem_post(&m_sem) ==0;
    }
    private:
    sem_t m_sem;
};
//封装互斥锁类
class locker
{
    public:
    //初始化
    locker()
    {
        /*通过异常来报告错误*/
        if(pthread_mutex_init(&m_mutex,NULL) != 0)
        {
            throw std::exception();
        }
    }
    //释放互斥量
    ~locker()
    {
     pthread_mutex_destroy(&m_mutex);
    }
    //加锁
    bool lock()
    {
        return pthread_mutex_lock(&m_mutex) == 0;
    }
    //释放锁
    bool unlock()
    {
        return pthread_mutex_unlock(&m_mutex) == 0;
    }
    private:
    //申请一个线程锁
    pthread_mutex_t m_mutex;
};
//封装条件变量的类
class cond
{
public:
cond()
{
    if(pthread_mutex_init(&m_mutex,NULL) != 0)
    {
        throw std::exception();
    }
    if(pthread_cond_init(&m_cond,NULL) != 0)
    {   
        //在构造函数中一旦出现问题，就应该立即释放已经成功分配的资源
        pthread_mutex_destroy(&m_mutex);
        throw std::exception();
    }
}
//销毁条件变量
~cond()
{
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);
}
//等待条件变量
bool wait()
{
    int ret =0;
    pthread_mutex_lock(&m_mutex);
    ret =pthread_cond_wait(&m_cond,&m_mutex);
    pthread_mutex_unlock(&m_mutex);
    return ret == 0;
}
//唤醒等待条件变量的线程
bool signal()
{
  return pthread_cond_signal(&m_cond) == 0;
}
   private:
   pthread_mutex_t m_mutex;
   pthread_cond_t m_cond;
};
#endif