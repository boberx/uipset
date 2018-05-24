#ifndef UIPSET_H
#define UIPSET_H

#include <stdio.h>
#include <stdarg.h>

#include "build/uipset.pb.h"

#include "common.h"

#define UIPSET_CMD_ADD 1
#define UIPSET_CMD_DEL 2
#define UIPSET_CMD_TST 3
#define UIPSET_CMD_LST 4

/*!
*	\brief описатель подключения к сервису
*/
struct uipset_client
{
	/*! сокет для работы с uipset */
	int m_sock;

	/*! последнне сообщение об ошибке */
	char m_lastmessage[1024];
};

/*!
*	\brief Создать подключение к серверу
*	\return 1 если всё норм
*/
int uipset_start ( struct uipset_client*, const char* );

/*!
*	\brief Отправить команду на сервер
*	\return 1 если запрос прошёл успешно, иначе 0
*/
int uipset_request ( struct uipset_client*, unsigned char, const char* const, const char* const, uipset_msg_response* );

/*!
*	\brief Отключиться от сервера
*/
void uipset_stop ( struct uipset_client* );

#endif
