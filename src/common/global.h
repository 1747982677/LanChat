#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>

namespace LanChat {
	//全局错误码定义
	enum class ErrorCode {
		Success = 0,
		//网络与系统(1xxx)
		NetworkError = 1001,     //网络连接失败
		Timeout = 1002,          //网络请求超时
		JsonParseError = 1003,   //JSON解析错误

		//认证与账户(2xxx)
		AuthFailed = 2001,		//鉴权失败（账号或密码错误）
		AccountExists = 2002,     //账号已存在
		PasswordInvalid = 2003,   //密码不符合要求
		VerifyCodeError = 2004,   //验证码错误或过期
		UserNotFound = 2005,      //用户不存在
		
		//业务逻辑(3xxx)
		FriendRequestRepeated = 3001, //重复添加好友
		TargetOffline = 3002,         //目标用户离线
	};
	//全局常量定义
	namespace Constants {
		const QString SERVER_URL = "ws://127.0.0.1:8080"; //默认服务器地址
		const int CONNECT_TIMEOUT_MS = 5000; //连接超时时间（5秒超时）
		const int HEARTBEAT_INTERVAL_MS = 30000; //心跳间隔（30秒）
		const QString DB_NAME = "lanchat.db"; //默认数据库文件名
	}

} // namespace LanChat

#endif // GLOBAL_H
