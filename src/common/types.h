#ifndef TYPES_H
#define TYPES_H

#include <QString>
#include <QDateTime>
#include <QMetaType>
#include <QJsonObject>
#include <QJsonDocument>
#include <QList>

namespace LanChat {
	//----------ö�ٶ���----------
	//��Ϣ����
	enum class MessageType {
		Text = 0,   //�ı���Ϣ
		Image = 1,  //ͼƬ��Ϣ
		Voice = 2,  //������Ϣ
		System = 99 //ϵͳ��Ϣ
	};
	//��Ϣ״̬��UI��ʾ�ã�
	enum class MessageStatus {
		Pending = 0, //������(תȦloading)
		Sent = 1,    //�ѷ���
		Read = 2,    //�Ѷ�(��ɫ�Թ�)
		Failed = 3   //����ʧ��(��ɫ��̾��)
	};
	//�û�״̬
	enum class UserStatus {
		Offline = 0, //����(��ɫ���ĵ�)
		Online = 1,  //����(��ɫʵ�ĵ�)
		Busy = 2,    //æµ(��ɫʵ�ĵ�)
	};
	//�������봦��״̬
	enum class FriendRequestStatus {
		Pending = 0, //������(��ʾͬ�ⰴť)
		Accepted = 1,//�ѽ���(��ʾ������)
		Rejected = 2 //�Ѿܾ�
	};
	//----------ʵ��ṹ��----------
	//�û���ϸ��Ϣ(�����ڸ�������ҳ����ϵ������)
	struct UserInfo{
		QString userId;		//�û�ΨһID
		QString account;	//�û��˺�
		QString nickname;	//�û��ǳ�
		QString avatarPath;	//ͷ���ļ�·��
		QString email;		//��������
		QString phone;		//��ϵ�绰
		QString signature;	//����ǩ��
		UserStatus status = UserStatus::Offline;	//�û�״̬

		//���л������û���ϸ��ϢתJSON���͸������
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
		//�����л�������������JSON���û���ϸ��Ϣ
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
	//������Ϣʵ��(����������򡢱��ش洢)
	struct Message {
		QString messageId;      //��ϢΨһID
		QString senderId;       //�������û�ID
		QString receiverId;     //�������û�ID
		QString content;        //��Ϣ����(�ı���ͼƬ·��)
		MessageType type = MessageType::Text; //��Ϣ����
		MessageStatus status = MessageStatus::Pending; //��Ϣ״̬
		qint64 timestamp = 0;      //��Ϣ����ʱ��(ʱ���)

		//�����������ж��Ƿ����Լ����͵���Ϣ(������:UI������������������)
		bool isSelf(const QString& currentUserId) const {
			return senderId == currentUserId;
		}
		//���л�����������ϢתJSON���͸������
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
		//�����л�������������JSON��������Ϣ
		static Message fromJson(const QJsonObject& json) {
			Message msg;
			msg.messageId = json["messageId"].toString();
			msg.senderId = json["senderId"].toString();
			msg.receiverId = json["receiverId"].toString();
			msg.content = json["content"].toString();
			msg.type = static_cast<MessageType>(json["type"].toInt());
			msg.timestamp = json["timestamp"].toVariant().toLongLong();
			msg.status = MessageStatus::Sent; // �յ����˵���Ϣ��Ĭ��Ϊ�ѷ���
			return msg;
		}
	};
	//�Ự/��ϵ��ʵ��(��������ϵ���б�����Ϣ�б�)
	struct ChatSession {
	//������Ϣ
		QString userId;			//�Է�ID
		QString nickname;		//�Է��ǳ�
		QString remark;			//��ע��
		QString avatarPath;		//ͷ���ļ�·��
		UserStatus status = UserStatus::Offline; //�û�״̬

	//�Ự״̬��Ϣ
		QString lastMessage;		//���һ����Ϣ����
		qint64 lastTime = 0;		//�����Ϣʱ��(ʱ���)
		int unreadCount = 0;		//δ����Ϣ��
		bool isTop = false;			//�Ƿ��ö�

	//����������UI��ʾ������(������ʾ��ע������ǳ�)
		QString displayName() const {
			return remark.isEmpty() ? nickname : remark;
		}
	};

	//���������¼(�����ں��������б�)
	struct FriendRequest {
		QString requestId;          //����ΨһID
		QString senderId;			//�������û�ID
		QString senderAccount;		//�������˺�
		QString senderNickname;		//�������ǳ�
		QString avatarPath;			//������ͷ��·��
		QString verifymsg;			//��֤��Ϣ(������ĳĳĳ)
		FriendRequestStatus status = FriendRequestStatus::Pending; //����״̬
		qint64 timestamp = 0;        //����ʱ��(ʱ���)
	};
	//Ӧ����������(�����ڣ�����ҳ��)
	struct AppConfig {
		bool darkTheme = false; // ��ɫģʽ/ǳɫģʽ
		int fontSize = 14;      // �����С (��Ӧ����)
		QString savePath;       // �ļ��洢·��
		bool rememberPassword = false; // ��ס����
	};

}

//----------Ԫ����ע��(qt����)----------
// ������ namespace �ⲿע�ᣬ�����źŲ��޷������Զ���ṹ��
Q_DECLARE_METATYPE(LanChat::UserInfo)
Q_DECLARE_METATYPE(LanChat::Message)
Q_DECLARE_METATYPE(LanChat::ChatSession)
Q_DECLARE_METATYPE(LanChat::FriendRequest)


#endif // TYPES_H
