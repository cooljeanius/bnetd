#
# Copyright (C) 1999 Rob Crittenden (rcrit@greyoak.com)
# Copyright (C) 1999, 2000, 2002 Ross Combs (rocombs@cs.nmsu.edu)
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place: Suite 330, Boston, MA 02111-1307, USA.
#

ACLOCAL_AMFLAGS = -I m4 --install

AM_DISTCHECK_CONFIGURE_FLAGS = --disable-dependency-tracking

install_sh      = @install_sh@
prefix		= @prefix@
exec_prefix	= @exec_prefix@

bindir		= @bindir@
sbindir		= @sbindir@
mandir		= @mandir@
confdir		= @sysconfdir@/bnetd
sharedir	= @localstatedir@
filesdir	= @localstatedir@/files
usersdir	= @localstatedir@/users
reportsdir	= @localstatedir@/reports
chanlogsdir	= @localstatedir@/chanlogs
charinfodir     = @localstatedir@/charinfo
charsavedir     = @localstatedir@/charsave
ladderdir       = @localstatedir@/ladders
bak_charsavedir = @localstatedir@/bak/charsave
bak_charinfodir = @localstatedir@/bak/charinfo

SRC_SRCDIR	= @srcdir@
SRC_BINDIR	= @srcdir@/../bin
SRC_SBINDIR	= @srcdir@/../sbin
SRC_MANDIR	= @srcdir@/../man
SRC_CONFDIR	= @srcdir@/../conf
SRC_SHAREDIR	= @srcdir@/../files
SRC_SCRIPTDIR	= @srcdir@/../scripts

SRC_ACDIR	= ${SRC_SRCDIR}/autoconf
SRC_BNETDDIR	= ${SRC_SRCDIR}/bnetd
SRC_D2CSDIR     = ${SRC_SRCDIR}/d2cs
SRC_D2DBSDIR    = ${SRC_SRCDIR}/d2dbs
SRC_BNIUTILSDIR	= ${SRC_SRCDIR}/bniutils
SRC_BNPASSDIR	= ${SRC_SRCDIR}/bnpass
SRC_BNPROXYDIR	= ${SRC_SRCDIR}/bnproxy
SRC_BNTRACKDDIR	= ${SRC_SRCDIR}/bntrackd
SRC_CLIENTDIR	= ${SRC_SRCDIR}/client
SRC_COMMONDIR	= ${SRC_SRCDIR}/common
SRC_COMPATDIR	= ${SRC_SRCDIR}/compat
SRC_DEBUGDIR	= ${SRC_SRCDIR}/debug

RMF		= rm -f
RMRF		= rm -rf

RMDIR		= @RMDIR@

ECHO		= @ECHO@

SED		= @SED@

INSTALL		= @INSTALL@
INSTALL_PROGRAM	= @INSTALL_PROGRAM@
INSTALLdir      = @INSTALLdir@
INSTALL_DATA	= @INSTALL_DATA@
INSTALL_SCRIPT	= @INSTALL_SCRIPT@
INSTALL_DIRS	= @INSTALL@ -d -m 755

# CC set by configure
# CFLAGS set by configure
AM_CPPFLAGS	= @DEFS@ @DEFINES@ -DBNETD_DEFAULT_CONF_FILE=\"${confdir}/bnetd.conf\" -DD2CS_DEFAULT_CONF_FILE=\"${confdir}/d2cs.conf\" -DD2DBS_DEFAULT_CONF_FILE=\"${confdir}/d2dbs.conf\" @CPPFLAGS@ -I"${SRC_SRCDIR}"
COMPILE		= @CC@ @CFLAGS@ ${AM_CPPFLAGS} -c
PREPROCESS	= @CPP@ ${AM_CPPFLAGS}
LINK_MANUALLY	= @CC@ @CFLAGS@ @LDFLAGS@
LINK_LIBS	= @LIBS@

sbin_PROGRAMS	=
bin_PROGRAMS	=


bnetd_SBIN	= ${SRC_SBINDIR}/bnetd
sbin_PROGRAMS	+= bnetd/bnetd
bnetd_bnetd_SOURCES = \
bnetd/account_db_mysql.c \
bnetd/account.c \
bnetd/account_wrap.c \
bnetd/adbanner.c \
bnetd/alias_command.c \
bnetd/autoupdate.c \
bnetd/bits.c \
bnetd/bits_chat.c \
bnetd/bits_ext.c \
bnetd/bits_game.c \
bnetd/bits_login.c \
bnetd/bits_motd.c \
bnetd/bits_net.c \
bnetd/bits_packet.c \
bnetd/bits_query.c \
bnetd/bits_rconn.c \
bnetd/bits_va.c \
bnetd/channel.c \
bnetd/channel_conv.c \
bnetd/character.c \
bnetd/command.c \
bnetd/connection.c \
bnetd/file.c \
bnetd/game.c \
bnetd/game_conv.c \
bnetd/game_rule.c \
bnetd/gametrans.c \
bnetd/handle_auth.c \
bnetd/handle_bits.c \
bnetd/handle_bnet.c \
bnetd/handle_bot.c \
bnetd/handle_d2cs.c \
bnetd/handle_file.c \
bnetd/handle_init.c \
bnetd/handle_irc.c \
bnetd/handle_telnet.c \
bnetd/handle_udp.c \
bnetd/helpfile.c \
bnetd/ipban.c \
bnetd/irc.c \
bnetd/ladder.c \
bnetd/ladder_calc.c \
bnetd/mail.c \
bnetd/main.c \
bnetd/message.c \
bnetd/prefs.c \
bnetd/query.c \
bnetd/realm.c \
bnetd/runprog.c \
bnetd/server.c \
bnetd/tick.c \
bnetd/timer.c \
bnetd/tracker.c \
bnetd/udptest_send.c \
bnetd/versioncheck.c \
bnetd/watch.c \
bnetd/friends.c \
common/db_mysql.c \
common/addr.c \
common/bn_type.c \
common/bnethash.c \
common/bnethashconv.c \
common/bnettime.c \
common/check_alloc.c \
common/conftable.c \
common/eventlog.c \
common/give_up_root_privileges.c \
common/hashtable.c \
common/hexdump.c \
common/list.c \
common/network.c \
common/packet.c \
common/proginfo.c \
common/queue.c \
common/token.c \
common/util.c \
compat/difftime.c \
compat/gettimeofday.c \
compat/inet_aton.c \
compat/inet_ntoa.c \
compat/memcpy.c \
compat/memset.c \
compat/pdir.c \
compat/psock.c \
compat/strcasecmp.c \
compat/strdup.c \
compat/strncasecmp.c \
compat/strerror.c \
compat/strftime.c \
compat/strsep.c \
compat/strtoul.c \
compat/uname.c
bnetd_OBJS	= \
${SRC_BNETDDIR}/account_db_mysql.o \
${SRC_BNETDDIR}/account.o \
${SRC_BNETDDIR}/account_wrap.o \
${SRC_BNETDDIR}/adbanner.o \
${SRC_BNETDDIR}/alias_command.o \
${SRC_BNETDDIR}/autoupdate.o \
${SRC_BNETDDIR}/bits.o \
${SRC_BNETDDIR}/bits_chat.o \
${SRC_BNETDDIR}/bits_ext.o \
${SRC_BNETDDIR}/bits_game.o \
${SRC_BNETDDIR}/bits_login.o \
${SRC_BNETDDIR}/bits_motd.o \
${SRC_BNETDDIR}/bits_net.o \
${SRC_BNETDDIR}/bits_packet.o \
${SRC_BNETDDIR}/bits_query.o \
${SRC_BNETDDIR}/bits_rconn.o \
${SRC_BNETDDIR}/bits_va.o \
${SRC_BNETDDIR}/channel.o \
${SRC_BNETDDIR}/channel_conv.o \
${SRC_BNETDDIR}/character.o \
${SRC_BNETDDIR}/command.o \
${SRC_BNETDDIR}/connection.o \
${SRC_BNETDDIR}/file.o \
${SRC_BNETDDIR}/game.o \
${SRC_BNETDDIR}/game_conv.o \
${SRC_BNETDDIR}/game_rule.o \
${SRC_BNETDDIR}/gametrans.o \
${SRC_BNETDDIR}/handle_auth.o \
${SRC_BNETDDIR}/handle_bits.o \
${SRC_BNETDDIR}/handle_bnet.o \
${SRC_BNETDDIR}/handle_bot.o \
${SRC_BNETDDIR}/handle_d2cs.o \
${SRC_BNETDDIR}/handle_file.o \
${SRC_BNETDDIR}/handle_init.o \
${SRC_BNETDDIR}/handle_irc.o \
${SRC_BNETDDIR}/handle_telnet.o \
${SRC_BNETDDIR}/handle_udp.o \
${SRC_BNETDDIR}/helpfile.o \
${SRC_BNETDDIR}/ipban.o \
${SRC_BNETDDIR}/irc.o \
${SRC_BNETDDIR}/ladder.o \
${SRC_BNETDDIR}/ladder_calc.o \
${SRC_BNETDDIR}/mail.o \
${SRC_BNETDDIR}/main.o \
${SRC_BNETDDIR}/message.o \
${SRC_BNETDDIR}/prefs.o \
${SRC_BNETDDIR}/query.o \
${SRC_BNETDDIR}/realm.o \
${SRC_BNETDDIR}/runprog.o \
${SRC_BNETDDIR}/server.o \
${SRC_BNETDDIR}/tick.o \
${SRC_BNETDDIR}/timer.o \
${SRC_BNETDDIR}/tracker.o \
${SRC_BNETDDIR}/udptest_send.o \
${SRC_BNETDDIR}/versioncheck.o \
${SRC_BNETDDIR}/watch.o \
${SRC_BNETDDIR}/friends.o \
${SRC_COMMONDIR}/db_mysql.o \
${SRC_COMMONDIR}/addr.o \
${SRC_COMMONDIR}/bn_type.o \
${SRC_COMMONDIR}/bnethash.o \
${SRC_COMMONDIR}/bnethashconv.o \
${SRC_COMMONDIR}/bnettime.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/conftable.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMMONDIR}/give_up_root_privileges.o \
${SRC_COMMONDIR}/hashtable.o \
${SRC_COMMONDIR}/hexdump.o \
${SRC_COMMONDIR}/list.o \
${SRC_COMMONDIR}/network.o \
${SRC_COMMONDIR}/packet.o \
${SRC_COMMONDIR}/proginfo.o \
${SRC_COMMONDIR}/queue.o \
${SRC_COMMONDIR}/token.o \
${SRC_COMMONDIR}/util.o \
${SRC_COMPATDIR}/difftime.o \
${SRC_COMPATDIR}/gettimeofday.o \
${SRC_COMPATDIR}/inet_aton.o \
${SRC_COMPATDIR}/inet_ntoa.o \
${SRC_COMPATDIR}/memcpy.o \
${SRC_COMPATDIR}/memset.o \
${SRC_COMPATDIR}/pdir.o \
${SRC_COMPATDIR}/psock.o \
${SRC_COMPATDIR}/strcasecmp.o \
${SRC_COMPATDIR}/strdup.o \
${SRC_COMPATDIR}/strncasecmp.o \
${SRC_COMPATDIR}/strerror.o \
${SRC_COMPATDIR}/strftime.o \
${SRC_COMPATDIR}/strsep.o \
${SRC_COMPATDIR}/strtoul.o \
${SRC_COMPATDIR}/uname.o

d2cs_SBIN       = ${SRC_SBINDIR}/d2cs
sbin_PROGRAMS	+= d2cs/d2cs
d2cs_d2cs_SOURCES = \
d2cs/bnetd.c \
d2cs/cmdline_parse.c \
d2cs/conf.c \
d2cs/connection.c \
d2cs/d2char_db_mysql.c \
d2cs/d2charfile.c \
d2cs/d2gs.c \
d2cs/d2ladder.c \
d2cs/game.c \
d2cs/gamequeue.c \
d2cs/handle_bnetd.c \
d2cs/handle_d2cs_db_mysql.c \
d2cs/handle_d2cs.c \
d2cs/handle_d2gs.c \
d2cs/handle_init.c \
d2cs/handle_signal.c \
d2cs/db_table_mysql.c \
d2cs/main.c \
d2cs/net.c \
d2cs/prefs.c \
d2cs/s2s.c \
d2cs/server.c \
d2cs/serverqueue.c \
d2cs/xstring.c \
common/db_mysql.c \
common/addr.c \
common/bn_type.c \
common/check_alloc.c \
common/eventlog.c \
common/hashtable.c \
common/hexdump.c \
common/list.c \
common/network.c \
common/packet.c \
common/queue.c \
common/util.c \
compat/difftime.c \
compat/gettimeofday.c \
compat/inet_aton.c \
compat/inet_ntoa.c \
compat/memcpy.c \
compat/memmove.c \
compat/memset.c \
compat/pdir.c \
compat/psock.c \
compat/strcasecmp.c \
compat/strdup.c \
compat/strncasecmp.c \
compat/strerror.c \
compat/strftime.c \
compat/strsep.c \
compat/strtoul.c \
compat/uname.c
d2cs_OBJS	= \
${SRC_D2CSDIR}/bnetd.o \
${SRC_D2CSDIR}/cmdline_parse.o \
${SRC_D2CSDIR}/conf.o \
${SRC_D2CSDIR}/connection.o \
${SRC_D2CSDIR}/d2char_db_mysql.o \
${SRC_D2CSDIR}/d2charfile.o \
${SRC_D2CSDIR}/d2gs.o \
${SRC_D2CSDIR}/d2ladder.o \
${SRC_D2CSDIR}/game.o \
${SRC_D2CSDIR}/gamequeue.o \
${SRC_D2CSDIR}/handle_bnetd.o \
${SRC_D2CSDIR}/handle_d2cs_db_mysql.o \
${SRC_D2CSDIR}/handle_d2cs.o \
${SRC_D2CSDIR}/handle_d2gs.o \
${SRC_D2CSDIR}/handle_init.o \
${SRC_D2CSDIR}/handle_signal.o \
${SRC_D2CSDIR}/db_table_mysql.o \
${SRC_D2CSDIR}/main.o \
${SRC_D2CSDIR}/net.o \
${SRC_D2CSDIR}/prefs.o \
${SRC_D2CSDIR}/s2s.o \
${SRC_D2CSDIR}/server.o \
${SRC_D2CSDIR}/serverqueue.o \
${SRC_D2CSDIR}/xstring.o \
${SRC_COMMONDIR}/db_mysql.o \
${SRC_COMMONDIR}/addr.o \
${SRC_COMMONDIR}/bn_type.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMMONDIR}/hashtable.o \
${SRC_COMMONDIR}/hexdump.o \
${SRC_COMMONDIR}/list.o \
${SRC_COMMONDIR}/network.o \
${SRC_COMMONDIR}/packet.o \
${SRC_COMMONDIR}/queue.o \
${SRC_COMMONDIR}/util.o \
${SRC_COMPATDIR}/difftime.o \
${SRC_COMPATDIR}/gettimeofday.o \
${SRC_COMPATDIR}/inet_aton.o \
${SRC_COMPATDIR}/inet_ntoa.o \
${SRC_COMPATDIR}/memcpy.o \
${SRC_COMPATDIR}/memmove.o \
${SRC_COMPATDIR}/memset.o \
${SRC_COMPATDIR}/pdir.o \
${SRC_COMPATDIR}/psock.o \
${SRC_COMPATDIR}/strcasecmp.o \
${SRC_COMPATDIR}/strdup.o \
${SRC_COMPATDIR}/strncasecmp.o \
${SRC_COMPATDIR}/strerror.o \
${SRC_COMPATDIR}/strftime.o \
${SRC_COMPATDIR}/strsep.o \
${SRC_COMPATDIR}/strtoul.o \
${SRC_COMPATDIR}/uname.o

d2dbs_SBIN      = ${SRC_SBINDIR}/d2dbs
sbin_PROGRAMS	+= d2dbs/d2dbs
d2dbs_d2dbs_SOURCES = \
d2dbs/d2ladder.c \
d2dbs/dbserver.c \
d2dbs/dbspacket.c \
d2dbs/charlock.c \
d2dbs/cmdline_parse.c \
d2dbs/handle_signal.c \
d2dbs/main.c \
d2dbs/prefs.c \
d2dbs/server.c \
d2cs/conf.c \
d2cs/xstring.c \
d2cs/d2char_db_mysql.c \
common/bn_type.c \
common/check_alloc.c \
common/eventlog.c \
common/list.c \
common/util.c \
common/db_mysql.c \
compat/psock.c \
compat/strerror.c \
compat/strsep.c
d2dbs_OBJS	= \
${SRC_D2DBSDIR}/d2ladder.o \
${SRC_D2DBSDIR}/dbserver.o \
${SRC_D2DBSDIR}/dbspacket.o \
${SRC_D2DBSDIR}/charlock.o \
${SRC_D2DBSDIR}/cmdline_parse.o \
${SRC_D2DBSDIR}/handle_signal.o \
${SRC_D2DBSDIR}/main.o \
${SRC_D2DBSDIR}/prefs.o \
${SRC_D2DBSDIR}/server.o \
${SRC_D2CSDIR}/conf.o \
${SRC_D2CSDIR}/xstring.o \
${SRC_D2CSDIR}/d2char_db_mysql.o \
${SRC_COMMONDIR}/bn_type.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMMONDIR}/list.o \
${SRC_COMMONDIR}/util.o \
${SRC_COMMONDIR}/db_mysql.o \
${SRC_COMPATDIR}/psock.o \
${SRC_COMPATDIR}/strerror.o \
${SRC_COMPATDIR}/strsep.o

sbnetd_SBIN	= ${SRC_SBINDIR}/sbnetd
sbin_PROGRAMS	+= sbnetd
sbnetd_SOURCES	= \
debug/sbnetd.c \
compat/strerror.c
sbnetd_OBJS	= \
${SRC_DEBUGDIR}/sbnetd.o \
${SRC_COMPATDIR}/strerror.o

bnproxy_SBIN	= ${SRC_SBINDIR}/bnproxy
sbin_PROGRAMS	+= bnproxy/bnproxy
bnproxy_bnproxy_SOURCES	= \
bnproxy/bnproxy.c \
bnproxy/virtconn.c \
common/bn_type.c \
common/check_alloc.c \
common/eventlog.c \
common/hexdump.c \
common/list.c \
common/network.c \
common/packet.c \
common/queue.c \
common/util.c \
compat/inet_ntoa.c \
compat/psock.c \
compat/strerror.c
bnproxy_OBJS	= \
${SRC_BNPROXYDIR}/bnproxy.o \
${SRC_BNPROXYDIR}/virtconn.o \
${SRC_COMMONDIR}/bn_type.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMMONDIR}/hexdump.o \
${SRC_COMMONDIR}/list.o \
${SRC_COMMONDIR}/network.o \
${SRC_COMMONDIR}/packet.o \
${SRC_COMMONDIR}/queue.o \
${SRC_COMMONDIR}/util.o \
${SRC_COMPATDIR}/inet_ntoa.o \
${SRC_COMPATDIR}/psock.o \
${SRC_COMPATDIR}/strerror.o

bntrackd_SBIN	= ${SRC_SBINDIR}/bntrackd
sbin_PROGRAMS	+= bntrackd/bntrackd
bntrackd_bntrackd_SOURCES= \
bntrackd/bntrackd.c \
common/check_alloc.c \
common/eventlog.c \
common/list.c \
common/util.c \
compat/inet_ntoa.c \
compat/memset.c \
compat/psock.c \
compat/strerror.c
bntrackd_OBJS= \
${SRC_BNTRACKDDIR}/bntrackd.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMMONDIR}/list.o \
${SRC_COMMONDIR}/util.o \
${SRC_COMPATDIR}/inet_ntoa.o \
${SRC_COMPATDIR}/memset.o \
${SRC_COMPATDIR}/psock.o \
${SRC_COMPATDIR}/strerror.o

bnchat_BIN	= ${SRC_BINDIR}/bnchat
bin_PROGRAMS	+= bnchat
bnchat_SOURCES	= \
client/bnchat.c \
client/client.c \
client/client_connect.c \
client/udptest.c \
common/bn_type.c \
common/bnethash.c \
common/bnethashconv.c \
common/bnettime.c \
common/check_alloc.c \
common/eventlog.c \
common/network.c \
common/packet.c \
common/proginfo.c \
common/queue.c \
common/util.c \
compat/gettimeofday.c \
compat/inet_ntoa.c \
compat/memset.c \
compat/psock.c \
compat/strerror.c
bnchat_OBJS	= \
${SRC_CLIENTDIR}/bnchat.o \
${SRC_CLIENTDIR}/client.o \
${SRC_CLIENTDIR}/client_connect.o \
${SRC_CLIENTDIR}/udptest.o \
${SRC_COMMONDIR}/bn_type.o \
${SRC_COMMONDIR}/bnethash.o \
${SRC_COMMONDIR}/bnethashconv.o \
${SRC_COMMONDIR}/bnettime.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMMONDIR}/network.o \
${SRC_COMMONDIR}/packet.o \
${SRC_COMMONDIR}/proginfo.o \
${SRC_COMMONDIR}/queue.o \
${SRC_COMMONDIR}/util.o \
${SRC_COMPATDIR}/gettimeofday.o \
${SRC_COMPATDIR}/inet_ntoa.o \
${SRC_COMPATDIR}/memset.o \
${SRC_COMPATDIR}/psock.o \
${SRC_COMPATDIR}/strerror.o

bnpass_BIN	= ${SRC_BINDIR}/bnpass
bin_PROGRAMS	+= bnpass/bnpass
bnpass_bnpass_SOURCES = \
bnpass/bnpass.c \
common/bnethash.c \
common/check_alloc.c \
common/eventlog.c \
compat/strerror.c
bnpass_OBJS	= \
${SRC_BNPASSDIR}/bnpass.o \
${SRC_COMMONDIR}/bnethash.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMPATDIR}/strerror.o

bnftp_BIN	= ${SRC_BINDIR}/bnftp
bin_PROGRAMS	+= bnftp
bnftp_SOURCES	= \
client/bnftp.c \
client/client.c \
common/bn_type.c \
common/bnettime.c \
common/check_alloc.c \
common/eventlog.c \
common/hexdump.c \
common/network.c \
common/packet.c \
common/queue.c \
common/util.c \
compat/gettimeofday.c \
compat/inet_ntoa.c \
compat/psock.c \
compat/strerror.c \
compat/strftime.c
bnftp_OBJS	= \
${SRC_CLIENTDIR}/bnftp.o \
${SRC_CLIENTDIR}/client.o \
${SRC_COMMONDIR}/bn_type.o \
${SRC_COMMONDIR}/bnettime.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMMONDIR}/hexdump.o \
${SRC_COMMONDIR}/network.o \
${SRC_COMMONDIR}/packet.o \
${SRC_COMMONDIR}/queue.o \
${SRC_COMMONDIR}/util.o \
${SRC_COMPATDIR}/gettimeofday.o \
${SRC_COMPATDIR}/inet_ntoa.o \
${SRC_COMPATDIR}/psock.o \
${SRC_COMPATDIR}/strerror.o \
${SRC_COMPATDIR}/strftime.o

bnbot_BIN	= ${SRC_BINDIR}/bnbot
bin_PROGRAMS	+= bnbot
bnbot_SOURCES	= \
client/bnbot.c \
client/client.c \
common/bn_type.c \
common/check_alloc.c \
common/eventlog.c \
common/network.c \
common/packet.c \
common/util.c \
compat/inet_ntoa.c \
compat/psock.c \
compat/strerror.c
bnbot_OBJS	= \
${SRC_CLIENTDIR}/bnbot.o \
${SRC_CLIENTDIR}/client.o \
${SRC_COMMONDIR}/bn_type.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMMONDIR}/network.o \
${SRC_COMMONDIR}/packet.o \
${SRC_COMMONDIR}/util.o \
${SRC_COMPATDIR}/inet_ntoa.o \
${SRC_COMPATDIR}/psock.o \
${SRC_COMPATDIR}/strerror.o

bnstat_BIN	= ${SRC_BINDIR}/bnstat
bin_PROGRAMS	+= bnstat
bnstat_SOURCES	= \
client/bnstat.c \
client/client.c \
client/client_connect.c \
client/udptest.c \
common/bn_type.c \
common/bnethash.c \
common/bnethashconv.c \
common/bnettime.c \
common/check_alloc.c \
common/eventlog.c \
common/network.c \
common/packet.c \
common/proginfo.c \
common/queue.c \
common/util.c \
compat/gettimeofday.c \
compat/inet_ntoa.c \
compat/memset.c \
compat/psock.c \
compat/strerror.c \
compat/strftime.c
bnstat_OBJS	= \
${SRC_CLIENTDIR}/bnstat.o \
${SRC_CLIENTDIR}/client.o \
${SRC_CLIENTDIR}/client_connect.o \
${SRC_CLIENTDIR}/udptest.o \
${SRC_COMMONDIR}/bn_type.o \
${SRC_COMMONDIR}/bnethash.o \
${SRC_COMMONDIR}/bnethashconv.o \
${SRC_COMMONDIR}/bnettime.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMMONDIR}/network.o \
${SRC_COMMONDIR}/packet.o \
${SRC_COMMONDIR}/proginfo.o \
${SRC_COMMONDIR}/queue.o \
${SRC_COMMONDIR}/util.o \
${SRC_COMPATDIR}/gettimeofday.o \
${SRC_COMPATDIR}/inet_ntoa.o \
${SRC_COMPATDIR}/memset.o \
${SRC_COMPATDIR}/psock.o \
${SRC_COMPATDIR}/strerror.o \
${SRC_COMPATDIR}/strftime.o

bnilist_BIN	= ${SRC_BINDIR}/bnilist
bin_PROGRAMS	+= bnilist
bnilist_SOURCES	= \
bniutils/bnilist.c \
bniutils/fileio.c \
bniutils/tga.c \
common/check_alloc.c \
compat/memcpy.c
bnilist_OBJS	= \
${SRC_BNIUTILSDIR}/bnilist.o \
${SRC_BNIUTILSDIR}/fileio.o \
${SRC_BNIUTILSDIR}/tga.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMPATDIR}/memcpy.o

bni2tga_BIN	= ${SRC_BINDIR}/bni2tga
bin_PROGRAMS	+= bni2tga
bni2tga_SOURCES	= \
bniutils/bni2tga.c \
bniutils/fileio.c \
common/check_alloc.c
bni2tga_OBJS	= \
${SRC_BNIUTILSDIR}/bni2tga.o \
${SRC_BNIUTILSDIR}/fileio.o \
${SRC_COMMONDIR}/check_alloc.o

bniextract_BIN	= ${SRC_BINDIR}/bniextract
bin_PROGRAMS	+= bniextract
bniextract_SOURCES= \
bniutils/bni.c \
bniutils/bniextract.c \
bniutils/fileio.c \
bniutils/tga.c \
common/check_alloc.c \
compat/memcpy.c
bniextract_OBJS= \
${SRC_BNIUTILSDIR}/bni.o \
${SRC_BNIUTILSDIR}/bniextract.o \
${SRC_BNIUTILSDIR}/fileio.o \
${SRC_BNIUTILSDIR}/tga.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMPATDIR}/memcpy.o

bnibuild_BIN	= ${SRC_BINDIR}/bnibuild
bin_PROGRAMS	+= bnibuild
bnibuild_SOURCES= \
bniutils/bni.c \
bniutils/bnibuild.c \
bniutils/fileio.c \
bniutils/tga.c \
common/check_alloc.c \
compat/memcpy.c
bnibuild_OBJS= \
${SRC_BNIUTILSDIR}/bni.o \
${SRC_BNIUTILSDIR}/bnibuild.o \
${SRC_BNIUTILSDIR}/fileio.o \
${SRC_BNIUTILSDIR}/tga.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMPATDIR}/memcpy.o

tgainfo_BIN	= ${SRC_BINDIR}/tgainfo
bin_PROGRAMS	+= tgainfo
tgainfo_SOURCES	= \
bniutils/fileio.c \
bniutils/tga.c \
bniutils/tgainfo.c \
common/check_alloc.c \
compat/memcpy.c
tgainfo_OBJS	= \
${SRC_BNIUTILSDIR}/fileio.o \
${SRC_BNIUTILSDIR}/tga.o \
${SRC_BNIUTILSDIR}/tgainfo.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMPATDIR}/memcpy.o

timetest_BIN	= ${SRC_BINDIR}/timetest
bin_PROGRAMS	+= timetest
timetest_SOURCES= \
debug/timetest.c \
common/bn_type.c \
common/bnettime.c \
common/check_alloc.c \
common/eventlog.c \
compat/gettimeofday.c \
compat/strerror.c
timetest_OBJS= \
${SRC_DEBUGDIR}/timetest.o \
${SRC_COMMONDIR}/bn_type.o \
${SRC_COMMONDIR}/bnettime.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMPATDIR}/gettimeofday.o \
${SRC_COMPATDIR}/strerror.o

listtest_BIN	= ${SRC_BINDIR}/listtest
bin_PROGRAMS	+= listtest
listtest_SOURCES= \
debug/listtest.c \
common/check_alloc.c \
common/eventlog.c \
common/list.c \
compat/strdup.c \
compat/strerror.c
listtest_OBJS= \
${SRC_DEBUGDIR}/listtest.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMMONDIR}/list.o \
${SRC_COMPATDIR}/strdup.o \
${SRC_COMPATDIR}/strerror.o

hashtabletest_BIN	= ${SRC_BINDIR}/hashtabletest
bin_PROGRAMS		+= hashtabletest
hashtabletest_SOURCES	= \
debug/hashtabletest.c \
common/check_alloc.c \
common/eventlog.c \
common/hashtable.c \
compat/strdup.c \
compat/strerror.c
hashtabletest_OBJS	= \
${SRC_DEBUGDIR}/hashtabletest.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMMONDIR}/hashtable.o \
${SRC_COMPATDIR}/strdup.o \
${SRC_COMPATDIR}/strerror.o

addrtest_BIN	= ${SRC_BINDIR}/addrtest
bin_PROGRAMS	+= addrtest
addrtest_SOURCES= \
debug/addrtest.c \
common/addr.c \
common/check_alloc.c \
common/eventlog.c \
common/list.c \
common/util.c \
compat/inet_aton.c \
compat/inet_ntoa.c \
compat/memset.c \
compat/memcpy.c \
compat/psock.c \
compat/strdup.c \
compat/strerror.c
addrtest_OBJS= \
${SRC_DEBUGDIR}/addrtest.o \
${SRC_COMMONDIR}/addr.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMMONDIR}/list.o \
${SRC_COMMONDIR}/util.o \
${SRC_COMPATDIR}/inet_aton.o \
${SRC_COMPATDIR}/inet_ntoa.o \
${SRC_COMPATDIR}/memset.o \
${SRC_COMPATDIR}/memcpy.o \
${SRC_COMPATDIR}/psock.o \
${SRC_COMPATDIR}/strdup.o \
${SRC_COMPATDIR}/strerror.o

hashtest_BIN	= ${SRC_BINDIR}/hashtest
bin_PROGRAMS	+= hashtest
hashtest_SOURCES= \
debug/hashtest.c \
common/check_alloc.c \
common/eventlog.c \
compat/strdup.c \
compat/strerror.c
hashtest_OBJS= \
${SRC_DEBUGDIR}/hashtest.o \
${SRC_COMMONDIR}/check_alloc.o \
${SRC_COMMONDIR}/eventlog.o \
${SRC_COMPATDIR}/strdup.o \
${SRC_COMPATDIR}/strerror.o

OBJS		= ${bnetd_OBJS} ${bnproxy_OBJS} ${d2cs_OBJS} ${d2dbs_OBJS} ${bntrackd_OBJS} ${bnchat_OBJS} ${bnpass_OBJS} ${bnbot_OBJS} ${bnftp_OBJS} ${bnstat_OBJS} ${bnilist_OBJS} ${bni2tga_OBJS} ${bniextract_OBJS} ${bnibuild_OBJS} ${tgainfo_OBJS} ${sbnetd_OBJS} ${timetest_OBJS} ${listtest_OBJS} ${hashtabletest_OBJS} ${addrtest_OBJS} ${hashtest_OBJS}
SBINS           = ${bnetd_SBIN} ${bntrackd_SBIN} ${d2cs_SBIN} ${d2dbs_SBIN}
BINS            = ${bnchat_BIN} ${bnpass_BIN} ${bnftp_BIN} ${bnbot_BIN} ${bnstat_BIN} ${bnilist_BIN} ${bni2tga_BIN} ${bniextract_BIN} ${bnibuild_BIN} ${tgainfo_BIN}
MAN1S		= bnetd.1 bnproxy.1 bntrackd.1 bnchat.1 bnpass.1 bnbot.1 bnftp.1 bnstat.1 bnetd.1 bni2tga.1 bnibuild.1 bniextract.1 bnilist.1 tgainfo.1
MAN5S		= bnetd.conf.5 bntext.5 bnmotd.txt.5 bnissue.txt.5 bnnews.txt.5
CONFS		= ad.list bnetd.conf bnetd_default_user bnwelcome.txt bnmotd.txt bnbitsmotd.txt bnissue.txt channel.list news.txt bnban bnhelp bnbitspasswd gametrans autoupdate versioncheck realm.list d2cs.conf d2dbs.conf
SHARES		= ad000001.pcx icons.bni tos.txt newbie.save IX86ver1.mpq bnserver.ini bnserver-D2DV.ini
OTHERRM		= ${bnproxy_SBIN} ${sbnetd_SBIN} ${timetest_BIN} ${listtest_BIN} ${hashtabletest_BIN} ${addrtest_BIN} ${hashtest_BIN} ${SRC_ACDIR}/ansi2knr ${SRC_ACDIR}/ansi2knr.o @ANSI2KNR_TMP@
DIRS		= ${SRC_BINDIR} ${SRC_SBINDIR}

# common dependencies for ${OBJS} cannot be expressed this way
# portably, unfortunately...
common_dependencies_for_objects: ${SRC_SRCDIR}/Makefile ${SRC_SRCDIR}/config.h ${SRC_COMMONDIR}/version.h ${SRC_COMMONDIR}/setup_before.h ${SRC_COMMONDIR}/setup_after.h ${SRC_ACDIR}/ansi2knr
.PHONY: common_dependencies_for_objects

.PHONY: all
all: @ANSI2KNR_DEP@ config ${SBINS} ${BINS}

# Hack: these should not be treated as targets in the first place:
.PHONY: -c -m 644

.PHONY: clean-local
clean-local:
	-${RMF} core
	-${RMF} ${OBJS}
	-${RMF} ${SBINS}
	-${RMF} ${BINS}
	-${RMF} ${OTHERRM}
	-${RMRF} *.dSYM || ${RMDIR} *.dSYM
	-${RMRF} ./*/*.dSYM || ${RMDIR} ./*/*.dSYM

CLEANFILES = a.out

.PHONY: distclean-local
distclean-local: clean-local
	-${RMF} ${SRC_CONFDIR}/bnetd.conf
	-${RMF} ${SRC_CONFDIR}/d2cs.conf
	-${RMF} ${SRC_CONFDIR}/d2dbs.conf
	-${RMF} ${SRC_SRCDIR}/config.cache ${SRC_SRCDIR}/config.log
	-${RMF} ${SRC_SRCDIR}/config.status
	-${RMF} ${SRC_SRCDIR}/Makefile ${SRC_SRCDIR}/config.h
	-${RMRF} ${SRC_SRCDIR}/autom4te.cache || ${RMDIR} autom4te.cache
	-${RMRF} ${DIRS} || ${RMDIR} ${DIRS}
	-find . -name .DS_Store -delete

DISTCLEANFILES = .DS_Store */.DS_Store *~ autoscan.log

.PHONY: clean-compile
clean-compile: clean-local all

.PHONY: config
config: ${SRC_SRCDIR}/Makefile ${SRC_SRCDIR}/config.h

if SRCDIR_IS_ELSEWHERE
${SRC_SRCDIR}/Makefile:
	if test ! -e ${SRC_SRCDIR}/Makefile; then \
	  if test -w ${SRC_SRCDIR}; then \
	    if test -e $(srcdir)/Makefile; then \
	      cp -v $(srcdir)/Makefile ${SRC_SRCDIR}; \
	    elif test -e $(builddir)/Makefile; then \
	      cp -v $(builddir)/Makefile ${SRC_SRCDIR}; \
	    elif test -e $(top_srcdir)/Makefile; then \
	      cp -v $(top_srcdir)/Makefile ${SRC_SRCDIR}; \
	    elif test -e $(top_builddir)/Makefile; then \
	      cp -v $(top_builddir)/Makefile ${SRC_SRCDIR}; \
	    elif test -x ${SRC_SRCDIR}/config.status; then \
	      pushd ${SRC_SRCDIR} && ./config.status && popd; \
	    elif test -r Makefile && test -n "$(MAKE)"; then \
	      $(MAKE) -f Makefile rebuild_Makefile; \
	    else \
	      ${ECHO} >> ${SRC_SRCDIR}/Makefile; \
	    fi; \
	  elif test -r ${SRC_SRCDIR}; then \
	    ls ${SRC_SRCDIR}; \
	  elif test -e ${SRC_SRCDIR}; then \
	    stat ${SRC_SRCDIR}; \
	  else \
	    ls; \
	  fi; \
	fi

${SRC_SRCDIR}/config.h:
	if test ! -e ${SRC_SRCDIR}/config.h; then \
	  if test -w ${SRC_SRCDIR}; then \
	    if test -e $(srcdir)/config.h; then \
	      cp -v $(srcdir)/config.h ${SRC_SRCDIR}; \
	    elif test -e $(builddir)/config.h; then \
	      cp -v $(builddir)/config.h ${SRC_SRCDIR}; \
	    elif test -e $(top_srcdir)/config.h; then \
	      cp -v $(top_srcdir)/config.h ${SRC_SRCDIR}; \
	    elif test -e $(top_builddir)/config.h; then \
	      cp -v $(top_builddir)/config.h ${SRC_SRCDIR}; \
	    elif test -x ${SRC_SRCDIR}/config.status; then \
	      pushd ${SRC_SRCDIR} && ./config.status && popd; \
	    elif test -r Makefile && test -n "$(MAKE)"; then \
	      $(MAKE) -f Makefile rebuild_config.h; \
	    else \
	      ${ECHO} >> ${SRC_SRCDIR}/config.h; \
	    fi; \
	  elif test -r ${SRC_SRCDIR}; then \
	    ls ${SRC_SRCDIR}; \
	  elif test -e ${SRC_SRCDIR}; then \
	    stat ${SRC_SRCDIR}; \
	  else \
	    ls; \
	  fi; \
	fi
endif

.PHONY: rebuild_configure
rebuild_configure: ${SRC_SRCDIR}/configure.ac
	@${ECHO} "The configure script is out of date; rebuilding..."
	cd ${SRC_SRCDIR} && autoconf
	@${ECHO} >> ${SRC_SRCDIR}/config.h.in
	@${ECHO} >> $@

.PHONY: rebuild_config.h.in
rebuild_config.h.in: ${SRC_SRCDIR}/configure.ac
	@${ECHO} "The config.h.in header is out of date; rebuilding..."
	cd ${SRC_SRCDIR} && autoheader
	@${ECHO} >> $@
	@${ECHO} >> ${SRC_SRCDIR}/configure

.PHONY: rebuild_config.status
rebuild_config.status: ${SRC_SRCDIR}/configure
	@${ECHO} "The config.status script is out of date; rechecking..."
	${SRC_SRCDIR}/config.status --recheck
	@${ECHO} >> $@

.PHONY: rebuild_config.h
rebuild_config.h: ${SRC_SRCDIR}/config.h.in ${SRC_SRCDIR}/config.status
	@${ECHO} "The config.h header is out of date; rewriting..."
	${SRC_SRCDIR}/config.status
	@${ECHO} >> ${SRC_SRCDIR}/Makefile
	@${ECHO} >> $@

.PHONY: rebuild_Makefile
rebuild_Makefile: ${SRC_SRCDIR}/Makefile.in ${SRC_SRCDIR}/config.status
	@${ECHO} "The Makefile is out of date; rewriting..."
	${SRC_SRCDIR}/config.status
	@${ECHO} >> ${SRC_SRCDIR}/config.h
	@${ECHO} >> $@

${SRC_BINDIR}:
	${INSTALL_DIRS} $@

${SRC_SBINDIR}:
	${INSTALL_DIRS} $@

.PHONY: install-all-bnetd-things
install-all-bnetd-things: install-sbin install-bin install-man install-conf install-share

.PHONY: install-server
install-server: install-sbin install-conf install-share

.PHONY: install-bnetd
install-bnetd: ${bnetd_SBIN}
	${INSTALL_DIRS} ${sbindir}
	@list='${bnetd_SBIN}'; for p in $$list; do \
        if test -f $$p; then \
          ${ECHO} "  ${INSTALL_PROGRAM} $$p ${sbindir}"; \
          ${INSTALL_PROGRAM} $$p ${sbindir}; \
        else :; fi; \
	done

.PHONY: install-bnproxy
install-bnproxy: ${bnproxy_SBIN}
	${INSTALL_DIRS} ${sbindir}
	@list='${bnproxy_SBIN}'; for p in $$list; do \
	  if test -f $$p; then \
	    ${ECHO} "  ${INSTALL_PROGRAM} $$p ${sbindir}"; \
	    ${INSTALL_PROGRAM} $$p ${sbindir}; \
	  else :; fi; \
	done

.PHONY: install-bntrackd
install-bntrackd: ${bntrackd_SBIN}
	${INSTALL_DIRS} ${sbindir}
	@list='${bntrackd_SBIN}'; for p in $$list; do \
	  if test -f $$p; then \
	    ${ECHO} "  ${INSTALL_PROGRAM} $$p ${sbindir}"; \
	    ${INSTALL_PROGRAM} $$p ${sbindir}; \
	  else :; fi; \
	done

.PHONY: install-sbin
install-sbin: ${SBINS}
	${INSTALL_DIRS} ${sbindir}
	@list='${SBINS}'; for p in $$list; do \
	  if test -f $$p; then \
	    ${ECHO} "  ${INSTALL_PROGRAM} $$p ${sbindir}"; \
	    ${INSTALL_PROGRAM} $$p ${sbindir}; \
	  else :; fi; \
	done

.PHONY: install-bin
install-bin: ${BINS}
	${INSTALL_DIRS} ${bindir}
	@list='${BINS}'; for p in $$list; do \
	  if test -f $$p; then \
	    ${ECHO} "  ${INSTALL_PROGRAM} $$p ${bindir}"; \
	    ${INSTALL_PROGRAM} $$p ${bindir}; \
	  else :; fi; \
	done

.PHONY: install-man-bnetd
install-man-bnetd:
	${INSTALL_DIRS} ${mandir} ${mandir}/man1 ${mandir}/man5
	@list='${MAN1S}'; for p in $$list; do \
	  if test -f ${SRC_MANDIR}/$$p; then \
	    ${ECHO} "  ${INSTALL_DATA} ${SRC_MANDIR}/$$p ${mandir}/man1"; \
	    ${INSTALL_DATA} ${SRC_MANDIR}/$$p ${mandir}/man1; \
	  else :; fi; \
	done
	@list='${MAN5S}'; for p in $$list; do \
	  if test -f ${SRC_MANDIR}/$$p; then \
	    ${ECHO} "  ${INSTALL_DATA} ${SRC_MANDIR}/$$p ${mandir}/man5"; \
	    ${INSTALL_DATA} ${SRC_MANDIR}/$$p ${mandir}/man5; \
	  else :; fi; \
	done

.PHONY: install-conf
install-conf: ${SRC_CONFDIR}/bnetd.conf ${SRC_CONFDIR}/d2cs.conf ${SRC_CONFDIR}/d2dbs.conf
	${INSTALL_DIRS} ${confdir}
	@list='${CONFS}'; for p in $$list; do \
	  if test -f ${SRC_CONFDIR}/$$p; then \
	    ${ECHO} "  ${INSTALL_DATA} ${SRC_CONFDIR}/$$p ${confdir}"; \
	    ${INSTALL_DATA} ${SRC_CONFDIR}/$$p ${confdir}; \
	  else :; fi; \
	done

.PHONY: install-share
install-share:
	${INSTALL_DIRS} ${sharedir} ${filesdir} ${usersdir} ${reportsdir} \
	${chanlogsdir} ${charinfodir} ${charsavedir} ${bak_charsavedir} \
	${bak_charinfodir} ${ladderdir}
	@list='${SHARES}'; for p in $$list; do \
	  if test -f ${SRC_SHAREDIR}/$$p; then \
	    ${ECHO} "  ${INSTALL_DATA} ${SRC_SHAREDIR}/$$p ${filesdir}"; \
	    ${INSTALL_DATA} ${SRC_SHAREDIR}/$$p ${filesdir}; \
	  else :; fi; \
	done
	-. ${SRC_SCRIPTDIR}/tos.sh ${filesdir}

.PHONY: uninstall-all-bnetd-things
uninstall-all-bnetd-things: uninstall-sbin uninstall-bin uninstall-man uninstall-conf uninstall-share

.PHONY: uninstall-sbin
uninstall-sbin:
	@list='${SBINS}'; for p in $$list; do \
	  ${ECHO} "  ${RMF} ${sbindir}/$$p"; \
	  ${RMF} ${sbindir}/$$p; \
	done

.PHONY: uninstall-bin
uninstall-bin:
	@list='${BINS}'; for p in $$list; do \
	  ${ECHO} "  ${RMF} ${bindir}/$$p"; \
	  ${RMF} ${bindir}/$$p; \
	done

.PHONY: uninstall-man
uinstall-man:
	@list='${MAN1S}'; for p in $$list; do \
	  ${ECHO} "  ${RMF} ${mandir}/man1/$$p"; \
	  ${RMF} ${mandir}/man1/$$p; \
	done
	@list='${MAN5S}'; for p in $$list; do \
	  ${ECHO} "  ${RMF} ${mandir}/man5/$$p"; \
	  ${RMF} ${mandir}/man5/$$p; \
	done

.PHONY: uninstall-conf
uninstall-conf:
	@list='${CONFS}'; for p in $$list; do \
	  ${ECHO} "  ${RMF} ${confdir}/$$p"; \
	  ${RMF} ${confdir}/$$p; \
	done

.PHONY: uninstall-share
uninstall-share:
	@list='${SHARES}'; for p in $$list; do \
	  ${ECHO} "  ${RMF} ${sharedir}/$$p"; \
	  ${RMF} ${sharedir}/$$p; \
	done

${SRC_CONFDIR}/bnetd.conf: ${SRC_CONFDIR}/bnetd.conf.in ${SRC_SRCDIR}/config.status
	@${RMF} $@
	${SED} -e "s@\(= *\"*\)/usr/local/var/@\1${sharedir}/@g" -e "s@\(= *\"*\)/usr/local/etc/@\1${confdir}/@g" ${SRC_CONFDIR}/bnetd.conf.in > $@

${SRC_CONFDIR}/d2cs.conf: ${SRC_CONFDIR}/d2cs.conf.in ${SRC_SRCDIR}/config.status
	@${RMF} $@
	${SED} -e "s@\(=[[:blank:]]*\"*\)/usr/local/var/@\1${sharedir}/@g" -e "s@\(=[[:blank:]]*\"*\)/usr/local/etc/@\1${confdir}/@g" ${SRC_CONFDIR}/d2cs.conf.in > $@

${SRC_CONFDIR}/d2dbs.conf: ${SRC_CONFDIR}/d2dbs.conf.in ${SRC_SRCDIR}/config.status
	@${RMF} $@
	${SED} -e "s@\(=[[:blank:]]*\"*\)/usr/local/var/@\1${sharedir}/@g" -e "s@\(=[[:blank:]]*\"*\)/usr/local/etc/@\1${confdir}/@g" ${SRC_CONFDIR}/d2dbs.conf.in > $@

${SRC_ACDIR}/ansi2knr: ${SRC_ACDIR}/ansi2knr.o
	@${RMF} $@
	${LINK_MANUALLY} ${SRC_ACDIR}/ansi2knr.o ${LINK_LIBS} -o $@

${SRC_ACDIR}/ansi2knr.o: ${SRC_ACDIR}/ansi2knr.c
	@${RMF} $@
	${COMPILE} ${SRC_ACDIR}/ansi2knr.c -o $@

SUFFIXES = .ansic .ansio
.ansic.ansio:
	@${RMF} $@
	@COMPILEANSI1@
	@COMPILEANSI2@
	@COMPILEANSI3@

${bnetd_SBIN}: ${bnetd_OBJS}
	@${INSTALL_DIRS} ${SRC_SBINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${bnetd_OBJS} ${LINK_LIBS} -o $@

${d2cs_SBIN}: ${d2cs_OBJS}
	@${INSTALL_DIRS} ${SRC_SBINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${d2cs_OBJS} ${LINK_LIBS} -o $@

${d2dbs_SBIN}: ${d2dbs_OBJS}
	@${INSTALL_DIRS} ${SRC_SBINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${d2dbs_OBJS} ${LINK_LIBS} -o $@

${sbnetd_SBIN}: ${sbnetd_OBJS}
	@${INSTALL_DIRS} ${SRC_SBINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${sbnetd_OBJS} ${LINK_LIBS} -o $@

${bnproxy_SBIN}: ${bnproxy_OBJS}
	@${INSTALL_DIRS} ${SRC_SBINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${bnproxy_OBJS} ${LINK_LIBS} -o $@

${bntrackd_SBIN}: ${bntrackd_OBJS}
	@${INSTALL_DIRS} ${SRC_SBINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${bntrackd_OBJS} ${LINK_LIBS} -o $@

${bnchat_BIN}: ${bnchat_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${bnchat_OBJS} ${LINK_LIBS} -o $@

${bnpass_BIN}: ${bnpass_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${bnpass_OBJS} ${LINK_LIBS} -o $@

${bnftp_BIN}: ${bnftp_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${bnftp_OBJS} ${LINK_LIBS} -o $@

${bnbot_BIN}: ${bnbot_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${bnbot_OBJS} ${LINK_LIBS} -o $@

${bnstat_BIN}: ${bnstat_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${bnstat_OBJS} ${LINK_LIBS} -o $@

${bnilist_BIN}: ${bnilist_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${bnilist_OBJS} ${LINK_LIBS} -o $@

${bni2tga_BIN}: ${bni2tga_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${bni2tga_OBJS} ${LINK_LIBS} -o $@

${bniextract_BIN}: ${bniextract_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${bniextract_OBJS} ${LINK_LIBS} -o $@

${bnibuild_BIN}: ${bnibuild_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${bnibuild_OBJS} ${LINK_LIBS} -o $@

${tgainfo_BIN}: ${tgainfo_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${tgainfo_OBJS} ${LINK_LIBS} -o $@

${timetest_BIN}: ${timetest_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${timetest_OBJS} ${LINK_LIBS} -o $@

${listtest_BIN}: ${listtest_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${listtest_OBJS} ${LINK_LIBS} -o $@

${hashtabletest_BIN}: ${hashtabletest_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${hashtabletest_OBJS} ${LINK_LIBS} -o $@

${addrtest_BIN}: ${addrtest_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${addrtest_OBJS} ${LINK_LIBS} -o $@

${hashtest_BIN}: ${hashtest_OBJS}
	@${INSTALL_DIRS} ${SRC_BINDIR}
	@${RMF} $@
	${LINK_MANUALLY} ${hashtest_OBJS} ${LINK_LIBS} -o $@
