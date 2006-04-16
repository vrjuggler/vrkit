# Spec file for IOV.
%define name    IOV
%define version	0.1.0
%define release	1

%define vrj_version    %(vrjuggler-config --version)
%define opensg_version %(osg-config --version)
%define boost_version  1.33

Name: %{name}
Summary: IOV
Version: %{version}
Release: %{release}
Source: %{name}-%{version}.tar.bz2
URL: http://www.infiscape.com
Group: System Environment/Libraries
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot
License: Copyright Infiscape Corporation
Requires: vrjuggler >= %{vrj_version}
Requires: OpenSG >= %{opensg_version}
Requires: boost >= %{boost_version}
Requires: cppdom >= 0.3.3
BuildRequires: vrjuggler-devel >= %{vrj_version}
BuildRequires: OpenSG-devel >= %{opensg_version}
BuildRequires: boost-devel >= %{boost_version}
BuildRequires: cppdom-devel >= 0.3.3
BuildRequires: gmtl >= 0.4.5
Provides: IOV = %{version}-%{release}
Obsoletes: IOV < %{version}-%{release}

%description
The Infiscape OpenSG Viewer library, which allows for rapid development of
cluster-ready VR Juggler applications with dynamically composed functionality.

%package devel
Summary: The IOV Headers
Group: System Environment/Libraries
Requires: IOV = %{version}-%{release}
Provides: IOV-devel = %{version}-%{release}

%description devel
Headers for IOV

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
/usr/local/infiscape/lib/libiov.so
/usr/local/infiscape/lib/IOV
/usr/local/infiscape/share

%files devel
%defattr(-, root, root)
/usr/local/infiscape/include/IOV
%doc ChangeLog
#%doc README AUTHORS ChangeLog COPYING

%changelog
