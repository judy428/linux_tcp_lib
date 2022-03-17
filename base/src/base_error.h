#ifndef BASE_ERROR_H
#define BASE_ERROR_H

enum BASE_ERROR
    {
        NAUT_AT_S_OK = 0,
        /**101*** tcp服务error*/
            /**1010** tcp服务参数error*/
        NAUT_AT_E_TCP_SERVER_PORT = 101001,                        /*输入端口错误 */
            /**1011** tcp服务运行error*/
        NAUT_AT_E_TCP_SERVER_SOCKET = 101101,                        /*建立socket错误 */
        NAUT_AT_E_TCP_SERVER_BIND = 101102,                        /*绑定错误 */
        NAUT_AT_E_TCP_SERVER_LISTEN = 101103,                        /*监听错误 */
        NAUT_AT_E_TCP_SERVER_EPOLL_CREATE = 101104,                  /*epoll_create错误 */
        NAUT_AT_E_TCP_SERVER_BIND_CPU = 101105,                  /*绑核失败 */
        NAUT_AT_E_TCP_SERVER_CLIENT_CLOSED = 101201,                  /*客户端被关闭 */
        NAUT_AT_E_TCP_SERVER_PUSH_CACHE_FAILED = 101202,                  /*服务器插入发送数据到缓冲失败 */
        NAUT_AT_E_TCP_SERVER_SEND_CACHE_FULL = 101203,                  /*服务器发送缓冲满 */
        

        /**101*** tcp客户端error*/
        NAUT_AT_E_TCP_CLIENT_SEND = 102001,                        /*客户端发送失败 */
    };

#endif