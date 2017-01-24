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

#include "launcher.h"
#include "qfreerdp.h"
#include <QApplication>
#include <QProcess>
#include <QRegularExpression>
#include <cstdio>

static const QRegularExpression re_version("FreeRDP version ([0-9.]*)");

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Perform some sanity checks early
    auto result = queryXFreeRDP(QStringLiteral("--version"));
    if (!result.second) {
        fputs("Error starting xfreerdp.  Is it in your PATH?\n", stderr);
        return 2;
    }
    QString version;
    for (QByteArray line : result.first.split('\n')) {
        auto strLine = QString::fromLocal8Bit(line);
        auto match = re_version.match(strLine);
        if (!match.hasMatch())
            continue;
        version = match.captured(1);
        break;
    }
    if (version.isEmpty()) {
        fputs("Could not determine xfreerdp version\n", stderr);
        return 1;
    }
    if (!version.startsWith("2.")) {
        fprintf(stderr, "xfreerdp reported version %s, but we require at least 2.0\n",
                version.toUtf8().data());
        return 1;
    }

    // Show the GUI
    Launcher launcher;
    launcher.restoreConfig();
    launcher.show();
    return app.exec();
}
