Dependencies
============

Required
--------

The following packages are required for compiling IOV on all platforms:

   * VR Juggler 2.0.3 or newer
      - VR Juggler should be built with CppDOM 0.7.10 (or newer) and
        GMTL 0.5.4 (or newer).
      - Best results will come from using VR Juggler 2.2 or newer.
   * OpenSG 1.8.0 or newer
      - Unforutnately, OpenSG 2.0 is not currently supported.
   * Boost 1.33.1 or newer
      - If using GCC 4.1 or newer as the compiler, Boost 1.34.0 or newer is
        required. This is due to a problem in boost::bind() that is exposed
        when using GCC 4.1 or newer. The problem was fixed in Boost 1.34.0.
   * SCons 0.97 or newer
   * Flagpoll 0.9.1 or newer

Optional
--------

The following packages are optional. The build will determine if they are
available and enable/disable features accordingly.

   * ffmpeg
      - Specifically, libavcodec and libavformat are used for video
        creation.
      - The use of ffmpeg on Windows has not yet been tested, but it should
        work as long as Flagpoll can find the ffmpeg installation.
   * Video for Windows
      - On Windows, Video for Windows (VFW) can be used for video creation
        in addition to ffmpeg.

Building on Linux
=================

Run SCons:

   % scons

To install IOV into, say /usr/local, run it this way:

   % scons prefix=/usr/local install

Building on Windows
===================

To build on Windows, the following requirements must be met:

   * Use VR Juggler 2.2 or newer
   * Use Visual Studio .NET 2003 (Visual C++ 7.1) or newer
   * Build only shared libraries

Other specific requirements are detailed below.

32-bit Windows
--------------

To force the use of Visual C++ 7.1, execute SCons as follows:

   > scons MSVS_VERSION=7.1

To force the use of Visual C++ 8.0 (32-bit only), execute SCons as follows:

   > scons MSVS_VERSION=8.0 var_arch=ia32

64-bit Windows
--------------

In addition to the general Windows requirements, the following conditions
must also be met:

   * Use Visual Studio 2005 Standard Edition or better
   * Have 64-bit versions of all dependencies
   * Use the 32-bit version of Python

There are some things to be aware of:

   * Compiling against a 64-bit version of ffmpeg built using 64-bit MinGW
     has not been tested.
   * One source tree can have only a 32-bit or a 64-bit build in it. The
     installation procedure does not currently support having 32- and 64-bit
     binaries installed side by side.

When running SCons, execute it as follows:

   > scons MSVS_VERSION=8.0 var_arch=x64

FFmpeg on Windows
-----------------

The video capture code that uses FFmpeg (see vrkit::video::EncoderFFmpeg)
can be used on Windows, though it takes quite a bit of preparatory work.
The standard FFmpeg code base, which usually must be acquired from the
FFmpeg Subversion repository, requires a C99-compliant compiler. Visual C++
is not such a compiler, and thus GCC must be used. Fortunately, there are
instructions explaining how to get rolling with FFmpeg using MinGW:

   http://arrozcru.no-ip.org/ffmpeg_wiki/tiki-index.php

Expect to spend at least a few hours getting everything set up.

IMPORTANT: 

   The instructions above indicate that --enable-memalign-hack should be
   used when configuring the FFmpeg build. With GCC 3.4.5 in MinGW, this
   seems to be insufficient. The only way that FFmpeg has been tested
   successfully by the vrkit developers is to use that configure script
   option and to build FFmpeg using the MinGW version of GCC 4.2.1. As of
   this writing, GCC 4.2.1 is available to MinGW users as a "technology
   preview" from the MinGW page on SourceForge.

The FFmpeg dependencies are optional, but it does not hurt to get them
compiled and usable by FFmpeg and vrkit. The vrkit build will detect which
FFmpeg dependencies are available and use them if they are present. However,
the catch is that all the .a files must be in the same directory as
libavformat.a, libavcodec.a, and libavutil.a.

IMPORTANT:

   vrkit is licensed under the LGPL. FFmpeg can be licensed under the GPL
   or the LGPL depending on which dependencies are to be enabled. To use
   vrkit legally, FFmpeg must be used under the terms of the LGPL, and that
   means that dependencies such as libx264 and libfaad cannot be enabled
   when building FFmpeg.

With FFmpeg compiled and ready for use, the vrkit build must told a few
things in order to be able to link everything correctly. The options that
must be given to the scons command are the following:

           mingw: The directory containing the MinGW installation. A common
                  value is C:\MinGW.
   mingw_gcc_lib: The directory containing libgcc.a. When using the
                  "technology preview" version of GCC 4.2.1, this would be
                  something similar to C:\MinGW\lib\gcc\mingw32\4.2.1-sjlj.
    mingw_ffmpeg: The path to the FFmpeg installation. If using MSYS, a
                  likely value is C:\MSYS\1.0\local (/usr/local in the eyes
                  of MSYS).

Building on Mac OS X
====================
