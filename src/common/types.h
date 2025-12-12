#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QDateTime>
#include <QMetaType>
#include <QJsonObject>
#include <QJsonDocument>
#include <QList>

namespace LanChat {
	//----------枚举定义----------
	//消息类型
	enum class MessageType {
		Text = 0,   //文本消息
		Image = 1,  //图片消息
		Voice = 2,  //语音消息
		System = 99 //系统消息
	};
	//消息状态（UI显示用）
	enum class MessageStatus {
		Pending = 0, //发送中(转圈loading)
		Sent = 1,    //已发送
		Read = 2,    //已读(绿色对勾)
		Failed = 3   //发送失败(红色感叹号)
	};
	//用户状态
	enum class UserStatus {
		Offline = 0, //离线(灰色空心点)
		Online = 1,  //在线(绿色实心点)
		Busy = 2,    //忙碌(黄色实心点)
	};
	//好友申请处理状态
	enum class FriendRequestStatus {
		Pending = 0, //待处理(显示同意按钮)
		Accepted = 1,//已接受(显示已添加)
		Rejected = 2 //已拒绝
	};
	//----------实体结构体----------
	//用户详细信息(可用于个人资料页、联系人详情)
	struct UserInfo{
		QString userId;		//用户唯一ID
		QString account;	//用户账号
		QString nickname;	//用户昵称
		QString avatarPath;	//头像文件路径
		QString email;		//电子邮箱
		QString phone;		//联系电话
		QString signature;	//个性签名
		UserStatus status = UserStatus::Offline;	//用户状态

		//序列化：将用户详细信息转JSON发送给网络层
		QJsonObject toJson() const {
			QJsonObject json;
			json["userId"] = userId;
			json["account"] = account;
			json["nickname"] = nickname;
			json["avatarPath"] = avatarPath;
			json["email"] = email;
			json["phone"] = phone;
			json["signature"] = signature;
			json["status"] = static_cast<int>(status);
			return json;
		}
		//反序列化：从网络层解析JSON成用户详细信息
		static UserInfo fromJson(const QJsonObject& json) {
			UserInfo userInfo;
			userInfo.userId = json["userId"].toString();
			userInfo.account = json["account"].toString();
			userInfo.nickname = json["nickname"].toString();
			userInfo.avatarPath = json["avatarPath"].toString();
			userInfo.email = json["email"].toString();
			userInfo.phone = json["phone"].toString();
			userInfo.signature = json["signature"].toString();
			userInfo.status = static_cast<UserStatus>(json["status"].toInt());
			return userInfo;
		}

	};
	//聊天消息实体(可用于聊天框、本地存储)
	struct Message {
		QString messageId;      //消息唯一ID
		QString senderId;       //发送者用户ID
		QString receiverId;     //接收者用户ID
		QString content;        //消息内容(文本或图片路径)
		MessageType type = MessageType::Text; //消息类型
		MessageStatus status = MessageStatus::Pending; //消息状态
		qint64 timestamp = 0;      //消息发送时间(时间戳)

		//辅助函数：判断是否是自己发送的消息(可用于:UI决定气泡在左还是在右)
		bool isSelf(const QString& currentUserId) const {
			return senderId == currentUserId;
		}
		//序列化：将聊天信息转JSON发送给网络层
		QJsonObject toJson() const {
			QJsonObject json;
			json["messageId"] = messageId;
			json["senderId"] = senderId;
			json["receiverId"] = receiverId;
			json["content"] = content;
			json["type"] = static_cast<int>(type);
			json["timestamp"] = timestamp;
			return json;
		}
		//反序列化：从网络层解析JSON成聊天信息
		static Message fromJson(const QJsonObject& json) {
			Message msg;
			msg.messageId = json["messageId"].toString();
			msg.senderId = json["senderId"].toString();
			msg.receiverId = json["receiverId"].toString();
			msg.content = json["content"].toString();
			msg.type = static_cast<MessageType>(json["type"].toInt());
			msg.timestamp = json["timestamp"].toVariant().toLongLong();
			msg.status = MessageStatus::Sent; // 收到别人的消息，默认为已发送
			return msg;
		}
	};
	//会话/联系人实体(可用于联系人列表、消息列表)
	struct ChatSession {
	//基础信息
		QString userId;			//对方ID
		QString nickname;		//对方昵称
		QString remark;			//备注名
		QString avatarPath;		//头像文件路径
		UserStatus status = UserStatus::Offline; //用户状态

	//会话状态信息
		QString lastMessage;		//最后一条消息内容
		qint64 lastTime = 0;		//最后消息时间(时间戳)
		int unreadCount = 0;		//未读消息数
		bool isTop = false;			//是否置顶

	//辅助函数：UI显示的名称(优先显示备注，其次昵称)
		QString displayName() const {
			return remark.isEmpty() ? nickname : remark;
		}
	};

	//好友申请记录(可用于好友申请列表)
	struct FriendRequest {
		QString requestId;          //申请唯一ID
		QString senderId;			//申请人用户ID
		QString senderAccount;		//申请人账号
		QString senderNickname;		//申请人昵称
		QString avatarPath;			//申请人头像路径
		QString verifymsg;			//验证消息(如我是某某某)
		FriendRequestStatus status = FriendRequestStatus::Pending; //处理状态
		qint64 timestamp = 0;        //申请时间(时间戳)
	};
	//应用设置配置(可用于：设置页面)
	struct AppConfig {
		bool darkTheme = false; // 深色模式/浅色模式
		int fontSize = 14;      // 字体大小 (对应滑块)
		QString savePath;       // 文件存储路径
		bool rememberPassword = false; // 记住密码
	};

}

//----------元类型注册(qt必须)----------
// 必须在 namespace 外部注册，否则信号槽无法传递自定义结构体
Q_DECLARE_METATYPE(LanChat::UserInfo)
Q_DECLARE_METATYPE(LanChat::Message)
Q_DECLARE_METATYPE(LanChat::ChatSession)
Q_DECLARE_METATYPE(LanChat::FriendRequest)


#endif // TYPES_H
