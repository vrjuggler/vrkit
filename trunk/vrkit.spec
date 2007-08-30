# vrkit is (C) Copyright 2005-2007
#    by Allen Bierbaum, Aron Bierbuam, Patrick Hartling, and Daniel Shipton
#
# This file is part of vrkit.
#
# vrkit is free software; you can redistribute it and/or modify it under the
# terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option)
# any later version.
#
# vrkit is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
# more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

# Spec file for vrkit.
%define name    vrkit
%define version	0.47.0
%define release	1

%define vrj_version    %(vrjuggler-config --version)
%define opensg_version %(osg-config --version)
%define boost_version  1.33

Name: %{name}
Summary: vrkit
Version: %{version}
Release: %{release}
Source: %{name}-%{version}.tar.bz2
URL: http://www.infiscape.com
Group: System Environment/Libraries
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
License: LGPL
Requires: vrjuggler = %{vrj_version}
Requires: OpenSG = %{opensg_version}
Requires: boost >= %{boost_version}
Requires: cppdom >= 0.3.3
BuildRequires: vrjuggler-devel = %{vrj_version}
BuildRequires: OpenSG-devel = %{opensg_version}
BuildRequires: boost-devel >= %{boost_version}
BuildRequires: cppdom-devel >= 0.3.3
BuildRequires: gmtl >= 0.4.5

%description
The vrkit library, which allows for rapid development of cluster-ready
VR Juggler applications based on OpenSG with functionality composed
dynamically.

%package devel
Summary: The vrkit Headers
Group: System Environment/Libraries
Requires: vrkit = %{version}-%{release}

%description devel
Headers for vrkit.

%prep
rm -rf $RPM_BUILD_ROOT
%setup -q
#%setup -DT -q

%build
# Build and install
scons optimize=yes BoostBaseDir=/usr BoostIncludeDir=/usr/include VrjBaseDir=/usr OpenSGBaseDir=/usr prefix=$RPM_BUILD_ROOT/usr/local/infiscape install

%install
# Remove all stupid scons temp files
find $RPM_BUILD_ROOT/usr/local/infiscape -name .sconsign -exec rm {} \;

%clean
rm -rf $RPM_BUILD_ROOT

%pre

%post

%preun

%postun

%files
%defattr(-, root, root)
/usr/local/infiscape/lib/libvrkit.so
/usr/local/infiscape/lib/vrkit
/usr/local/infiscape/share

%files devel
%defattr(-, root, root)
/usr/local/infiscape/include/vrkit
%doc ChangeLog
#%doc README AUTHORS ChangeLog COPYING

%changelog
