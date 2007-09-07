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

   The instructions from the above website indicate that the option
   --enable-memalign-hack must be used when configuring the FFmpeg build.
   With GCC 3.4.5 in MinGW, this seems to be insufficient. The only way that
   FFmpeg has been tested successfully by the vrkit developers is to use that
   configure script option and to build FFmpeg using the MinGW version of GCC
   4.2.1. As of this writing, GCC 4.2.1 is available to MinGW users as a
   "technology preview" from the MinGW page on SourceForge.

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

** 32-bit FFmpeg **

With FFmpeg compiled and ready for use, the vrkit build must told a few
things in order to be able to link everything correctly. The options that
must be given to the scons command are the following:

           mingw: The directory containing the MinGW installation. A common
                  value is C:\MinGW.
   mingw_gcc_lib: The directory containing libgcc.a. When using the
                  "technology preview" version of GCC 4.2.1, this would be
                  something similar to C:\MinGW\lib\gcc\mingw32\4.2.1-sjlj.
    mingw_ffmpeg: The path to the FFmpeg installation. If using MSYS, a
                  likely value is C:\msys\1.0\local (/usr/local in the eyes
                  of MSYS).

** 64-bit FFmpeg **

Building a 64-bit version of FFmpeg for Windows at thie time is a difficult
task. As of this writing, 64-bit support in MinGW is still in the experimental
phase, but things can be made to work. As the 64-bit support in MinGW
evolves and stabilizes, it is expected that a 64-bit FFmpeg installation for
Windows will become much easier to create.

IMPORTANT:

   The following documents one way that the FFmpeg libraries were compiled
   successfully for use on 64-bit Windows. There is no guarantee that these
   steps will work for anyone else. Please do not send questions about this
   to the vrkit mailing lists. The difficulties lie with setting up GCC,
   GNU Binutils, MinGW, and FFmpeg itself, not with vrkit.

MORE IMPORTANT:

   The following steps will not work at the moment. The 64-bit MinGW support
   is not yet to the point where FFmpeg can be compiled and used as a 64-bit
   library. More than anything, the information presented below is a journal
   of what was attempted when trying to get a 64-bit version of FFmpeg
   working on Windows.

The following steps must be followed to create the cross-compiler environment
for generating 64-bit MinGW binaries. These can be performed from a Cygwin
shell or from a MinGW/MSYS shell, but it's easiest to build everything from
an MSYS shell. If the Cygwin version of GCC is used, be sure to tell
configure scripts to add -mno-cygwin to CFLAGS so that the software compiled
for use with MinGW does not depend on Cygwin.

   1. Get the source code for GNU Binutils 2.18. It must be build to target
      the x86_64-pc-mingw32 platform. To build correctly, the Cygwin version
      of makeinfo must be used because the MinGW version is too old. The
      following shows the commands need to build and install things:

         export MAKEINFO=/c/cygwin/bin/makeinfo.exe
         PATH=$PATH:/c/cygwin/bin
         configure --target=x86_64-pc-mingw32
         make
         make install

      This will install to /usr/local (which should be C:\msys\1.0\local if
      using an MSYS shell).
   2  Get the patch for GCC 4.3.0 needed for it to compile correctly. The
      patch can be downloaded from the following link:

         http://downloads.sourceforge.net/mingw/mingw-w64_gccpatch.txt

      Note that this patch may not be necessary if the changes have been
      integrated into the GCC source tree.
   3. Get the latest GCC 4.3.0 sources from the GCC Subversion repository and
      patch the source using the patch(1) command from the root of the GCC
      source tree as follows:

         patch < mingw-w64_gccpatch.txt

      (The above assumes that mingw-w64_gccpatch.txt has been copied into the
      root of the GCC source tree. If it has not, just fill in the path to
      the file.) Then, build gcc (the C compiler only) as a cross-compiler
      using these commands:

         configure --target=x86_64-pc-mingw32
         make all-gcc
         make install-gcc

      This installs to /usr/local.
   4. Get the latest version of the MinGW headers and C runtime for w64.
      These are available from the MinGW project page on SourceForge under
      the "Snapshot" package. Put the header tree in the directory
      /usr/local/x86_64-pc-mingw32/include.
   5. Modify the w64 CRT makefile to add -fno-leading-underscore to the list
      of flags set in the $(CFLAGS) variable.
   6. Build the w64 CRT using make. Some changes may be needed to the
      makefile as it was likely generated for use with the Cygwin GCC.
   7. Copy the CRT .o files (CRT_fp10.o, CRT_fp8.o, binmode.o, crt1.o,
      crt2.o, crtbegin.o, crtend.o, dllcrt1.o, dllcrt2.o, txtmode.o) and .a
      files into /usr/local/x86_64-pc-mingw32/lib. There may be a lot of
      files involved.
   8. Go back to the GCC build and run the following commands to build and
      install libgcc.a and libstdc++.a:

         make
         make install

After all of that, configure and build FFmpeg in a MinGW or MSYS shell using
the following commands:

   configure --enable-memalign-hack --cross-prefix=x86_64-pc-mingw32-
             --prefix=/usr/local/x86_64-pc-mingw32 --disable-mmx
             --disable-ffmpeg

If 64-bit version of the FFmpeg dependencies are available, enable their use
by adding the appropriate flags to the configure script command as normal.

Next, modify the generated config.h to remove the lines defining the
symbols ARCH_X86 and ARCH_X86_32. This is needed to prevent the i386 assembly
code from being used. As of this writing, the GNU Binutils 2.18 assembler
will fail when trying to assemble most of the inline assembly code.

Now, things may get hairy. In some instances, we have found that
x86_64-pc-mingw32-ar.exe will not create valid static libraries. If the
x86_64-pc-mingw32-ar.exe that was compiled above is in good working order,
then skip down to the part about modifying $(CFLAGS) in common.mak.

It is not clear why x86_64-pc-mingw32-ar.exe would generate bad output, but
if it should happen that tools complain about a static library having an
invalid format, there is a workaround. Change the FFmpeg config.mak file to
use the Visual C++ linker instead of x86_64-pc-mingw32-ar.exe. This is done
by modifying the lines assigning a value to the $(AR) and $(RANLIB) variables
to be the following:

AR="C:\Program Files (x86)\Microsoft Visual Studio 8\VC\bin\amd64\link.exe"
RANLIB=:

It is absolutely critical that x86_64-pc-mingw32-ranlib.exe not be used on
.a files compiled by the Visual C++ linker. They will end up being invalid.

Then, change the target used to generate static libraries in the generated
common.mak. The original version should look similar to the following:

$(LIB): $(STATIC_OBJS)
	rm -f $@
	$(AR) rc $@ $^ $(EXTRAOBJS)
	$(RANLIB) $@

Change it to be the following instead:

$(LIB): $(STATIC_OBJS)
	rm -f $@
	$(AR) -lib -MACHINE:X64 -out:$@ $^ $(EXTRAOBJS)

Finally, change the assignment of the $(CFLAGS) variable in common.mak to
include the GCC option -fno-leading-underscore, as shown below:

CFLAGS += -DHAVE_AV_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE \
          -D_ISOC9X_SOURCE -I$(BUILD_ROOT) -I$(SRC_PATH) \
          -I$(SRC_PATH)/libavutil $(OPTFLAGS) -fno-leading-underscore

The actual flags preceding -fno-leading-underscore may be different than
what is shown above. The important thing is to get -fno-leading-underscore
in there somewhere.

After doing so, run the following to build and install the 64-bit version of
FFmpeg:

   make
   make install

With FFmpeg compiled and ready for use, the vrkit build must told a few
things in order to be able to link everything correctly. The options that
must be given to the scons command are the following:

           mingw: The directory containing the MinGW installation. For the
                  64-bit version built with a cross-compiler, a likely value
                  will be C:\msys\1.0\local\x86_64-pc-mingw32.
   mingw_gcc_lib: The directory containing libgcc.a. When using GCC built as
                  a cross-compiler for the x64 architecture, a likely value
                  will be C:\msys\1.0\local\lib\x86_64-pc-mingw32\4.3.0.
    mingw_ffmpeg: The path to the FFmpeg installation. For the 64-bit version
                  built with a cross-compiler, a likely value will be
                  C:\msys\1.0\local\x86_64-pc-mingw32.

Building on Mac OS X
====================
