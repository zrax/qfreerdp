# This file is part of qfreerdp.
#
# qfreerdp is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# qfreerdp is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with qfreerdp; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

cmake_minimum_required(VERSION 3.0)
project(qfreerdp)

set(CMAKE_AUTOMOC TRUE)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Wextra ${CMAKE_CXX_FLAGS}")

set(qfreerdp_HEADERS
    launcher.h
    qfreerdp.h
)

set(qfreerdp_SOURCES
    launcher.cpp
    main.cpp
)

add_executable(qfreerdp ${qfreerdp_HEADERS} ${qfreerdp_SOURCES})
target_link_libraries(qfreerdp Qt5::Core Qt5::Widgets)
