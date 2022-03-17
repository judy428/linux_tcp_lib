/*****************************************************************************
 common Module Copyright (c) 2017. All Rights Reserved.

 FileName: common.h
 Version: 1.0
 Date: 2017.04.10

 History:
 zhujy     2017.04.10   1.0     Create
 ******************************************************************************/


namespace common{
   const char* const COMMON_TAG = "common"; 

    enum RISK_CTRL_ERROR
    {
        NAUT_AT_S_OK = 0,
        NAUT_AT_E_LOG_INIT_INVALID = 30000,                         /*日志文件初始化失败 */
        NAUT_AT_E_TRIAL_SERVER_START_FAILED = 30001,                /*系统启动失败 */
        NAUT_AT_E_CONFIGFILE_INVALID = 30100,                       /*配置文件找不到或者不合法 */
        NAUT_AT_E_CONFIG_INVALID = 30101,                           /*配置不合法 */
        NAUT_AT_E_CONFIG_READ_INVALID = 30102,                      /*配置文件读取失败 */
        NAUT_AT_E_DATA_HANDLE_START_ERROR = 30103,                  /*数据处理实例启动失败 */
        NAUT_AT_E_TRIAL_SERVER_START_ERROR = 30104,                 /*试算处理实例启动失败 */
        NAUT_AT_E_TCP_CLIENT_OPEN_FALIED = 30105,                   /*连接管理服务器失败 */
        NAUT_AT_E_TCP_SERVER_NOTIFY_PARAM_ERROR = 30400,            /*回调函数如参如参失败 */
        NAUT_AT_E_TCP_SERVER_NOTIFY_RECV_ERROR = 30401,             /*回调函数收到错误信息 */
        NAUT_AT_E_TCP_CLIENT_NOTIFY_PARAM_ERROR = 30402,            /*客户端回调函数如参如参失败 */
        NAUT_AT_E_TRIAL_HANDLE_ERROR_EXCHANGE = 30500,              /*交易所信息错误 */
        NAUT_AT_E_DATA_HANDLE_ERROR_POSITION = 30501,               /*持仓信息失误*/
        NAUT_AT_E_CLIENT_NOTIFY_ERROR_CONTRACE = 30502,             /*获取合约信息失误*/
        NAUT_AT_E_CLIENT_NOTIFY_ERROR_INSTRATE = 30503,             /*获取保证金率信息失误*/
        NAUT_AT_E_CLIENT_NOTIFY_ERROR_DCEPAIR = 30504,              /*获取大商所优先级信息失误*/
        NAUT_AT_E_CLIENT_NOTIFY_ERROR_EXPIRE = 30505,               /*获取最后交易日合约信息失误*/
        NAUT_AT_E_TRIVL_UNIT_ERROR_INSTRUMENT = 30601,              /*合约代码错误*/
        NAUT_AT_E_TCP_CLI_CLOSE_CONN = 30701,              			/*与管理墨款的连接中断*/
    };

    
}