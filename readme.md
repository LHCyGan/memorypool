### 简介
该项目实现了一个高并发内存池，参考了google的开源项目tcmalloc实现的简易版；其功能就是实现高效的多线程内存管理。由功能可知，高并发指的是高效的多线程，而内存池则是实现内存管理的

```
mkdir build && cd build
```

```
cmake ..
```

```
make
```



### 内存池的介绍

- 1、池化技术
  我们知道，向系统申请和释放资源都有较大的开销，而池化技术就是程序先向系统申请过量的资源，而这些资源由我们自己管理，这样就能避免频繁的申请和释放资源导致的开销。
  其实，在计算机中，除了我们上面所说的内存池，还有我们之前提到过的数据结构池，以及线程池、对象池、连接池等等，都利用了池化技术。

- 2、内存池
  内存池指的是程序预先向操作系统申请足够大的一块内存空间；此后，程序中需要申请内存时，不需要直接向操作系统申请，而是直接从内存池中获取；同理，程序释放内存时，也不是将内存直接还给操作系统，而是将内存归还给内存池。当程序退出（或者特定时间）时，内存池才将之前申请的内存真正释放。

- 3、内存池主要解决的问题
  由上可知，内存池首要解决的是效率问题，其次从系统的内存分配器角度出发，还需要解决内存碎片的问题。那么什么是内存碎片问题呢？
  内存碎片分为外碎片和内碎片。

外碎片由下图所示：对于我们申请的内存，可能因为频繁的申请和释放内存导致内存空间不连续，那么就会出现明明由足够大的内存空间，但是我们却申请不出连

内存池的介绍
1、池化技术
我们知道，向系统申请和释放资源都有较大的开销，而池化技术就是程序先向系统申请过量的资源，而这些资源由我们自己管理，这样就能避免频繁的申请和释放资源导致的开销。
其实，在计算机中，除了我们上面所说的内存池，还有我们之前提到过的数据结构池，以及线程池、对象池、连接池等等，都利用了池化技术。

2、内存池
内存池指的是程序预先向操作系统申请足够大的一块内存空间；此后，程序中需要申请内存时，不需要直接向操作系统申请，而是直接从内存池中获取；同理，程序释放内存时，也不是将内存直接还给操作系统，而是将内存归还给内存池。当程序退出（或者特定时间）时，内存池才将之前申请的内存真正释放。

3、内存池主要解决的问题
由上可知，内存池首要解决的是效率问题，其次从系统的内存分配器角度出发，还需要解决内存碎片的问题。那么什么是内存碎片问题呢？
内存碎片分为外碎片和内碎片。

> - 外碎片：对于我们申请的内存，可能因为频繁的申请和释放内存导致内存空间不连续，那么就会出现明明由足够大的内存空间，但是我们却申请不出连续的空间出来，这便是外碎片问题了。
> - 内碎片则是由于一些对齐的需求，导致分配出去的内存空间无法被利用。

### TODO

- [x]  定长的内存池
