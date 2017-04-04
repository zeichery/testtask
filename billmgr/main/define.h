#ifndef DEFINE_H
#define DEFINE_H

#define BILLMGR_SQL_DUMP_FILE "/usr/local/mgr5/var/test_billmgr.sql"
#define BILLMGR_CONF_FILE "/usr/local/mgr5/etc/billmgr.conf"

#define RUSSIAN_RUBLE "RUB"
#define RUSSIAN_RUBLE_CODE "126"
#define EURO "EUR"
#define USD "USD"
#define RUSSIAN_COUNTRY "RU"
#define RUSSIAN_LANGCODE "ru"
#define SHORT_WAITER_EXISTING	15
#define LONG_WAITER_EXISTING	60

#ifdef INTEG
#define INTEG_INSTALL_WAITER 1200
#define INTEG_SHORT_WAITER 60
#define INTEG_SERVICE_DELETE_WAITER 180
#define INTEG_RO_EXISTING 10
#endif

#endif // DEFINE_H

