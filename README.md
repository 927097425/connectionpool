# connectionpool
用于服务器并发访问mysql数据库的情景
# 项目背景
为了提高MySQL数据库（基于C/S设计）的访问瓶颈，除了在服务器端增加缓存服务器缓存常用的数据之外（例如redis），还可以增加连接池，来提高MySQL Server的访问效率，在高并发情况下，大量的TCP三次握手、MySQL Server连接认证、MySQL Server关闭连接回收资源和TCP四次挥手所耗费的性能时间也是很明显的，增加连接池就是为了减少这一部分的性能损耗。在市场上比较流行的连接池包括阿里的druid，c3p0以及apache dbcp连接池，它们对于短时间内大量的数据库增删改查操作性能的提升是很明显的，但是它们有一个共同点就是，全部由Java实现的。 本项目就是为了在C/C++项目中，提供MySQL Server的访问效率，实现基于C++代码的数据库连接池模块。
# 连接池功能点介绍
连接池一般包含了数据库连接所用的ip地址、port端口号、用户名和密码以及其它的性能参数，例如初始连接量，最大连接量，最大空闲时间、连接超时时间等，该项目是基于C++语言实现的连接池，主要也是实现以上几个所有连接池都支持的通用基础功能。

### 初始连接量（initSize）
表示连接池事先会和MySQL Server创建initSize个数的connection连接，当应用发起MySQL访问时，不用再创建和MySQL Server新的连接，直接从连接池中获取一个可用的连接就可以，使用完成后，并不去释放connection，而是把当前connection再归还到连接池当中。 

### 最大连接量（maxSize）
当并发访问MySQL Server的请求增多时，初始连接量已经不够使用了，此时会根据新的请求数量去创建更多的连接给应用去使用，但是新创建的连接数量上限是maxSize，不能无限制的创建连接，因为每一个连接都会占用一个socket资源，一般连接池和服务器程序是部署在一台主机上的，如果连接池占用过多的socket资源，那么服务器就不能接收太多的客户端请求了。当这些连接使用完成后，再次归还到连接池当中来维护。 

### 最大空闲时间（maxIdleTime）
当访问MySQL的并发请求多了以后，连接池里面的连接数量会动态增加，上限是maxSize个，当这些连接用完再次归还到连接池当中。如果在指定的maxIdleTime里面，这些新增加的连接都没有被再次使用过，那么新增加的这些连接资源就要被回收掉，只需要保持初始连接量initSize个连接就可以了。 

### 连接超时时间（connectionTimeout）
当MySQL的并发请求量过大，连接池中的连接数量已经到达 maxSize了，而此时没有空闲的连接可供使用，那么此时应用从连接池获取连接无法成功，它通过阻塞的方式获取连接的时间如果超过connectionTimeout时间，那么获取连接失败，无法访问数据库。

# 功能实现设计
ConnectionPool.cpp和ConnectionPool.h：连接池代码实现 
Connection.cpp和Connection.h：数据库操作代码、增删改查代码实现 

连接池主要包含了以下功能点： 
1.连接池只需要一个实例，所以ConnectionPool以单例模式进行设计 。
2.从ConnectionPool中可以获取和MySQL的连接Connection 。
3.空闲连接Connection全部维护在一个线程安全的Connection队列中，使用线程互斥锁保证队列的线程安全。 
4.如果Connection队列为空，还需要再获取连接，此时需要动态创建连接。
5.队列中空闲连接时间超过maxIdleTime的就要被释放掉，只保留初始的initSize个连接就可以了，这个功能点放在独立的线程中。
6.如果Connection队列为空，而此时连接的数量已达上限maxSize，那么等待connectionTimeout时间，如果还获取不到空闲的连接，那么获取连接失败，此处从Connection队列获取空闲连接，使用带超时时间的mutex互斥锁来实现连接超时时间 。
7.用户获取的连接用shared_ptr智能指针来管理，用lambda表达式定制连接释放的功能（不真正释放连接，而是把连接归还到连接池中） 
8.连接的生产和连接的消费采用生产者-消费者线程模型来设计，使用了线程间的同步通信机制条件变量和互斥锁 。

# 压力测试
通过测试模块Test.h和Test.c实现了压力测试，分别测试了1000、5000、10000数据量下的并发请求响应时间。
![image](https://github.com/927097425/connectionpool/assets/78626482/5fc60a26-5643-4ab5-ac16-dd390dd980e3)

![image](https://github.com/927097425/connectionpool/assets/78626482/e2bfdbb6-b7a1-4736-99f2-05cbff1d472a)

![image](https://github.com/927097425/connectionpool/assets/78626482/640d1e51-1f5a-44d0-afbc-b52dcc8bf956)

列成表如下
### 单线程
<table>
    <tr>
        <td align = "center">数据量</td> 
        <td align = "center">未使用连接池花费时间(ms)</td> 
        <td align = "center">使用连接池花费时间(ms)</td> 
   </tr>
    <tr>
  		<td align = "center">1000</td> 
        <td align = "center">8740</td> 
        <td align = "center">2795</td> 
    </tr>
    <tr>
        <td align = "center">5000</td> 
        <td align = "center">45124</td> 
        <td align = "center">15618</td> 
    </tr>
    <tr>
        <td align = "center">10000</td> 
        <td align = "center">91379</td> 
        <td align = "center">27566</td> 
    </tr>
</table>

### 四线程

<table>
    <tr>
        <td align = "center">数据量</td> 
        <td align = "center">未使用连接池花费时间(ms)</td> 
        <td align = "center">使用连接池花费时间(ms)</td> 
   </tr>
    <tr>
  		<td align = "center">1000</td> 
        <td align = "center">8708</td> 
        <td align = "center">1235</td> 
    </tr>
    <tr>
        <td align = "center">5000</td> 
        <td align = "center">44666</td> 
        <td align = "center">6093</td> 
    </tr>
    <tr>
        <td align = "center">10000</td> 
        <td align = "center">91228</td> 
        <td align = "center">12538</td> 
    </tr>
</table>

可见线程池的添加能够使高并发访问mysql数据库的效率提高。
