/* This file is part of qfreerdp.
 *
 * qfreerdp is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * qfreerdp is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with qfreerdp; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _QFREERDP_H
#define _QFREERDP_H

#include <QtGlobal>
#include <QStringList>
#include <QProcess>

#if (QT_VERSION < QT_VERSION_CHECK(5, 7, 0))
/* Simplified backport of QOverload */
template <typename... Args>
struct QOverload
{
    template <typename R, typename T>
    static Q_DECL_CONSTEXPR auto of(R (T::*ptr)(Args...)) Q_DECL_NOTHROW -> decltype(ptr)
    { return ptr; }
};
#endif

template <typename... Args>
QPair<QByteArray, bool> queryXFreeRDP(const Args&... queryParams)
{
    QProcess proc;
    proc.start(QStringLiteral("xfreerdp"), QStringList { queryParams... });
    if (!proc.waitForStarted())
        return qMakePair(QByteArray{}, false);
    proc.waitForFinished();
    return qMakePair(proc.readAll(), true);
}

#endif
