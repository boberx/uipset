#ifndef IPSET_H
#define IPSET_H

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <libipset/session.h>
#include <libipset/types.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#include "debug.h"

/*!
*	\brief Добавить запись в набор
*	\param[in] set Имя набора
*	\param[in] ent Добавляемое значение
*	\return 1 если запись добавлена, 0 если ошибка
*/
int ipset_add ( const char* const set, const char* const ent );

/*!
*	\brief Удалить запись из набора
*	\warning Если записи в наборе нет, то считаем, что удалили
*	\param[in] set Имя набора
*	\param[in] ent Проверяемое значение
*	\return 1 если запись удалена (или записи нет в наборе), 0 если ошибка
*/
int ipset_del ( const char* const set, const char* const ent );

/*!
*	\brief Вывести содержимое набора
*	\param[in] set Имя набора
*	\param[out] lst Строка, содержащяя список значений, разделённых новой строкой (\n)
*	\warning переменная lst должна быть ощичена, после использования
*	\return 1 если список существует (если lst 0, то список пустой), 0 если не существует, < 0 если ошибка
*/
int ipset_lst ( const char* const set, const char** lst );

/*!
*	\brief Проверка наличия значения в наборе
*	\param[in] set Имя набора
*	\param[in] ent Проверяемое значение
*	\return 1 если найдено, 0 если не найдено, < 0 если ошибка
*/
int ipset_tst ( const char* const set, const char* const ent );

#endif
