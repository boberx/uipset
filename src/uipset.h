#ifndef UIPSET_H
#define UIPSET_H

#include "build/uipset.pb.h"

#include "common.h"

#define UIPSET_CMD_ADD 1
#define UIPSET_CMD_DEL 2

/*!
*/
struct uipset_client
{
	int sock;
	int err;
};

/*!
*	\brief Создать подключение к серверу
*	\return 0 если всё норм
*/
bool uipset_start ( struct uipset_client*, const char* );

/*!
*	\brief Отправить команду на сервер
*	\return 0 если всё норм
*/
bool uipset_request ( struct uipset_client*, unsigned char, const char*, const char* );

/*!
*	\brief Отключиться от сервера
*/
void uipset_stop ( struct uipset_client* );

#endif
